#include "../include/project.h"
// tewss
int main(void) {
  /* initialize subsystems */
  // config_load(NULL);
  // manager_init();
  // process_init();
  // machine_init();
  // network_init();
  // ui_init();

  long ticks = get_clock_ticks_per_second();
  printf("Clock ticks per second: %ld\n", ticks);
  long uptime = get_system_uptime();
  printf("System uptime: %ld seconds\n", uptime);
  int cpus = get_number_of_cpus();
  printf("Number of CPUs: %d\n", cpus);
  int page_size = get_page_size();
  printf("Page size: %d bytes\n", page_size);
  long total_ram = get_total_ram_b();
  printf("Total RAM: %ld bytes\n", total_ram);

  // int pid_test = 1; // Usually, PID 1 is the init/systemd process
  // printf("Fetching info for process with PID %d...\n", pid_test);
  // process_info info;
  // if (get_process_info(pid_test, &info) == 0) {
  //     info.cpu_usage = info.cpu_stime + info.cpu_utime;
  //     printf("Process %d: %s\nState: %c\nUptime %lds\nCpu usage: %lu\nMemory
  //     usage: %lu\n", pid_test, info.name, (char)info.state, info.uptime,
  //     info.cpu_usage, info.mem_usage);
  // } else {
  //     printf("Failed to get process info for PID %d\n", pid_test);
  // }


  pthread_t th;

  process_info *process_list = NULL;
  bool stop_flag = false;
  /* structure pour passer deux arguments au thread (allouée dynamiquement) */
  struct thread_args {
    process_info **process_list_ptr;
    bool *stop_flag_ptr;
  } *args = malloc(sizeof(*args));
  if (!args) {
    fprintf(stderr, "Failed to allocate thread args\n");
    return 1;
  }
  args->process_list_ptr = &process_list;
  args->stop_flag_ptr = &stop_flag;

  pthread_create(&th, NULL, get_all_processes, (void *)args);

  for (int i = 0; i < 10; i++) {
    sleep(1);
    printf("process_list[%i].pid = %d\n", i, process_list[i].pid);
    /*  
    if (process_list == NULL) {
      fprintf(stderr, "Failed to retrieve process list.\n");
      return 1;
    }*/
    size_t count = 0;
    while (process_list[count].pid != 0) {
      count++;
    }
    printf("Total processes retrieved: %zu\n", count);
    print_all_processes(process_list, count);

    printf("System information retrieval complete.\n");

    /* shutdown */
    // ui_shutdown();
    // manager_shutdown();


    if (i == 5) {
      stop_flag = true;
      printf("#DEBUG Signaled to stop process retrieval thread.\n");
    }
  }

  /* signaler au thread d'arrêter */
  stop_flag = true;

  pthread_join(th, NULL);
  free(process_list);
  free(args);

  return 0;
}
