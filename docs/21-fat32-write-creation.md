# Journal de bord - Perrytel OS

## 21. Écriture de Bloc SD (CMD24) & Création de Fichiers FAT32 (`touch`, `cat`, `ls`)

**Date :** 3 Août 2026  
**Sujet :** Implémentation du pilote d'écriture physique SD (CMD24), allocation dynamique de clusters dans la table FAT, écriture du contenu dans la zone de données, création d'entrées 32 octets dans le Répertoire Racine et intégration des commandes Shell (`ls`, `cat`, `touch`).

---

### 1. Introduction
Aujourd'hui, 3 Août 2026, PerryTel OS a accompli l'une de ses fonctionnalités les plus importantes : **l'écriture binaire physique sur carte SD et la création de fichiers au format FAT32**. Le système est désormais capable de réserver un cluster libre dans la table FAT, d'y transférer des données utilisateur, d'inscrire l'entrée de répertoire 8.3 correspondante, et de relire les fichiers créés avec les commandes `ls` et `cat`.

---

### 2. État d'avancement et Architecture Technique

Le mécanisme d'écriture et d'exploration repose sur 4 niveaux d'abstraction :

- **Drapeau de Compilation Noyau (`build.sh`)** :
  - Ajout de l'option `-mgeneral-regs-only` aux `EXTRA_CFLAGS` de GCC AArch64 pour interdire l'auto-vectorisation SIMD/NEON dans le noyau et éviter les pièges d'accès registres `CPACR_EL1` (ESR `0x1FE00000`).
- **Écriture Physique de Bloc SD (`drivers/sd.c`)** :
  - Implémentation de `sd_write_block()` s'appuyant sur la commande **CMD24** (*WRITE_SINGLE_BLOCK*).
  - Gestion du registre `BLKSIZECNT` (`0x04`), attente de préparation du tampon FIFO `STATUS` (`0x24` - bit 10), écriture des 128 mots 32 bits dans le registre `DATA` (`0x20`), et acquittement d'interruption `INTERRUPT` (`0x30` - bit 1).
- **Allocation de Cluster FAT32 & Création de Fichiers (`drivers/fat32.c`)** :
  - Parcours séquentiel des secteurs de la table FAT à la recherche de la première entrée libre `0x00000000`.
  - Inscription du marqueur de fin de fichier `0x0FFFFFFF` (EOF) et réécriture du secteur FAT sur la carte SD avec `sd_write_block()`.
  - Écriture du tampon de données 512 octets à l'adresse LBA : `starting_data_sector + (global_cluster - 2) * sectors_per_cluster`.
  - Recherche d'un logement libre de 32 octets dans le secteur racine (`0x00` ou `0xE5`).
  - Formatage 8.3 du nom (11 caractères complétés par des espaces `' '` / `0x20`), affectation des bits de cluster fort/faible (`cluster_entry_high` / `cluster_entry_low`), de l'attribut `0x20` et de la taille en octets.
  - Réécriture du secteur du répertoire racine sur la carte SD.
- **Commandes Shell (`kernel/shell.c`)** :
  - `ls` : Lecture et affichage des entrées validées du répertoire racine.
  - `cat` : Recherche par nom propre (`fat32_get_name`) et affichage du contenu textuel.
  - `touch` : Invoque `fat32_create_file` avec prise en charge des guillemets dans `strsplit()`.

---

### 3. Fichiers et Ressources
- **Pilote SD Écriture** : [drivers/sd.c](file:///home/sacha/Dev/PerryTel/drivers/sd.c), [include/drivers/sd.h](file:///home/sacha/Dev/PerryTel/include/drivers/sd.h).
- **Création de Fichiers FAT32** : [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c), [include/drivers/fat32.h](file:///home/sacha/Dev/PerryTel/include/drivers/fat32.h).
- **Commandes Shell (`ls`, `cat`, `touch`)** : [kernel/shell.c](file:///home/sacha/Dev/PerryTel/kernel/shell.c).
- **Options de Compilation** : [build.sh](file:///home/sacha/Dev/PerryTel/build.sh).

---

### 4. Bilan et Prochaines Étapes
- [x] **Écriture de Bloc SD (`sd_write_block`)** : Validée.
- [x] **Création de Fichier FAT32 (`fat32_create_file`)** : Implémentée et validée (`FICHIER.TXT`).
- [x] **Commandes Shell (`ls`, `cat`, `touch`)** : Fonctionnelles.
- [ ] **Rebase & Fusion de la branche `feat/uart-minitel`** : Intégration du pilote d'interruption Minitel et consolidation de la table des interruptions.
- [ ] **Couche de répertoires (`cd`, `mkdir`)** : Support des sous-dossiers VFS.

---

*Note générée pour la vault Obsidian - Perrytel OS Project.*
