#include "../include/project.h"

/*=========
* METHODS: *
==========*/

bool params_validate(parameters_table *params, int params_count) {
    //Validation foreach parameters which takes values:
    for (int i=0; i<params_count; i++) {
        //Current parameter:
        parameters_table *param = &params[i];

        //Rules for each type of parameters:
        switch(param->parameter_type) {
            case PARAM_REMOTE_CONFIG:
                //The parameters isn't correct if the value is null:
                if (strlen(param->parameter_value.str_param) == 0) {
                    return false;
                }
                break;

                case PARAM_CONNEXION_TYPE:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;

                case PARAM_PORT:
                    //The parameters isn't correct if the value is null:
                    if (param->parameter_value.int_param == 0) {
                        return false;
                    }
                    break;

                case PARAM_LOGIN:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;
                        
                case PARAM_REMOTE_SERVER:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;

                case PARAM_USERNAME:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;
                
                case PARAM_PASSWORD:
                    //The parameters isn't correct if the value is null:
                    if (strlen(param->parameter_value.str_param) == 0) {
                        return false;
                    }
                    break;
            }
        }

    //If nothing goes wrong we return true:
    return true;
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