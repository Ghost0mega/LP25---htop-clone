#include "../include/project.h"
#include <dirent.h>
#include <ctype.h>
#include <string.h>


int process_init(void) {
    printf("process: init\n");
    return 0;
}

void process_poll(void) {
    /* stub: would poll process state here */
}

int get_processes(process_info **out_list) {
    DIR *d = opendir("/proc");
    if (!d) return -1;

    process_info *arr = NULL;
    int cap = 0, n = 0;
    struct dirent *entry;

    while ((entry = readdir(d)) != NULL) {
        if (!isdigit((unsigned char)entry->d_name[0]))
            continue;

        int pid = atoi(entry->d_name);

        char name_buf[256] = "";
        char path[256];

        /* try /proc/<pid>/comm first */
        snprintf(path, sizeof(path), "/proc/%d/comm", pid);
        FILE *f = fopen(path, "r");
        if (f) {
            if (fgets(name_buf, sizeof(name_buf), f)) {
                name_buf[strcspn(name_buf, "\n")] = '\0';
            }
            fclose(f);
        } else {
            /* fallback to cmdline */
            snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
            f = fopen(path, "r");
            if (f) {
                if (fgets(name_buf, sizeof(name_buf), f)) {
                    /* cmdline is NUL separated; keep as is */
                }
                fclose(f);
            }
        }

        if (n >= cap) {
            int newcap = cap ? cap * 2 : 128;
            process_info *tmp = realloc(arr, newcap * sizeof(process_info));
            if (!tmp) {
                free(arr);
                closedir(d);
                return -1;
            }
            arr = tmp;
            cap = newcap;
        }

    arr[n].pid = pid;
    snprintf(arr[n].name, sizeof(arr[n].name), "%s", name_buf);
        arr[n].user[0] = '\0';
        arr[n].cpu_usage = 0.0;
        arr[n].mem_usage = 0.0;
        arr[n].uptime = 0;
        n++;
    }

    closedir(d);
    *out_list = arr;
    return n;
}

