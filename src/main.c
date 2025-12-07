#include "../include/project.h"
// tewss
int main(void) {

  process_info *process_list = NULL;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

  if (manager_start_process_thread(&process_list, &mutex) != 0) {
      fprintf(stderr, "Failed to start process thread\n");
      return 1;
  }

  for (int i = 0; i < 10; i++) {
    sleep(1);
    
    pthread_mutex_lock(&mutex);
    if (process_list == NULL) {
        printf("process_list is NULL\n");
        pthread_mutex_unlock(&mutex);
        continue;
    }

    printf("process_list[%i].pid = %d\n", i, process_list[i].pid);

    size_t count = 0;
    while (process_list[count].pid != 0) {
      count++;
    }
    printf("Total processes retrieved: %zu\n", count);
    print_all_processes(process_list, count);
    pthread_mutex_unlock(&mutex);

    printf("System information retrieval complete.\n");

    if (i == 5) {
      printf("#DEBUG Signaled to stop process retrieval thread.\n");
      break;
    }
  }

  manager_stop_process_thread();
  
  if (process_list) free(process_list);

  return 0;
}
