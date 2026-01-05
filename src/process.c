#include "../include/project.h"
#include <ctype.h>
#include <dirent.h>
#include <string.h>

/**
 * Get the number of clock ticks per second.
 * @return Number of clock ticks per second.
 */
long get_clock_ticks_per_second(void) {
  static long ticks = 0;
  if (ticks == 0) {
    ticks = sysconf(_SC_CLK_TCK);
  }
  return ticks;
}

/**
 * Get the system uptime in seconds.
 * @return System uptime in seconds.
 */
long get_system_uptime(void) {
  FILE *fp = fopen("/proc/uptime", "r");
  if (!fp) {
    return -1;
  }
  double uptime_seconds;
  if (fscanf(fp, "%lf", &uptime_seconds) != 1) {
    fclose(fp);
    return -1;
  }
  fclose(fp);
  return (long)uptime_seconds;
}

/**
 * Get the total RAM in bytes.
 * @return Total RAM in bytes.
 */
long get_total_ram_b(void) {
  FILE *fp = fopen("/proc/meminfo", "r");
  if (!fp) {
    return -1;
  }
  char line[256];
  long total_ram_kb = -1;
  while (fgets(line, sizeof(line), fp)) {
    if (sscanf(line, "MemTotal: %ld kB", &total_ram_kb) == 1) {
      break;
    }
  }
  fclose(fp);
  if (total_ram_kb == -1) {
    return -1;
  }
  return total_ram_kb * 1024; // Convert to bytes
}

/* Process handling API (stub) */
/**
 * Get the number of CPUs in the system.
 * @return Number of CPUs.
 */
int get_number_of_cpus(void) { return sysconf(_SC_NPROCESSORS_ONLN); }

/**
 * Get the system page size in bytes.
 * @return Page size in bytes.
 */
int get_page_size(void) { return sysconf(_SC_PAGESIZE); }
void print_process_info(process_info *info) {
  printf("PID: %d \t| Name: %s \t| State: %c \t| CPU Usage: %.1f%% \t| Memory "
         "Usage: %lu \t| Uptime: %ld seconds\n",
         info->pid, info->name, (char)info->state, info->cpu_usage,
         info->mem_usage, info->uptime);
}

void print_all_processes(process_info *processes, size_t count) {
  for (size_t i = 0; i < count; i++) {
    if (processes[i].pid != 0) {
      print_process_info(&processes[i]);
    }
  }
}

/* Process handling API (stub) */

/**
 * Fill the struct info with data about the process with the given pid.
 * @param pid Process ID.
 * @param info Pointer to process_info_t struct to fill.
 * @return 0 on success, other on failure (e.g., process not found).
 */
int get_process_info(int pid, process_info *info) {
  char path[64];
  snprintf(path, sizeof(path), "/proc/%d/stat", pid);
  FILE *fp = fopen(path, "r");
  if (!fp) {
    return -1;
  }

  char buf[1024];
  size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
  fclose(fp);
  buf[n] = '\0';

  char comm[256];
  char state_char;
  long rss;

  // Reference:
  // https://stackoverflow.com/questions/39066998/what-are-the-meaning-of-values-at-proc-pid-stat
  int matched = sscanf(
      buf,
      "%d (%255[^)]) %c "    // 1-3: pid, comm, state
      "%*d %*d %*d %*d %*d " // 4-8: ppid, pgrp, session, tty_nr, tpgid
      "%*u %*u %*u %*u %*u " // 9-13: flags, minflt, cminflt, majflt, cmajflt
      "%lu %lu "             // 14-15: utime, stime
      "%*d %*d %*d %*d %*d %*d " // 16-21: cutime, cstime, priority, nice,
                                 // num_threads, itrealvalue
      "%lu "                     // 22: starttime
      "%*u "                     // 23: vsize
      "%ld",                     // 24: rss (pages)
      &info->pid, comm, &state_char, &info->cpu_utime, &info->cpu_stime,
      &info->starttime, &rss);

  if (matched != 7) {
    return -1;
  }

  strncpy(info->name, comm, sizeof(info->name));
  info->name[sizeof(info->name) - 1] = '\0';
  info->state = (process_state)state_char;

  // Calculate Memory Usage
  long page_size = get_page_size();
  long total_ram = get_total_ram_b();
  // printf("Total RAM: %ld bytes\n", total_ram);
  // printf("RSS (pages): %ld\n", rss);
  // printf("Page size: %ld bytes\n", page_size);
  // printf("RSS in bytes: %ld\n", rss * page_size);

  if (total_ram > 0) {
    info->mem_usage = (unsigned long)(rss * page_size);
  } else {
    info->mem_usage = 0;
  }
  info->cpu_usage = info->cpu_utime + info->cpu_stime;

  info->uptime = get_system_uptime() - (info->starttime / get_clock_ticks_per_second());
  info->remote_config_index = -1;  // -1 indique un processus local

  return 0;
}

