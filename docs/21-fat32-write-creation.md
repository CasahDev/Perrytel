# Journal de bord - Perrytel OS

## 21. Écriture de Bloc SD (CMD24) & Création de Fichiers FAT32

**Date :** 3 Août 2026  
**Sujet :** Implémentation du pilote d'écriture physique SD (CMD24), allocation dynamique de clusters dans la table FAT, écriture du contenu dans la zone de données et création d'entrées 32 octets dans le Répertoire Racine.

---

### 1. Introduction
Aujourd'hui, 3 Août 2026, PerryTel OS a accompli l'une de ses fonctionnalités les plus importantes : **l'écriture binaire physique sur carte SD et la création de fichiers au format FAT32**. Le système est désormais capable de réserver un cluster libre dans la table FAT, d'y transférer des données utilisateur et d'inscrire l'entrée de répertoire correspondante.

---

### 2. État d'avancement et Architecture Technique

Le mécanisme d'écriture repose sur 3 niveaux d'abstraction :

- **Écriture Physique de Bloc SD (`drivers/sd.c`)** :
  - Implémentation de `sd_write_block()` s'appuyant sur la commande **CMD24** (*WRITE_SINGLE_BLOCK*).
  - Gestion du registre `BLKSIZECNT` (`0x04`), attente de préparation du tampon FIFO `STATUS` (`0x24` - bit 10), écriture des 128 mots 32 bits dans le registre `DATA` (`0x20`), et acquittement d'interruption `INTERRUPT` (`0x30` - bit 1).
- **Allocation de Cluster FAT32 (`drivers/fat32.c`)** :
  - Parours séquentiel des secteurs de la table FAT à la recherche de la première entrée libre `0x00000000`.
  - Inscription du marqueur de fin de fichier `0x0FFFFFFF` (EOF) et réécriture du secteur FAT sur la carte SD avec `sd_write_block()`.
- **Écriture dans la Zone de Données & Répertoire Racine (`drivers/fat32.c`)** :
  - Calcul de l'adresse LBA du secteur de données du cluster alloué : `lba_sector = starting_data_sector + (global_cluster - 2) * sectors_per_cluster`.
  - Écriture du tampon de données 512 octets sur le disque.
  - Recherche d'un logement libre de 32 octets dans le secteur racine (`0x00` ou `0xE5`).
  - Formatage 8.3 du nom (11 caractères complétés par des espaces `0x20`), affectation des bits de cluster fort/faible (`cluster_entry_high` / `cluster_entry_low`), de l'attribut `0x20` et de la taille en octets.
  - Réécriture du secteur du répertoire racine sur la carte SD.

---

### 3. Fichiers et Ressources
- **Pilote SD Écriture** : [drivers/sd.c](file:///home/sacha/Dev/PerryTel/drivers/sd.c), [include/drivers/sd.h](file:///home/sacha/Dev/PerryTel/include/drivers/sd.h).
- **Création de Fichiers FAT32** : [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c), [include/drivers/fat32.h](file:///home/sacha/Dev/PerryTel/include/drivers/fat32.h).
- **Commandes Shell (`ls`, `cat`)** : [kernel/shell.c](file:///home/sacha/Dev/PerryTel/kernel/shell.c).

---

### 4. Bilan et Prochaines Étapes
- [x] **Écriture de Bloc SD (`sd_write_block`)** : Validée.
- [x] **Création de Fichier FAT32 (`fat32_create_file`)** : Implémentée.
- [ ] **Création de la commande Shell `touch`** : Permettre à l'utilisateur de créer dynamiquement des fichiers depuis la console.
- [ ] **Rebase & Fusion de la branche `feat/uart-minitel`** : Intégration du pilote d'interruption Minitel et consolidation de la table des interruptions.

---

*Note générée pour la vault Obsidian - Perrytel OS Project.*
