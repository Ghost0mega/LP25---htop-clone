# Projet LP25 - A2025

## Contexte

le projet consiste à concevoir un **programme de gestion de processus pour systèmes Linux**, capable de fonctionner aussi bien **en local que sur des hôtes distants**. L’objectif est de fournir une interface interactive, inspirée de l’outil `htop`, permettant : 
- d’afficher dynamiquement la liste des processus actifs sur une ou plusieurs machines,
- de visualiser des informations détaillées (PID, utilisateur, consommation CPU/mémoire, temps d’exécution, etc.), 
- et d’interagir directement avec les processus (mise en pause, arrêt, reprise, redémarrage).
L’outil reposera sur des mécanismes standards de communication et d’administration à distance (SSH, etc.), compatibilité avec les principales distributions Linux.


## Architecture

- **manager** : Implémente les fonctionnalités pour orchestration de tous les modules
- **process** : Implémente les fonctionnalités de gestion de processus sur une machine linux
- **network** : Implémente les fonctionnalités de communication réseau en permettant l'envoi de données à un serveur distant et la réception de données à partir d'un port spécifié. Les protocoles ssh et telnet sont implémentés pour la communication
- **ui** : Implémente les fonctionnalités de gestion de l'affichage pour l'utilisateur (affichage des processus, gestion des événements claviers, etc.). 

## Options du programme

Le programme dispose de plusieurs options :
- `-h` ou `--help` : affiche l'aide du programme ainsi que la syntaxe d'exécution du programme
- `--dry-run` : test l'accès à la liste des processus sur la machine locale et/ou distante sans les afficher
- `-c` ou `--remote-config` : spécifie le chemin vers le fichier de configuration contenant les informations de connexion sur les machines distantes
- `-t` ou `--connexion-type` : spécifie le type de connexion à utiliser pour la connexion sur les machines distantes (ssh, telnet)
- `-P` ou `--port` : spécifie le port à utiliser pour le type de connexion choisi. Si cette option n'est pas spécifié, alor le port par défaut du type de connexion est choisi
- `-l` ou `--login` : spécifie l'identifiant de connexion et la machine distante. Ex : `--login user@remote_server`. `remote_server` est soit l'IP ou le nom DNS de la machine distante
- `-s` ou `--remote-server` : spécifie le nom DNS ou l'IP de la machine distante
- `-u` ou `--username` : spécifie le nom de l'utilisateur à utiliser pour la connexion
- `-p` ou `--password` : spécifie le mot de passe à utiliser pour la connexion
- `-a` ou `--all` : spécifie au programme de collecter à la fois la liste des processus sur la machine local et les machines distantes. S'utilise uniquement si l'option `-c` ou `-s` est utilisé.

**NB** : Lorsqu'aucune option n'est spécifiée, alors le programme affiche uniquement les processus de la machine locale (*la machine sur laquelle le programme est exécuté*).

### L'option `-c` ou `--remote-config` 

Cette option permet de spécifier le chemin vers un fichier au format texte contenant les informations de connexion sur les machines distantes. Pour cela : 

Le format du fichier suit la structure suivante :
```bash
nom_serveur1:adresse_serveur:port:username:password:type_connexion1
nom_serveur2:adresse_serveur:port:username:password:type_connexion2
```

- `nom_serveur` : spécifie le nom à utiliser pour l'affichage des processus de cette machine dans le tableau de bord mutualisé
- `adresse_serveur` : spécifie l'adresse IP ou le nom DNS de la machine distante
- `port` : spécifie le port à utiliser pour la connexion sur la machine distante
- `username` : spécifie le nom d'utilisateur à utiliser pour la connexion
- `password` : spécifie le mot de passe à utiliser pour la connexion
- `type_connexion` : spécifie le protocole à utiliser pour la connexion (*ssh, telnet*)

Le fichier de configuration doit être caché (*renommé comme suit : `.nomfichier`*) et avoir les droits suivants `rw-------` ou `600`.  Lors de l'exécution du programme, si le fichier de configuration n'a pas les bons droits, une alerte est levée pour attirer le regard de l'utilisateur.

Lorsqu'aucun chemin de fichier de config n'est fourni, alors le fichier est cherché dans le répertoire courant sous le nom `.config`.

### L'option `-s` ou `--remote-server`

Cette option permet de spécifier une machine distante via son adresse IP ou son nom de domaine (DNS).
Si les options `-u` et `-p` ne sont pas specifiés dans la commande, alors il faudra les demandé interactivement lors de l'exécution.

### L'option `-l` ou `--login`

Cette option permet de spécifier l'indentifiant et l'adresse de la machine distante suivant la syntaxe suivante `-l user@server1.edu` ou `--login user2@192.168.1.1`.

Si l'option `-p` n'est pas spécifié dans la commande, alors il faudra demandé interactivement le mot de passe de l'utilisateur.

## Affichage mutualisé des processus

