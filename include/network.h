#ifndef NETWORK_H
#define NETWORK_H

typedef enum connection_type {
    CONN_SSH,
    CONN_TELNET
} connection_type_t;

typedef struct remote_config {
    char name[64];
    char address[128];
    int port;
    char username[64];
    char password[64];
    connection_type_t type;
} remote_config_t;

/* Network API (stub) */

int network_init(void);
void network_poll(void);

#endif /* NETWORK_H */
