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

    printf("Fetching info for process with PID 10768...\n");
    process_info info;
    if (get_process_info(10768, &info) == 0) {
        printf("Process 10768: %s, State: %c\nUptime %lds\nCpu time: %lu, Cpu usage: %.2lf\nMemory usage: %.2lf\n", info.name, (char)info.state, uptime, info.cpu_utime + info.cpu_stime, info.mem_usage);
    } else {
        printf("Failed to get process info for PID 10768\n");
    }

    /* shutdown */
    // ui_shutdown();
    // manager_shutdown();

    return 0;
}