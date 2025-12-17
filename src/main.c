#include "../include/project.h"

#define PARAMETER_BUFFER_SIZE 16

int main(int argc, char *argv[]) {
    //Control of the parameters:
    parameters_table given_parameters[PARAMETER_BUFFER_SIZE]; //parameters received by the user.
    parameters_table default_parameters[] = { //Default parameters.
        {.parameter_type = PARAM_HELP, .parameter_value.flag_param = false},
        {.parameter_type = PARAM_DRY_RUN, .parameter_value.flag_param = false},
        {.parameter_type = PARAM_REMOTE_CONFIG, .parameter_value.str_param = "./config/remote.txt"}, // /!\: Reste à définir un chemin par défaut pour la configuration réseau
        {.parameter_type = PARAM_CONNEXION_TYPE, .parameter_value.str_param = "local"}, //By default the programm run on the local machine.
        {.parameter_type = PARAM_PORT, .parameter_value.int_param = 0}, // /!\: Reste à définir un port local.
        {.parameter_type = PARAM_LOGIN, .parameter_value.str_param = ""}, // /!\: Reste à définir un login par défaut.
        {.parameter_type = PARAM_REMOTE_SERVER, .parameter_value.str_param = "localhost"}, // /!\: Reste à définir un serveur par défaut.
        {.parameter_type = PARAM_USERNAME, .parameter_value.str_param = ""},
        {.parameter_type = PARAM_PASSWORD, .parameter_value.str_param = ""},
        {.parameter_type = PARAM_ALL, .parameter_value.flag_param = false},
    };
    int parameters_count = 0, default_parameters_count = sizeof(default_parameters) / sizeof(parameters_table), opt = 0;

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
    while((opt = getopt_long(argc, argv, "hdc:t:P:l:s:u:p:a", my_opts, NULL)) != -1) {
        switch (opt){

            //Return the manual of the programm:
            case 'h':
                given_parameters[parameters_count].parameter_type = PARAM_HELP;
                given_parameters[parameters_count].parameter_value.flag_param = true;
                parameters_count++;
                manual();
                break;
            
            case 1:
                given_parameters[parameters_count].parameter_type = PARAM_DRY_RUN;
                given_parameters[parameters_count].parameter_value.flag_param = true;
                parameters_count++;
                break;

            case 'c':
                given_parameters[parameters_count].parameter_type = PARAM_REMOTE_CONFIG;
                strcpy(given_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 't':
                given_parameters[parameters_count].parameter_type = PARAM_CONNEXION_TYPE;
                strcpy(given_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 'P':
                given_parameters[parameters_count].parameter_type = PARAM_PORT;
                given_parameters[parameters_count].parameter_value.int_param = atoi(optarg);
                parameters_count++;
                break;

            case 'l':
                given_parameters[parameters_count].parameter_type = PARAM_LOGIN;
                strcpy(given_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 's':
                given_parameters[parameters_count].parameter_type = PARAM_REMOTE_SERVER;
                strcpy(given_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 'u':
                given_parameters[parameters_count].parameter_type = PARAM_USERNAME;
                strcpy(given_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 'p':
                given_parameters[parameters_count].parameter_type = PARAM_PASSWORD;
                strcpy(given_parameters[parameters_count].parameter_value.str_param, optarg);
                parameters_count++;
                break;

            case 'a':
                given_parameters[parameters_count].parameter_type = PARAM_ALL;
                given_parameters[parameters_count].parameter_value.flag_param = true;
                parameters_count++;
                break;

            case '?':
                printf("Invalid Arguments\n");
                break;

            default:
                printf("Local mode\n");
                break;
        }
    }

    //Validate the final parameters:
    if (!params_validate(given_parameters, parameters_count)) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
int main(void) {

  process_info *process_list = NULL;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  if (manager_start_process_thread(&process_list, &mutex) != 0) {
    fprintf(stderr, "Failed to start process thread\n");
    return 1;
  }

  ui_loop(&process_list, &mutex);

  manager_stop_process_thread();

  if (process_list)
    free(process_list);

  return 0;
}
