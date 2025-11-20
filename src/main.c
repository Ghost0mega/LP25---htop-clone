#include "../include/project.h"

int main(void) {
    /* initialize subsystems */
    config_load(NULL);
    manager_init();
    process_init();
    machine_init();
    network_init();
    ui_init();

    printf("Hello, World! (from project layout)\n");

    /* shutdown */
    ui_shutdown();
    manager_shutdown();

    return 0;
}