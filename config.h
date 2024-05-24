#ifndef CONFIG_H
#define CONFIG_H

#define MAX_BLACKLIST 100
#define MAX_DOMAIN_LENGTH 256
#define CONFIG_FILE_LENGTH 512

typedef struct {
    char upstream_dns_server[INET_ADDRSTRLEN];
    char blacklist[MAX_BLACKLIST][MAX_DOMAIN_LENGTH];
    int blacklist_count;
} config_t;

int load_config(const char *filename, config_t *config);
void print_config(const config_t *config);

#endif
