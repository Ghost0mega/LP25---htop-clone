#include "../include/project.h"

int main(void) {
    /* initialize subsystems */
    // config_load(NULL);
    // manager_init();
    // process_init();
    // machine_init();
    // network_init();
    // ui_init();
    
    long ticks = get_clock_ticks_per_second();
    printf("Clock ticks per second: %ld\n", ticks);
    long uptime = get_system_uptime();
    printf("System uptime: %ld seconds\n", uptime);
    int cpus = get_number_of_cpus();
    printf("Number of CPUs: %d\n", cpus);
    int page_size = get_page_size();
    printf("Page size: %d bytes\n", page_size);
    long total_ram = get_total_ram_b();
    printf("Total RAM: %ld bytes\n", total_ram);
    get_process_info(10768, NULL);

    /* shutdown */
    // ui_shutdown();
    // manager_shutdown();

    return 0;
}