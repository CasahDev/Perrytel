# Journal de bord - Perrytel OS

## 01. Mise en place initiale et corrections critiques

**Date :** 24 Mai 2026
**Sujet :** Correction du bootloader et réorganisation de l'architecture.

---

### 1. Analyse des problèmes initiaux
Au lancement du projet, le noyau ne produisait aucun affichage dans QEMU. Trois causes majeures ont été identifiées :

1. **Décalage de l'adresse de base** : La directive `.org 0x80000` dans `boot.S` créait un décalage inutile de 512 Ko, car le linker script positionnait déjà le code à cette adresse.
2. **Conflit multicœur** : Sur Raspberry Pi 4, les 4 cœurs démarrent simultanément à `0x80000`. Sans contrôle, ils entraient tous en collision sur la pile et l'initialisation de l'UART.
3. **Boucle BSS corrompue** : Le nettoyage de la section BSS écrivait 8 octets mais ne décrémentait le compteur que de 1, causant un écrasement massif de la mémoire vive.

### 2. Solutions apportées

#### Bootloader (`arch/aarch64/boot.S`)
- Ajout d'une vérification du `mpidr_el1` pour ne laisser passer que le CPU 0. Les autres cœurs sont mis en attente (`wfe`).
- Initialisation correcte de la pile (`sp`) juste en dessous de l'adresse de chargement.
- Correction de la boucle BSS pour utiliser des sauts de 8 octets (`subs w6, w6, #8`).

#### UART & Echo (`kernel/main.c`)
- Modification du driver UART pour gérer les retours chariot (`\r`).
- Désormais, le noyau renvoie `\r\n` quand il reçoit `\r`, permettant un affichage correct dans les terminaux série.

### 3. Réorganisation "Meaty Skeleton" (AArch64)
Le projet a été restructuré pour séparer les responsabilités :

```text
/
├── arch/aarch64/      # Code spécifique au CPU (boot, linker)
├── drivers/           # Drivers matériels (UART)
├── include/           # Headers globaux
├── kernel/            # Logique OS indépendante du matériel
└── build.sh           # Script de compilation modulaire
```

**Bénéfice :** Cette structure permet de porter l'OS sur d'autres modèles de Pi ou d'autres architectures sans réécrire la logique du noyau.

### 4. État actuel
- [x] Bootloader fonctionnel.
- [x] Affichage UART opérationnel sur QEMU.
- [x] Code structuré et modulaire.
- [ ] Système d'interruptions (Prochaine étape).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
