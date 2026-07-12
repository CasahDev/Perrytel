# Journal de bord - Perrytel OS

## 13. Création du Shell Interactif (Asynchrone)

**Date :** 12 Juillet 2026
**Sujet :** Implémentation d'un interpréteur de commandes basé sur les interruptions UART.

---

### 1. Contexte et Objectif
Avec la bibliothèque `string.h` opérationnelle, l'objectif était de permettre à l'utilisateur d'interagir avec l'OS en tapant des commandes textuelles. L'OS fonctionnant sur un modèle d'interruptions matérielles (le processeur "dort" via l'instruction `wfe` et se réveille lorsqu'une touche est pressée), il a fallu concevoir une architecture asynchrone pour capturer les frappes une à une.

### 2. Architecture du Shell

L'architecture repose sur un mécanisme de producteur (le clavier) / consommateur (le noyau) via une mémoire partagée.

#### 1. Le Buffer Global (`drivers/uart.c`)
- **`shell_buffer[256]`** : Tableau stockant la commande en cours de frappe.
- **`shell_index`** : Curseur indiquant l'emplacement de la prochaine lettre (protégé par un modulo `% 256` pour éviter les failles de dépassement de tampon - Buffer Overflow).
- **`command_ready`** : Drapeau (`volatile bool`) qui signale au noyau principal qu'une ligne entière a été validée.

#### 2. La Capture (Producteur)
Dans la routine d'interruption `uart_handle_interrupt()` :
- Les caractères "normaux" sont ajoutés au `shell_buffer` et affichés à l'écran (écho).
- Si la touche `Entrée` (`\r`) est détectée, le caractère de fin de chaîne `\0` est inséré à la place, et le drapeau `command_ready` est levé. 
- *Note technique :* La gestion de la touche effacer (Backspace) a été volontairement reportée à l'intégration finale du vrai clavier Minitel.

#### 3. L'Exécution (Consommateur)
Dans la boucle infinie de `kernel/main.c` :
- Une fonction `execute_command()` est appelée en boucle.
- Si `command_ready` est faux, la fonction retourne immédiatement (non-bloquante).
- Si `command_ready` est vrai, l'OS compare le `shell_buffer` avec des chaînes de caractères connues grâce à `strcmp()` (ex: `"help"`).
- Une fois la commande traitée, l'index est remis à 0, le drapeau rabaissé, et un nouveau prompt (`Perry> `) est affiché à l'écran.

### 3. État actuel
- [x] Saisie de texte asynchrone fluide sur l'émulateur QEMU via UART.
- [x] Protection contre les buffer overflows.
- [x] Analyse rudimentaire d'une commande sans arguments.
- [ ] Refactorisation de la détection des commandes (séparation Commande / Arguments).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
