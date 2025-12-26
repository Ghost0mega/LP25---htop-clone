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
 * @param port The port to test.
 */
bool is_port_free(int port);

/**
 * Return true if the given file match this schema:
 * server_name1:server_adress:port:username:password:connection_type1
 * server_name2:server_adress:port:username:password:connection_type2
 * @param path File's path
 */
bool is_config_file_valid(char path[STR_MAX]);

/**
 * Initialize the network connection.
 * @param parameters Parameters uesd to use the connection.
 * @param params_count Number of parameters.
 */
int network_init(parameters_table parameters, int params_count);

void network_poll(void);

#endif /* NETWORK_H */
