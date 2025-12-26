#include "../include/project.h"

/*=========
* METHODS: *
==========*/

parameters_table *manage_arguments(int argc, char *argv[], int *out_count) {
    //Initialization:
    int parameters_count=0;
    int opt;

    //Verfying the output:
    if (!out_count) return NULL;

    //Creat the empty table:
    parameters_table *given_parameters = calloc(PARAMETER_BUFFER_SIZE, sizeof(parameters_table));
    if (!given_parameters) return NULL;

    //Define the accpeted arguments:
    struct option my_opts[] = {
        {.name = "help", .has_arg = 0, .flag = 0, .val = 'h'},
        {.name = "dry-run", .has_arg = 0, .flag = 0, .val = 1},
        {.name = "remote-config", .has_arg = 1, .flag = 0, .val = 'c'},
        {.name = "connexion-type", .has_arg = 1, .flag = 0, .val = 't'},
        {.name = "port", .has_arg = 1, .flag = 0, .val = 'P'},
        {.name = "login", .has_arg = 1, .flag = 0, .val = 'l'},
        {.name = "remote-server", .has_arg = 1, .flag = 0, .val = 's'},
        {.name = "username", .has_arg = 1, .flag = 0, .val = 'u'},
        {.name = "password", .has_arg = 1, .flag = 0, .val = 'p'},
        {.name = "all", .has_arg = 0, .flag = 0, .val = 'a'},
        {0, 0, 0, 0}
    };

    //Do the modification of the arguments:
    while ((opt = getopt_long(argc, argv, "hdc:t:P:l:s:u:p:a", my_opts, NULL)) != -1) {

        if (parameters_count >= PARAMETER_BUFFER_SIZE) {
            fprintf(stderr, "ERROR: Too many parameters\n");
            free(given_parameters);
            return NULL;
        }

        switch (opt) {
            case 'h':
                given_parameters[parameters_count].parameter_type = PARAM_HELP;
                given_parameters[parameters_count].parameter_value.flag_param = true;
                parameters_count++;
                break;
        
            case 1:
                given_parameters[parameters_count].parameter_type = PARAM_DRY_RUN;
                given_parameters[parameters_count].parameter_value.flag_param = true;
                parameters_count++;
                break;

            case 'c':
                given_parameters[parameters_count].parameter_type = PARAM_REMOTE_CONFIG;
                strncpy(given_parameters[parameters_count].parameter_value.str_param, optarg, STR_MAX-1);
                given_parameters[parameters_count].parameter_value.str_param[STR_MAX-1] = '\0';
                parameters_count++;
                break;

            case 't':
                given_parameters[parameters_count].parameter_type = PARAM_CONNEXION_TYPE;
                strncpy(given_parameters[parameters_count].parameter_value.str_param, optarg, STR_MAX-1);
                given_parameters[parameters_count].parameter_value.str_param[STR_MAX-1] = '\0';
                parameters_count++;
                break;

            case 'P':
                given_parameters[parameters_count].parameter_type = PARAM_PORT;
                given_parameters[parameters_count].parameter_value.int_param = atoi(optarg);
                parameters_count++;
                break;

            case 'l':
                given_parameters[parameters_count].parameter_type = PARAM_LOGIN;
                strncpy(given_parameters[parameters_count].parameter_value.str_param, optarg, STR_MAX-1);
                given_parameters[parameters_count].parameter_value.str_param[STR_MAX-1] = '\0';
                parameters_count++;
                break;

            case 's':
                given_parameters[parameters_count].parameter_type = PARAM_REMOTE_SERVER;
                strncpy(given_parameters[parameters_count].parameter_value.str_param, optarg, STR_MAX-1);
                given_parameters[parameters_count].parameter_value.str_param[STR_MAX-1] = '\0';
                parameters_count++;
                break;

            case 'u':
                given_parameters[parameters_count].parameter_type = PARAM_USERNAME;
                strncpy(given_parameters[parameters_count].parameter_value.str_param, optarg, STR_MAX-1);
                given_parameters[parameters_count].parameter_value.str_param[STR_MAX-1] = '\0';
                parameters_count++;
                break;

            case 'p':
                given_parameters[parameters_count].parameter_type = PARAM_PASSWORD;
                strncpy(given_parameters[parameters_count].parameter_value.str_param, optarg, STR_MAX-1);
                given_parameters[parameters_count].parameter_value.str_param[STR_MAX-1] = '\0';
                parameters_count++;
                break;

            case 'a':
                given_parameters[parameters_count].parameter_type = PARAM_ALL;
                given_parameters[parameters_count].parameter_value.flag_param = true;
                parameters_count++;
                break;

            case '?':
                fprintf(stderr,"ERROR: Invalid Arguments\n");
                break;

            default:
                fprintf(stderr, "ERROR: Invalid argument\n");
                free(given_parameters);
                return NULL;
                break;
        }
    }

    if (!params_validate(given_parameters, parameters_count)) {
        free(given_parameters);
        return NULL;
    }

    *out_count = parameters_count;
    return given_parameters;
}

