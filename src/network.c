#include "../include/project.h"

/*=========
* METHODS: *
==========*/

/**
 * Escape special shell characters in a string for use in sshpass
 * @param src Source string
 * @param dst Destination buffer (should be at least 2x size of src)
 * @param dst_size Size of destination buffer
 * @return 0 on success, -1 on failure
 */
static int escape_shell_string(const char *src, char *dst, size_t dst_size) {
  if (!src || !dst)
    return -1;

  size_t src_len = strlen(src);
  size_t dst_pos = 0;

  for (size_t i = 0; i < src_len; i++) {
    char c = src[i];

    // Check if we have enough space (worst case: character needs escaping)
    if (dst_pos + 2 >= dst_size) {
      return -1; // Buffer overflow prevention
    }

    // Escape special characters for shell
    switch (c) {
    case '\'':
      // Replace ' with '\''
      dst[dst_pos++] = '\'';
      dst[dst_pos++] = '\\';
      dst[dst_pos++] = '\'';
      dst[dst_pos++] = '\'';
      break;
    case '"':
    case '\\':
    case '$':
    case '`':
      // Escape these with backslash
      if (dst_pos + 2 >= dst_size)
        return -1;
      dst[dst_pos++] = '\\';
      dst[dst_pos++] = c;
      break;
    default:
      dst[dst_pos++] = c;
      break;
    }
  }

  dst[dst_pos] = '\0';
  return 0;
}

/**
 * Parse login in format user@server and extract username and server address.
 * @param login Login string in format user@server.
 * @param username Buffer to store extracted username.
 * @param server Buffer to store extracted server address.
 * @return 0 on success, -1 on failure.
 */
int parse_login(const char *login, char *username, char *server) {
  /* Vérifie les pointeurs d'entrée */
  if (!login || !username || !server)
    return -1;

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
      fprintf(stderr, "ERROR: The port is not available, already used by "
                      "another process.\n");
    } else {
      fprintf(stderr, "ERROR: Could not bind to port (%d) %s\n", errno,
              strerror(errno));
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
  FILE *fptr;
  fptr = fopen(path, "r");

  if (fptr == NULL) {
    fprintf(stderr, "ERROR: Cannot access the configuration file.\n");
    return false;
  }

  struct stat file_stat;
  if (stat(path, &file_stat) != 0) {
    fprintf(stderr, "ERROR: Cannot stat the configuration file.\n");
    fclose(fptr);
    return false;
  }
  if ((file_stat.st_mode & 0777) != 0600) {
    fprintf(
        stderr,
        "ERROR: Configuration file must have permissions 600 (rw-------)\n");
    fclose(fptr);
    return false;
  }

  const char *slash = strrchr(path, '/');
  const char *basename = slash ? slash + 1 : path;
  if (basename[0] != '.') {
    fprintf(stderr,
            "ERROR: Configuration file must be hidden (start with a dot).\n");
    fclose(fptr);
    return false;
  }

  char line[STR_MAX];
  int line_count = 0;
  while (fgets(line, sizeof(line), fptr)) {
    line_count++;
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }
    if (strlen(line) == 0)
      continue;

    // Count colons (should have exactly 4 now for 5 fields)
    int colon_count = 0;
    for (char *p = line; *p; p++) {
      if (*p == ':')
        colon_count++;
    }
    if (colon_count != 5) {
      fprintf(stderr,
              "ERROR: Line %d in config file has invalid format. Expected 6 "
              "fields (name:address:port:username:password:connexiontype).\n",
              line_count);
      fclose(fptr);
      return false;
    }

    char line_copy[STR_MAX];
    strncpy(line_copy, line, STR_MAX - 1);
    line_copy[STR_MAX - 1] = '\0';

    char *fields[5];
    char *token = strtok(line_copy, ":");
    int field_count = 0;
    while (token && field_count < 5) {
      fields[field_count++] = token;
      token = strtok(NULL, ":");
    }

    // Field 0: name
    if (strlen(fields[0]) == 0) {
      fprintf(stderr, "ERROR: Line %d: server name cannot be empty.\n",
              line_count);
      fclose(fptr);
      return false;
    }
    // Field 1: address
    if (strlen(fields[1]) == 0) {
      fprintf(stderr, "ERROR: Line %d: server address cannot be empty.\n",
              line_count);
      fclose(fptr);
      return false;
    }
    // Field 2: port
    int port = atoi(fields[2]);
    if (port < 1 || port > 65535) {
      fprintf(stderr, "ERROR: Line %d: port must be between 1 and 65535.\n",
              line_count);
      fclose(fptr);
      return false;
    }
    // Field 3: username
    if (strlen(fields[3]) == 0) {
      fprintf(stderr, "ERROR: Line %d: username cannot be empty.\n",
              line_count);
      fclose(fptr);
      return false;
    }
    // Field 4: password
    if (strlen(fields[4]) == 0) {
      fprintf(stderr, "ERROR: Line %d: password cannot be empty.\n",
              line_count);
      fclose(fptr);
      return false;
    }
  }

  fclose(fptr);
  return true;
}

