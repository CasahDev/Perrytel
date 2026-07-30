# Journal de bord - Perrytel OS

## 18. Transition et Isolation du Mode Utilisateur (AArch64 / EL0)

**Date :** 30 Juillet 2026  
**Sujet :** Transition du niveau d'exception EL1 vers EL0, isolation mémoire MMU (AP bits), routine `eret` et gestion transparente des IRQs.

---

### 1. Introduction
Aujourd'hui, 30 Juillet 2026, marque l'accomplissement d'un jalon d'architecture fondamental pour Perrytel OS : le processeur est désormais capable d'effectuer un basculement matériel sécurisé du niveau privilège **EL1 (Noyau)** vers le niveau non-privilégié **EL0 (Mode Utilisateur)**, tout en maintenant l'isolation mémoire et la gestion des interruptions IRQ sous AArch64.

### 2. État d'avancement et Défis Techniques
La mise en place de la transition d'espace d'exécution a soulevé plusieurs défis d'ingénierie système et de sécurité MMU qui ont tous été résolus :

- **Routine d'assemblage du saut (`switch_to_user_mode`) dans `user_mode.S`** :
  Création de la fonction d'assemblage configurant le registre de statut `SPSR_EL1` sur le mode **EL0t** (valeur `0x000` avec IRQs démasquées), l'adresse de retour `ELR_EL1` avec le point d'entrée du programme utilisateur, et le pointeur de pile utilisateur `SP_EL0`. La transition s'effectue via l'instruction `eret`.
- **Gestion des permissions d'accès MMU (Attributs AP)** :
  Configuration des descripteurs de la table `L2` dans `mmu.c`. Le premier bloc de 2 Mo (`L2.pages[0]`) hébergeant le code du noyau reste strictement réservé au niveau EL1 (`AP = 00`) pour satisfaire la sécurité matérielle PAN (*Privileged Access Never*). Les blocs de RAM suivants (`L2.pages[1]` et au-delà) reçoivent le dromadaire d'accès utilisateur (`AP = 0x40`).
- **Alignement de la section utilisateur dans l'éditeur de liens (`linker.ld`)** :
  Mise en place d'une section dédiée `.user_text` alignée sur la frontière de 2 Mégaoctets (`0x200000`). La fonction utilisateur `user_main_entry` y est affectée via l'attribut de section GCC (`__attribute__((section(".user_text")))`), lui garantissant d'être exécutée sur une page dotée des permissions d'accès EL0.
- **Positionnement du Tas (Heap) et de la Pile Utilisateur** :
  Déplacement de l'adresse de départ du tas (`heap_start`) à `0x300000` (3 Mo) dans `memory.c`. La pile utilisateur allouée par `kalloc(4096)` réside ainsi dans l'espace RAM autorisé en EL0, permettant aux écritures de pile (`SP_EL0`) de réussir sans faute de données.
- **Routage des interruptions IRQ en EL0 (`vectors.S`)** :
  Correction du vecteur d'interruption `irq_invalid_el0_64` (index 9) dans `vectors.S` pour qu'il pointe directement vers `irq_handler_el1h`. Le Timer matériel et les interruptions système interrompent et reprennent désormais le code EL0 de manière 100 % transparente.

### 3. Architecture Retenue
- **Privilèges** : Séparation stricte entre le code noyau (EL1) et le code applicatif (EL0).
- **Ressources** : Fichiers `kernel/user.c`, `include/kernel/user.h`, `arch/aarch64/user_mode.S` et section `.user_text` dans `linker.ld`.

### 4. Étapes futures (Roadmap)
La transition EL0 et l'isolation mémoire étant entièrement validées :
- [ ] **Fusion de la branche `feat/uart-minitel`** : Intégration du pilote UART Minitel et du contrôleur d'interruptions.
- [ ] **Appels Système (Syscalls `svc #0`)** : Implémentation des handlers d'exceptions synchrones `svc` pour permettre aux applications EL0 de demander des services au noyau (affichage UART, allocation).
- [ ] **Pilote Carte SD & EMMC** : Poursuite du développement du stockage physique pour charger des binaires utilisateur séparés.

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
