# Journal de bord - Perrytel OS

## 12. Création de la bibliothèque standard C (string.h) en bare-metal

**Date :** 12 Juillet 2026
**Sujet :** Implémentation des fonctions de manipulation de chaînes de caractères.

---

### 1. Contexte et Objectif
Dans un environnement "bare-metal" sans système d'exploitation sous-jacent, la bibliothèque standard du C (`libc`) n'est pas disponible. Pour pouvoir développer un Shell interactif capable d'analyser des commandes textuelles et d'afficher des données, il a fallu recréer notre propre équivalent de `<string.h>` à partir de zéro.

### 2. Fonctions implémentées (`kernel/string.c`)

#### `strlen` (Calcul de la longueur)
- **Rôle :** Compte le nombre de caractères avant d'atteindre le marqueur de fin de chaîne (`\0`).
- **Signature :** `unsigned long strlen(const char *str)`

#### `strcmp` (Comparaison performante)
- **Rôle :** Compare deux chaînes de caractères pour vérifier si elles sont identiques.
- **Optimisation "Unix Way" :** La boucle lit les caractères un par un en s'arrêtant dès qu'une différence est détectée. Cela évite un appel coûteux à `strlen` sur l'entièreté des chaînes, optimisant ainsi considérablement les performances.

#### `strcpy` (Copie de mémoire)
- **Rôle :** Copie une chaîne source dans une mémoire de destination.
- **Sécurité :** Une attention particulière a été portée à la copie manuelle du caractère nul (`\0`) final à l'index terminal, indispensable pour éviter les dépassements de mémoire lors des lectures futures.

#### `itoa` (Integer to ASCII)
- **Rôle :** Convertit une variable entière (ex: `123`) en chaîne de caractères affichable (ex: `"123"`).
- **Algorithme d'extraction :** Utilisation du modulo (`% 10`) pour extraire le dernier chiffre et ajout de la valeur ASCII `'0'` pour le convertir en caractère. Division entière (`/ 10`) pour décaler le nombre.
- **Inversion sur place :** Comme les chiffres sont extraits à l'envers (unités, puis dizaines), un algorithme d'inversion par permutation ("swap") utilisant une variable temporaire `temp` a été mis en place.
- **Gestion de la mémoire :** Contrairement à une erreur classique consistant à renvoyer un tableau local (détruit à la fin de la fonction), la mémoire (`char *dest`) est désormais allouée et fournie par l'appelant.

### 3. État actuel
- [x] Compilation bare-metal réussie (incluant `<stdbool.h>`).
- [x] Fonctions utilitaires opérationnelles et sécurisées (gestion stricte du `\0`).
- [ ] Branchement sur la gestion du clavier Minitel.
- [ ] Création de l'interpréteur de commandes (Shell).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
