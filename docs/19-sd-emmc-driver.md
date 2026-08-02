# Journal de bord - Perrytel OS

## 19. Pilote Matériel de Carte SD et Contrôleur EMMC2 (BCM2711 / Raspberry Pi 4)

**Date :** 30 Juillet 2026  
**Sujet :** Développement du pilote de bloc physique EMMC2, négociation de bus 400 kHz / 25 MHz, séquence des commandes SD (CMD0 à CMD17) et transfert de secteurs de 512 octets.

---

### 1. Introduction
Aujourd'hui, 30 Juillet 2026, PerryTel OS a franchi une étape majeure dans la gestion du stockage matériel : le développement et la validation complète du pilote de bloc pour le contrôleur **EMMC2** du Broadcom BCM2711 (Raspberry Pi 4). Le système d'exploitation est désormais capable d'initialiser une carte micro-SD physique, d'ajuster dynamiquement l'horloge du bus et d'effectuer des lectures de secteurs de 512 octets en mémoire RAM.

---

### 2. État d'avancement et Architecture Technique

Le développement du pilote a été découpé en plusieurs phases d'ingénierie matérielle et MMIO :

- **Abstraction MMIO Centralisée (`kernel/mmio.h`)** :
  Mise en place d'une interface d'accès mémoire `static inline` centralisée (`mmio_read` / `mmio_write`) garantissant zéro surcoût d'appel et une visibilité globale pour l'ensemble des pilotes du noyau (UART, GIC, EMMC2).
- **Réinitialisation Matérielle et Négociation d'Horloge (`CONTROL1`)** :
  - Exécution du reset logiciel global du contrôleur EMMC2 via le bit `GLOBAL_RESET` (`1 << 24`).
  - Configuration de l'horloge lente initiale de 400 kHz via le diviseur `0x7C << 8` pour la phase de découverte SD.
  - Basculement dynamique à la haute fréquence de 25 MHz via le diviseur `2 << 8` une fois la carte en état de transfert.
- **Séquence des Commandes SD (`drivers/sd.c`)** :
  - **CMD0 (`GO_IDLE_STATE`)** : Passage de la carte SD en état de repos.
  - **CMD8 (`SEND_IF_COND`)** : Négociation du support de la tension 3.3V (`0x1AA`) et confirmation de la spécification SD v2.0+.
  - **CMD55 / ACMD41 (`SD_SEND_OP_COND`)** : Boucle d'initialisation et d'alimentation avec le masque `0x40FF8000` (support SDHC/SDXC haute capacité).
  - **CMD2 & CMD3 (`ALL_SEND_CID` / `SEND_RELATIVE_ADDR`)** : Lecture du numéro d'identification usine et récupération de l'adresse relative de carte (**RCA**).
  - **CMD7 (`SELECT_CARD`)** : Sélection de la carte via son adresse RCA pour passer en mode *Transfert de Données*.
  - **CMD16 (`SET_BLOCKLEN`)** : Verrouillage de la taille des secteurs de données à 512 octets.
- **Transfert de Secteurs (`sd_read_block`)** :
  - Émission de la commande **CMD17 (`READ_SINGLE_BLOCK`)**.
  - Attente de la disponibilité du tampon FIFO via le bit 11 (*Read Transfer Ready*) du registre `STATUS` (`0x24`).
  - Transfert des 512 octets (128 mots de 32 bits) depuis le registre `DATA` (`0x20`) vers la mémoire RAM du noyau.
  - Attente et acquittement du bit 1 (*Data Transfer Done*) dans le registre `INTERRUPT` (`0x30`) via le masque `0x2`.

---

### 3. Ressources et Fichiers Clés
- **Pilote et Définitions** : [drivers/sd.c](file:///home/sacha/Dev/PerryTel/drivers/sd.c), [include/drivers/sd.h](file:///home/sacha/Dev/PerryTel/include/drivers/sd.h).
- **Interface MMIO** : [include/kernel/mmio.h](file:///home/sacha/Dev/PerryTel/include/kernel/mmio.h).
- **Emplacement Matériel MMIO** : `0xFE300000` (Contrôleur SDHCI principal du Broadcom BCM2711, raccordé sous QEMU et sur le matériel, configuré en Device nGnRnE dans la MMU).
- **Configuration des Masques d'Interruptions** : Activation complète des voyants de statut et de signalement aux registres `0x34` (`IRPT_MASK`) et `0x38` (`IRPT_EN`) avec la valeur `0xFFFFFFFF`.
- **Lecture de Secteurs et Partitionnement MBR/FAT32** :
  - Configuration du registre de taille de bloc `0x04` (`BLKSIZECNT`) avec la valeur `(1 << 16) | 512` (`0x00010200`).
  - Ajout des drapeaux de transfert de données `1 << 21` (*Data Present*) et `1 << 4` (*Read Direction*) sur la commande `CMD17`.
  - Lecture réussie du secteur 0 MBR, extraction du type de partition `0x0C` (FAT32 LBA) et lecture du secteur de démarrage FAT32 (secteur 2048).

---

### 4. Validation et Prochaines Étapes
Le pilote de bloc SDHCI/EMMC est **100 % opérationnel et validé sous QEMU**. La valeur hexadécimale `0x0C` extraite de la table de partitions MBR a été imprimée avec succès sur le terminal UART.

- [x] **Décodeur de Table de Partitions (MBR)** : Lecture du secteur 0 et extraction du secteur de démarrage de la partition FAT32 (secteur 2048).
- [ ] **Pilote de Système de Fichiers (FAT32)** : Décodage du secteur de démarrage BPB (Volume Boot Record), du répertoire racine (Root Directory) et des clusters.
- [ ] **Commandes Shell de Fichiers (`ls`, `cat`)** : Permettre à l'utilisateur de lister et de lire le contenu des fichiers de la carte SD depuis le terminal PerryTel OS.

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
