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


void ui_loop() {
  int ch; // caractère saisi par getch()
  process_info *plist = NULL; // tableau des processus
  int count = get_processes(&plist); // nombre de processus récupérés
  if (count < 0) { // protection si get_processes a échoué
    count = 0; 
  }

  /* Etat de l'interface */
  int selection = 0; // index du processus actuellement sélectionné
  int offset = 0; // index du premier processus affiché
  int tab = 0; // onglet courant dans le menu
  char key[128] = ""; // message texte récapitulant la dernière touche F détectée

  /* Initialisation de ncurses */
  initscr(); // démarre le mode ncurses 
  noecho(); // n'affiche pas les touches saisies 
  cbreak(); // mode cbreak : saisie caractère par caractère 
  keypad(stdscr, TRUE); // active les touches spéciales (flèches, F-keys) 

  int h, w; // hauteur et largeur de l'écran
  getmaxyx(stdscr, h, w);

  /* Fenêtre secondaire pour la liste qui permet de manipuler le contenu indépendamment */
  WINDOW *listwin = newwin(h - 5, w, 5, 0); // hauteur = h-5, largeur = w, position y = 5 
  scrollok(listwin, FALSE); // on gère manuellement le scrolling des éléments via la variable offset donc on désactive le scrolling automatique

  int loop = 1; // flag pour controller la boucle principale

  while (loop) {

    // A chaque itération on libère l'ancienne liste, on recharge la nouvelle pour garder les informations à jour
    free(plist);
    plist = NULL;
    count = get_processes(&plist);
    if (count < 0) {
      count = 0;
    }

    /* Protections liées au redimensionnement de la fenetre */
    if (selection >= count) {
      selection = (count > 0 ? count - 1 : 0); // si l'index sélection dépasse, on le réduit
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
      int new_h = h - 5; // conserver la même logique de placement qu'avant
      if (new_h < 1) {
        new_h = 1; // s'assurer d'une hauteur minimale
      }
      wresize(listwin, new_h, w); // redimensionne la fenetre
      mvwin(listwin, 5, 0); // repositionne la fenetre 
      wclear(listwin); // efface les anciens contenus
    }

    /* Efface l'écran principal et affiche le menu */
    clear();

    /* Ligne de status : indications pour l'utilisateur */
    printw("Appuyez sur F1..F8 (q pour quitter) | Tab: %d | Selection: %d\n", tab, selection);
    printw("Key: %s\n", key); /* affiche la dernière touche F détectée */
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
    int max_lines = h - 7; // nombre de lignes disponibles pour la liste 

    /* Boucle d'affichage : on parcourt les processus visibles en tenant compte de l'offset */
    for (int i = offset; i < count && i < offset + max_lines; i++) {
      if (i == selection) {
        wattron(listwin, A_REVERSE); // surligne la ligne sélectionnée 
      }

      if (plist) {
        /* Affichage différent selon l'onglet actif */
        switch (tab) {
          case 0: {
            /* Onglet "Total" : affiche pid, nom tronqué, CPU, MEM et un emplacement pour info réseau */
            char truncated_name[21];
            /* On tronque/formatte proprement le nom du processus */
            snprintf(truncated_name, sizeof(truncated_name), "%-20.20s", plist[i].name);
            wprintw(listwin, "%-6d %-20s %-8.2f %-8.2f %-15s\n", plist[i].pid, truncated_name, plist[i].cpu_usage, plist[i].mem_usage, "(Info réseau)");
            break;
          }
          case 1:
            /* Onglet "Processus" : liste simple pid + nom */
            wprintw(listwin, "%6d  %s\n", plist[i].pid, plist[i].name);
            break;
          case 2:
            /* Onglet "CPU" : affiche pid et pourcentage CPU */
            wprintw(listwin, "%6d  CPU: %.2f%%\n", plist[i].pid, plist[i].cpu_usage);
            break;
          case 3:
            /* Onglet "Mémoire" : affiche pid et pourcentage mémoire */
            wprintw(listwin, "%6d  MEM: %.2f%%\n", plist[i].pid, plist[i].mem_usage);
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

    /* Gestion des touches */
    ch = getch(); // lecture d'une touche
    switch (ch) {
      case KEY_RESIZE: // detecte le redimensionnement de la fenêtre
        /* Redemande les dimensions pour que la prochaine boucle réajuste tout */
        getmaxyx(stdscr, h, w);
        break;

      /* Touches de fonctions : on mémorise simplement quelle touche a été pressée (a changer par les actions associées) */
      case KEY_F(1): snprintf(key, sizeof(key), "F1 détectée"); break;
      case KEY_F(2): snprintf(key, sizeof(key), "F2 détectée"); break;
      case KEY_F(3): snprintf(key, sizeof(key), "F3 détectée"); break;
      case KEY_F(4): snprintf(key, sizeof(key), "F4 détectée"); break;
      case KEY_F(5): snprintf(key, sizeof(key), "F5 détectée"); break;
      case KEY_F(6): snprintf(key, sizeof(key), "F6 détectée"); break;
      case KEY_F(7): snprintf(key, sizeof(key), "F7 détectée"); break;
      case KEY_F(8): snprintf(key, sizeof(key), "F8 détectée"); break;

      /* Navigation : flèche haut -> sélection monte, gérer scroll si besoin */
      case KEY_UP:
        if (selection > 0) {
          selection--;
          if (selection < offset) offset--; // décale vers le haut si on sort de la fenêtre 
        }
        break;

      /* Flèche bas -> sélection descend, gérer scroll si besoin */
      case KEY_DOWN:
        if (selection < count - 1) {
          selection++;
          if (selection >= offset + max_lines) offset++; // décale vers le bas si on dépasse 
        }
        break;

      /* Changer d'onglet à gauche/droite */
      case KEY_LEFT:
        if (tab > 0) tab--;
        break;

      case KEY_RIGHT:
        if (tab < 4) tab++;
        break;

      /* Quitter l'application */
      case 'q':
      case 'Q':
        loop = 0;
    }
    refresh(); // assure l'affichage principal mis à jour 
  }

  /* Nettoyage : libération de la mémoire et fin du mode ncurses */
  free(plist);
  delwin(listwin);
  endwin();
}