# Journal de bord - Perrytel OS

## 23. Suppression de Fichiers (`rm`) et Répertoires Récursifs (`rmdir`) en FAT32

**Date :** 13 Août 2026  
**Sujet :** Implémentation du système de suppression physique de fichiers (`rm`) et de répertoires récursifs (`rmdir`), purge des chaînes de clusters dans la table FAT, et optimisation par clauses de garde (`0x00`, `0xE5`).

---

### 1. Introduction

Cette étape achève le cycle de vie des données sur le système de fichiers FAT32 de PerryTel OS. Après la lecture, l'écriture et la création de répertoires, le noyau est désormais capable de supprimer physiquement des fichiers (`rm`) et des répertoires récursifs (`rmdir`), de libérer les clusters occupés dans la table FAT pour récupérer de l'espace disque, et d'assurer l'intégrité de la carte SD sans laisser de clusters orphelins.

---

### 2. État d'avancement et Architecture Technique

#### A. Suppression de Fichiers (`fat32_remove_file`, `rm`)
La suppression d'un fichier nécessite l'exécution séquentielle de deux opérations matérielles :
1. **Marquage dans le dossier parent** :
   - Inscription de l'octet de suppression **`0xE5` (`DELETED`)** sur le premier octet de la structure de 32 octets (`entry->name[0] = 0xE5`).
   - Réécriture immédiate du secteur du dossier parent sur la carte SD via `sd_write_block()`.
2. **Purge de la chaîne de clusters dans la FAT** :
   - Parcours de la chaîne de clusters du fichier jusqu'à la marque de fin `0x0FFFFFF8` (`EOF`).
   - Réinitialisation de chaque entrée de cluster à **`0x00000000` (`EMPTY_CLUSTER`)** dans le secteur correspondant de la table FAT (`starting_fat_sector + (cluster / 128)`).
   - Réécriture de chaque secteur de FAT modifié sur la carte SD pour restituer l'espace de stockage disponible au système.

#### B. Suppression Récursive de Répertoires (`fat32_remove_directory`, `rmdir`)
L'effacement d'un répertoire s'appuie sur un **parcours d'arbre post-fixe (*Post-Order Traversal*)** :
1. **Règle d'or** : Effacer d'abord tous les enfants (fichiers et sous-dossiers), puis supprimer l'entrée du répertoire parent en tout dernier.
2. **Exploration des enfants** :
   - Lecture du secteur de données du sous-dossier (`starting_data_sector + (cluster - 2) * sectors_per_cluster`).
   - Pour chaque entrée valide (hors entrées virtuelles `.` et `..`) :
     - Si l'entrée est un fichier classique (`ARCHIVE`) : Appel de `fat32_remove_file()`.
     - Si l'entrée est un sous-dossier (`DIRECTORY`) : Appel récursif de `fat32_remove_directory()`.
3. **Destruction du dossier courant** :
   - Une fois le contenu vidé à 100 %, l'entrée du dossier dans son parent est marquée à `0xE5` (`DELETED`) et son cluster propre est réinitialisé à `0x00000000` dans la table FAT.

#### C. Optimisations de Performance & Clauses de Garde (*Guard Clauses*)
- **Marqueur `0x00` (*End of Directory*)** :
  - L'apparition de l'octet `0x00` sur le nom d'une entrée garantit la fin absolue du répertoire en FAT32.
  - L'insertion de la clause de garde `if (entries[i].name[0] == EMPTY) break;` interrompt instantanément le parcours sans lire les secteurs vides restants.
- **Marqueur `0xE5` (*Deleted Slot*)** :
  - La clause de garde `if (entries[i].name[0] == DELETED) continue;` ignore les slots précédemment effacés et poursuit la recherche sur les slots suivants.
- **Valeur Sentinelle `0`** :
  - Utilisation de la valeur sentinelle `parent_sector == 0` pour sélectionner par défaut `get_current_directory_sector()`, permettant d'éviter l'écrasement des secteurs lors des appels récursifs profonds.

---

### 3. Synthèse des Fonctions du Pilote

| Fonction | Fichier Source | Description / Rôle |
| :--- | :--- | :--- |
| `fat32_remove_file()` | [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c) | Marque une entrée à `0xE5` et réinitialise sa chaîne de clusters à `0` dans la FAT. |
| `fat32_remove_directory()` | [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c) | Algorithme récursif post-fixe nettoyant le contenu d'un dossier avant sa destruction. |
| `rm()` / `rmdir()` | [kernel/shell.c](file:///home/sacha/Dev/PerryTel/kernel/shell.c) | Commandes Shell validant les arguments et contrôlant l'attribut `DIRECTORY`. |

---

### 4. Étapes futures (Roadmap)

- [ ] **Refactorisation VFS (Branche Git Dédiée)** : Implémentation du polymorphisme par pointeurs de fonctions (`vfs_driver_t`) consigné dans `docs/architecture-vfs-polymorphism-c.md`.
- [ ] **Syscall 63 (`sys_read`)** : Saisie clavier interactive en Mode Utilisateur EL0.
- [ ] **Programmes d'Exécutables Autonomes (`/bin/`)** : Chargement et exécution de binaires ELF indépendants.

---
*Document consigné pour la vault Obsidian - PerryTel OS Project.*
