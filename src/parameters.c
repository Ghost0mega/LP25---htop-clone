#include "../include/project.h"

/*=========
* METHODS: *
==========*/

parameters_table *manage_arguments(int argc, char *argv[], int *out_count) {
    int parameters_count=0;
    int opt;

    if (!out_count) return NULL;

    parameters_table *given_parameters = calloc(PARAMETER_BUFFER_SIZE, sizeof(parameters_table));
    if (!given_parameters) return NULL;

    struct option my_opts[] = {
        {.name = "help", .has_arg = 0, .flag = 0, .val = 'h'},
        {.name = "dry-run", .has_arg = 0, .flag = 0, .val = 1},
        {.name = "remote-config", .has_arg = 1, .flag = 0, .val = 'c'},
        {.name = "port", .has_arg = 1, .flag = 0, .val = 'P'},
        {.name = "login", .has_arg = 1, .flag = 0, .val = 'l'},
        {.name = "remote-server", .has_arg = 1, .flag = 0, .val = 's'},
        {.name = "username", .has_arg = 1, .flag = 0, .val = 'u'},
        {.name = "password", .has_arg = 1, .flag = 0, .val = 'p'},
        {.name = "all", .has_arg = 0, .flag = 0, .val = 'a'},
        {.name = "connexion-type", .has_arg = 1, .flag = 0, .val = 't'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hdc:P:l:s:u:p:at:", my_opts, NULL)) != -1) {

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

            case 't':
                given_parameters[parameters_count].parameter_type = PARAM_CONNECTION_TYPE;
                strncpy(given_parameters[parameters_count].parameter_value.str_param, optarg, STR_MAX-1);
                given_parameters[parameters_count].parameter_value.str_param[STR_MAX-1] = '\0';
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
    bool has_config_file = false;
    bool has_login = false;
    bool has_all_flag = false;

    for (int i = 0; i < params_count; i++) {
        if (params[i].parameter_type == PARAM_ALL) {
            has_all_flag = true;
            break;
        }
    }
    
    // Validation de l'option -a/--all : nécessite -c ou -s
    if (has_all_flag) {
        bool has_config_or_server = false;
        for (int i = 0; i < params_count; i++) {
            if (params[i].parameter_type == PARAM_REMOTE_CONFIG || 
                params[i].parameter_type == PARAM_REMOTE_SERVER ||
                params[i].parameter_type == PARAM_LOGIN) {
                has_config_or_server = true;
                break;
            }
        }
        if (!has_config_or_server) {
            fprintf(stderr, "ERROR: Option -a/--all requires -c/--remote-config, -s/--remote-server, or -l/--login.\n");
            return false;
        }
    }

    for (int i=0; i<params_count; i++) {
        parameters_table *param = &params[i];

        switch(param->parameter_type) {
            default:
                break;

            case PARAM_REMOTE_CONFIG:
                if (strlen(param->parameter_value.str_param) == 0) {
                    return false;
                } else {
                    if (access(param->parameter_value.str_param, R_OK) != 0) {
                        fprintf(stderr,"ERROR: Cannot access the configuration file.\n");
                        return false;
                    }
                    if (!is_config_file_valid(param->parameter_value.str_param)) {
                        fprintf(stderr,"ERROR: Follow this format for the configuration file:\nserver_name1:server_adress:port:username:password:connexiontype\n");
                        return false;
                    }
                }
                has_config_file = true;
                break;

            case PARAM_PORT:
                if (!has_config_file) {
                    if ((param->parameter_value.int_param < 1) || (param->parameter_value.int_param > 65535)) {
                        fprintf(stderr,"ERROR: Please enter a port value between 1 and 65535.\n");
                        return false;
                    }
                }
                break;

            case PARAM_LOGIN:
                if (!has_config_file) {
                    if (strlen(param->parameter_value.str_param) == 0) {
                        fprintf(stderr, "ERROR: Login cannot be empty.\n");
                        return false;
                    }
                    char *at_sign = strchr(param->parameter_value.str_param, '@');
                    if (at_sign == NULL) {
                        fprintf(stderr, "ERROR: Login must be in the format user@server.\n");
                        return false;
                    }
                    has_login = true;
                }
                break;
                
            case PARAM_REMOTE_SERVER:
                if (!has_config_file) {
                    if (strlen(param->parameter_value.str_param) == 0) {
                        fprintf(stderr, "ERROR: Remote server address cannot be empty.\n");
                        return false;
                    }
                    if (has_login) {
                        fprintf(stderr, "ERROR: Cannot use both -l/--login and -s/--remote-server.\n");
                        return false;
                    }
                }
                break;

            case PARAM_USERNAME:
                if (!has_config_file) {
                    if (strlen(param->parameter_value.str_param) == 0) {
                        fprintf(stderr, "ERROR: Username cannot be empty.\n");
                        return false;
                    }
                    if (has_login) {
                        fprintf(stderr, "ERROR: Cannot use both -l/--login and -u/--username.\n");
                        return false;
                    }
                }
                break;
                
            case PARAM_PASSWORD:
                if (!has_config_file) {
                    if (strlen(param->parameter_value.str_param) == 0) {
                        fprintf(stderr, "ERROR: Password cannot be empty.\n");
                        return false;
                    }
                }
                break;

            case PARAM_CONNECTION_TYPE:
                if (strlen(param->parameter_value.str_param) == 0) {
                    fprintf(stderr, "ERROR: Connection type cannot be empty.\n");
                    return false;
                }
                // Convertir en minuscules pour la comparaison
                char type_lower[STR_MAX];
                strncpy(type_lower, param->parameter_value.str_param, STR_MAX-1);
                type_lower[STR_MAX-1] = '\0';
                for (int j = 0; type_lower[j]; j++) {
                    type_lower[j] = tolower(type_lower[j]);
                }
                // Seul SSH est supporté pour le moment
                if (strcmp(type_lower, "ssh") != 0) {
                    fprintf(stderr, "ERROR: Connection type '%s' is not supported. Only 'ssh' is currently implemented.\n", 
                            param->parameter_value.str_param);
                    return false;
                }
                break;
        }
    }

    return true;
}

bool is_param_type(parameters_table *params, int params_count, parameters_id_table type) {
    bool found = false;

    for (int i = 0; i < params_count; i++) {
        if (params[i].parameter_type == type) {
            found = true;
            break;
        }
    }

    return found;
}

int get_int_parameters(parameters_table *params, int params_count, parameters_id_table type) {
    for (int i = 0; i < params_count; i++) {
        if (params[i].parameter_type == type) {
            return params[i].parameter_value.int_param;
        }
    }
    return -1;
}

int get_string_parameters(parameters_table *params, int params_count, parameters_id_table type, char *string) {
    for (int i = 0; i < params_count; i++) {
        if (params[i].parameter_type == type) {
            strcpy(string, params[i].parameter_value.str_param);
            return 0;
        }
    }
    return 1;
}

void manual() {
    printf(COLOR_BOLD "NOM\n" COLOR_OFF "\thtop-clone - interactive process viewer\n\n");
    printf(COLOR_BOLD "SYNOPSIS\n" COLOR_OFF "\t[OPTIONS] arguments...\n\n");
    printf(COLOR_BOLD "DESCRIPTION\n" COLOR_OFF "\tShow all the process in an interactive table in real time.\n\n");
    printf(COLOR_BOLD "OPTION\n" COLOR_OFF);
    printf("\t-h --help\t\tPrint the manual in the terminal.\n");
    printf("\t-d --dry-run\t\tTest access to the list of processes on the local and/or remote machine without displaying them.\n");
    printf("\t-c --remote-config\tSpecifies the path to the configuration file containing the SSH connection information on remote machines.\n");
    printf("\t\t\t\tFormat: name:address:port:username:password:connection_type (ssh or telnet)\n");
    printf("\t-P --port\t\tSpecifies the SSH port to use. If this option is not specified, the default port 22 is used.\n");
    printf("\t-l --login\t\tSpecifies the connection identifier and the remote machine. Ex : --login user@remote_server. remote_server is either the IP or the DNS name of the remote machine.\n");
    printf("\t-s --remote-server\tSpecifies the DNS name or IP of the remote machine.\n");
    printf("\t-u --username\t\tSpecifies the username to use for the SSH connection.\n");
    printf("\t-p --password\t\tSpecifies the password to use for the SSH connection.\n");
    printf("\t-t --connexion-type\tSpecifies the connection type to use for remote machines (ssh, telnet). Only 'ssh' is currently implemented.\n");
    printf("\t-a --all\t\tSpecifies to the program to collect both the list of processes on the local machine and remote machines. Used only if the -c, -s, or -l option is used.\n");
}