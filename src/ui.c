#include "../include/project.h"

int nombre_processus(int pids[]) {
  // Simuler la récupération des PID des processus
  int count = 5; // Supposons qu'il y a 5 processus
  for (int i = 0; i < count; i++) {
    pids[i] = 1000 + i; // Exemple de PID
  }
  return count;
}

void ui_loop() {
  int ch;                             // variable pour stocker la touche pressée
  int pids[1024];                     // tableau pour stocker les PID
  int count = nombre_processus(pids); // recuperer le nombre de processus
  int selection =
      0; // initialisation de l'incice pour le deplacement dans la liste de pids
  char key[128] = ""; // buffer pour afficher la touche pressée

  initscr();            // initialise ncurses
  noecho();             // ne pas afficher les touches
  cbreak();             // lecture immédiate des touches
  keypad(stdscr, TRUE); // permet d'utiliser F1..F12, flèches, etc.

  int looping = 1;

  while (looping) {
    clear();

    printw("Appuyez sur F1..F8 (q pour quitter)\n");
    printw("Key: %s\n\n", key);
    // Afficher la liste des PID et l'indice avec selection
    for (int i = 0; i < count; i++) {
      if (i == selection)
        attron(A_REVERSE);

      printw("%d\n", pids[i]);

      if (i == selection)
        attroff(A_REVERSE);
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
    case 'q':
    case 'Q':
      looping = 0;
    }
    refresh();
  }
  endwin();
}
