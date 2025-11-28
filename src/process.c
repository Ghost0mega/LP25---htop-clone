#include "../include/project.h"


void* task_every_seconds(void* fp) {
    for (int i = 0; i < 5; i++) {
        char buffer[128];
        FILE *fp;

        // Exécute la commande
        fp = popen("ps -ef | awk '{print $2, $7}'", "r");

        if (fp == NULL) {
            perror("Erreur popen");
            exit(1);
        }

        fgets(buffer, sizeof(buffer), fp);

        // Lit la sortie
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);
        }

        // Ferme le processus
        pclose(fp);

        sleep(2);
    }
    return NULL;
}

void* task_once(void* arg) {
    printf("Action exécutée une seule fois !\n");
    return NULL;
}
