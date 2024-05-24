#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "config.h"

int load_config(const char *filename, config_t *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return -1;
    }

    char line[CONFIG_FILE_LENGTH];
    int blacklist_index = 0;
    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, " \t\n");
        if (!token || token[0] == '#') continue;

        if (strcmp(token, "upstream_dns_server") == 0) {
            token = strtok(NULL, " \t\n");
            if (token) strlcpy(config->upstream_dns_server, token, INET_ADDRSTRLEN);
        } else if (strcmp(token, "blacklist") == 0) {
            token = strtok(NULL, " \t\n");
            while (token && blacklist_index < MAX_BLACKLIST) {
                strlcpy(config->blacklist[blacklist_index++], token, MAX_DOMAIN_LENGTH);
                token = strtok(NULL, " \t\n");
            }
        }
    }

    config->blacklist_count = blacklist_index;
    fclose(file);
    return 0;
}

void print_config(const config_t *config) {
    printf("Upstream DNS Server: %s\n", config->upstream_dns_server);
    printf("Blacklist:\n");
    for (int i = 0; i < config->blacklist_count; ++i) {
        printf("  %s\n", config->blacklist[i]);
    }
}
