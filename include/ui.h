#ifndef UI_H
#define UI_H

#include <ncurses.h>

/**
 *  Generate fake processus
 */
int nombre_processus(int pids[]);

/**
 * Main ui loop
 */
void ui_loop();

#endif /* UI_H */
