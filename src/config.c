#include "../include/project.h"

/* Global configurations for remote machines */
remote_config *g_remote_configs = NULL;
int g_remote_configs_count = 0;

/*
 * Lit un fichier de configuration de machines distantes au format :
 * nom_serveur:adresse_serveur:port:username:password:type_connexion
 *
 * Remplit le tableau global g_remote_configs et g_remote_configs_count.
 * Renvoie 0 en cas de succès, -1 en cas d'erreur.
 */
int config_load(const char *path) {
  if (!path)
    return -1;

  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "ERROR: Cannot open config file %s\n", path);
    return -1;
  }

  if (g_remote_configs) {
    free(g_remote_configs);
    g_remote_configs = NULL;
    g_remote_configs_count = 0;
  }

  char line[STR_MAX];
  int line_count_total = 0;
  while (fgets(line, sizeof(line), f)) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
      line[len - 1] = '\0';
    if (strlen(line) == 0)
      continue;
    line_count_total++;
  }

  if (line_count_total == 0) {
    fprintf(stderr, "ERROR: Config file is empty\n");
    fclose(f);
    return -1;
  }

  g_remote_configs = calloc(line_count_total, sizeof(remote_config));
  if (!g_remote_configs) {
    fprintf(stderr, "ERROR: Failed to allocate memory for configs\n");
    fclose(f);
    return -1;
  }

  rewind(f);

  int line_no = 0;
  int config_idx = 0;

  while (fgets(line, sizeof(line), f)) {
    line_no++;
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
      line[len - 1] = '\0';
    if (strlen(line) == 0)
      continue;

    char copy[STR_MAX];
    strncpy(copy, line, STR_MAX - 1);
    copy[STR_MAX - 1] = '\0';

    // Now expecting 5 fields instead of 6: name:address:port:username:password
    char *fields[5] = {0};
    char *tok = strtok(copy, ":");
    int i = 0;
    while (tok && i < 5) {
      fields[i++] = tok;
      tok = strtok(NULL, ":");
    }
    if (i != 5) {
      fprintf(stderr,
              "ERROR: Bad format in config %s at line %d (expected 5 fields)\n",
              path, line_no);
      free(g_remote_configs);
      g_remote_configs = NULL;
      fclose(f);
      return -1;
    }

    remote_config *cfg = &g_remote_configs[config_idx];

    strncpy(cfg->name, fields[0], sizeof(cfg->name) - 1);
    cfg->name[sizeof(cfg->name) - 1] = '\0';

    strncpy(cfg->address, fields[1], sizeof(cfg->address) - 1);
    cfg->address[sizeof(cfg->address) - 1] = '\0';

    cfg->port = atoi(fields[2]);

    strncpy(cfg->username, fields[3], sizeof(cfg->username) - 1);
    cfg->username[sizeof(cfg->username) - 1] = '\0';

    strncpy(cfg->password, fields[4], sizeof(cfg->password) - 1);
    cfg->password[sizeof(cfg->password) - 1] = '\0';

    printf("Config[%d]: name=%s addr=%s port=%d user=%s type=SSH\n",
           config_idx + 1, cfg->name, cfg->address, cfg->port, cfg->username);

    config_idx++;
  }

  g_remote_configs_count = config_idx;
  fclose(f);
  return 0;
}
