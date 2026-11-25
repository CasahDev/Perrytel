# Journal de bord - Perrytel OS

## 24. Infrastructure des Appels Système (Syscalls EL0/EL1) & E/S Interactives

**Date :** 13 Août 2026  
**Sujet :** Architecture des appels système (*Syscalls*), passage des arguments par registres ARM64 (`x8`, `x0`-`x2`), instruction `svc #0`, implémentation de `sys_read` (63), `sys_write` (64), `sys_exit` (93), et démasquage des interruptions matérielles (`DAIF`).

---

### 1. Introduction

Cette étape établit la passerelle d'exécution sécurisée entre les applications en **Mode Utilisateur (EL0)** et le **Noyau (EL1)**. Grâce à l'instruction d'interruption logicielle `svc #0` et au respect des normes POSIX / ABI ARM64 Linux, PerryTel OS fournit une API C complète permettant aux binaires utilisateur d'interagir avec le matériel (affichage console, lecture clavier interactive, fin de processus) sans enfreindre la protection mémoire de la MMU.

---

### 2. État d'avancement et Architecture Technique

#### A. Convention des Registres ARM64 (`svc #0`)
Lorsqu'une application en mode EL0 invoque un appel système :
- **Registre `x8`** : Numéro d'identifiant du Syscall (ex: 63 pour `read`, 64 pour `write`, 93 pour `exit`).
- **Registres `x0`, `x1`, `x2`** : Arguments transmises du mode utilisateur vers le noyau (`arg0`, `arg1`, `arg2`).
- **Registre `x0` (Retour)** : Valeur de statut ou nombre d'octets transférés renvoyé par le noyau au mode utilisateur.

#### B. Table des Appels Système Implementés

| ID Syscall | Nom POSIX | Rôle & Fonctionnement |
| :--- | :--- | :--- |
| **`63`** | `sys_read` | Lecture clavier interactive. Consomme le tampon circulaire UART `rx_buffer` et transfère les caractères au tampon utilisateur EL0 jusqu'à l'appui sur Entrée (`\r`/`\n`). |
| **`64`** | `sys_write` | Écriture console. Extrait le pointeur de chaîne `char *` en mémoire utilisateur et l'affiche sur l'UART (`uart_puts`). |
| **`93`** | `sys_exit` | Clôture de processus. Affiche le code de sortie utilisateur, nettoie la pile et rend la main au Shell (`print_shell`). |

#### C. Gestion de l'Attente Bloquante & Démasquage IRQ (`msr daifclr, #2`)
- **Comportement Matériel ARM64** : Lors du basculement d'exception de EL0 vers EL1 via `svc #0`, le processeur ARM64 masque automatiquement les interruptions matérielles dans `PSTATE.DAIF` (bit `I = 1`).
- **Déblocage de l'Attente Clavier** : Pour permettre aux interruptions UART d'alimenter le tampon `rx_buffer` pendant qu'un Syscall attend la saisie utilisateur (`wfe`), le gestionnaire de Syscall démasque les IRQ avec l'instruction assembleur :
  $$\text{\_\_asm\_\_ \text{volatile}("msr daifclr, \#2");}$$
- **Normalisation de la Touche Entrée** : Traduction automatique de la touche `\r` (code ASCII 13) en `\n` (code ASCII 10) pour débloquer l'appel système `sys_read` conformément aux standards POSIX.

#### D. Couche d'Abstraction Utilisateur (`user/perrylib.c` & `include/perrylib.h`)
- Implémentation des routines d'assemblage `syscall1`, `syscall2`, `syscall3` utilisant les liaisons de registres GCC (`register uint64_t x8 __asm__("x8")`).
- Offre des wrappers C de haut niveau : `printf()`, `read()`, `exit()`.

---

### 3. Synthèse des Composants d'Appels Système

| Composant | Fichier Source | Description / Rôle |
| :--- | :--- | :--- |
| `vectors.S` / `el0_sync_handler` | [arch/aarch64/vectors.S](file:///home/sacha/Dev/PerryTel/arch/aarch64/vectors.S) | Interception matérielle de l'instruction `svc #0` et routage vers la table. |
| `syscall_handlers[]` | [kernel/syscall.c](file:///home/sacha/Dev/PerryTel/kernel/syscall.c) | Table d'indexation des pointeurs de fonctions de Syscalls noyau. |
| `uart_read()` / `sys_read` | [kernel/syscall.c](file:///home/sacha/Dev/PerryTel/kernel/syscall.c) | Gestionnaire du Syscall 63 transférant les caractères du tampon UART vers l'EL0. |
| `uart_get_char_async()` | [drivers/uart.c](file:///home/sacha/Dev/PerryTel/drivers/uart.c) | Dépilage asynchrone du tampon circulaire UART `rx_buffer`. |
| `perrylib` | [user/perrylib.c](file:///home/sacha/Dev/PerryTel/user/perrylib.c) | Bibliothèque C EL0 autonome compilée dans la section `.user_text`. |

---

### 4. Étapes futures (Roadmap)

- [ ] **Chargeur d'Exécutables Autonomes (`/bin/`)** : Lecture et exécution de binaires bruts `.bin` ou d'exécutables `ELF64` depuis la carte SD.
- [ ] **Ordonnanceur Multi-Tâches Préemptif (*Scheduler*)** : Changement de contexte par interruption Horloge ARM64 (*ARM Generic Timer*).
- [ ] **Gestion des Arguments de Ligne de Commande (`argc`, `argv`)** : Transmission des paramètres du Shell vers la pile utilisateur des binaires EL0.

---
*Document consigné pour la vault Obsidian - PerryTel OS Project.*
