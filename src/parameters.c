#include "../include/project.h"

/*=========
* METHODS: *
==========*/

bool params_validate(parameters_table *params, int params_count) {
    //Initialisation:
    bool local_mode;

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
                //The parameters isn't valid if it is null, or the user cannot acess the given path.
                if (strlen(param->parameter_value.str_param) == 0) {
                    return false;
                } else {
                    if (access(param->parameter_value.str_param, R_OK) != 0) {
                        fprintf(stderr,"ERREUR: Impossible d'accéder au fihcier de configuration réseau.\n");
                        return false;
                    }
                }
                break;

            case PARAM_PORT:
                //Tested only if the program isn't for a local use:
                if (!local_mode) {
                    //The parameters isn't correct if the value is less than 1024 or bigger than 65535:
                    if ((param->parameter_value.int_param < 1024) || (param->parameter_value.int_param > 65535)) {
                        fprintf(stderr,"ERREUR: Le port doit être dans compris entre 1024 et 65535.\n");
                        return false;
                    } else {
                        //We verify that the port is free:
                        if(!is_port_free(param->parameter_value.int_param)) return false;
                        fprintf(stderr,"ERREUR: Le port sélectionner est déjà utilisé.\n");
                    }
                }
                break;

            case PARAM_LOGIN:
                //Tested only if the program isn't for a local use:
                if (!local_mode) {
                    //fill with the conditions
                }
                break;
                        
            case PARAM_REMOTE_SERVER:
                //Tested only if the program isn't for a local use:
                if (!local_mode) {
                    //fill with the conditions
                }
                break;

            case PARAM_USERNAME:
                //Tested only if the program isn't for a local use:
                if (!local_mode) {
                    //fill with the conditions
                }
                break;
                
            case PARAM_PASSWORD:
                //Tested only if the program isn't for a local use:
                if (!local_mode) {
                    //fill with the conditions
                }
                break;
            }
        }

    //If nothing goes wrong we return true:
    return true;
}

bool is_port_free(int port) {
    //à remplir avec les besoin reseaux, quand on sera là
    return false;
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