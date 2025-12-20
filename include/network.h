#ifndef NETWORK_H
#define NETWORK_H

#define STR_MAX 1024

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

/**
 * Return true if the given file match this schema:
 * server_name1:server_adress:port:username:password:connection_type1
 * server_name2:server_adress:port:username:password:connection_type2
 */
bool is_config_file_valid(char path[STR_MAX]);

int network_init(void);
void network_poll(void);

#endif /* NETWORK_H */
