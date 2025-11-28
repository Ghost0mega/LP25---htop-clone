#ifndef PROCESS_H
#define PROCESS_H

typedef struct process_info {
    int pid;
    char name[256];
    char user[64];
    double cpu_usage;
    double mem_usage;
    long uptime;
} process_info_t;

/* Process handling API (stub) */

void* task_every_seconds(void* fp);

void* task_once(void* arg);

#endif /* PROCESS_H */
