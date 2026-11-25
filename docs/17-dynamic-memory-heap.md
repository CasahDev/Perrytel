# Journal de bord - Perrytel OS

## 17. Allocateur Mémoire Dynamique (Heap) : Allocation, Splitting et Coalescing

**Date :** 29 Juillet 2026  
**Sujet :** Implémentation complète de la gestion mémoire dynamique (`kalloc`, `kfree`), découpage des blocs libres (*Splitting*), fusion (*Coalescing*) et commande de diagnostic `meminfo`.

---

### 1. Introduction
Aujourd'hui, 29 Juillet 2026, constitue une avancée majeure pour Perrytel OS : au-dessus du système de mémoire virtuelle et de la MMU activée, le noyau est désormais doté d'un **allocateur mémoire dynamique du tas (Heap)** complet. Le noyau dispose d'une gestion granulaire de la mémoire vive avec découpage et fusion des blocs libres.

### 2. État d'avancement et Défis Techniques
L'implémentation du gestionnaire de tas sur liste chaînée a nécessité la résolution de plusieurs défis conceptuels et d'ingénierie mémoire :

- **Positionnement de l'en-tête et arithmétique de pointeurs** :
  L'utilisation d'une arithmétique en octets (`uint8_t*`) a permis d'éviter les pièges de multiplication automatique liés aux pointeurs de structures en C.
- **Découpage des blocs libres (*Splitting*) dans `kalloc`** :
  Lorsqu'une allocation réutilise un grand bloc libre dont le surplus dépasse la taille d'un en-tête (`struct block_meta`) plus un seuil utile, le bloc est découpé en deux. Le premier conserve la taille exacte allouée et le second réintègre la liste des blocs libres avec la taille résiduelle.
- **Fusion dynamique des blocs libres (*Coalescing*) dans `kfree`** :
  Lors de la libération d'une adresse mémoire, une boucle d'absorption parcourt les blocs adjacents. Si le bloc suivant est également marqué comme libre, les blocs sont fusionnés en étendant la taille et en ajustant le pointeur de chaînage, éliminant ainsi la fragmentation externe.
- **Sécurité et protection contre les pointeurs nuls** :
  La condition de boucle dans `kfree` évalue la non-nullité du pointeur de bloc suivant en premier (`next_block != NULL`) pour tirer parti de l'évaluation court-circuit du langage C et prévenir les erreurs de déférencement sur pointeurs nuls.
- **Diagnostic interactif dans le Shell (`meminfo`)** :
  Création d'une commande `meminfo` dans le shell interactif permettant d'inspecter en direct le nombre de blocs, le nombre d'octets alloués et libres.

### 3. Architecture Retenue
- **Structures** : Liste chaînée simplifiée basée sur `struct block_meta` au sommet de la mémoire vive située après le symbole de fin du noyau (`__end`).
- **Interface** : Fonctions noyau `kalloc(size)`, `kfree(ptr)` et commande shell `meminfo`.

### 4. Étapes futures (Roadmap)
Maintenant que la MMU et le gestionnaire de mémoire dynamique sont pleinement opérationnels :
- [ ] **Pilote Carte SD (EMMC / SDHost)** : Implémentation du pilote matériel pour accéder aux secteurs de la carte SD du Raspberry Pi 4.
- [ ] **Système de fichiers (FAT32 / Custom FS)** : Mise en place d'un système de fichiers pour lire et charger des exécutables.
- [ ] **Espaces Mémoire Isolés (Mode Utilisateur EL0)** : Séparation des tables de traduction entre l'espace noyau EL1 et l'espace utilisateur EL0.

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
