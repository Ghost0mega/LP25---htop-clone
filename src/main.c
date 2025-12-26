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
    
    //dry run local case:
    if (is_param_type(params, params_count, PARAM_DRY_RUN)) {
        free(params);
        return dry_run();
    }

    //Normal local run:
    free(params);
    return ui_and_process_loop();
}
