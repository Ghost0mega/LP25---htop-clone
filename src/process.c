#include "../include/project.h"

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
  printf("PID: %d \t| Name: %s \t| State: %c \t| CPU Usage: %lu \t| Memory "
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

  info->uptime =
      get_system_uptime() - (info->starttime / get_clock_ticks_per_second());

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

  fgets(buffer, sizeof(buffer), fp);

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
      pid_list = realloc(pid_list, (count + 1000) * sizeof(int));
      if (pid_list == NULL) {
        perror("Erreur realloc");
        pclose(fp);
        exit(1);
      }
    }
  }

  pid_list = realloc(pid_list, count * sizeof(int)); // Resize to actual count
  if (pid_list == NULL) {
    perror("Erreur realloc final");
    pclose(fp);
    exit(1);
  }

  pclose(fp);
  return pid_list;
}

process_info *get_all_processes() {
  size_t count = 0;
  int *pid_list = get_all_pids(NULL);
  if (pid_list == NULL) {
    return NULL;
  }

  while (pid_list[count + 3] !=
         0) { // skipping 3 lines removes ghost processes for some reason
    // printf("PID: %d\n", pid_list[count]);
    count++;
  }
  // printf("Total processes found: %zu\n", count);
  process_info *process_list = malloc(count * sizeof(process_info));
  if (process_list == NULL) {
    free(pid_list);
    return NULL;
  }

  for (size_t i = 0; i < count; i++) {
    if (pid_list[i] != 0) {
      get_process_info(pid_list[i], &process_list[i]);
    }
    // print_process_info(&process_list[i]);
  }

  free(pid_list);
  return process_list;
}
