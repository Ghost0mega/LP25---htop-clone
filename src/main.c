#include "../include/project.h"

int main(int argc, char *argv[]) {
    //Initialization:
    int params_count;

    //Get all the arguments:
    parameters_table *params = manage_arguments(argc, argv, &params_count);
    if (!params) return EXIT_FAILURE;

    //Case of --help or -h:
    if (is_param_type(params, params_count, PARAM_HELP)) {
        free(params);
        manual();
        return EXIT_SUCCESS;
    }
    
    //Initialize network configuration if remote options are specified:
    if (is_param_type(params, params_count, PARAM_REMOTE_CONFIG) ||
        is_param_type(params, params_count, PARAM_REMOTE_SERVER) ||
        is_param_type(params, params_count, PARAM_LOGIN)) {
        if (network_init(params, params_count) != 0) {
            fprintf(stderr, "ERROR: Failed to initialize network configuration.\n");
            free(params);
            return EXIT_FAILURE;
        }
    }
    
    //dry run case (local and/or remote):
    if (is_param_type(params, params_count, PARAM_DRY_RUN)) {
        int ret = dry_run(params, params_count);
        free(params);
        return ret;
    }

    //Normal run (local and/or remote):
    free(params);
    return ui_and_process_loop();
}
