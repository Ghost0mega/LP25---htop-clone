#include "../include/project.h"



int main(int argc, char *argv[]) {

    int arg_return = manage_arguments(argc, argv);
    switch (arg_return) {
        default:
            break;
        
        //error case.
        case 1:
            return EXIT_FAILURE;
            break;
        
        //--help, so no error but we stop the program.
        case 2:
            return EXIT_SUCCESS;
            break;
    }

    process_info *process_list = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    if (manager_start_process_thread(&process_list, &mutex) != 0) {
        fprintf(stderr, "Failed to start process thread\n");
        return EXIT_FAILURE;
    }

    ui_loop(&process_list, &mutex);

    manager_stop_process_thread();

    if (process_list)
        free(process_list);

    return EXIT_SUCCESS;
}
