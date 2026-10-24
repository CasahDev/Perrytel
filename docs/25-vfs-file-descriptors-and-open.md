# Journal de bord - Perrytel OS

## 25. VFS : Table des Descripteurs (fd), Syscall open() et Streaming I/O

**Date :** 24 Octobre 2026  
**Sujet :** Refactorisation modulaire des pilotes (FAT32, UART, Syscalls), modélisation des descripteurs de fichiers VFS (`struct vfs_file`, `fd_list`), implémentation du Syscall `SYS_OPEN` (56), streaming de lecture par morceaux (`fat32_read`), et architecture du décodeur clavier Minitel vers `STDIN`.

---

### 1. Introduction

Cette étape marque la transition fondamentale de PerryTel OS vers le modèle d'ingénierie UNIX : **l'unification des entrées/sorties sous l'abstraction d'un Système de Fichiers Virtuel (VFS)**. 

Afin de permettre l'exécution future de binaires autonomes en **Mode Utilisateur (EL0)** stockés dans `/bin/`, le noyau s'émancipe de la lecture de fichiers en bloc monolithique pour adopter le mécanisme standard des **Descripteurs de Fichiers (*File Descriptors - fd*)** et de la lecture en flux séquentiel (*Streaming I/O*).

---

### 2. Architecture Technique & Modularisation

#### A. Découpage Modulaire des Pilotes
Le pilote FAT32 et les gestionnaires de Syscalls ont été scindés en sous-modules spécialisés afin d'améliorer la maintenabilité et de respecter le principe de responsabilité unique :
- **FAT32 (`drivers/fat32/`)** :
  - `fat32_init.c` : Géométrie du volume et analyse MBR/Boot Sector.
  - `fat32_state.c` : Variables statiques d'état et getters/setters de secteur.
  - `fat32_alloc.c` : Allocation de clusters libres dans la table FAT.
  - `fat32_file.c` : Création, suppression et streaming de fichiers (`fat32_open`, `fat32_read`).
  - `fat32_dir.c` : Navigation arborescente (`fat32_change_dir`, `fat32_create_directory`).
  - `fat32_vfs.c` : Instanciation de l'interface polymorphique `fat32_driver`.
- **Syscalls (`kernel/syscalls/`)** :
  - `uart_syscalls.c` : Gestionnaires d'E/S console.
  - `file_syscalls.c` : Gestionnaires `sys_open`, `sys_read` et table des descripteurs.
  - `exit_syscall.c` : Gestionnaire de terminaison de processus `sys_exit`.

---

### 3. Modélisation VFS & Table des Descripteurs (`fd`)

#### A. La Structure `vfs_file` (`include/kernel/vfs.h`)
Chaque fichier ou périphérique ouvert est représenté par une structure d'état en mémoire noyau :
```c
enum fd_type {
    STDIN,
    STDOUT,
    FILE
};

typedef struct vfs_file {
    bool is_open;
    const struct vfs_driver *driver;
    enum fd_type type;
    uint64_t private_id;  // Premier cluster FAT32 ou handle périphérique
    uint64_t size;        // Taille totale du fichier en octets
    uint64_t offset;      // Curseur de lecture/écriture courant
} vfs_file;
```

#### B. La Table des Descripteurs `fd_list` (`kernel/syscalls/file_syscalls.c`)
- Un tableau statique de 32 entrées (`vfs_file fd_list[32]`) stocké dans la mémoire noyau.
- **`fd = 0` (STDIN)** : Réservé et configuré avec `driver = &uart_vfs_driver`.
- **`fd = 1 / 2` (STDOUT / STDERR)** : Réservés pour la console.
- **`fd >= 3`** : Attribués dynamiquement aux fichiers physiques de la carte SD lors des appels `open()`.

---

### 4. Le Cycle de Vie des E/S : `open()` et `read()` par Flux

#### A. L'Appel Système `open()` (Syscall 56)
1. Le programme utilisateur appelle `open("FICHIER.TXT")`.
2. Le gestionnaire noyau `sys_open()` alloue une case libre via `get_free_fd()`.
3. Le pilote FAT32 extrait les métadonnées (`size` et cluster initial `private_id`) sans charger le contenu en mémoire.
4. La case est marquée `is_open = true`, `offset = 0`, et le descripteur `fd` entier est renvoyé à l'EL0.

