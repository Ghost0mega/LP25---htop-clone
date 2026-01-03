#include "../include/project.h"

/*=========
* METHODS: *
==========*/

/**
 * Parse login in format user@server and extract username and server address.
 * @param login Login string in format user@server.
 * @param username Buffer to store extracted username.
 * @param server Buffer to store extracted server address.
 * @return 0 on success, -1 on failure.
 */
int parse_login(const char *login, char *username, char *server) {
    /* Vérifie les pointeurs d'entrée */
    if (!login || !username || !server) return -1;

    // Find the @ character
    char *at_sign = strchr(login, '@');
    if (at_sign == NULL) {
        return -1;
    }

    // Extract username (before @)
    size_t username_len = at_sign - login;
    /* Vérifie la longueur du nom d'utilisateur */
    if (username_len == 0 || username_len >= STR_MAX) {
        return -1;
    }
    strncpy(username, login, username_len);
    username[username_len] = '\0';

    // Extract server (after @)
    size_t server_len = strlen(at_sign + 1);
    /* Vérifie la longueur du serveur */
    if (server_len == 0 || server_len >= STR_MAX) {
        return -1;
    }
    strcpy(server, at_sign + 1);

    return 0;
}

bool is_port_free(int port) {
    struct sockaddr_in serv_addr;

    /* Crée un socket temporaire pour tenter le bind sur la machine locale */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return false;
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    /* Essai de bind : si échec, le port est occupé ou inaccessible */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        if (errno == EADDRINUSE) {
            fprintf(stderr, "ERROR: The port is not available, already used by another process.\n");
        } else {
            fprintf(stderr, "ERROR: Could not bind to port (%d) %s\n", errno, strerror(errno));
        }
        close(sockfd);
        return false;
    }

    if (close(sockfd) < 0) {
        fprintf(stderr, "ERROR: did not close fd: %s\n", strerror(errno));
        return false;
    }

    return true;
}

bool is_config_file_valid(char path[STR_MAX]) {
    //Initialization:
    FILE *fptr;
    fptr = fopen(path, "r");

    //First we test if we can access to the file:
    if (fptr == NULL) {
        fprintf(stderr,"ERROR: Cannot access the configuration file.\n");
        return false;
    }

    //Check file permissions (must be 600 = rw-------)
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        fprintf(stderr, "ERROR: Cannot stat the configuration file.\n");
        fclose(fptr);
        return false;
    }
    if ((file_stat.st_mode & 0777) != 0600) {
        fprintf(stderr, "ERROR: Configuration file must have permissions 600 (rw-------)\n");
        fclose(fptr);
        return false;
    }

    // Ensure file is hidden (basename starts with '.')
    const char *slash = strrchr(path, '/');
    const char *basename = slash ? slash + 1 : path;
    if (basename[0] != '.') {
        fprintf(stderr, "ERROR: Configuration file must be hidden (start with a dot).\n");
        fclose(fptr);
        return false;
    }

    /* Valide le contenu ligne par ligne :
     * chaque ligne doit être au format :
     * nom_serveur:adresse_serveur:port:username:password:type_connexion
     */
    char line[STR_MAX];
    int line_count = 0;
    while (fgets(line, sizeof(line), fptr)) {
        line_count++;
        //Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        //Skip empty lines
        if (strlen(line) == 0) continue;

        //Count colons (should have exactly 5)
        int colon_count = 0;
        for (char *p = line; *p; p++) {
            if (*p == ':') colon_count++;
        }
        if (colon_count != 5) {
            fprintf(stderr, "ERROR: Line %d in config file has invalid format. Expected 6 fields separated by colons.\n", line_count);
            fclose(fptr);
            return false;
        }

        //Parse and validate each field
        char line_copy[STR_MAX];
        strncpy(line_copy, line, STR_MAX - 1);
        line_copy[STR_MAX - 1] = '\0';

        char *fields[6];
        char *token = strtok(line_copy, ":");
        int field_count = 0;
        while (token && field_count < 6) {
            fields[field_count++] = token;
            token = strtok(NULL, ":");
        }

        //Validate fields
        // Field 0: nom_serveur (must not be empty)
        if (strlen(fields[0]) == 0) {
            fprintf(stderr, "ERROR: Line %d: server name cannot be empty.\n", line_count);
            fclose(fptr);
            return false;
        }
        // Field 1: adresse_serveur (must not be empty)
        if (strlen(fields[1]) == 0) {
            fprintf(stderr, "ERROR: Line %d: server address cannot be empty.\n", line_count);
            fclose(fptr);
            return false;
        }
        // Field 2: port (must be valid number between 1024 and 65535)
        int port = atoi(fields[2]);
        if (port < 1024 || port > 65535) {
            fprintf(stderr, "ERROR: Line %d: port must be between 1024 and 65535.\n", line_count);
            fclose(fptr);
            return false;
        }
        // Field 3: username (must not be empty)
        if (strlen(fields[3]) == 0) {
            fprintf(stderr, "ERROR: Line %d: username cannot be empty.\n", line_count);
            fclose(fptr);
            return false;
        }
        // Field 4: password (must not be empty)
        if (strlen(fields[4]) == 0) {
            fprintf(stderr, "ERROR: Line %d: password cannot be empty.\n", line_count);
            fclose(fptr);
            return false;
        }
        // Field 5: type_connexion (must be ssh or telnet)
        strupper(fields[5]);
        if (strcmp(fields[5], "SSH") != 0 && strcmp(fields[5], "TELNET") != 0) {
            fprintf(stderr, "ERROR: Line %d: connection type must be ssh or telnet.\n", line_count);
            fclose(fptr);
            return false;
        }
    }

    //Once all is done we close the file:
    fclose(fptr);

    //If nothing goes wrong we send true:
    return true;
}

