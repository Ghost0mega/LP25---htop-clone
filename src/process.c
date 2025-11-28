#include "../include/project.h"


void* task_every_seconds(void* fp) {
    while (1) {
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

int main() {
    pthread_t th1, th2;


    pthread_create(&th1, NULL, task_every_seconds, NULL);
    pthread_create(&th2, NULL, task_once, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    
    return 0;
}