#### B. La Lecture par Morceaux `fat32_read()` (`drivers/fat32/fat32_file.c`)
La fonction lit les données bloc par bloc (512 octets) directement sur la carte SD :
- **Calcul LBA dynamique** : Recalculé à chaque itération à partir de `base_sector + (file->offset / 512)`.
- **Gestion des fractions de blocs (*Chunks*)** :
  - Détermination du reste du fichier : `remaining = file->size - file->offset`.
  - Bornage automatique de la taille du morceau pour ne jamais dépasser ni la fin du fichier, ni la limite `count` demandée par l'application :
    $$\text{chunk} = \min(\text{remaining}, 512, \text{count} - \text{bytes\_read})$$
- **Progression d'Offset & POSIX Compliance** : Mise à jour synchronisée de `file->offset` et de `bytes_read`. La fonction renvoie le nombre d'octets réellement transférés lors de l'appel (et `0` à la fin de fichier).

---

### 5. Intégration du Décodeur Clavier Minitel (`kernel/keyboard.c`)

Pour unifier la lecture clavier sous l'interface VFS (`fd = 0`), le traitement est divisé en deux couches :
1. **Couche Matérielle (`drivers/uart/uart.c`)** : L'interruption UART dépose les octets bruts dans le tampon circulaire `rx_buffer`.
2. **Couche Décodeur / Machine à États (`kernel/keyboard.c`)** :
   - Dépile les octets de `rx_buffer`.
   - Intercepte l'octet d'échappement Minitel `0x13`.
   - Traduit la séquence `0x13 0x41` (Touche *Envoi*) en saut de ligne `\n`.
   - Traduit la séquence `0x13 0x47` (Touche *Correction*) en retour arrière `\b`.
3. **Couche VFS (`uart_vfs_driver.read`)** : Fait appel au décodeur clavier pour alimenter le tampon utilisateur lors d'un `readf(0, buffer, count)`.

---

### 6. Synthèse des Fichiers et Rôles

| Composant | Fichier Source | Description |
| :--- | :--- | :--- |
| `vfs_file`, `vfs_driver` | [include/kernel/vfs.h](file:///home/sacha/Dev/PerryTel/include/kernel/vfs.h) | Définitions de l'abstraction VFS et de la structure de descripteur. |
| `fd_list`, `sys_open`, `sys_read` | [kernel/syscalls/file_syscalls.c](file:///home/sacha/Dev/PerryTel/kernel/syscalls/file_syscalls.c) | Gestion de la table des fichiers ouverts et aiguillage Syscall. |
| `fat32_open`, `fat32_read` | [drivers/fat32/fat32_file.c](file:///home/sacha/Dev/PerryTel/drivers/fat32/fat32_file.c) | Recherche d'entrée et streaming physique de secteurs SD. |
| `uart_vfs_driver` | [drivers/uart/uart.c](file:///home/sacha/Dev/PerryTel/drivers/uart/uart.c) | Pilote de flux pour la console série UART. |
| `open()`, `readf()` | [user/perrylib.c](file:///home/sacha/Dev/PerryTel/user/perrylib.c) | Interface de programmation en espace utilisateur (EL0). |

---

### 7. Liste des Tâches pour la Prochaine Session (TODO List)

- [ ] **Décodeur Clavier Minitel (`kernel/keyboard.c`) vers STDIN** :
  - Transformer `keyboard_update()` en fonction d'extraction (`keyboard_get_char()` ou `keyboard_read()`) qui retourne les caractères ASCII décodés.
  - Lier cette fonction au pointeur `.read` de `uart_vfs_driver` dans `drivers/uart/uart.c`.
- [ ] **Finalisation du Câblage Noyau pour `SYS_READ`** :
  - Ajouter l'appel à `init_fd()` dans `kernel_main()` (`kernel/main.c`).
  - Remplacer `register_syscall_handler(SYS_READ, uart_read)` par `register_syscall_handler(SYS_READ, sys_read)` dans `kernel/main.c`.
- [ ] **Implémentation du Syscall `SYS_CLOSE` (57)** :
  - Définir `SYS_CLOSE` dans `include/perrylib.h` et implémenter `sys_close()` pour réinitialiser `fd_list[fd].is_open = false`.
- [ ] **Chargeur d'Exécutables Autonomes (`/bin/`)** :
  - Ajouter une règle de compilation dans `build.sh` pour compiler un programme utilisateur autonome (ex: `apps/cat.c`) lié à l'adresse virtuelle `0x200000`.
  - Mettre à jour `exec()` dans `kernel/shell.c` pour charger le binaire depuis `/bin/`, initialiser la pile utilisateur et basculer en EL0 via `switch_to_user_mode()`.

---
*Document consigné pour la vault Obsidian - PerryTel OS Project.*
