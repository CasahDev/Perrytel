# Journal de bord - Perrytel OS

## 15. Multitâche et Scheduler (Round-Robin)

**Date :** 14 Juillet 2026
**Sujet :** Implémentation du Scheduler préemptif et changements de contexte en assembleur/C.

---

### 1. Introduction
Aujourd'hui marque une étape fondamentale dans l'évolution de Perrytel OS : le passage d'un système mono-tâche à un système **multitâche préemptif**. Le noyau est désormais capable de jongler entre plusieurs processus de manière transparente grâce au Timer matériel.

### 2. État d'avancement et Défis Techniques
L'implémentation du planificateur de tâches (Scheduler) a nécessité une synchronisation millimétrée entre le code assembleur de bas niveau (`vectors.S`) et la logique en C (`sched.c`). Plusieurs défis majeurs ont été résolus :

- **Le Context Switch (Changement de Contexte)** : 
  L'assembleur sauvegarde désormais l'intégralité des registres (`x0` à `x30`, `elr_el1`, `spsr_el1`) sur la pile. Le pointeur de pile (`SP`) est ensuite passé à la fonction C via le registre `x0`.
- **Alignement Mémoire (Data Abort)** : 
  Une erreur de type "Alignment Fault" a été corrigée en gardant à l'esprit que le registre `SP` en ARM64 doit strictement être un multiple de 16. La structure `TrapFrame` a été modifiée avec un `padding` de 8 octets pour s'aligner parfaitement sur la topologie de la pile d'interruption.
- **La sourde-oreille aux IRQs** : 
  Lors de la création d'une tâche, le registre `SPSR` a été ajusté à `0x345` (au lieu de `0x3C5`) pour s'assurer que les tâches naissent avec le bit IRQ démasqué, permettant ainsi au Timer de les préempter à nouveau.
- **La protection du fil d'exécution principal** : 
  Pour que le fil `main` ne détruise pas la première tâche lors de la première interruption, l'index `0` du tableau statique des tâches lui a été formellement réservé.

### 3. Architecture Retenue
Le système repose sur un algorithme *Round-Robin* (tourniquet) élégant et sans allocation dynamique :
- Le tableau `tasks[]` stocke l'état des processus.
- Le `timer_handler` (appelé par le routeur `irq_handler`) retourne le nouveau pointeur de pile (`SP`) calculé par `schedule(sp)`.
- L'assembleur récupère ce pointeur et effectue la restauration de la nouvelle tâche via l'instruction `eret`.

### 4. Étapes futures (Roadmap)
Maintenant que le système peut exécuter plusieurs processus simultanément, voici les chantiers suivants pour la réalisation complète de l'OS :
- [ ] Drivers clavier minitel (En attente de validation et de merge sur sa branche)
- [ ] EMMC / SD Host (Pilote matériel pour la carte SD)
- [ ] Système de fichiers (Lecture/Écriture)
- [ ] Drivers Ethernet (Réseau)

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
