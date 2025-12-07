#include "../include/project.h"

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
