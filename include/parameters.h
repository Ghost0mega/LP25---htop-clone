#ifndef PARAMETERS_H
#define PARAMETERS_H
#include "../include/project.h"

#define STR_MAX 1024
#define PARAMETER_BUFFER_SIZE 16
#define COLOR_BOLD  "\e[1m"
#define COLOR_OFF   "\e[m"

/*=========
*  TYPES:  *
==========*/

typedef enum {
    PARAM_HELP,
    PARAM_DRY_RUN,
    PARAM_REMOTE_CONFIG,
    PARAM_PORT,
    PARAM_LOGIN,
    PARAM_REMOTE_SERVER,
    PARAM_USERNAME,
    PARAM_PASSWORD,
    PARAM_ALL,
    PARAM_CONNECTION_TYPE,
} parameters_id_table;

typedef union {
	int int_param;
	bool flag_param;
	char str_param[STR_MAX];
} data_wrapper_table;

typedef struct parameters_table {
    parameters_id_table parameter_type;
    data_wrapper_table parameter_value;
} parameters_table;

/*=========
* METHODS: *
==========*/

/**
 * Manage the given arguments of the programm.
 * @param argc Number of arguments in the command line.
 * @param argv Arguments of the command line.
 * @param out_count Final count of parameters.
 */
parameters_table *manage_arguments(int argc, char *argv[], int *out_count);

/**
 * Return true if the given parameters are valid.
 * @param params Table of parameters.
 * @param params_count Number of element in the table.
 */
bool params_validate(parameters_table *params, int params_count);

/**
 * Return true if there is the given arguments.
 * @param params Table of parameters.
 * @param params_count Number of element in the table.
 * @param type The type that we want to test.
 */
bool is_param_type(parameters_table *params, int params_count, parameters_id_table type);

/**
 * Get the value of the parameters of the given type.
 * @param params Table of parameters.
 * @param params_cpunt Number of elements in the table.
 * @param type The type want to get the value.
 * @param string Return string.
 * @return 0 success - 1 fail
 */
int get_string_parameters(parameters_table *params, int params_count, parameters_id_table type, char *string);

/**
 * Get the value of the parameters of the given type.
 * @param params Table of parameters.
 * @param params_cpunt Number of elements in the table.
 * @param type The type want to get the value.
 * @return int
 */
int get_int_parameters(parameters_table *params, int params_count, parameters_id_table type);

/**
 * Print the manual in the terminal.
 */
void manual();

#endif /* PARAMETERS_H */