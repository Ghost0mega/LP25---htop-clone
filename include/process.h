#ifndef PROCESS_H
#define PROCESS_H

typedef struct process_info {
    int pid;
    char name[256];
    char user[64];
    double cpu_usage;
    double mem_usage;
    long uptime;

    unsigned long cpu_utime;
    unsigned long cpu_stime;
    unsigned long starttime;
} process_info;

/* Getting process information */

/**
 * Get the number of clock ticks per second.
 * @return Number of clock ticks per second.
 */
long get_clock_ticks_per_second(void);

/**
 * Get the system uptime in seconds.
 * @return System uptime in seconds.
 */
long get_system_uptime(void);

/**
 * Get the total RAM in bytes.
 * @return Total RAM in bytes.
 */
long get_total_ram_b(void);

/**
 * Get the number of CPUs in the system.
 * @return Number of CPUs.
 */
int get_number_of_cpus(void);

/**
 * Get the system page size in bytes.
 * @return Page size in bytes.
 */
int get_page_size(void);

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
int get_process_info(int pid, process_info *info);

/**
 * Get a list of all current processes.
 * @param count Pointer to size_t to store the number of processes.
 * @return Dynamically allocated array of process_info structs. Caller must free.
 */
process_info *get_all(size_t *count);

#endif /* PROCESS_H */
