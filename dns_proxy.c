#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ldns/ldns.h>
#include "config.h"

#define BUFFER_SIZE 512
#define DNS_PORT 5000

void handle_dns_request(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len, const config_t *config);
int is_blacklisted(const char *domain, const config_t *config);
void generate_blacklist_response(ldns_pkt *query_pkt, ldns_pkt **response_pkt);

int main() {
    config_t config;
    if (load_config("config.txt", &config) != 0) {
        fprintf(stderr, "Failed to load configuration\n");
        return EXIT_FAILURE;
    }

    print_config(&config);

    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // Allow address reuse
    int optval = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Bind to DNS port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DNS_PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("DNS proxy server listening on port %d...\n", DNS_PORT);

    // Main loop to process DNS requests
    while (1) {
        char buffer[BUFFER_SIZE];
        ssize_t received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (received < 0) {
            perror("recvfrom");
            continue;
        }

        handle_dns_request(sockfd, &client_addr, addr_len, &config);
    }

    close(sockfd);
    return 0;
}

void handle_dns_request(int sockfd, struct sockaddr_in *client_addr, socklen_t addr_len, const config_t *config) {
    char buffer[BUFFER_SIZE];
    ssize_t received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)client_addr, &addr_len);
    if (received < 0) {
        perror("recvfrom");
        return;
    }

    ldns_pkt *query_pkt;
    ldns_status status = ldns_wire2pkt(&query_pkt, (const uint8_t *)buffer, received);
    if (status != LDNS_STATUS_OK) {
        fprintf(stderr, "Failed to parse DNS query: %s\n", ldns_get_errorstr_by_id(status));
        return;
    }

    ldns_rr_list *question = ldns_pkt_question(query_pkt);
    if (!question) {
        fprintf(stderr, "Failed to get DNS question section\n");
        ldns_pkt_free(query_pkt);
        return;
    }

    ldns_rr *rr = ldns_rr_list_rr(question, 0);
    ldns_rdf *rdf = ldns_rr_owner(rr);
    char *domain = ldns_rdf2str(rdf);

    // Remove trailing dot from domain
    if (domain[strlen(domain) - 1] == '.') {
        domain[strlen(domain) - 1] = '\0';
    }

    // Check if the domain is blacklisted
    if (is_blacklisted(domain, config)) {
        ldns_pkt *response_pkt;
        generate_blacklist_response(query_pkt, &response_pkt);
        uint8_t *response_data;
        size_t response_size;
        status = ldns_pkt2wire(&response_data, response_pkt, &response_size);
        if (status != LDNS_STATUS_OK) {
            fprintf(stderr, "Failed to generate DNS response: %s\n", ldns_get_errorstr_by_id(status));
        } else {
            sendto(sockfd, response_data, response_size, 0, (struct sockaddr *)client_addr, addr_len);
            free(response_data);
        }
        ldns_pkt_free(response_pkt);
    } else {
        // Forward the request to the upstream DNS server
        struct sockaddr_in upstream_addr;
        memset(&upstream_addr, 0, sizeof(upstream_addr));
        upstream_addr.sin_family = AF_INET;
        upstream_addr.sin_port = htons(53);  // DNS standard port
        inet_pton(AF_INET, config->upstream_dns_server, &upstream_addr.sin_addr);

        sendto(sockfd, buffer, received, 0, (struct sockaddr *)&upstream_addr, sizeof(upstream_addr));

        char response[BUFFER_SIZE];
        ssize_t response_len = recvfrom(sockfd, response, BUFFER_SIZE, 0, NULL, NULL);
        if (response_len < 0) {
            perror("recvfrom upstream");
            return;
        }

        sendto(sockfd, response, response_len, 0, (struct sockaddr *)client_addr, addr_len);
    }

    ldns_pkt_free(query_pkt);
    free(domain);
}

int is_blacklisted(const char *domain, const config_t *config) {
    for (int i = 0; i < config->blacklist_count; i++) {
        if (strcmp(domain, config->blacklist[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void generate_blacklist_response(ldns_pkt *query_pkt, ldns_pkt **response_pkt) {
    *response_pkt = ldns_pkt_new();
    ldns_pkt_set_id(*response_pkt, ldns_pkt_id(query_pkt));
    ldns_pkt_set_qr(*response_pkt, true);
    ldns_pkt_set_opcode(*response_pkt, ldns_pkt_get_opcode(query_pkt));
    ldns_pkt_set_rcode(*response_pkt, LDNS_RCODE_NXDOMAIN);
}