//Initialize network connection with the given parameters
int network_init(parameters_table *parameters, int params_count) {
    //Initialization:
    remote_config remote;
    memset(&remote, 0, sizeof(remote));

    //Case if there is a given config file or a default .config in cwd:
    char config_path[STR_MAX] = {0};
    if (is_param_type(parameters, params_count, PARAM_REMOTE_CONFIG)) {
        get_string_parameters(parameters, params_count, PARAM_REMOTE_CONFIG, config_path);
    } else {
        // If no config param, look for default hidden file ".config" in cwd
        if (access(".config", R_OK) == 0) {
            strncpy(config_path, ".config", STR_MAX-1);
            config_path[STR_MAX-1] = '\0';
        }
    }

    if (strlen(config_path) > 0) {
        if (!is_config_file_valid(config_path)) {
            fprintf(stderr, "ERROR: Invalid configuration file: %s\n", config_path);
            return -1;
        }
        // Load and parse entries (implementation in config_load)
        if (config_load(config_path) != 0) {
            fprintf(stderr, "ERROR: Failed to load configuration file: %s\n", config_path);
            return -1;
        }
        // For now, we stop here since config may provide multiple remotes to handle.
        return 0;
    }

    //Case if there is a login option (user@server):
    if (is_param_type(parameters, params_count, PARAM_LOGIN)) {
        char login[STR_MAX];
        char username[STR_MAX];
        char server[STR_MAX];
        
        get_string_parameters(parameters, params_count, PARAM_LOGIN, login);
        if (parse_login(login, username, server) != 0) {
            fprintf(stderr, "ERROR: Failed to parse login.\n");
            return -1;
        }
        strcpy(remote.username, username);
        strcpy(remote.address, server);
    }
    //Case if there is a remote server option:
    else if (is_param_type(parameters, params_count, PARAM_REMOTE_SERVER)) {
        char server[STR_MAX];
        get_string_parameters(parameters, params_count, PARAM_REMOTE_SERVER, server);
        strcpy(remote.address, server);

        //Get username:
        if (is_param_type(parameters, params_count, PARAM_USERNAME)) {
            char username[STR_MAX];
            get_string_parameters(parameters, params_count, PARAM_USERNAME, username);
            strcpy(remote.username, username);
        } else {
            //Ask for username interactively:
            char username[STR_MAX];
            fprintf(stdout, "Please enter the username:\n");
            if (fgets(username, sizeof(username), stdin) == NULL) {
                fprintf(stderr, "ERROR: Failed to read username.\n");
                return -1;
            }
            // Trim newline
            size_t ln = strlen(username);
            if (ln > 0 && username[ln-1] == '\n') username[ln-1] = '\0';
            strcpy(remote.username, username);
        }
    } else {
        //Local mode - no remote configuration needed
        return 0;
    }

    //Get connection type first (so we can set default port)
    if (is_param_type(parameters, params_count, PARAM_CONNEXION_TYPE)) {
        char type_str[STR_MAX];
        get_string_parameters(parameters, params_count, PARAM_CONNEXION_TYPE, type_str);
        strupper(type_str);
        if (strcmp(type_str, "SSH") == 0) {
            remote.type = CONN_SSH;
        } else if (strcmp(type_str, "TELNET") == 0) {
            remote.type = CONN_TELNET;
        } else {
            fprintf(stderr, "ERROR: Invalid connection type. Use ssh or telnet.\n");
            return -1;
        }
    } else {
        //Default to SSH
        remote.type = CONN_SSH;
    }

    //Get port (use provided value or default for the connection type)
    if (is_param_type(parameters, params_count, PARAM_PORT)) {
        remote.port = get_int_parameters(parameters, params_count, PARAM_PORT);
    } else {
        remote.port = (remote.type == CONN_SSH) ? 22 : 23;
    }

    //Get password:
    if (is_param_type(parameters, params_count, PARAM_PASSWORD)) {
        char password[STR_MAX];
        get_string_parameters(parameters, params_count, PARAM_PASSWORD, password);
        strcpy(remote.password, password);
    } else {
        //Ask for password interactively (no-echo)
        char *pw = getpass("Please enter the password for the connection: ");
        if (!pw) {
            fprintf(stderr, "ERROR: Failed to read password.\n");
            return -1;
        }
        strncpy(remote.password, pw, sizeof(remote.password)-1);
        remote.password[sizeof(remote.password)-1] = '\0';
    }

    //TODO: Store the remote config and establish connection
    printf("Remote config: user=%s, server=%s, port=%d\n", remote.username, remote.address, remote.port);

    return 0;
}

