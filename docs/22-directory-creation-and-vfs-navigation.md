# Journal de bord - Perrytel OS

## 22. Création de Répertoires FAT32, Navigation VFS (`mkdir`, `cd`) et Gestion Mémoire

**Date :** 10 Août 2026  
**Sujet :** Implémentation du système de répertoires hiérarchiques FAT32 (`mkdir`, `cd`), allocation dynamique de clusters, correction des fuites mémoire et isolation de la bibliothèque C utilisateur (`perrylib`).

---

### 1. Introduction

Cette étape marque la transition de PerryTel OS d'un système de fichiers plat vers un **système de fichiers virtuel hiérarchique (VFS)** complet. Le noyau gère désormais l'allocation dynamique multi-secteurs dans la table FAT32, la création de dossiers physiques (`mkdir`), la navigation dans l'arborescence (`cd`, `cd ..`), et l'affichage dynamique du répertoire courant dans l'invite de commande du Shell (`print_shell`).

---

### 2. État d'avancement et Architecture Technique

Plusieurs avancées majeures ont été réalisées et validées :

#### A. Allocation Dynamique de Clusters Multi-Secteurs (`fat32_allocate_cluster`)
- **Principe de géométrie FAT32** : Chaque secteur de 512 octets de la table FAT contient 128 entrées de 32 bits (4 octets par entrée).
- **Parcours universel** : Implémentation du parcours multi-secteurs à l'aide de la formule mathématique :
  $$\text{Cluster Global} = (\text{Secteur FAT} \times 128) + \text{Index Relatif}$$
- **Prévention du débordement** : Typage des compteurs de secteur et de cluster en `uint32_t` pour garantir la compatibilité avec les cartes SD jusqu'à 32 Gio sans risque d'overflow 8 bits.
- **Réservation** : Marquage automatique du cluster alloué avec la constante de fin de chaîne `0x0FFFFFFF`.

#### B. Création de Répertoires FAT32 (`fat32_create_directory`, `mkdir`)
Chaque création de dossier implique une écriture à deux emplacements matériels distincts :
1. **Secteur du Nouveau Dossier** :
   - Écriture des deux entrées obligatoires de 32 octets : `.` (*Dot*, pointant vers le cluster du dossier lui-même) et `..` (*Dot-Dot*, pointant vers le cluster du dossier parent ou `0` si le parent est la racine).
   - Inscription de l'attribut de répertoire `0x10` (`ATTR_DIRECTORY`).
2. **Secteur du Dossier Parent** :
   - Inscription d'une nouvelle structure de 32 octets dans le cluster courant.
   - Formatage 8.3 du nom de dossier complété par des espaces.
   - Fixation de la taille d'entrée à 0 (conformément aux exigences des spécifications FAT32).

#### C. Navigation Arborescente (`cd`, `cd ..`) et Prompt Dynamique
- **Gestionnaire d'état du pilote (`drivers/fat32.c`)** :
  - Centralisation de la variable d'état `current_directory_cluster` (initialisée à 2 pour le répertoire racine).
  - Implémentation des fonctions d'accès `get_current_directory_cluster()` et `get_current_directory_sector()`.
- **Validation des attributs dans `cd`** :
  - Vérification obligatoire du masque d'attribut `entry->attributes & 0x10` pour interdire la navigation dans des fichiers texte classiques.
- **Support de `cd ..`** :
  - Extraction du cluster parent depuis l'entrée `..` et réaffectation vers le cluster racine `2` lorsque la valeur retournée est `0`.
- **Suivi du chemin texte (`current_path`) dans le Shell** :
  - Maintien de la chaîne `current_path` dans `shell.c`.
  - Élagage dynamique lors des retours en arrière (`cd ..`) et concaténation lors des descentes (`cd docs`).
  - Affichage instantané du prompt dans `print_shell()` sans latence de lecture SD : `Perry (/docs) -> `.

#### D. Isolation et Sécurité Mémoire (MMU & Heap Audit)
- **Déplacement dans `.user_text`** :
  - Déclaration explicite d'attribut `__attribute__((section(".user_text")))` sur l'ensemble des fonctions de `user/string.c` et `user/perrylib.c`.
  - Prévention des fautes d'exécution MMU (*Instruction Abort / Permission Fault*) lors des sauts depuis le mode EL0.
- **Assainissement du Tas (*Heap*)** :
  - Correction des fuites de mémoire dans `sd_init()`, `sys_exit_handler()` et la commande `cat()`.
  - Validation par la commande `meminfo` : la mémoire occupée reste stable à 4096 octets (pile utilisateur).

---

### 3. Synthèse des Modificateurs et Fonctions Majeures

| Composant | Fichier Source | Description / Rôle |
| :--- | :--- | :--- |
| `fat32_allocate_cluster()` | [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c) | Recherche multi-secteurs et réservation de cluster dans la table FAT. |
| `fat32_create_directory()` | [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c) | Initialisation du secteur d'un dossier (`.`, `..`) et enregistrement dans le parent. |
| `get_current_directory_sector()` | [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c) | Conversion du cluster courant en secteur LBA physique pour `sd_read_block`. |
| `cd()` / `mkdir()` | [kernel/shell.c](file:///home/sacha/Dev/PerryTel/kernel/shell.c) | Routage des commandes utilisateur et gestion du chemin `current_path`. |
| `perrylib` / `user_string` | [user/perrylib.c](file:///home/sacha/Dev/PerryTel/user/perrylib.c) | Bibliothèque C utilisateur autonome compilée dans la section `.user_text` (`0x200000`). |

---

### 4. Étapes futures (Roadmap)

- [ ] **Syscall 63 (`sys_read`)** : Implémentation de la saisie utilisateur depuis le mode EL0 via interruptions UART.
- [ ] **Exécutables Autonomes en `/bin/`** : Compilation de binaires indépendants chargés en mémoire utilisateur.
- [ ] **Suppression de Fichiers / Répertoires (`rm`, `rmdir`)** : Libération des clusters dans la FAT et marquage à `0xE5`.
- [ ] **Support VFS Complet (`cd /chemin/absolu`)** : Résolution de chemins composés complexes par découpage `/`.

---
*Document consigné pour la vault Obsidian - PerryTel OS Project.*
