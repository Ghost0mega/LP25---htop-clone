#ifndef UI_H
#define UI_H

#include "process.h"
#include <ncurses.h>
#include <pthread.h>

// Main ui loop
void ui_loop(process_info **process_list, pthread_mutex_t *lock);

#endif // UI_H
