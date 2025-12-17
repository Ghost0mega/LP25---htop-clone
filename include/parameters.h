#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <stdbool.h>
#include <unistd.h>

#define STR_MAX 1024
#define PARAMETER_BUFFER_SIZE 16
#define COLOR_BOLD  "\e[1m"
#define COLOR_OFF   "\e[m"

/*=========
*  TYPES:  *
==========*/

typedef enum {
    PARAM_HELP, //Manual of the programm.
    PARAM_DRY_RUN,  //Test the the access to the proccess in local and distant machine, without show them.
    PARAM_REMOTE_CONFIG, //Define the location of the configuration file to the distant machines.
    PARAM_CONNEXION_TYPE, //Define the type of connection to use on distant machine.
    PARAM_PORT, //Select the port to connect.
    PARAM_LOGIN, //Define the identifiant to the distant machine.
    PARAM_REMOTE_SERVER, //Define DNS or IP of the distant machine.
    PARAM_USERNAME, //Specify the username to use for the connection.
    PARAM_PASSWORD, //Specify the password to use to connect.
    PARAM_ALL, //Specify the programm to collect the process from the local and distant machines.
} parameters_id_table;


typedef union {
	int int_param; //Used for port
	bool flag_param; //Used for help, dry-run and all
	char str_param[STR_MAX];
} data_wrapper_table;

typedef struct {
    parameters_id_table parameter_type;
    data_wrapper_table parameter_value;
} parameters_table;

/*=========
* METHODS: *
==========*/

/**
 * Manage the given arguments of the programm.
 */
int manage_arguments(int argc, char *argv[]);

/**
 *  Return true if the given parameters are valid.
 */
bool params_validate(parameters_table *params, int params_count);

/**
 * Return true if the given port is free to use.
 */
bool is_port_free(int port);

/**
 * Print the manual in the terminal.
 */
void manual();

#endif /* PARAMETERS_H */