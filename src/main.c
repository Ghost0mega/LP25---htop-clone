#include "../include/project.h"

int main(void) {

    pthread_t th1, th2;


    pthread_create(&th1, NULL, task_every_seconds, NULL);
    pthread_create(&th2, NULL, task_once, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    
    return 0;

}