#include "../include/project.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

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