L’affichage de la console s’inspire de la commande `htop`. Un système d’onglets permet de visualiser les processus de chaque machine, chaque onglet correspondant à une machine distincte. L’utilisateur peut naviguer entre les onglets et interagir avec les processus à l’aide de raccourcis clavier :
- **F1** : Afficher l’aide
- **F2** : Passer à l’onglet suivant
- **F3** : Revenir à l’onglet précédent
- **F4** : Rechercher un processus
- **F5** : Mettre un processus en pause
- **F6** : Arrêter un processus
- **F7** : Tuer un processus
- **F8** : Redémarrer un processus

## Modalités d'évaluation

L'évaluation porte sur 3 éléments :
- Le rapport
    - un canvas vous est fourni sur la plateforme moodle ; vous devrez détailler plus précisément ce qui sera présenté à la soutenance, ainsi que des résultats quantitatifs et/ou qualitatifs de votre travail.
- La soutenance
    - une présentation de 10 minutes de tout le groupe (en équilibrant les temps de parole) dans laquelle :
        - vous rappellerez le contexte du projet, les grandes lignes de son fonctionnement (2 minutes)
        - vous expliquerez les choix que vous avez eu à faire et pourquoi (3 minutes)
        - vous présenterez les difficultés les plus importantes rencontrées et vos analyse à ce sujet (2 minutes)
        - vous proposerez un RETEX (retour d'expérience) dans lequel vous répondrez du mieux que possible à la question suivante : "_fort(e)s de notre expérience sur le projet, que ferions-nous différemment si nous devions le recommencer aujourd'hui ?_"
        - vous ferez une brève conclusion (1 minute)
    - vous répondrez ensuite à des questions posées par les enseignants pendant une dizaine de minutes.
- Le code
    - fourni dans un dépôt git avec l'historique des contributions de tous les membres,
    - avec un Makefile qui permet la compilation simplement avec la commande `make`
    - capacité à effectuer les traitements demandés dans le sujet,
    - capacité à traiter les cas particuliers sujets à erreur (pointeurs NULL, etc.)
    - Respect des conventions d'écriture de code
    - Documentation du code
        - Avec des commentaires au format doxygen en entêtes de fonction (si nécessaire)
        - Des commentaires pertinents sur le flux d'une fonction (astuces, cas limites, détails de l'algorithme, etc.)
    - Absence ou faible quantité de fuites mémoire (vérifiables avec `valgrind`)
    - **ATTENTION !** le code doit compiler sous Linux ! Un code non compatible avec un système Linux sera pénalisé de 5 points sur 20.


## Annexes

### Convention de code
Il est attendu dans ce projet que le code rendu satisfasse un certain nombre de conventions (ce ne sont pas des contraintes du langages mais des choix au début d'un projet) :
- **indentations** : les indentations seront faites sur un nombre d'espaces à votre discrétion, mais ce nombre **doit être cohérent dans l'ensemble du code**.
- **Déclaration des pointeurs** : l'étoile du pointeur est séparée du type pointé par un espace, et collée au nom de la variable, ainsi :
    - `int *a` est correct
    - `int* a`, `int*a` et `int * a` sont incorrects
- **Nommage des variables, des types et des fonctions** : vous utiliserez le _snake case_, i.e. des noms sans majuscule et dont les parties sont séparées par des underscores `_`, par exemple :
    - `ma_variable`, `variable`, `variable_1` et `variable1` sont corrects
    - `maVariable`, `Variable`, `VariableUn` et `Variable1` sont incorrects
- **Position des accolades** : une accolade s'ouvre sur la ligne qui débute son bloc (fonction, if, for, etc.) et est immédiatement suivie d'un saut de ligne. Elle se ferme sur la ligne suivant la dernière instruction. L'accolade fermante n'est jamais suivie d'instructions à l'exception du `else` ou du `while` (structure `do ... while`) qui suit l'accolade fermante. Par exemple :
```c
for (...) {
	/*do something*/
}

if (true) {
	/*do something*/
} else {
	/*do something else*/
}

int max(int a, int b) {
	return a;
}
```

sont corrects mais :

```c
for (int i=0; i<5; ++i)
{ printf("%d\n", i);
}

for (int i=0; i<5; ++i) {
	printf("%d\n", i); }

if () {/*do something*/
}
else
{
	/*do something else*/}
```

sont incorrects.

- Espacement des parenthèses : la parenthèse ouvrante après `if`, `for`, et `while` est séparée de ces derniers par un espace. Après un nom de fonction, l'espace est collé au dernier caractère du nom. Il n'y a pas d'espace après une parenthèse ouvrante, ni avant une parenthèse fermante :
    - `while (a == 3)`, `for (int i=0; i<3; ++i)`, `if (a == 3)` et `void ma_fonction(void)` sont corrects
    - `while(a == 3 )`, `for ( i=0;i<3 ; ++i)`, `if ( a==3)` et `void ma_fonction (void )` sont incorrects
- Basé sur les exemples ci dessus, également, les opérateurs sont précédés et suivis d'un espace, sauf dans la définition d'une boucle `for` où ils sont collés aux membres de droite et de gauche.
- Le `;` qui sépare les termes de la boucle `for` ne prend pas d'espace avant, mais en prend un après.
