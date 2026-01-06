#ifndef PROCESS_H
#define PROCESS_H

#include <pthread.h>
#include <stdbool.h>

typedef enum {
  PROCESS_STATE_RUNNING = 'R',
  PROCESS_STATE_SLEEPING = 'S',
  PROCESS_STATE_DISK_SLEEP = 'D',
  PROCESS_STATE_ZOMBIE = 'Z',
  PROCESS_STATE_STOPPED = 'T',
  PROCESS_STATE_TRACING_STOP = 't',
  PROCESS_STATE_DEAD = 'X',
  PROCESS_STATE_WAKEKILL = 'K',
  PROCESS_STATE_WAKING = 'W',
  PROCESS_STATE_PARKED = 'P',
  PROCESS_STATE_IDLE = 'I',
  PROCESS_STATE_UNKNOWN = '?'
} process_state;

typedef struct process_info {
  int pid;
  process_state state;
  char name[256];
  float cpu_usage;
  unsigned long mem_usage;
  long uptime;
  unsigned long cpu_utime;
  unsigned long cpu_stime;
  unsigned long starttime;
  int remote_config_index; // -1 pour local, sinon index dans g_remote_configs

  // Nouveaux champs pour les statistiques réseau
  unsigned long long net_bytes_sent; // Total octets envoyés
  unsigned long long net_bytes_recv; // Total octets reçus
  float net_send_rate;               // Débit d'envoi (octets/sec)
  float net_recv_rate;               // Débit de réception (octets/sec)
} process_info;

typedef struct {
  process_info **process_list_ptr;
  bool *stop_flag_ptr;
  pthread_mutex_t *mutex;
  bool include_remote; /* Include remote processes in the list */
  bool include_local;  /* Include local processes in the list */
} thread_args_t;

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
 * Print process information.
 * @param info Pointer to process_info struct.
 */
void print_process_info(process_info *info);

/**
 * Print all processes information.
 * @param processes Pointer to array of process_info structs.
 * @param count Number of processes in the array.
 */
void print_all_processes(process_info *processes, size_t count);

/* Process handling API (stub) */

/**
 * Fill the struct info with data about the process with the given pid.
 * @param pid Process ID.
 * @param info Pointer to process_info_t struct to fill.
 * @return 0 on success, other on failure (e.g., process not found).
 */
int get_process_info(int pid, process_info *info);

/**
 * Get a list of all PIDs in the system.
 * @param arg Unused parameter.
 * @return Pointer to an array of PIDs (must be freed by caller).
 */
int *get_all_pids(void *arg);

/**
 * Get information about all processes in the system.
 * @return Pointer to an array of process_info structs (must be freed by
 * caller).
 */
void *get_all_processes(void *args);

/* Récupère les processus en cours d'exécution sur le système (/proc).
Alloue un tableau d'informations sur les processus (process_info) qui doit être
libéré par l'appelant avec free(). Renvoie le nombre d'entrées en cas de succès,
ou -1 en cas d'erreur.
 */
int get_processes(process_info **out_list);

/**
 * Get network statistics for a process from /proc/[pid]/net/dev
 * @param pid Process ID
 * @param bytes_sent Pointer to store total bytes sent
 * @param bytes_recv Pointer to store total bytes received
 * @return 0 on success, -1 on failure
 */
int get_process_network_stats(int pid, unsigned long long *bytes_sent,
                              unsigned long long *bytes_recv);

#endif /* PROCESS_H */
