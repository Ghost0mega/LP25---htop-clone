#ifndef MANAGER_H
#define MANAGER_H

#include "process.h"
#include <pthread.h>

/*=========  FORWARD DECLARATIONS:  ==========*/

typedef struct parameters_table parameters_table;

/**
 * Start the process retrieval thread (local processes only).
 * @param process_list_ptr Pointer to the process list pointer.
 * @param mutex Pointer to the mutex protecting the process list.
 * @return 0 on success, non-zero on failure.
 */
int manager_start_process_thread(process_info **process_list_ptr,
                                 pthread_mutex_t *mutex);

/**
 * Start the process retrieval thread (local + remote processes).
 * @param process_list_ptr Pointer to the process list pointer.
 * @param mutex Pointer to the mutex protecting the process list.
 * @return 0 on success, non-zero on failure.
 */
int manager_start_process_thread_with_remote(process_info **process_list_ptr,
                                             pthread_mutex_t *mutex);

/**
 * Stop the process retrieval thread.
 */
void manager_stop_process_thread(void);

/**
 * Loop the UI with all the process datas.
 */
int ui_and_process_loop();

/**
 * Main UI and process loop with parameters.
 * @param include_local Include local processes.
 * @param include_remote_only Include only remote processes (no local).
 */
int ui_and_process_loop_with_params(bool include_local,
                                    bool include_remote_only);

/**
 * Execute the dry run mode (tests access to local and/or remote processes).
 * @param parameters Table of parameters from CLI.
 * @param params_count Number of parameters.
 */
int dry_run(parameters_table *parameters, int params_count);

#endif /* MANAGER_H */
