#ifndef MANAGER_H
#define MANAGER_H

#include <pthread.h>
#include "process.h"


/**
 * Start the process retrieval thread.
 * @param process_list_ptr Pointer to the process list pointer.
 * @param mutex Pointer to the mutex protecting the process list.
 * @return 0 on success, non-zero on failure.
 */
int manager_start_process_thread(process_info **process_list_ptr, pthread_mutex_t *mutex);

/**
 * Stop the process retrieval thread.
 */
void manager_stop_process_thread(void);

#endif /* MANAGER_H */