/**
 * Execute a remote command via SSH and parse the result as process list.
 * Uses system() to execute ssh command and captures output.
 */
int network_get_processes_ssh(remote_config *config, process_info **out_processes) {
    if (!config || !out_processes) return -1;

    fprintf(stdout, "INFO: Connecting to %s (%s) via SSH...\n", config->name, config->address);

    /* Create temporary file for output */
    char tmpfile[256];
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/htop_ssh_%d.txt", getpid());

    /* Build SSH command to get process list */
    char ssh_cmd[STR_MAX];
    snprintf(ssh_cmd, sizeof(ssh_cmd),
             "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -p %d %s@%s "
             "'ps aux' > %s 2>&1",
             config->password, config->port, config->username, config->address, tmpfile);

    int ret = system(ssh_cmd);
    if (ret != 0) {
        fprintf(stderr, "ERROR: SSH connection failed to %s\n", config->address);
        unlink(tmpfile);
        return -1;
    }

    /* Parse the ps aux output */
    FILE *fp = fopen(tmpfile, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot read SSH output\n");
        unlink(tmpfile);
        return -1;
    }

    /* Count non-header lines */
    char line[STR_MAX];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        count++;
    }
    count--; /* Remove header line */
    
    if (count <= 0) {
        fprintf(stderr, "WARNING: No processes found on %s\n", config->name);
        fclose(fp);
        unlink(tmpfile);
        *out_processes = NULL;
        return 0;
    }

    /* Allocate process array */
    *out_processes = calloc(count, sizeof(process_info));
    if (!*out_processes) {
        fprintf(stderr, "ERROR: Cannot allocate process array\n");
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    /* Parse ps aux output (simplified) */
    rewind(fp);
    if (fgets(line, sizeof(line), fp) == NULL) {
        /* Handle error or empty file */
        fclose(fp);
        unlink(tmpfile);
        return 0;
    }
    int idx = 0;
    
    while (fgets(line, sizeof(line), fp) && idx < count) {
        /* Simple parsing: USER PID %CPU %MEM VSZ RSS TTY STAT START TIME COMMAND */
        int pid;
        char user[64], cmd[256];
        float cpu, mem;
        
        if (sscanf(line, "%s %d %f %f %*s %*s %*s %*s %*s %*s %s",
                   user, &pid, &cpu, &mem, cmd) == 5) {
            (*out_processes)[idx].pid = pid;
            strncpy((*out_processes)[idx].name, cmd, sizeof((*out_processes)[idx].name)-1);
            (*out_processes)[idx].name[sizeof((*out_processes)[idx].name)-1] = '\0';
            (*out_processes)[idx].cpu_usage = cpu;
            (*out_processes)[idx].mem_usage = (unsigned long)(mem * 1024 * 1024); /* Convert % to bytes approx */
            (*out_processes)[idx].state = PROCESS_STATE_RUNNING;
            idx++;
        }
    }

    fclose(fp);
    unlink(tmpfile);
    
    fprintf(stdout, "INFO: Retrieved %d processes from %s\n", idx, config->name);
    return idx;
}

/**
 * Execute a remote command via Telnet and parse the result as process list.
 * Uses expect or nc for telnet connection (simplified implementation).
 */
