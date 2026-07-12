# Journal de bord - Perrytel OS

## 10. Moteur de Rendu Vidéotex (Screen Buffer)

**Date :** 12 Juillet 2026
**Sujet :** Implémentation d'un moteur de rendu graphique 2D avec "Double Buffering" pour le terminal Minitel (et compatibilité ANSI pour l'émulateur QEMU).

---

### 1. Problématiques d'affichage
Un terminal série comme le Minitel (à 1200 bauds) est extrêmement lent pour redessiner un écran entier (40 colonnes x 24 lignes). L'envoi des 960 caractères prendrait près d'une seconde, créant un "flickering" (clignotement) désagréable à chaque frappe clavier. 
De plus, sans mémorisation de l'état de l'écran, il est impossible d'implémenter des touches comme "Correction" (Backspace) pour effacer un caractère sans casser l'interface.

### 2. Architecture du Double Buffering (VRAM)
L'OS intègre désormais une architecture graphique professionnelle séparant le dessin en mémoire de l'affichage matériel :
- **La mémoire vidéo virtuelle (`vram`) :** Matrice de 24x40 caractères représentant l'écran idéal. Les fonctions de dessin de l'OS (comme `minitel_putc`) modifient uniquement cette matrice en RAM de façon instantanée.
- **La mémoire vidéo passée (`previous_vram`) :** Matrice mémorisant l'état exact actuellement affiché sur le tube cathodique physique.
- **Le `Dirty Flag` (`screen_dirty`) :** Variable booléenne qui indique à la boucle principale (Scheduler) qu'un changement a eu lieu et qu'un rafraîchissement est nécessaire. Cela évite au CPU de recalculer l'écran lors d'un réveil intempestif (ex: interruption du Timer).

### 3. La synchronisation matérielle (`minitel_flush`)
Lorsqu'un rafraîchissement est requis, la fonction de `flush` compare `vram` avec `previous_vram`. 
Dès qu'une différence est détectée :
1. Le curseur matériel est déplacé précisément sur la coordonnée cible via la séquence d'échappement absolue `Locate`.
2. Le nouveau caractère est envoyé à l'UART.
3. Le tableau `previous_vram` est mis à jour.
Le reste de l'écran n'est jamais re-dessiné, garantissant des performances maximales. L'écran matériel n'est jamais vidé via la commande "Clear Screen", ce qui élimine totalement le clignotement.

### 4. Abstraction Matérielle (HAL) QEMU / Minitel
Les commandes matérielles de contrôle (Curseur, Positionnement, Clear) utilisent des macros de préprocesseur (`#ifdef QEMU`) pour traduire dynamiquement le protocole Vidéotex (Minitel) en protocole ANSI (Standard PC). L'OS peut ainsi être développé, débogué et utilisé avec une fidélité visuelle parfaite sur un terminal Linux moderne.

### 5. Bilan
L'OS possède désormais son "Moteur 2D". La fondation graphique est solide et prête à accueillir des interfaces interactives (formulaires, menus, fenêtres).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
