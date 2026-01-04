#ifndef NETWORK_H
#define NETWORK_H

#include "process.h"

#define STR_MAX 1024

/*=========
*  FORWARD DECLARATIONS:  *
==========*/
typedef struct parameters_table parameters_table;

/*=========
*  TYPES:  *
==========*/

typedef struct remote_config {
    char name[64];
    char address[128];
    int port;
    char username[64];
    char password[64];
} remote_config;

/* Global array to store remote configurations */
#define MAX_REMOTE_CONFIGS 16
extern remote_config *g_remote_configs;
extern int g_remote_configs_count;


/*=========
* METHODS: *
==========*/

/**
 * Parse login in format user@server and extract username and server address.
 * @param login Login string in format user@server.
 * @param username Buffer to store extracted username.
 * @param server Buffer to store extracted server address.
 * @return 0 on success, -1 on failure.
 */
int parse_login(const char *login, char *username, char *server);

/**
 * Return true if the given port is free to use on the localhost.
 * @param port The port to test.
 */
bool is_port_free(int port);

/**
 * Return true if the given file match this schema:
 * server_name1:server_adress:port:username:password
 * @param path File's path
 */
bool is_config_file_valid(char path[STR_MAX]);

/**
 * Initialize the network configuration from CLI parameters or fichier .config.
 * @param parameters Pointeur vers le tableau de paramètres CLI.
 * @param params_count Nombre d'éléments dans le tableau.
 * @return 0 on success, -1 on error.
 */
int network_init(parameters_table *parameters, int params_count);

/**
 * Establish SSH connection and retrieve process list from a remote machine.
 * @param config Pointer to remote_config structure.
 * @param out_processes Pointer to array of process_info (must be freed by caller).
 * @return Number of processes on success, -1 on error.
 */
int network_get_processes_ssh(remote_config *config, process_info **out_processes);

/**
 * Poll remote processes from all configured remote machines.
 * @param all_processes Pointer to array of all processes (local + remote).
 * @param local_count Number of local processes already in the array.
 * @return Total count of processes (local + remote), or -1 on error.
 */
int network_poll_all_processes(process_info **all_processes, int local_count);

void network_poll(void);

#endif /* NETWORK_H */