int network_get_processes_telnet(remote_config *config, process_info **out_processes) {
    if (!config || !out_processes) return -1;

    fprintf(stdout, "INFO: Connecting to %s (%s) via Telnet...\n", config->name, config->address);

    /* Telnet implementation would be similar to SSH but using telnet protocol
     * For now, provide a simplified version using nc (netcat) */
    char tmpfile[256];
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/htop_telnet_%d.txt", getpid());

    /* Build command - this is simplified and would need expect or similar for real telnet */
    char telnet_cmd[STR_MAX];
    snprintf(telnet_cmd, sizeof(telnet_cmd),
             "(echo %s; echo %s; echo 'ps aux'; sleep 1) | "
             "nc -w 2 %s %d > %s 2>&1",
             config->username, config->password, config->address, config->port, tmpfile);

    int ret = system(telnet_cmd);
    if (ret != 0) {
        fprintf(stderr, "ERROR: Telnet connection failed to %s\n", config->address);
        unlink(tmpfile);
        return -1;
    }

    /* Parse output (similar to SSH) */
    FILE *fp = fopen(tmpfile, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot read Telnet output\n");
        unlink(tmpfile);
        return -1;
    }

    char line[STR_MAX];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "PID") == NULL) count++;
    }

    if (count <= 0) {
        fprintf(stderr, "WARNING: No processes found on %s\n", config->name);
        fclose(fp);
        unlink(tmpfile);
        *out_processes = NULL;
        return 0;
    }

    *out_processes = calloc(count, sizeof(process_info));
    if (!*out_processes) {
        fprintf(stderr, "ERROR: Cannot allocate process array\n");
        fclose(fp);
        unlink(tmpfile);
        return -1;
    }

    rewind(fp);
    int idx = 0;
    while (fgets(line, sizeof(line), fp) && idx < count) {
        int pid;
        char cmd[256];
        float cpu, mem;
        
        if (sscanf(line, "%*s %d %f %f %*s %*s %*s %*s %*s %*s %s",
                   &pid, &cpu, &mem, cmd) == 4) {
            (*out_processes)[idx].pid = pid;
            strncpy((*out_processes)[idx].name, cmd, sizeof((*out_processes)[idx].name)-1);
            (*out_processes)[idx].name[sizeof((*out_processes)[idx].name)-1] = '\0';
            (*out_processes)[idx].cpu_usage = cpu;
            (*out_processes)[idx].mem_usage = (unsigned long)(mem * 1024 * 1024);
            (*out_processes)[idx].state = PROCESS_STATE_RUNNING;
            idx++;
        }
    }

    fclose(fp);
    unlink(tmpfile);
    
    fprintf(stdout, "INFO: Retrieved %d processes from %s\n", idx, config->name);
    return idx;
}

/**
 * Poll all remote processes and merge with local processes.
 */
int network_poll_all_processes(process_info **all_processes, int local_count) {
    if (!all_processes || local_count < 0 || g_remote_configs_count <= 0) {
        return local_count;
    }

    int total_count = local_count;

    /* Process each remote configuration */
    for (int i = 0; i < g_remote_configs_count; i++) {
        remote_config *cfg = &g_remote_configs[i];
        process_info *remote_procs = NULL;
        int remote_count = 0;

        /* Get processes based on connection type */
        if (cfg->type == CONN_SSH) {
            remote_count = network_get_processes_ssh(cfg, &remote_procs);
        } else if (cfg->type == CONN_TELNET) {
            remote_count = network_get_processes_telnet(cfg, &remote_procs);
        }

        if (remote_count > 0 && remote_procs) {
            /* Reallocate main array to fit new processes */
            process_info *temp = realloc(*all_processes, 
                                        (total_count + remote_count) * sizeof(process_info));
            if (!temp) {
                fprintf(stderr, "ERROR: Failed to reallocate process array\n");
                free(remote_procs);
                return total_count;
            }
            *all_processes = temp;

            /* Copy remote processes and add server name info */
            for (int j = 0; j < remote_count; j++) {
                (*all_processes)[total_count + j] = remote_procs[j];
                /* Append server name to process name for identification */
                char new_name[512];
                snprintf(new_name, sizeof(new_name), "[%s] %s", 
                        cfg->name, remote_procs[j].name);
                strncpy((*all_processes)[total_count + j].name, new_name, 
                       sizeof((*all_processes)[total_count + j].name)-1);
                (*all_processes)[total_count + j].name[
                    sizeof((*all_processes)[total_count + j].name)-1] = '\0';
            }

            total_count += remote_count;
            free(remote_procs);
        }
    }

    return total_count;
}

void network_poll(void) {
    /* stub: poll network stats */
}