bool params_validate(parameters_table *params, int params_count) {
    //Initialization:
    bool local_mode = true;
    bool has_config_file = false;

    //Verifying if we are in local mode:
    for (int i = 0; i < params_count; i++) {
        if (params[i].parameter_type == PARAM_CONNEXION_TYPE && strcmp(params[i].parameter_value.str_param, "local") != 0) {
            local_mode = false;
            break;
        }
    }

    //Validation foreach parameters which takes values:
    for (int i=0; i<params_count; i++) {
        //Current parameter: (equivalent to the foreach in other languages)
        parameters_table *param = &params[i];

        //Rules for each type of parameters:
        switch(param->parameter_type) {
            default:
                break;

            case PARAM_REMOTE_CONFIG:
                if (!local_mode) {
                    //The parameters isn't valid if it is null, or the user cannot acess the given path.
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    } else {
                        if (access(param->parameter_value.str_param, R_OK) != 0) {
                            fprintf(stderr,"ERROR: Cannot access the configuration file.\n");
                            return false;
                        }
                        if (!is_config_file_valid(param->parameter_value.str_param)) {
                            fprintf(stderr,"ERROR: Follow this format for the configuration file:\nserver_name1:server_adress:port:username:password:connection_type1\nserver_name2:server_adress:port:username:password:connection_type2\n");
                            return false;
                        }
                    }
                    has_config_file = true;
                }
                break;

            case PARAM_PORT:
                //Tested only if the program isn't for a local use:
                if (!local_mode && !has_config_file) {
                    //The parameters isn't correct if the value is less than 1024 or bigger than 65535:
                    if ((param->parameter_value.int_param < 1024) || (param->parameter_value.int_param > 65535)) {
                        fprintf(stderr,"ERROR: Please enter a port value between 1024 and 65535.\n");
                        return false;
                    } else {
                        //We verify that the port is free:
                        if(!is_port_free(param->parameter_value.int_param)) {
                            fprintf(stderr,"ERROR: The selected port is alrezdy used.\n");
                            return false;
                        }
                    }
                }
                break;

            case PARAM_LOGIN:
                //Tested only if the program isn't for a local use:
                if (!local_mode && !has_config_file) {
                    //fill with the conditions
                }
                break;
            case PARAM_REMOTE_SERVER:
                //Tested only if the program isn't for a local use:
                if (!local_mode && !has_config_file) {
                    //fill with the conditions
                }
                break;

            case PARAM_USERNAME:
                //Tested only if the program isn't for a local use:
                if (!local_mode && !has_config_file) {
                    //fill with the conditions
                }
                break;
                
            case PARAM_PASSWORD:
                //Tested only if the program isn't for a local use:
                if (!local_mode && !has_config_file) {
                    //fill with the conditions
                }
                break;
            }
        }

    //If nothing goes wrong we return true:
    return true;
}

bool is_param_type(parameters_table *params, int params_count, parameters_id_table type_to_check) {
    //Initialization:
    bool found = false;

    for (int i = 0; i < params_count; i++) {
        if (params[i].parameter_type == type_to_check) {
            found = true;
            break;
        }
    }

    return found;
}


void manual() {
    printf(COLOR_BOLD "NOM\n" COLOR_OFF "\thtop-clone - interactive process viewer\n\n");
    printf(COLOR_BOLD "SYNOPSIS\n" COLOR_OFF "\t[OPTIONS] arguments...\n\n");
    printf(COLOR_BOLD "DESCRIPTION\n" COLOR_OFF "\tShow all the process in an interactive table in real time.\n\n");
    printf(COLOR_BOLD "OPTION\n" COLOR_OFF);
    printf("\t-h --help\tPrint the manual in the terminal.\n");
    printf("\t-d --dry-run\tTest access to the list of processes on the local and/or remote machine without displaying them.\n");
    printf("\t-c --remote-config\tSpecifies the path to the configuration file containing the connection information on remote machines.\n");
    printf("\t-t --connexion-type\tSpecifies the type of connection to use for connection on remote machines (ssh, telnet).\n");
    printf("\t-P --port\tSpecifies the port to use for the chosen connection type. If this option is not specified, then the default port of the connection type is chosen.\n");
    printf("\t-l --login\tSpecifies the connection identifier and the remote machine. Ex : --login user@remote_server. remote_server is either the IP or the DNS name of the remote machine.\n");
    printf("\t-s --remote-server\tSpecifies the DNS name or IP of the remote machine.\n");
    printf("\t-u --username\tSpecifies the username to use for the connection.\n");
    printf("\t-p --password\tSpecifies the password to use for the connection.\n");
    printf("\t-a --all\tSpecifies to the program to collect both the list of processes on the local machine and remote machines. Used only if the -c or -s option is used.\n");
}