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
    if (!path) return -1;

    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot open config file %s\n", path);
        return -1;
    }

    /* Libère les configs précédentes si présentes */
    if (g_remote_configs) {
        free(g_remote_configs);
        g_remote_configs = NULL;
        g_remote_configs_count = 0;
    }

    /* Compter les lignes valides d'abord */
    char line[STR_MAX];
    int line_count_total = 0;
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (strlen(line) == 0) continue;
        line_count_total++;
    }

    if (line_count_total == 0) {
        fprintf(stderr, "ERROR: Config file is empty\n");
        fclose(f);
        return -1;
    }

    /* Allouer le tableau de configs */
    g_remote_configs = calloc(line_count_total, sizeof(remote_config));
    if (!g_remote_configs) {
        fprintf(stderr, "ERROR: Failed to allocate memory for configs\n");
        fclose(f);
        return -1;
    }

    /* Revenir au début du fichier */
    rewind(f);

    int line_no = 0;
    int config_idx = 0;
    
    while (fgets(line, sizeof(line), f)) {
        line_no++;
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        if (strlen(line) == 0) continue;

        /* Copie de la ligne pour tokenisation */
        char copy[STR_MAX];
        strncpy(copy, line, STR_MAX-1);
        copy[STR_MAX-1] = '\0';

        /* Découpage en champs séparés par ':' */
        char *fields[6] = {0};
        char *tok = strtok(copy, ":");
        int i = 0;
        while (tok && i < 6) {
            fields[i++] = tok;
            tok = strtok(NULL, ":");
        }
        if (i != 6) {
            fprintf(stderr, "ERROR: Bad format in config %s at line %d\n", path, line_no);
            free(g_remote_configs);
            g_remote_configs = NULL;
            fclose(f);
            return -1;
        }

        /* Remplir la structure remote_config */
        remote_config *cfg = &g_remote_configs[config_idx];
        
        strncpy(cfg->name, fields[0], sizeof(cfg->name)-1);
        cfg->name[sizeof(cfg->name)-1] = '\0';
        
        strncpy(cfg->address, fields[1], sizeof(cfg->address)-1);
        cfg->address[sizeof(cfg->address)-1] = '\0';
        
        cfg->port = atoi(fields[2]);
        
        strncpy(cfg->username, fields[3], sizeof(cfg->username)-1);
        cfg->username[sizeof(cfg->username)-1] = '\0';
        
        strncpy(cfg->password, fields[4], sizeof(cfg->password)-1);
        cfg->password[sizeof(cfg->password)-1] = '\0';
        
        /* Déterminer le type de connexion */
        char type_upper[16];
        strncpy(type_upper, fields[5], sizeof(type_upper)-1);
        type_upper[sizeof(type_upper)-1] = '\0';
        strupper(type_upper);
        
        if (strcmp(type_upper, "SSH") == 0) {
            cfg->type = CONN_SSH;
        } else if (strcmp(type_upper, "TELNET") == 0) {
            cfg->type = CONN_TELNET;
        } else {
            fprintf(stderr, "ERROR: Invalid connection type at line %d\n", line_no);
            free(g_remote_configs);
            g_remote_configs = NULL;
            fclose(f);
            return -1;
        }

        printf("Config[%d]: name=%s addr=%s port=%d user=%s type=%s\n",
               config_idx+1, cfg->name, cfg->address, cfg->port, cfg->username, 
               (cfg->type == CONN_SSH) ? "SSH" : "TELNET");
        
        config_idx++;
    }

    g_remote_configs_count = config_idx;
    fclose(f);
    return 0;
}
