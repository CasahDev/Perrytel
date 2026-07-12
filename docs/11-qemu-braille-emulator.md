# Journal de bord - Perrytel OS

## 11. Émulateur Mosaïque Vidéotex (Traduction Braille UTF-8)

**Date :** 12 Juillet 2026
**Sujet :** Rendu visuel du Mode Mosaïque Minitel (G1) sur le terminal Linux de QEMU.

---

### 1. Le Mode Mosaïque (Vidéotex G1)
Le Minitel possède un mode graphique où chaque caractère est divisé en une grille de 2 colonnes sur 3 lignes (soit 6 "pixels"). 
Pour activer ce mode sur une ligne, le Minitel utilise un octet de contrôle spécifique appelé "Attribut Série" :
*   **Mosaïque (ON)** : `0x0E` (Shift Out)
*   **Texte Normal (OFF)** : `0x0F` (Shift In)

Cet octet prend physiquement la place d'une case (affichée comme un espace) et affecte tous les caractères situés à sa droite. Les blocs à dessiner sont ensuite générés via une somme mathématique de leurs bits (de 0 à 63), et envoyés avec un décalage ASCII (base `0x20` ou `0x40`).

### 2. Le Défi de l'Émulateur
Si on envoie ces données telles quelles dans QEMU, le terminal Linux va interpréter les caractères Mosaïque comme du simple texte ASCII (on verra des `p`, des `~`, des `|`, etc.). 
Pour développer et designer les interfaces sans avoir besoin du vrai Minitel, un traducteur à la volée a été implémenté dans l'OS.

### 3. La solution : Le Braille Unicode
Les caractères Braille Unicode (de `U+2800` à `U+283F`) possèdent la même structure visuelle que les caractères Minitel : une grille de points. 

L'OS intercepte les caractères Mosaïque, extrait la valeur des 6 pixels (0 à 63), et la convertit vers la norme Braille selon le mapping suivant :
*   `Bit 0` (Haut-Gauche) -> Minitel & Braille
*   `Bit 1` (Haut-Droit) -> Minitel (`Bit 3` en Braille)
*   `Bit 2` (Milieu-Gauche) -> Minitel (`Bit 1` en Braille)
*   `Bit 3` (Milieu-Droit) -> Minitel (`Bit 4` en Braille)
*   `Bit 4` (Bas-Gauche) -> Minitel (`Bit 2` en Braille)
*   `Bit 5` (Bas-Droit) -> Minitel (`Bit 5` en Braille)

### 4. Injection UTF-8 via UART
L'OS envoie ensuite le caractère Braille au terminal Linux de QEMU en utilisant l'encodage UTF-8 (qui nécessite 3 octets pour ce bloc Unicode).
La formule magique matérielle pour imprimer le bloc est extrêmement élégante et rapide :
```c
uart_putc(0xE2);
uart_putc(0xA0);
uart_putc(0x80 + braille_pixels);
```

### 5. Bilan
Grâce à ce *Refactoring* et à ce traducteur (protégé par `#ifdef QEMU`), l'OS permet désormais d'afficher des "Sprites" (comme un logo complet) sous forme de tableaux 2D, avec un rendu pixel-perfect sur PC. Le code graphique est prêt pour le Shell interactif.

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
