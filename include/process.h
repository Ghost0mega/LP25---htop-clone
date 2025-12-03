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

/* Récupère les processus en cours d'exécution sur le système (/proc). 
Alloue un tableau d'informations sur les processus (process_info) qui doit être libéré par l'appelant avec free(). 
Renvoie le nombre d'entrées en cas de succès, ou -1 en cas d'erreur.
 */
int get_processes(process_info **out_list);

#endif /* PROCESS_H */
