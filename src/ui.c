#include "../include/project.h"

void ui_loop() {
  int ch; // variable pour stocker la touche pressée
  process_info *plist = NULL; // liste dynamique récupérée via get_processes
  int count = get_processes(&plist); // recuperer le nombre de processus
  if (count < 0) {
    count = 0;
  }
  int selection = 0; // initialisation de l'incice pour le deplacement dans la liste de pids
  int tab = 0; // onglet courant (0=Processus, 1=CPU, 2=Mémoire, ...)
  char key[128] = ""; // buffer pour afficher la touche pressée

  initscr();            // initialise ncurses
  noecho();             // ne pas afficher les touches
  cbreak();             // lecture immédiate des touches
  keypad(stdscr, TRUE); // permet d'utiliser F1..F12, flèches, etc.

  int loop = 1;

  while (loop) {
    clear();

    printw("Appuyez sur F1..F8 (q pour quitter) | Tab: %d | Selection: %d\n", tab, selection);
    printw("Key: %s\n", key);
    printw("Onglets: ");
    
    // Afficher les onglets avec mise en surbrillance de l'onglet courant (attron sert a ça)
    for (int t = 0; t < 4; t++) {
      if (t == tab) {
        attron(A_REVERSE);
      }
      switch (t) {
        case 0: 
          printw("[Processus]"); 
          break;
        case 1: 
          printw("[CPU]"); 
          break;
        case 2: 
          printw("[Mémoire]"); 
          break;
          case 3: 
          printw("[Réseau]"); 
        break;
      }
      if (t == tab) {
        attroff(A_REVERSE);
      }
      printw(" ");
    }
    printw("\n\n");

    // Afficher la liste des PID et l'indice avec selection
    for (int i = 0; i < count; i++) {
      if (i == selection) {
        attron(A_REVERSE);
      }

      if (plist) {
        switch (tab) {
          case 0: // Processus
            printw("%6d  %s\n", plist[i].pid, plist[i].name);
            break;
          case 1: // CPU
            printw("%6d  CPU: %.2f%%\n", plist[i].pid, plist[i].cpu_usage);
            break;
          case 2: // Mémoire
            printw("%6d  MEM: %.2f%%\n", plist[i].pid, plist[i].mem_usage);
            break;
          case 3: // Réseau
            printw("%6d  (Info réseau)\n", plist[i].pid);
            break;
        }
      }
      else {
        printw("(no process)\n");
      }
      
      if (i == selection) {
        attroff(A_REVERSE);
      }
    }

    ch = getch();

    /*  les snprintf sont temporaires a remplacer par les fonctions associées :
        F1 : Afficher l’aide
        F2 : Passer à l’onglet suivant
        F3 : Revenir à l’onglet précédent
        F4 : Rechercher un processus
        F5 : Mettre un processus en pause
        F6 : Arrêter un processus
        F7 : Tuer un processus
        F8 : Redémarrer un processus
    */

    switch (ch) {
      case KEY_F(1):
        snprintf(key, sizeof(key), "F1 détectée");
        break;
      case KEY_F(2):
        snprintf(key, sizeof(key), "F2 détectée");
        break;
      case KEY_F(3):
        snprintf(key, sizeof(key), "F3 détectée");
        break;
      case KEY_F(4):
        snprintf(key, sizeof(key), "F4 détectée");
        break;
      case KEY_F(5):
        snprintf(key, sizeof(key), "F5 détectée");
        break;
      case KEY_F(6):
        snprintf(key, sizeof(key), "F6 détectée");
        break;
      case KEY_F(7):
        snprintf(key, sizeof(key), "F7 détectée");
        break;
      case KEY_F(8):
        snprintf(key, sizeof(key), "F8 détectée");
        break;
      case KEY_UP:
        if (selection > 0)
          selection--; // deplacer la selection vers le haut
        break;

      case KEY_DOWN:
        if (selection < count - 1)
          selection++; // deplacer la selection vers le bas
        break;
      case KEY_LEFT:
        if (tab > 0)
          tab--;  // aller a l'onglet precedent
        break;
      case KEY_RIGHT:
        if (tab < 3)  // nombre tab -> onglets max
          tab++;  // aller a l'onglet suivant
        break;
      case 'q':
      case 'Q':
        loop = 0;
    }
    refresh();
  }

  /* libérer la liste récupérée */
  free(plist);
  endwin();
}