int get_process_network_stats(int pid, unsigned long long *bytes_sent, unsigned long long *bytes_recv) {
    if (!bytes_sent || !bytes_recv) return -1;
    
    char path[128];
    snprintf(path, sizeof(path), "/proc/%d/net/dev", pid);
    
    FILE *fp = fopen(path, "r");
    if (!fp) {
        *bytes_sent = 0;
        *bytes_recv = 0;
        return -1;
    }
    
    char line[512];
    unsigned long long total_recv = 0;
    unsigned long long total_sent = 0;
    
    // Ignorer les 2 premières lignes (en-têtes)
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);
    
    // Parser chaque interface réseau
    while (fgets(line, sizeof(line), fp)) {
        char interface[32];
        unsigned long long recv_bytes, recv_packets, recv_errs, recv_drop;
        unsigned long long send_bytes, send_packets, send_errs, send_drop;
        
        int matched = sscanf(line, "%31[^:]:%llu %llu %llu %llu %*u %*u %*u %*u %llu %llu %llu %llu",
                            interface, 
                            &recv_bytes, &recv_packets, &recv_errs, &recv_drop,
                            &send_bytes, &send_packets, &send_errs, &send_drop);
        
        if (matched >= 9) {
            char *iface_name = interface;
            while (*iface_name == ' ' || *iface_name == '\t') iface_name++;
            
            if (strcmp(iface_name, "lo") != 0) {
                total_recv += recv_bytes;
                total_sent += send_bytes;
            }
        }
    }
    
    fclose(fp);
    *bytes_recv = total_recv;
    *bytes_sent = total_sent;
    return 0;
}

int *get_all_pids(void *arg) {
  (void)arg; // Unused parameter
  char buffer[128];
  FILE *fp;

  // Exécute la commande
  fp = popen("ps -ef | awk '{print $2}'", "r");

  if (fp == NULL) {
    perror("Erreur popen");
    exit(1);
  }

  /* skip header line if any */
  fgets(buffer, sizeof(buffer), fp); /* skip header */

  int *pid_list = malloc(1000 * sizeof(int));
  if (pid_list == NULL) {
    perror("Erreur malloc");
    pclose(fp);
    exit(1);
  }

  int count = 0;
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    pid_list[count] = atoi(buffer);
    count++;
    if (count >= 1000) {
      int *tmp = realloc(pid_list, (count + 1000) * sizeof(int));
      if (tmp == NULL) {
        perror("Erreur realloc");
        free(pid_list);
        pclose(fp);
        exit(1);
      }
      pid_list = tmp;
    }
  }

  /* add sentinel 0 */
  int *tmp = realloc(pid_list, (count + 1) * sizeof(int));
  if (tmp == NULL) {
    perror("Erreur realloc final");
    free(pid_list);
    pclose(fp);
    exit(1);
  }
  pid_list = tmp;
  pid_list[count] = 0;

  pclose(fp);
  return pid_list;
}

unsigned long long get_total_system_cpu_time(void) {
  FILE *fp = fopen("/proc/stat", "r");
  if (!fp)
    return 0;
  char buf[1024];
  if (!fgets(buf, sizeof(buf), fp)) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
  if (sscanf(buf, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user,
             &nice, &system, &idle, &iowait, &irq, &softirq, &steal) < 8) {
    return 0;
  }
  return user + nice + system + idle + iowait + irq + softirq + steal;
}