// Initialize network connection with the given parameters
int network_init(parameters_table *parameters, int params_count) {
  remote_config remote;
  memset(&remote, 0, sizeof(remote));

  char config_path[STR_MAX] = {0};
  if (is_param_type(parameters, params_count, PARAM_REMOTE_CONFIG)) {
    get_string_parameters(parameters, params_count, PARAM_REMOTE_CONFIG,
                          config_path);
  } else if (!is_param_type(parameters, params_count, PARAM_LOGIN) &&
             !is_param_type(parameters, params_count, PARAM_REMOTE_SERVER)) {
    if (access(".config", R_OK) == 0) {
      strncpy(config_path, ".config", STR_MAX - 1);
      config_path[STR_MAX - 1] = '\0';
    }
  }

  if (strlen(config_path) > 0) {
    if (!is_config_file_valid(config_path)) {
      fprintf(stderr, "ERROR: Invalid configuration file: %s\n", config_path);
      return -1;
    }
    if (config_load(config_path) != 0) {
      fprintf(stderr, "ERROR: Failed to load configuration file: %s\n",
              config_path);
      return -1;
    }
    return 0;
  }

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
  } else if (is_param_type(parameters, params_count, PARAM_REMOTE_SERVER)) {
    char server[STR_MAX];
    get_string_parameters(parameters, params_count, PARAM_REMOTE_SERVER,
                          server);
    strcpy(remote.address, server);

    if (is_param_type(parameters, params_count, PARAM_USERNAME)) {
      char username[STR_MAX];
      get_string_parameters(parameters, params_count, PARAM_USERNAME, username);
      strcpy(remote.username, username);
    } else {
      char username[STR_MAX];
      fprintf(stdout, "Please enter the username:\n");
      if (fgets(username, sizeof(username), stdin) == NULL) {
        fprintf(stderr, "ERROR: Failed to read username.\n");
        return -1;
      }
      size_t ln = strlen(username);
      if (ln > 0 && username[ln - 1] == '\n')
        username[ln - 1] = '\0';
      strcpy(remote.username, username);
    }
  } else {
    return 0;
  }

  // Get port (use provided value or default SSH port 22)
  if (is_param_type(parameters, params_count, PARAM_PORT)) {
    remote.port = get_int_parameters(parameters, params_count, PARAM_PORT);
  } else {
    remote.port = 22;
  }

  // Get password
  if (is_param_type(parameters, params_count, PARAM_PASSWORD)) {
    char password[STR_MAX];
    get_string_parameters(parameters, params_count, PARAM_PASSWORD, password);
    strcpy(remote.password, password);
  } else {
    char *pw = getpass("Please enter the password for the SSH connection: ");
    if (!pw) {
      fprintf(stderr, "ERROR: Failed to read password.\n");
      return -1;
    }
    strncpy(remote.password, pw, sizeof(remote.password) - 1);
    remote.password[sizeof(remote.password) - 1] = '\0';
  }

  if (g_remote_configs) {
    free(g_remote_configs);
    g_remote_configs = NULL;
    g_remote_configs_count = 0;
  }

  g_remote_configs = calloc(1, sizeof(remote_config));
  if (!g_remote_configs) {
    fprintf(stderr, "ERROR: Cannot allocate remote config\n");
    return -1;
  }

  strncpy(g_remote_configs[0].name, remote.address,
          sizeof(g_remote_configs[0].name) - 1);
  g_remote_configs[0].name[sizeof(g_remote_configs[0].name) - 1] = '\0';
  strncpy(g_remote_configs[0].address, remote.address,
          sizeof(g_remote_configs[0].address) - 1);
  g_remote_configs[0].address[sizeof(g_remote_configs[0].address) - 1] = '\0';
  g_remote_configs[0].port = remote.port;
  strncpy(g_remote_configs[0].username, remote.username,
          sizeof(g_remote_configs[0].username) - 1);
  g_remote_configs[0].username[sizeof(g_remote_configs[0].username) - 1] = '\0';
  strncpy(g_remote_configs[0].password, remote.password,
          sizeof(g_remote_configs[0].password) - 1);
  g_remote_configs[0].password[sizeof(g_remote_configs[0].password) - 1] = '\0';
  g_remote_configs_count = 1;

  return 0;
}

