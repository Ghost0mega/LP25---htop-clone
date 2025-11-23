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
int get_number_of_cpus(void) {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

/**
 * Get the system page size in bytes.
 * @return Page size in bytes.
 */
int get_page_size(void) {
    return sysconf(_SC_PAGESIZE);
}

/**
 * Get the total number of processes in the system.
 * @return Total number of processes.
 */
int get_process_amount(void);

/* Process handling API (stub) */

/**
 * Initialize the process subsystem.
 * @return 0 on success, other on failure.
 */
// int process_init(void);

/**
 * Fill the struct info with data about the process with the given pid.
 * @param pid Process ID.
 * @param info Pointer to process_info_t struct to fill.
 * @return 0 on success, other on failure (e.g., process not found).
 */
int get_process_info(int pid, process_info *info) {
    FILE *fp;
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }
    // char stat[1024];
    // size_t bytes = fread(stat, 1, sizeof(stat) - 1, fp);
    // if (bytes == 0) {
    //     if (ferror(fp)) {
    //         return -1;
    //     }
    //     stat[0] = '\0';
    // } else {
    //     stat[bytes] = '\0';
    // }
    // printf("Path : %s \nStat : %s \n", path, stat);
    fclose(fp);
    return 0;
}

/**
 * Get a list of all current processes.
 * @param count Pointer to size_t to store the number of processes.
 * @return Dynamically allocated array of process_info structs. Caller must free.
 */
process_info *get_all(size_t *count);
