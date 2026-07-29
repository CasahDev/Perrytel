# Journal de bord - Perrytel OS

## 16. Configuration et Activation de la MMU (AArch64 / Raspberry Pi 4)

**Date :** 29 Juillet 2026  
**Sujet :** Activation de la MMU (Memory Management Unit), pagination et attribution des zones mémoire et périphériques.

---

### 1. Introduction
Aujourd'hui, 29 Juillet 2026, marque la finalisation d'un jalon fondamental de Perrytel OS sur architecture AArch64 (Raspberry Pi 4) : la mise sous tension de la **MMU** (Memory Management Unit) et l'activation des caches d'instructions et de données. Le noyau s'exécute désormais avec un système d'adressage virtuel sécurisé et performant.

### 2. État d'avancement et Défis Techniques
L'implémentation du sous-système MMU a nécessité une rigueur absolue dans la manipulation des registres système ARMv8-A et le découpage des tables de traduction d'adresses. Plusieurs défis majeurs ont été identifiés et résolus :

- **Correction de la table racine (TTBR0_EL1)** :
  La table racine de niveau 1 (L1) a été rattachée au registre `TTBR0_EL1` en lui transmettant l'adresse mémoire physique de l'instance `L1`, permettant au matériel de démarrer correctement sa marche dans les tables (*table walk*).
- **Protection du noyau et du premier bloc mémoire (L2[0])** :
  Une erreur initiale consistait à invalider l'index 0 de la table L2. Le code du noyau (`kernel8.img`), la pile et les vecteurs d'exceptions se situant à l'adresse `0x80000` (dans les premiers 2 Mo), le premier bloc a été réintégré comme zone mémoire normale avec accès complet.
- **Cartographie du 4ème Gigaoctet pour le MMIO du Raspberry Pi 4** :
  L'UART0 et le GIC du Raspberry Pi 4 se trouvant à l'adresse `0xFE000000`, la tranche de 3 Go à 4 Go a été mappée au niveau L1 (`L1.pages[3]`) avec l'adresse de base `0xC0000000`, le descripteur de bloc de 1 Go, le bit d'accès et l'attribut **Device nGnRnE** (slot 1 du MAIR).
- **Invalider le TLB et maîtriser la synchronisation matérielle** :
  Une séquence stricte à 4 temps a été mise en place avec la barrière `DSB SY`, l'invalidation complète du TLB (`tlbi VMALLE1IS`), une deuxième barrière `DSB SY` et la barrière d'instructions `ISB`.
- **Activation matérielle sécurisée (SCTLR_EL1)** :
  L'activation de la MMU (bit 0), du cache de données (bit 2) et du cache d'instructions (bit 12 avec décalage `1 << 12`) a été réalisée via une séquence **Read-Modify-Write** (`mrs` / `or` / `msr`) sur `SCTLR_EL1` afin d'éviter d'écraser les autres registres de configuration d'architecture.

### 3. Architecture Retenue
Le système de pagination repose sur une topologie claire :
- **TCR_EL1** : Granule de 4 Ko, espace virtuel de 32 bits (T0SZ=32), espace physique de 40 bits.
- **MAIR_EL1** : Slot 0 = Normal Cacheable (`0xFF`), Slot 1 = Device nGnRnE (`0x00`).
- **L1 & L2** : 1er Gigaoctet mappé par blocs de 2 Mo (mémoire RAM normale) et 4ème Gigaoctet mappé en bloc de 1 Go (zone périphérique Device).

### 4. Étapes futures (Roadmap)
La MMU et les caches étant entièrement opérationnels et validés au runtime sous QEMU, la suite du développement va pouvoir s'appuyer sur la mémoire virtuelle :
- [ ] **Gestionnaire de Tas (Heap Allocator / kalloc)** : Perfectionnement de l'allocateur dynamique de mémoire noyau.
- [ ] **Isolation Kernel/User (EL1 / EL0)** : Mise en place des espaces d'adresses virtuels étanches pour les processus utilisateurs.
- [ ] **Système de fichiers & Pilotes** : Poursuite de l'intégration du stockage et des pilotes matériels.

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
