#include "../include/project.h"
#include <signal.h>
#include <locale.h>

/*
ui_loop - boucle principale de l'interface utilisateur ncurses
 
Cette fonction initialise ncurses, récupère périodiquement la liste
des processus via get_processes(), affiche un tableau interactif
(avec 5 onglets) et gère la navigation clavier (flèches, F1..F8, q).

Comportement principal :
- Récupère la liste des processus à chaque itération.
- Gère la sélection et le scrolling (offset) pour l'affichage.
- Affiche différentes vues selon l'onglet sélectionné (tab).
- Nettoie proprement ncurses à la sortie.
*/

// Fonction pour formater les octets en unités lisibles
void format_bytes(float bytes, char *buffer, size_t buffer_size) {
    if (bytes < 1024) {
        snprintf(buffer, buffer_size, "%.0f B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.2f KB", bytes / 1024.0);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(buffer, buffer_size, "%.2f MB", bytes / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buffer_size, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

// Fonction pour rechercher un processus par nom ou PID
int search_process(process_info *plist, size_t count, const char *search_term) {
    if (!plist || !search_term || strlen(search_term) == 0) return -1;
    
    // Essayer d'abord comme PID
    int search_pid = atoi(search_term);
    
    for (size_t i = 0; i < count; i++) {
        if (plist[i].pid == 0) break;
        
        // Recherche par PID
        if (search_pid > 0 && plist[i].pid == search_pid) {
            return i;
        }
        
        // Recherche par nom (insensible à la casse)
        char name_lower[256];
        char search_lower[256];
        strncpy(name_lower, plist[i].name, sizeof(name_lower)-1);
        strncpy(search_lower, search_term, sizeof(search_lower)-1);
        
        for (int j = 0; name_lower[j]; j++) {
            name_lower[j] = tolower(name_lower[j]);
        }
        for (int j = 0; search_lower[j]; j++) {
            search_lower[j] = tolower(search_lower[j]);
        }
        
        if (strstr(name_lower, search_lower) != NULL) {
            return i;
        }
    }
    
    return -1;
}

// Fonction pour afficher une fenêtre de saisie
int input_dialog(const char *prompt, char *buffer, int buffer_size) {
    int h, w;
    getmaxyx(stdscr, h, w);
    
    int dialog_h = 7;
    int dialog_w = 60;
    int start_y = (h - dialog_h) / 2;
    int start_x = (w - dialog_w) / 2;
    
    WINDOW *dialog = newwin(dialog_h, dialog_w, start_y, start_x);
    if (!dialog) return -1;
    
    box(dialog, 0, 0);
    mvwprintw(dialog, 1, 2, "%s", prompt);
    mvwprintw(dialog, 3, 2, "Input: ");
    mvwprintw(dialog, 5, 2, "Press ENTER to confirm, ESC to cancel");
    wrefresh(dialog);
    
    echo();
    nocbreak();
    cbreak();
    
    int ch;
    int pos = 0;
    buffer[0] = '\0';
    
    while (1) {
        wmove(dialog, 3, 9 + pos);
        wrefresh(dialog);
        
        ch = wgetch(dialog);
        
        if (ch == '\n' || ch == KEY_ENTER) {
            buffer[pos] = '\0';
            break;
        } else if (ch == 27) { // ESC
            delwin(dialog);
            noecho();
            return -1;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (pos > 0) {
                pos--;
                buffer[pos] = '\0';
                mvwprintw(dialog, 3, 9, "%-40s", buffer);
            }
        } else if (ch >= 32 && ch < 127 && pos < buffer_size - 1) {
            buffer[pos++] = (char)ch;
            buffer[pos] = '\0';
            mvwprintw(dialog, 3, 9, "%s", buffer);
        }
    }
    
    noecho();
    delwin(dialog);
    return 0;
}

// Fonction pour afficher une confirmation
int confirm_dialog(const char *message) {
    int h, w;
    getmaxyx(stdscr, h, w);
    
    int dialog_h = 7;
    int dialog_w = 60;
    int start_y = (h - dialog_h) / 2;
    int start_x = (w - dialog_w) / 2;
    
    WINDOW *dialog = newwin(dialog_h, dialog_w, start_y, start_x);
    if (!dialog) return 0;
    
    box(dialog, 0, 0);
    mvwprintw(dialog, 2, 2, "%s", message);
    mvwprintw(dialog, 4, 2, "Press 'y' to confirm, 'n' to cancel");
    wrefresh(dialog);
    
    int ch;
    while (1) {
        ch = wgetch(dialog);
        if (ch == 'y' || ch == 'Y') {
            delwin(dialog);
            return 1;
        } else if (ch == 'n' || ch == 'N' || ch == 27) {
            delwin(dialog);
            return 0;
        }
    }
}

// Fonction pour afficher un message d'information
void info_dialog(const char *message, int is_error) {
    int h, w;
    getmaxyx(stdscr, h, w);
    
    int dialog_h = 6;
    int dialog_w = 60;
    int start_y = (h - dialog_h) / 2;
    int start_x = (w - dialog_w) / 2;
    
    WINDOW *dialog = newwin(dialog_h, dialog_w, start_y, start_x);
    if (!dialog) return;
    
    box(dialog, 0, 0);
    if (is_error) {
        wattron(dialog, A_BOLD);
    }
    mvwprintw(dialog, 2, 2, "%s", message);
    if (is_error) {
        wattroff(dialog, A_BOLD);
    }
    mvwprintw(dialog, 4, 2, "Press any key to continue");
    wrefresh(dialog);
    
    timeout(-1);
    wgetch(dialog);
    timeout(1000);
    
    delwin(dialog);
}

void ui_loop(process_info **process_list_ptr, pthread_mutex_t *mutex) {
  int ch; // caractère saisi par getch()
  
  /* Etat de l'interface */
  int selection = 0; // index du processus actuellement sélectionné
  int offset = 0; // index du premier processus affiché
  int tab = 0; // onglet courant dans le menu
  char key[512] = ""; // message texte récapitulant la dernière touche F détectée
  long total_ram = get_total_ram_b(); // Récupérer la RAM totale une seule fois

  /* Initialisation de ncurses */
  setlocale(LC_ALL, "");
  initscr(); // démarre le mode ncurses 
  noecho(); // n'affiche pas les touches saisies 
  cbreak(); // mode cbreak : saisie caractère par caractère 
  keypad(stdscr, TRUE); // active les touches spéciales (flèches, F-keys) 
  timeout(1000); // Rafraichissement toutes les 1000ms si pas d'input

  int h, w; // hauteur et largeur de l'écran
  getmaxyx(stdscr, h, w);

  /* Fenêtre secondaire pour la liste qui permet de manipuler le contenu indépendamment */
  WINDOW *listwin = newwin(h - 11, w, 11, 0); // hauteur = h-11, largeur = w, position y = 5 
  scrollok(listwin, FALSE); // on gère manuellement le scrolling des éléments via la variable offset donc on désactive le scrolling automatique

  int loop = 1; // flag pour controller la boucle principale
  int space_value = 5; // espace entre les colonnes dans l'affichage


  while (loop) {
    pthread_mutex_lock(mutex);
    process_info *plist = *process_list_ptr;
    size_t count = 0;
    if (plist) {
        while (plist[count].pid != 0) {
            count++;
        }
    }

    /* Protections liées au redimensionnement de la fenetre */
    if (selection >= (int)count) {
      selection = (count > 0 ? (int)count - 1 : 0); // si l'index sélection dépasse, on le réduit
    }
    if (offset > selection) {
      offset = selection; // l'offset ne doit pas être après la sélection 
    }
    if (offset < 0) {
      offset = 0; // protection minimale comme precedemment
    }

    /* Ajustement de la fenêtre en cas de redimensionnement de la fenetre */
    getmaxyx(stdscr, h, w);
    if (listwin) {
      int new_h = h - space_value; // conserver la même logique de placement qu'avant
      if (new_h < 1) {
        new_h = 1; // s'assurer d'une hauteur minimale
      }
      wresize(listwin, new_h, w); // redimensionne la fenetre
      mvwin(listwin, space_value, 0); // repositionne la fenetre 
      wclear(listwin); // efface les anciens contenus
    }

    /* Efface l'écran principal et affiche le menu */
    clear();

    /* Ligne de status : indications pour l'utilisateur */
    printw("Appuyez sur q pour quitter et F1 pour l'aide | Tab: %d | Selection: %d\n", tab, selection);
    printw("%s\n", key); /* affiche la dernière touche F détectée */
    printw("Onglets: ");

    /* Affichage des onglets (Total, Processus, CPU, Mémoire, Réseau)
       L'onglet actif est inversé visuellement (A_REVERSE).
    */
    for (int t = 0; t < 5; t++) {
      if (t == tab) attron(A_REVERSE);
      switch (t) {
        case 0: printw("[Total]"); break;
        case 1: printw("[Processus]"); break;
        case 2: printw("[CPU]"); break;
        case 3: printw("[Mémoire]"); break;
        case 4: printw("[Réseau]"); break;
      }
      if (t == tab) {
        attroff(A_REVERSE);
      }
      printw(" ");
    }
    printw("\n\n");
    refresh(); // raffraichissement de l'ecran 

    /* Affichage des processus avec le scrolling */
    wclear(listwin);
    int max_lines = h - space_value; // nombre de lignes disponibles pour la liste 

    /* Boucle d'affichage : on parcourt les processus visibles en tenant compte de l'offset */
    for (int i = offset; i < (int)count && i < offset + max_lines; i++) {
      if (i == selection) {
        wattron(listwin, A_REVERSE); // surligne la ligne sélectionnée 
      }

      if (plist) {
        /* Affichage différent selon l'onglet actif */
        switch (tab) {
          case 0: {
            if (plist == NULL || plist[i].pid == 0 || plist[i].name == NULL) break;
            char state_char = (char)plist[i].state;
            char truncated_name[21];
            char ram_usage_str[16];
            char uptime_str[16];
            char net_info_str[32];
            
            snprintf(truncated_name, sizeof(truncated_name), "%-20.20s", plist[i].name);
            
            if (plist[i].mem_usage < 1024) {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%lu B", plist[i].mem_usage);
            } else if (plist[i].mem_usage < 1024 * 1024) {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%.2f KB", plist[i].mem_usage / 1024.0);
            } else if (plist[i].mem_usage < 1024 * 1024 * 1024) {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%.2f MB", plist[i].mem_usage / (1024.0 * 1024.0));
            } else {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%.2f GB", plist[i].mem_usage / (1024.0 * 1024.0 * 1024.0));
            }
            
            long uptime = plist[i].uptime;
            if (uptime < 60) {
              snprintf(uptime_str, sizeof(uptime_str), "%ld s", uptime);
            } else if (uptime < 3600) {
              snprintf(uptime_str, sizeof(uptime_str), "%ld:%02ld", uptime / 60, uptime % 60);
            } else {
              long hours = uptime / 3600;
              long minutes = (uptime % 3600) / 60;
              long seconds = uptime % 60;
              snprintf(uptime_str, sizeof(uptime_str), "%ld:%02ld:%02ld", hours, minutes, seconds);
            }
            
            char send_str[16], recv_str[16];
            format_bytes(plist[i].net_send_rate, send_str, sizeof(send_str));
            format_bytes(plist[i].net_recv_rate, recv_str, sizeof(recv_str));
            snprintf(net_info_str, sizeof(net_info_str), "Up:%s/s Down:%s/s", send_str, recv_str);
            
            wprintw(listwin, "%6d \t%-20s \t%c \t%6.2f%% \t%8s \t%7s \t%s\n",
                    plist[i].pid, truncated_name, state_char,
                    plist[i].cpu_usage, ram_usage_str, uptime_str, net_info_str);
            break;
          }
          case 1:
            wprintw(listwin, "%6d  %s\n", plist[i].pid, plist[i].name);
            break;
          case 2:
            wprintw(listwin, "%6d  CPU: %.1f%%\n", plist[i].pid, plist[i].cpu_usage);
            break;
          case 3: {
            float mem_percent = 0.0f;
            if (total_ram > 0) {
                mem_percent = ((float)plist[i].mem_usage / total_ram) * 100.0f;
            }
            char mem_size_str[16];
            format_bytes((float)plist[i].mem_usage, mem_size_str, sizeof(mem_size_str));
            wprintw(listwin, "%6d  MEM: %.2f%% (%s)\n", plist[i].pid, mem_percent, mem_size_str);
            break;
          }
          case 4: {
            char send_str[16], recv_str[16];
            format_bytes(plist[i].net_send_rate, send_str, sizeof(send_str));
            format_bytes(plist[i].net_recv_rate, recv_str, sizeof(recv_str));
            wprintw(listwin, "%6d  ↑ %s/s  ↓ %s/s\n", plist[i].pid, send_str, recv_str);
            break;
          }
        }
      } else {
        /* Si la liste n'existe pas, afficher un message d'erreur simple */
        wprintw(listwin, "(no process)\n");
      }

      if (i == selection) {
        wattroff(listwin, A_REVERSE);
      }
    }
    wrefresh(listwin); // envoie le contenu de la 2eme fenetre à l'écran
    pthread_mutex_unlock(mutex);

    /* Gestion des touches */
    ch = getch(); // lecture d'une touche
    switch (ch) {
      case KEY_RESIZE: // detecte le redimensionnement de la fenêtre
        /* Redemande les dimensions pour que la prochaine boucle réajuste tout */
        getmaxyx(stdscr, h, w);
        break;

      /* F1 : Afficher l'aide */
      case KEY_F(1):
        space_value = 11;
        snprintf(key, sizeof(key), 
        "Aide :\n"
        "F2 : Passer a l'onglet suivant\n"
        "F3 : Revenir a l'onglet precedent\n"
        "F4 : Rechercher un processus\n"
        "F5 : Mettre un processus en pause (SIGSTOP)\n"
        "F6 : Arreter un processus (SIGTERM)\n"
        "F7 : Tuer un processus (SIGKILL)\n"
        "F8 : Redemarrer un processus (SIGCONT)\n"); 
        break;
        
      /* F2 : Passer à l'onglet suivant */
      case KEY_F(2): 
        space_value = 5; 
        snprintf(key, sizeof(key), ""); 
        if (tab < 4) tab++;
        break;
        
      /* F3 : Revenir à l'onglet précédent */
      case KEY_F(3): 
        space_value = 5; 
        snprintf(key, sizeof(key), ""); 
        if (tab > 0) tab--;
        break;
        
      /* F4 : Rechercher un processus */
      case KEY_F(4): {
        space_value = 5;
        snprintf(key, sizeof(key), "");
        
        char search_term[256];
        if (input_dialog("Rechercher un processus (nom ou PID):", search_term, sizeof(search_term)) == 0) {
            pthread_mutex_lock(mutex);
            process_info *plist_search = *process_list_ptr;
            size_t count_search = 0;
            if (plist_search) {
                while (plist_search[count_search].pid != 0) {
                    count_search++;
                }
            }
            
            int found_idx = search_process(plist_search, count_search, search_term);
            pthread_mutex_unlock(mutex);
            
            if (found_idx >= 0) {
                selection = found_idx;
                // Ajuster l'offset pour que la sélection soit visible
                if (selection < offset) {
                    offset = selection;
                } else if (selection >= offset + max_lines) {
                    offset = selection - max_lines + 1;
                }
                char msg[256];
                snprintf(msg, sizeof(msg), "Processus trouve: %s (PID: %d)", 
                         plist_search[found_idx].name, plist_search[found_idx].pid);
                info_dialog(msg, 0);
            } else {
                info_dialog("Processus non trouve", 1);
            }
        }
        break;
      }
        
      /* F5 : Mettre un processus en pause (SIGSTOP) */
      case KEY_F(5): {
          space_value = 5;
          snprintf(key, sizeof(key), "");
          
          pthread_mutex_lock(mutex);
          process_info *plist_pause = *process_list_ptr;
          if (plist_pause && selection >= 0) {
              int target_pid = plist_pause[selection].pid;
              int remote_idx = plist_pause[selection].remote_config_index;
              char target_name[256];
              strncpy(target_name, plist_pause[selection].name, sizeof(target_name)-1);
              pthread_mutex_unlock(mutex);
              
              char msg[256];
              if (remote_idx >= 0) {
                  snprintf(msg, sizeof(msg), "Mettre en pause le processus distant %s (PID: %d) sur %s ?", 
                          target_name, target_pid, g_remote_configs[remote_idx].name);
              } else {
                  snprintf(msg, sizeof(msg), "Mettre en pause le processus %s (PID: %d) ?", 
                          target_name, target_pid);
              }
              
              if (confirm_dialog(msg)) {
                  int result;
                  if (remote_idx >= 0) {
                      result = network_kill_process(remote_idx, target_pid, SIGSTOP);
                  } else {
                      result = (kill(target_pid, SIGSTOP) == 0) ? 0 : -1;
                  }
                  
                  if (result == 0) {
                      snprintf(msg, sizeof(msg), "Processus %d mis en pause", target_pid);
                      info_dialog(msg, 0);
                  } else {
                      snprintf(msg, sizeof(msg), "Erreur: impossible de mettre en pause le processus %d", target_pid);
                      info_dialog(msg, 1);
                  }
              }
          } else {
              pthread_mutex_unlock(mutex);
              info_dialog("Aucun processus selectionne", 1);
          }
          break;
      }

      /* F6 : Arrêter un processus (SIGTERM) */
      case KEY_F(6): {
          space_value = 5;
          snprintf(key, sizeof(key), "");
          
          pthread_mutex_lock(mutex);
          process_info *plist_term = *process_list_ptr;
          if (plist_term && selection >= 0) {
              int target_pid = plist_term[selection].pid;
              int remote_idx = plist_term[selection].remote_config_index;
              char target_name[256];
              strncpy(target_name, plist_term[selection].name, sizeof(target_name)-1);
              pthread_mutex_unlock(mutex);
              
              char msg[256];
              if (remote_idx >= 0) {
                  snprintf(msg, sizeof(msg), "Arreter le processus distant %s (PID: %d) sur %s ?", 
                          target_name, target_pid, g_remote_configs[remote_idx].name);
              } else {
                  snprintf(msg, sizeof(msg), "Arreter le processus %s (PID: %d) ?", 
                          target_name, target_pid);
              }
              
              if (confirm_dialog(msg)) {
                  int result;
                  if (remote_idx >= 0) {
                      result = network_kill_process(remote_idx, target_pid, SIGTERM);
                  } else {
                      result = (kill(target_pid, SIGTERM) == 0) ? 0 : -1;
                  }
                  
                  if (result == 0) {
                      snprintf(msg, sizeof(msg), "Signal SIGTERM envoye au processus %d", target_pid);
                      info_dialog(msg, 0);
                  } else {
                      snprintf(msg, sizeof(msg), "Erreur: impossible d'arreter le processus %d", target_pid);
                      info_dialog(msg, 1);
                  }
              }
          } else {
              pthread_mutex_unlock(mutex);
              info_dialog("Aucun processus selectionne", 1);
          }
          break;
      }

      /* F7 : Tuer un processus (SIGKILL) */
      case KEY_F(7): {
          space_value = 5;
          snprintf(key, sizeof(key), "");
          
          pthread_mutex_lock(mutex);
          process_info *plist_kill = *process_list_ptr;
          if (plist_kill && selection >= 0) {
              int target_pid = plist_kill[selection].pid;
              int remote_idx = plist_kill[selection].remote_config_index;
              char target_name[256];
              strncpy(target_name, plist_kill[selection].name, sizeof(target_name)-1);
              pthread_mutex_unlock(mutex);
              
              char msg[256];
              if (remote_idx >= 0) {
                  snprintf(msg, sizeof(msg), "TUER (SIGKILL) le processus distant %s (PID: %d) sur %s ?", 
                          target_name, target_pid, g_remote_configs[remote_idx].name);
              } else {
                  snprintf(msg, sizeof(msg), "TUER (SIGKILL) le processus %s (PID: %d) ?", 
                          target_name, target_pid);
              }
              
              if (confirm_dialog(msg)) {
                  int result;
                  if (remote_idx >= 0) {
                      result = network_kill_process(remote_idx, target_pid, SIGKILL);
                  } else {
                      result = (kill(target_pid, SIGKILL) == 0) ? 0 : -1;
                  }
                  
                  if (result == 0) {
                      snprintf(msg, sizeof(msg), "Processus %d tue (SIGKILL)", target_pid);
                      info_dialog(msg, 0);
                  } else {
                      snprintf(msg, sizeof(msg), "Erreur: impossible de tuer le processus %d", target_pid);
                      info_dialog(msg, 1);
                  }
              }
          } else {
              pthread_mutex_unlock(mutex);
              info_dialog("Aucun processus selectionne", 1);
          }
          break;
      }

      /* F8 : Redémarrer un processus (SIGCONT) */
      case KEY_F(8): {
          space_value = 5;
          snprintf(key, sizeof(key), "");
          
          pthread_mutex_lock(mutex);
          process_info *plist_cont = *process_list_ptr;
          if (plist_cont && selection >= 0) {
              int target_pid = plist_cont[selection].pid;
              int remote_idx = plist_cont[selection].remote_config_index;
              char target_name[256];
              strncpy(target_name, plist_cont[selection].name, sizeof(target_name)-1);
              pthread_mutex_unlock(mutex);
              
              char msg[256];
              if (remote_idx >= 0) {
                  snprintf(msg, sizeof(msg), "Redemarrer le processus distant %s (PID: %d) sur %s ?", 
                          target_name, target_pid, g_remote_configs[remote_idx].name);
              } else {
                  snprintf(msg, sizeof(msg), "Redemarrer le processus %s (PID: %d) ?", 
                          target_name, target_pid);
              }
              
              if (confirm_dialog(msg)) {
                  int result;
                  if (remote_idx >= 0) {
                      result = network_kill_process(remote_idx, target_pid, SIGCONT);
                  } else {
                      result = (kill(target_pid, SIGCONT) == 0) ? 0 : -1;
                  }
                  
                  if (result == 0) {
                      snprintf(msg, sizeof(msg), "Processus %d redemarre (SIGCONT)", target_pid);
                      info_dialog(msg, 0);
                  } else {
                      snprintf(msg, sizeof(msg), "Erreur: impossible de redemarrer le processus %d", target_pid);
                      info_dialog(msg, 1);
                  }
              }
          } else {
              pthread_mutex_unlock(mutex);
              info_dialog("Aucun processus selectionne", 1);
          }
          break;
      }

      /* Navigation : flèche haut -> sélection monte, gérer scroll si besoin */
      case KEY_UP:
        if (selection > 0) {
          selection--;
          if (selection < offset) offset--; // décale vers le haut si on sort de la fenêtre 
        }
        break;

      /* Flèche bas -> sélection descend, gérer scroll si besoin */
      case KEY_DOWN:
        if (selection < (int)count - 1) {
          selection++;
          if (selection >= offset + max_lines) offset++; // décale vers le bas si on dépasse 
        }
        break;

      /* Quitter l'application */
      case 'q':
      case 'Q':
        loop = 0;
    }
    refresh(); // assure l'affichage principal mis à jour 
  }

  /* Nettoyage : libération de la mémoire et fin du mode ncurses */
  delwin(listwin);
  endwin();
}