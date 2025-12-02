#ifndef PROCESS_H
#define PROCESS_H

typedef struct process_info {
    int pid;
    char name[256];
    char user[64];
    double cpu_usage;
    double mem_usage;
    long uptime;
} process_info;

/* Process handling API (stub) */

int process_init(void);
void process_poll(void);

#endif /* PROCESS_H */
