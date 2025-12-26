#include "../include/project.h"

/*=========
* METHODS: *
==========*/

bool is_port_free(int port) {
    //Initialization:
    struct sockaddr_in serv_addr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if( sockfd < 0 ) {
        return false;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        if( errno == EADDRINUSE ) {
            fprintf(stderr,"ERROR: The port is not available, already to other process.\n");
        } else {
            fprintf(stderr,"ERROR: Could not bind to process (%d) %s\n", errno, strerror(errno));
        }
    }

    if (close (sockfd) < 0 ) {
        printf("ERROR: did not close fd: %s\n", strerror(errno));
        return false;
    }

    return true;
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

//faudrat surement changer le type de retour pour remote_config
int network_init(parameters_table parameters, int params_count) {
    //Initialization:
    remote_config remote;

    //Case if there is a given config file:
    if (is_param_type(&parameters, params_count, PARAM_REMOTE_CONFIG)) {

    } else {
        //First we add the connection type:
        
        //Address:
        char address[STR_MAX];
        if (is_param_type(&parameters, params_count, PARAM_REMOTE_SERVER)) {
            //Case if there is a given address:
            get_string_parameters(&parameters, params_count, PARAM_REMOTE_SERVER, address);
            strcpy(remote.address, address);
        } else {
            //If there is no given address, ask it to the user:
            fprintf(stdout,"Please enter the DNS or IP of the distant server:\n");
            scanf("%s",&address);
            strcpy(remote.address, address);
        }

        //Port:
        if (is_param_type(&parameters, params_count, PARAM_PORT)) {
            //Case if there is a given port:
            remote.port = get_int_parameters(&parameters, params_count, PARAM_PORT);
        } else {
            //If there is no port given, ask it to the user:
            int port=0;
            do {
                fprintf(stdout,"Please enter a port value between 1024 and 65535:\n");
                scanf("%d",&port);
            } while (is_port_free(port) && ((port < 1024) || (port > 65535)));
            remote.port = port;
        }

        //Username:
        char username[STR_MAX];
        if (is_param_type(&parameters, params_count, PARAM_USERNAME)) {
            //Case if there is a given address:
            get_string_parameters(&parameters, params_count, PARAM_USERNAME, username);
            strcpy(remote.address, username);
        } else {
            //If there is no given address, ask it to the user:
            fprintf(stdout,"Please enter the username of type user@server1.edu or user2@192.168.1.1:\n");
            scanf("%s",&username);
            strcpy(remote.address, username);
        }

        //Password:
        char password[STR_MAX];
        if (is_param_type(&parameters, params_count, PARAM_PASSWORD)) {
            //Case if there is a given address:
            get_string_parameters(&parameters, params_count, PARAM_PASSWORD, password);
            strcpy(remote.password, password);
        } else {
            //If there is no given address, ask it to the user:
            fprintf(stdout,"Please enter the password for the connection.\n");
            scanf("%s",&password);
            strcpy(remote.password, password);
        }
    }
    return EXIT_SUCCESS;
}

void network_poll(void) {
    /* stub: poll network stats */
}
