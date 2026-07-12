# Journal de bord - Perrytel OS

## 14. Architecture des Commandes et Arguments

**Date :** 12 Juillet 2026
**Sujet :** Refactorisation du Shell pour supporter l'architecture "Array of Structs" et le passage d'arguments (`argc`, `argv`).

---

### 1. Contexte et Objectif
Après avoir validé la capture asynchrone des touches, le noyau devait être capable de comprendre des commandes complexes avec des arguments (ex: `echo bonjour`) de manière extensible. L'objectif était de quitter l'approche naïve des blocs `if / else if` pour une véritable architecture de routeur de commandes.

### 2. Le Découpage Sans Allocation (strsplit)
Dans un OS bare-metal dépourvu de gestionnaire de mémoire (`malloc`), le défi principal pour extraire les arguments est d'éviter toute allocation dynamique.
- **La solution :** La fonction `strsplit` (inspirée de `strtok`) a été développée pour mutiler la chaîne originale en remplaçant les caractères espace `' '` par des fins de chaînes `'\0'`.
- **Résultat :** Les adresses mémoires du début de chaque mot sont stockées dans un tableau statique `char *argv[]`, fournissant au système un jeu complet de pointeurs vers chaque argument, sans allouer de mémoire supplémentaire.

### 3. Architecture "Array of Structs"
L'OS utilise désormais le concept de pointeurs de fonctions, standard dans les noyaux UNIX et systèmes embarqués, pour séparer la détection de la commande de son exécution.

#### La structure (shell.h) :
Chaque commande est définie par un nom (`const char *name`) et un pointeur vers la fonction C associée :
```c
typedef void (*function)(int argc, char **argv);

typedef struct {
    const char *name;
    function exec;
} Command;
```

#### Le Registre (shell.c) :
Les commandes sont référencées dans un tableau global. L'ajout d'une commande se fait en deux étapes simples : écrire la fonction `void mafonction(int argc, char **argv)`, puis l'ajouter au catalogue :
```c
Command list[] = {
    {"help", help},
    {"time", time_command}
};
```

### 4. Le Cycle d'Exécution (`exec`)
Lorsque l'utilisateur presse `Entrée` :
1. Le buffer brut est envoyé à `exec()`.
2. `strsplit()` génère `argc` et `argv`.
3. L'OS boucle sur le tableau `list[]` et compare `argv[0]` avec le nom des commandes via `strcmp()`.
4. Si une correspondance est trouvée, la fonction `exec(argc, argv)` associée est appelée, transférant l'exécution à la commande interne.

### 5. État actuel
- [x] Découpage des mots asynchrone sans allocation (`strsplit`).
- [x] Architecture par catalogue de fonctions opérationnelle.
- [x] Validation du passage d'arguments (testé dans les commandes via `strcmp(argv[n], ...)`).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