/**
 * Execute a remote command via SSH and parse the result as process list.
 * Uses system() to execute ssh command and captures output.
 */
int network_get_processes_ssh(remote_config *config,
                              process_info **out_processes) {
  if (!config || !out_processes)
    return -1;

  /* Create temporary file for output */
  char tmpfile[256];
  snprintf(tmpfile, sizeof(tmpfile), "/tmp/htop_ssh_%d.txt", getpid());

  /* Escape the password for safe shell usage */
  char escaped_password[STR_MAX * 2];
  if (escape_shell_string(config->password, escaped_password,
                          sizeof(escaped_password)) != 0) {
    fprintf(stderr, "ERROR: Password contains invalid characters\n");
    return -1;
  }

  /* Build SSH command to get process list (avoid truncation by sizing
   * dynamically) */
  int ssh_cmd_len =
      snprintf(NULL, 0,
               "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -p %d %s@%s "
               "'ps aux' > %s 2>&1",
               escaped_password, config->port, config->username,
               config->address, tmpfile);
  if (ssh_cmd_len < 0) {
    fprintf(stderr, "ERROR: Failed to build SSH command\n");
    return -1;
  }
  char *ssh_cmd = calloc((size_t)ssh_cmd_len + 1, sizeof(char));
  if (!ssh_cmd) {
    fprintf(stderr, "ERROR: Cannot allocate SSH command buffer\n");
    return -1;
  }
  snprintf(ssh_cmd, (size_t)ssh_cmd_len + 1,
           "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -p %d %s@%s "
           "'ps aux' > %s 2>&1",
           escaped_password, config->port, config->username, config->address,
           tmpfile);

  int ret = system(ssh_cmd);
  if (ret != 0) {
    fprintf(stderr, "ERROR: SSH connection failed to %s\n", config->address);
    unlink(tmpfile);
    free(ssh_cmd);
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
  int config_index = -1;
  for (int ci = 0; ci < g_remote_configs_count; ci++) {
    if (&g_remote_configs[ci] == config) {
      config_index = ci;
      break;
    }
  }

  while (fgets(line, sizeof(line), fp) && idx < count) {
    int pid;
    char user[64], stat_str[16], cmd[256];
    float cpu, mem;

    if (sscanf(line, "%63s %d %f %f %*s %*s %*s %15s %*s %*s %255[^\n]", user,
               &pid, &cpu, &mem, stat_str, cmd) == 6) {
      (*out_processes)[idx].pid = pid;

      char *trimmed = cmd;
      while (*trimmed && isspace((unsigned char)*trimmed)) {
        trimmed++;
      }

      snprintf((*out_processes)[idx].name, sizeof((*out_processes)[idx].name),
               "%s", trimmed);
      (*out_processes)[idx].cpu_usage = cpu;
      (*out_processes)[idx].mem_usage = (unsigned long)(mem * 1024 * 1024);

      switch (stat_str[0]) {
      case 'R':
        (*out_processes)[idx].state = PROCESS_STATE_RUNNING;
        break;
      case 'S':
        (*out_processes)[idx].state = PROCESS_STATE_SLEEPING;
        break;
      case 'D':
        (*out_processes)[idx].state = PROCESS_STATE_DISK_SLEEP;
        break;
      case 'Z':
        (*out_processes)[idx].state = PROCESS_STATE_ZOMBIE;
        break;
      case 'T':
        (*out_processes)[idx].state = PROCESS_STATE_STOPPED;
        break;
      case 't':
        (*out_processes)[idx].state = PROCESS_STATE_TRACING_STOP;
        break;
      case 'X':
        (*out_processes)[idx].state = PROCESS_STATE_DEAD;
        break;
      case 'K':
        (*out_processes)[idx].state = PROCESS_STATE_WAKEKILL;
        break;
      case 'W':
        (*out_processes)[idx].state = PROCESS_STATE_WAKING;
        break;
      case 'P':
        (*out_processes)[idx].state = PROCESS_STATE_PARKED;
        break;
      case 'I':
        (*out_processes)[idx].state = PROCESS_STATE_IDLE;
        break;
      default:
        (*out_processes)[idx].state = PROCESS_STATE_UNKNOWN;
        break;
      }

      (*out_processes)[idx].uptime = 0;
      (*out_processes)[idx].remote_config_index = config_index; // NOUVEAU

      idx++;
    }
  }

  fclose(fp);
  unlink(tmpfile);
  free(ssh_cmd);
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

  for (int i = 0; i < g_remote_configs_count; i++) {
    remote_config *cfg = &g_remote_configs[i];
    process_info *remote_procs = NULL;
    int remote_count = 0;

    // Only SSH is supported now
    remote_count = network_get_processes_ssh(cfg, &remote_procs);

    if (remote_count > 0 && remote_procs) {
      process_info *temp =
          realloc(*all_processes,
                  (total_count + remote_count + 1) * sizeof(process_info));
      if (!temp) {
        fprintf(stderr, "ERROR: Failed to reallocate process array\n");
        free(remote_procs);
        return total_count;
      }
      *all_processes = temp;

      for (int j = 0; j < remote_count; j++) {
        (*all_processes)[total_count + j] = remote_procs[j];
      }

      total_count += remote_count;
      (*all_processes)[total_count].pid = 0;
      free(remote_procs);
    }
  }

  return total_count;
}

int network_kill_process(int config_index, int pid, int signal) {
  if (config_index < 0 || config_index >= g_remote_configs_count) {
    return -1;
  }

  remote_config *config = &g_remote_configs[config_index];

  /* Escape password for safe shell usage */
  char escaped_password[STR_MAX * 2];
  if (escape_shell_string(config->password, escaped_password,
                          sizeof(escaped_password)) != 0) {
    fprintf(stderr, "ERROR: Password contains invalid characters\n");
    return -1;
  }

  /* Build SSH command to send signal (avoid truncation by sizing dynamically)
   */
  int ssh_cmd_len =
      snprintf(NULL, 0,
               "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -p %d %s@%s "
               "'kill -%d %d' 2>&1",
               escaped_password, config->port, config->username,
               config->address, signal, pid);
  if (ssh_cmd_len < 0) {
    fprintf(stderr, "ERROR: Failed to build SSH command\n");
    return -1;
  }
  char *ssh_cmd = calloc((size_t)ssh_cmd_len + 1, sizeof(char));
  if (!ssh_cmd) {
    fprintf(stderr, "ERROR: Cannot allocate SSH command buffer\n");
    return -1;
  }
  snprintf(ssh_cmd, (size_t)ssh_cmd_len + 1,
           "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -p %d %s@%s "
           "'kill -%d %d' 2>&1",
           escaped_password, config->port, config->username, config->address,
           signal, pid);

  int ret = system(ssh_cmd);
  if (ret != 0) {
    fprintf(stderr, "ERROR: Failed to send signal to remote process %d on %s\n",
            pid, config->name);
    free(ssh_cmd);
    return -1;
  }

  free(ssh_cmd);
  return 0;
}
