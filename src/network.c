#include "../include/project.h"

/*=========
* METHODS: *
==========*/

bool is_port_free(int port) {
    //à remplir avec les besoin reseaux, quand on sera là
    return false;
}

bool is_config_file_valid(char path[STR_MAX]) {
    //Initialization:
    FILE *fptr;
    fptr = fopen(path, 'r');

    //First we test if we can access to the file:
    if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot access the configuration file.\n");
        return false;
    }

    /*
    FILL HERE
    */

    //Once all is done we close the file:
    fclose(fptr);

    //If nothing goes wrong we send true:
    return true;
}

int network_init(void) {
    printf("network: init\n");
    return 0;
}

void network_poll(void) {
    /* stub: poll network stats */
}
