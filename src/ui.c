#include "../include/project.h"

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



void ui_loop(process_info **process_list_ptr, pthread_mutex_t *mutex) {
  int ch; // caractère saisi par getch()
  
  /* Etat de l'interface */
  int selection = 0; // index du processus actuellement sélectionné
  int offset = 0; // index du premier processus affiché
  int tab = 0; // onglet courant dans le menu
  char key[512] = ""; // message texte récapitulant la dernière touche F détectée

  /* Initialisation de ncurses */
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
      int new_h = h - 11; // conserver la même logique de placement qu'avant
      if (new_h < 1) {
        new_h = 1; // s'assurer d'une hauteur minimale
      }
      wresize(listwin, new_h, w); // redimensionne la fenetre
      mvwin(listwin, 11, 0); // repositionne la fenetre 
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
    int max_lines = h - 13; // nombre de lignes disponibles pour la liste 

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
            /* Onglet "Total" : affiche pid, nom tronqué, CPU, MEM et un emplacement pour info réseau */
            char state_char = (char)plist[i].state;
            char truncated_name[21];
            char ram_usage_str[16];
            char uptime_str[16];
            /* On tronque/formatte proprement le nom du processus */
            snprintf(truncated_name, sizeof(truncated_name), "%-20.20s", plist[i].name);
            /* On formate l'usage mémoire en Ko/Mo/Go */
            if (plist[i].mem_usage < 1024) {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%lu B", plist[i].mem_usage);
            } else if (plist[i].mem_usage < 1024 * 1024) {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%.2f KB", plist[i].mem_usage / 1024.0);
            } else if (plist[i].mem_usage < 1024 * 1024 * 1024) {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%.2f MB", plist[i].mem_usage / (1024.0 * 1024.0));
            } else {
              snprintf(ram_usage_str, sizeof(ram_usage_str), "%.2f GB", plist[i].mem_usage / (1024.0 * 1024.0 * 1024.0));
            }
            /* On formate le uptime en secondes/minutes/heures */
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
            /* Affichage final de la ligne */
            wprintw(listwin, "%6d \t%-20s \t%c \t%6.2f%% \t%8s \t%7s \t(net-info)\n",
                    plist[i].pid, truncated_name, state_char,
                    plist[i].cpu_usage, ram_usage_str, uptime_str);
            break;
          }
          case 1:
            /* Onglet "Processus" : liste simple pid + nom */
            wprintw(listwin, "%6d  %s\n", plist[i].pid, plist[i].name);
            break;
          case 2:
            /* Onglet "CPU" : affiche pid et pourcentage CPU */
            wprintw(listwin, "%6d  CPU: %.1f%%\n", plist[i].pid, plist[i].cpu_usage);
            break;
          case 3:
            /* Onglet "Mémoire" : affiche pid et pourcentage mémoire */
            wprintw(listwin, "%6d  MEM: %ld%%\n", plist[i].pid, plist[i].mem_usage);
            break;
          case 4:
            /* Onglet "Réseau" : actuellement placeholder */
            wprintw(listwin, "%6d  (Info réseau)\n", plist[i].pid);
            break;
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

      /* Touches de fonctions : on mémorise simplement quelle touche a été pressée (a changer par les actions associées) */
      case KEY_F(1): snprintf(key, sizeof(key), 
      "Aide :\n"
      "F2 : Passer à longlet suivant\n"
      "F3 : Revenir à longlet précédent\n"
      "F4 : Rechercher un processus\n"
      "F5 : Mettre un processus en pause\n"
      "F6 : Arrêter un processus\n"
      "F7 : Tuer un processus\n"
      "F8 : Redémarrer un processus\n"); break;
      
      case KEY_F(2): snprintf(key, sizeof(key), "\n""\n""\n""\n""\n""\n""\n""\n"); if (tab > 0) tab--; break;
      
      case KEY_F(3): snprintf(key, sizeof(key), "\n""\n""\n""\n""\n""\n""\n""\n"); if (tab < 4) tab++; break;
      
      case KEY_F(4): snprintf(key, sizeof(key), "Rechercher un processus"); break;
      
      case KEY_F(5): snprintf(key, sizeof(key), "Arrêter un processus"); break;
      case KEY_F(6): snprintf(key, sizeof(key), "Arrêter un processus"); break;
      case KEY_F(7): snprintf(key, sizeof(key), "Tuer un processus"); break;
      case KEY_F(8): snprintf(key, sizeof(key), "Redémarrer un processus"); break;

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
  // free(plist); // Managed by manager
  delwin(listwin);
  endwin();
}
