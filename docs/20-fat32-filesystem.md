# Journal de bord - Perrytel OS

## 20. Système de Fichiers FAT32 & Décodage du Répertoire Racine (Root Directory)

**Date :** 3 Août 2026  
**Sujet :** Implémentation du décodeur de système de fichiers FAT32, détection automatique de capacité SDHC/SDSC, géométrie du volume BPB et lecture du répertoire racine.

---

### 1. Introduction
Aujourd'hui, 3 Août 2026, PerryTel OS a franchi une étape historique dans sa couche système : la lecture et le décodage effectifs du système de fichiers **FAT32** à partir de la carte SD. Le noyau est désormais capable de lire le secteur MBR 0, de localiser le secteur de démarrage BPB (secteur 2048), de calculer la géométrie complète du volume et de lister les fichiers enregistrés dans le répertoire racine.

---

### 2. État d'avancement et Architecture Technique

Le développement du système de fichiers FAT32 s'appuie sur une architecture logicielle découpée et rigoureuse :

- **Abstraction d'Adressage Universelle (`drivers/sd.c`)** :
  - Détection automatique du type de carte SD via le bit 30 (*Card Capacity Status - CCS*) dans la réponse `RESP0` de la commande `ACMD41`.
  - Conversion dynamique de l'adresse de lecture dans `sd_read_block()` : adressage par numéro de secteur LBA direct pour les cartes SDHC/SDXC (`is_sdhc == 1`), et adressage par octet ($LBA \times 512$) pour les cartes SDSC/QEMU de 64 Mo (`is_sdhc == 0`).
- **Structures C Compactes (`include/drivers/fat32.h`)** :
  - `fat32_boot_sector` : Modélisation exacte de 512 octets du secteur de démarrage (BPB + EBR) munie du qualificatif `__attribute__((packed))`.
  - `FSInfo` : Modélisation des 512 octets de la structure d'informations système.
  - `fat32_dir_entry` : Modélisation exacte de 32 octets pour chaque entrée de fichier ou dossier.
- **Initialisation & Géométrie FAT32 (`drivers/fat32.c`)** :
  - Calcul de l'adresse du premier secteur de la table FAT (`starting_fat_sector = starting_partition_sector + reserved_sector_number`).
  - Calcul de l'adresse de la zone de données (`starting_data_sector = starting_fat_sector + sectors_per_fat * fats_number`).
  - Calcul du secteur du répertoire racine (*Cluster 2*) : `root_sector = starting_data_sector + (root_directory_cluster - 2) * sectors_per_cluster`.
- **Parsing du Répertoire Racine** :
  - Parcours des entrées de 32 octets du répertoire racine.
  - Filtrage des marqueurs d'entrées vides (`0x00`), d'entrées supprimées (`0xE5`) et d'entrées de noms longs LFN (`attributes == 0x0F`).
  - Affichage propre des 11 caractères du nom de fichier sur la console UART via `uart_putc()`.

---

### 3. Fichiers et Ressources
- **En-tête FAT32** : [include/drivers/fat32.h](file:///home/sacha/Dev/PerryTel/include/drivers/fat32.h).
- **Implémentation du Système de Fichiers** : [drivers/fat32.c](file:///home/sacha/Dev/PerryTel/drivers/fat32.c).
- **Pilote de Bloc SD** : [drivers/sd.c](file:///home/sacha/Dev/PerryTel/drivers/sd.c), [include/drivers/sd.h](file:///home/sacha/Dev/PerryTel/include/drivers/sd.h).

---

### 4. Validation et Prochaines Étapes
- [x] **Décodage du Répertoire Racine (Root Directory)** : Affichage réussi du nom de fichier `TEST    TXT` sur la console UART sous QEMU.
- [ ] **Lecture du Contenu d'un Fichier** : Suivi de la chaîne de clusters dans la table FAT pour lire le contenu textuel d'un fichier en mémoire.
- [ ] **Commandes Shell de Fichiers (`ls`, `cat`)** : Intégration du système de fichiers au terminal PerryTel OS pour permettre l'exploration interactive par l'utilisateur.

---

*Note générée pour la vault Obsidian - Perrytel OS Project.*