void *get_all_processes(void *arg) {
  thread_args_t *args = (thread_args_t *)arg;

  process_info **process_list_ptr = args->process_list_ptr;
  bool *stop_flag = args->stop_flag_ptr;
  pthread_mutex_t *mutex = args->mutex;
  bool include_remote = args->include_remote;
  bool include_local = args->include_local;

  process_info *prev_list = NULL;
  unsigned long long prev_system_time = 0;

  while (!(*stop_flag)) {
    size_t count = 0;
    int *pid_list = get_all_pids(NULL);
    if (pid_list == NULL) {
      return NULL;
    }

    while (pid_list[count] != 0) {
      count++;
    }

    // If we don't want local processes, set count to 0
    if (!include_local) {
      count = 0;
    }

    process_info *process_list = calloc(count + 1, sizeof(process_info));
    if (process_list == NULL) {
      free(pid_list);
      return NULL;
    }

    unsigned long long current_system_time = get_total_system_cpu_time();

    for (size_t i = 0; i < count; i++) {
      if (pid_list[i] != 0) {
          get_process_info(pid_list[i], &process_list[i]);
          
          // Récupérer les statistiques réseau
          unsigned long long net_sent = 0, net_recv = 0;
          get_process_network_stats(pid_list[i], &net_sent, &net_recv);
          process_list[i].net_bytes_sent = net_sent;
          process_list[i].net_bytes_recv = net_recv;
          
          // Calculate CPU usage
          if (prev_list && prev_system_time > 0) {
              size_t j = 0;
              while (prev_list[j].pid != 0) {
                  if (prev_list[j].pid == process_list[i].pid) {
                      unsigned long long proc_time = process_list[i].cpu_utime + process_list[i].cpu_stime;
                      unsigned long long prev_proc_time = prev_list[j].cpu_utime + prev_list[j].cpu_stime;
                      unsigned long long system_delta = current_system_time - prev_system_time;
                      
                      if (system_delta > 0) {
                          process_list[i].cpu_usage = (float)(proc_time - prev_proc_time) / system_delta * 100.0 * get_number_of_cpus();
                      }
                      
                      // Calculer les débits réseau (octets/sec)
                      if (process_list[i].net_bytes_sent >= prev_list[j].net_bytes_sent) {
                          process_list[i].net_send_rate = (float)(process_list[i].net_bytes_sent - prev_list[j].net_bytes_sent);
                      } else {
                          process_list[i].net_send_rate = 0.0f;
                      }
                      
                      if (process_list[i].net_bytes_recv >= prev_list[j].net_bytes_recv) {
                          process_list[i].net_recv_rate = (float)(process_list[i].net_bytes_recv - prev_list[j].net_bytes_recv);
                      } else {
                          process_list[i].net_recv_rate = 0.0f;
                      }
                      
                      break;
                  }
                  j++;
              }
          }
          
          // Si pas trouvé dans prev_list, initialiser à 0
          if (!prev_list || prev_list[0].pid == 0) {
              process_list[i].net_send_rate = 0.0f;
              process_list[i].net_recv_rate = 0.0f;
          }
      }
  }
    /* terminateur */
    process_list[count].pid = 0;

    /* Ajouter les processus distants si demandé */
    if (include_remote) {
        int total_count = network_poll_all_processes(&process_list, count);
        if (total_count > (int)count) {
            /* Mark end of list */
            process_list[total_count].pid = 0;
            count = total_count;
        }
    }

    /* publier la nouvelle liste et libérer l'ancienne */

    if (mutex)
      pthread_mutex_lock(mutex);
    if (*process_list_ptr != NULL) {
      free(*process_list_ptr);
    }
    *process_list_ptr = process_list;
    if (mutex)
      pthread_mutex_unlock(mutex);

    prev_list = process_list; // Keep reference for next iteration (NOTE: *process_list_ptr owns it, but we know it won't be freed until we do it next loop)
    prev_system_time = current_system_time;

    sleep(1);
    free(pid_list);
  }

  /* libérer la dernière liste avant de quitter */
  if (mutex)
    pthread_mutex_lock(mutex);
  if (*process_list_ptr != NULL) {
    free(*process_list_ptr);
    *process_list_ptr = NULL;
  }
  if (mutex)
    pthread_mutex_unlock(mutex);
  return NULL;
}
