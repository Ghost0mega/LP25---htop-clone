#ifndef NETWORK_H
#define NETWORK_H

/*=========
*  TYPES:  *
==========*/

typedef enum connection_type {
    CONN_SSH,
    CONN_TELNET
} connection_type;

typedef struct remote_config {
    char name[64];
    char address[128];
    int port;
    char username[64];
    char password[64];
    connection_type type;
} remote_config;

/* Network API (stub) */

/*=========
* METHODS: *
==========*/

/**
 * Return true if the given port is free to use.
 */
bool is_port_free(int port);

int network_init(void);
void network_poll(void);

#endif /* NETWORK_H */
