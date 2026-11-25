# Architecture Système - Perrytel OS

## Conception du VFS (Virtual File System) & Polymorphisme Orienté Objet en C Pur

**Date :** 11 Août 2026  
**Sujet :** Abstraction des pilotes de système de fichiers (FAT32, exFAT, ext2, NTFS), élimination des `switch/case` par tables de pointeurs de fonctions (*VTABLE*) et modèle de conception orienté objet sans C++.

---

### 1. Introduction & Objectif d'Architecture

Dans la conception d'un noyau de système d'exploitation de classe industrielle, la couche de contrôle utilisateur (Shell) et les interfaces d'appels système (**Syscalls**) ne doivent jamais dépendre d'un pilote matériel ou d'un format de système de fichiers spécifique (comme FAT32).

L'objectif de cette architecture est de mettre en place un **Système de Fichiers Virtuel (VFS)** permettant d'interchanger dynamiquement les pilotes de stockage (FAT32, exFAT, ext2, RAMDisk) sans utiliser le moindre branchement `switch/case`, en s'appuyant sur le polymorphisme par **structures de pointeurs de fonctions** (l'équivalent des interfaces et méthodes virtuelles pures de C++).

---

### 2. Le Modèle de Conception : Table de Méthodes Virtuelles (*Operations Struct*)

#### A. Le Contrat d'Interface (`vfs_driver_t`)
L'interface générique du système de fichiers est définie par une structure C contenant la liste des signatures de fonctions virtuelles obligatoires pour tout pilote :

- **Pointeur d'initialisation / montage** : `mount`
- **Pointeurs d'entrées/sorties de fichiers** : `read_file`, `create_file`, `remove_file`
- **Pointeurs de gestion de répertoires** : `create_dir`, `remove_dir`, `list_dir`
- **Pointeurs de navigation** : `change_dir`

```c
typedef struct vfs_driver {
    const char *name;
    int (*read_file)(const char *path, char *buffer);
    int (*create_file)(const char *path, const char *content);
    int (*create_dir)(const char *path);
    int (*remove_file)(const char *path);
    int (*remove_dir)(const char *path);
    int (*list_dir)(const char *path);
} vfs_driver_t;
```

#### B. L'Enregistrement des Pilotes Concrets (FAT32, exFAT, ext2...)
Chaque pilote de stockage implémente les fonctions correspondant à sa propre géométrie disque et instancie une structure constante globale :

1. **Pilote FAT32 ([drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c))** :
   - Associe `.read_file = fat32_read_file`
   - Associe `.create_dir = fat32_create_directory`
   - Associe `.remove_dir = fat32_remove_directory`

2. **Futur Pilote exFAT / ext2 (`drivers/exfat.c`)** :
   - Associe `.read_file = exfat_read_file`
   - Associe `.create_dir = exfat_create_directory`
   - Associe `.remove_dir = exfat_remove_directory`

---

### 3. Exécution Polymorphe Sans Branchement (`switch/case`)

Le noyau conserve une référence vers le système de fichiers actif :

$$\text{vfs\_driver\_t *active\_fs} = \&\text{fat32\_driver};$$

Lorsqu'une commande Shell (`cat`, `ls`, `mkdir`, `rm`) ou un Syscall en Mode Utilisateur (EL0) demande une opération sur le système de fichiers :

1. **Appel indirect transparent** :
   Le noyau ou le Shell appelle directement le pointeur de fonction :
   $$\text{active\_fs}\rightarrow\text{read\_file}(\text{chemin}, \text{tampon});$$
2. **Exécution matérielle** :
   Le processeur lit l'adresse mémoire de la fonction et saute directement dans le code du pilote enregistré sans effectuer la moindre comparaison ou instruction `if` / `switch`.
3. **Changement de système de fichiers à la volée** :
   Pour basculer la partition active vers un autre format de disque, une seule assignation de pointeur suffit :
   $$\text{active\_fs} = \&\text{exfat\_driver};$$

---

### 4. Avantages d'Ingénierie pour PerryTel OS

- **Découplage Total (Encapsulation)** : Le Shell et les Syscalls n'ont pas accès aux détails matériels (secteurs LBA, clusters, masques `0xE5` ou `0x10`).
- **Extensibilité Infinie (Principe Open/Closed)** : L'ajout d'un nouveau pilote de système de fichiers n'exige la modification d'aucune ligne de code dans le Shell ou les Syscalls.
- **Performance Maximale** : Le saut par pointeur de fonction est exécuté en un seul cycle d'instruction indirecte (`blr xN` en assembleur ARM64).

---

### 5. Roadmap de Refactorisation VFS

- [ ] Création du header d'interface générique `include/kernel/vfs.h`.
- [ ] Raccordement du pilote `fat32_driver` à la structure VFS.
- [ ] Redirection des commandes Shell (`ls`, `cat`, `mkdir`, `cd`, `rm`) vers les pointeurs de l'interface `active_fs`.
- [ ] Isolation de la gestion des chemins d'accès absolus (`/dossier1/dossier2`).

---
*Fiche d'architecture spécifique consignée pour la vault Obsidian - PerryTel OS Project.*
