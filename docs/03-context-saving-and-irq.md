# Journal de bord - Perrytel OS

## 03. Sauvegarde de contexte et gestionnaire IRQ initial

**Date :** 30 Mai 2026
**Sujet :** Implémentation du stacking des registres et liaison C/Assembly pour les IRQ.

---

### 1. Sauvegarde et Restauration du Contexte (`vectors.S`)
Pour permettre une reprise correcte du code après une exception, les mécanismes de sauvegarde (`kernel_entry`) et de restauration (`kernel_exit`) ont été implémentés sous forme de macros :
- **Stacking complet** : Sauvegarde des registres `x0` à `x29` par paires (`stp`) pour l'efficacité, ainsi que `x30` (Link Register).
- **Métadonnées d'exception** : Sauvegarde des registres `ELR_EL2` (adresse de retour) et `SPSR_EL2` (état du processeur) sur la pile. Cela permet d'autoriser des exceptions imbriquées à l'avenir.
- **Allocation** : Une zone de `16 * 17` octets est réservée sur la pile à chaque entrée d'exception.

### 2. Transition vers le C (`kernel/exception.c`)
Un gestionnaire d'interruption a été créé en C pour simplifier la logique métier du noyau :
- **Liaison** : Le label `irq_handler_el1h` dans l'assembleur appelle désormais une fonction `irq_handler` définie en C.
- **Affichage de débogage** : Implémentation de `show_invalid_entry_message` pour diagnostiquer les exceptions inattendues (Sync, FIQ, SError) en affichant les registres `ESR` et `ELR`.

### 3. Ajustement pour le Raspberry Pi 4 (Mode EL2)
Le noyau opérant actuellement en **EL2** (Hypervisor Mode) sur RPi4, la configuration a été ajustée pour utiliser les registres systèmes correspondants :
- **VBAR_EL2** : La table des vecteurs est désormais enregistrée dans `VBAR_EL2` au lieu de `VBAR_EL1`.
- **Registres de retour** : Utilisation de `ELR_EL2` et `SPSR_EL2` pour la gestion des sauts de retour d'exception.

### 4. Tests de validation
- **Exception logicielle** : Utilisation de l'instruction `brk #0` dans `main.c` pour tester le branchement vers la table des vecteurs et la récupération de l'exception `Synchronous`.
- **Visualisation UART** : Confirmation via console série que le CPU identifie correctement le type d'exception et l'adresse de l'instruction fautive.

### 5. État actuel
- [x] Sauvegarde complète des registres (x0-x30).
- [x] Redirection fonctionnelle vers le gestionnaire C.
- [x] Support du mode EL2 confirmé.
- [ ] Initialisation du GIC (Generic Interrupt Controller).
- [ ] Démasquage des IRQ au niveau CPU (`DAIF`).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
