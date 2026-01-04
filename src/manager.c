#include "../include/project.h"

static pthread_t proc_thread;
static bool proc_stop_flag = false;
static thread_args_t *proc_args = NULL;


int manager_start_process_thread(process_info **process_list_ptr, pthread_mutex_t *mutex) {
    proc_stop_flag = false;
    proc_args = malloc(sizeof(thread_args_t));
    if (!proc_args) {
        return -1;
    }
    proc_args->process_list_ptr = process_list_ptr;
    proc_args->stop_flag_ptr = &proc_stop_flag;
    proc_args->mutex = mutex;
    proc_args->include_remote = false;  /* Default: no remote processes */
    proc_args->include_local = true;    /* Always include local by default */

    if (pthread_create(&proc_thread, NULL, get_all_processes, proc_args) != 0) {
        free(proc_args);
        return -1;
    }
    return 0;
}

int manager_start_process_thread_with_remote(process_info **process_list_ptr, pthread_mutex_t *mutex) {
    proc_stop_flag = false;
    proc_args = malloc(sizeof(thread_args_t));
    if (!proc_args) {
        return -1;
    }
    proc_args->process_list_ptr = process_list_ptr;
    proc_args->stop_flag_ptr = &proc_stop_flag;
    proc_args->mutex = mutex;
    proc_args->include_remote = true;   /* Include remote processes */
    proc_args->include_local = false;   /* Don't include local by default with remote */

    if (pthread_create(&proc_thread, NULL, get_all_processes, proc_args) != 0) {
        free(proc_args);
        return -1;
    }
    return 0;
}

int manager_start_process_thread_with_all(process_info **process_list_ptr, pthread_mutex_t *mutex) {
    proc_stop_flag = false;
    proc_args = malloc(sizeof(thread_args_t));
    if (!proc_args) {
        return -1;
    }
    proc_args->process_list_ptr = process_list_ptr;
    proc_args->stop_flag_ptr = &proc_stop_flag;
    proc_args->mutex = mutex;
    proc_args->include_remote = true;   /* Include remote processes */
    proc_args->include_local = true;    /* Also include local processes */

    if (pthread_create(&proc_thread, NULL, get_all_processes, proc_args) != 0) {
        free(proc_args);
        return -1;
    }
    return 0;
}

void manager_stop_process_thread(void) {
    proc_stop_flag = true;
    pthread_join(proc_thread, NULL);
    if (proc_args) {
        free(proc_args);
        proc_args = NULL;
    }
}

int ui_and_process_loop() {
    process_info *process_list = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    /* Start process thread with or without remote processes */
    int ret;
    if (g_remote_configs_count > 0) {
        ret = manager_start_process_thread_with_remote(&process_list, &mutex);
    } else {
        ret = manager_start_process_thread(&process_list, &mutex);
    }
    
    if (ret != 0) {
        fprintf(stderr, "ERROR: Failed to start process thread\n");
        return EXIT_FAILURE;
    }

    ui_loop(&process_list, &mutex);

    manager_stop_process_thread();

    if (process_list) free(process_list);
    
    return EXIT_SUCCESS;
}

int ui_and_process_loop_with_params(bool include_local, bool include_remote_only) {
    process_info *process_list = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    /* Start process thread based on parameters */
    int ret;
    if (include_remote_only) {
        // Remote only (no local processes)
        ret = manager_start_process_thread_with_remote(&process_list, &mutex);
    } else if (include_local && g_remote_configs_count > 0) {
        // Both local and remote
        ret = manager_start_process_thread_with_all(&process_list, &mutex);
    } else {
        // Local only (default)
        ret = manager_start_process_thread(&process_list, &mutex);
    }
    
    if (ret != 0) {
        fprintf(stderr, "ERROR: Failed to start process thread\n");
        return EXIT_FAILURE;
    }

    ui_loop(&process_list, &mutex);

    manager_stop_process_thread();

    if (process_list) free(process_list);
    
    return EXIT_SUCCESS;
}

int dry_run(parameters_table *parameters, int params_count) {
    //Initialization:
    process_info *process_list = NULL;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    fprintf(stdout, "DRY-RUN: Testing access to processes...\n");

    //Test local process access:
    if (manager_start_process_thread(&process_list, &mutex) != 0) {
        fprintf(stderr, "ERROR: (DRY-RUN) Failed to start process thread\n");
        return EXIT_FAILURE;
    }

    manager_stop_process_thread();

    if (process_list) free(process_list);

    //Test remote access if remote options were provided:
    if (is_param_type(parameters, params_count, PARAM_REMOTE_CONFIG) ||
        is_param_type(parameters, params_count, PARAM_REMOTE_SERVER) ||
        is_param_type(parameters, params_count, PARAM_LOGIN)) {
        fprintf(stdout, "DRY-RUN: Testing remote server connection...\n");
        if (network_init(parameters, params_count) != 0) {
            fprintf(stderr, "ERROR: (DRY-RUN) Failed to test remote connection.\n");
            return EXIT_FAILURE;
        }
    }

    fprintf(stdout, "DRY-RUN: All tests passed successfully.\n");
    return EXIT_SUCCESS;
}