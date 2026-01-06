#ifndef MACHINE_H
#define MACHINE_H
#include "network.h"
#include "process.h"

#include "network.h"

typedef struct machine_info {
  char name[64];
  int process_count;
  process_info *process_list; // malloc
  remote_config *remote;      // NULL si machine locale
} machine_info_t;

/* Machine/system info API (stub) */

int machine_init(void);
void machine_poll(void);

#endif /* MACHINE_H */
