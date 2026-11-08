# Journal de bord - Perrytel OS

## 28. Câblage Matériel, Convertisseur de Niveau et Intégration Minitel 2

**Date :** 8 Novembre 2026  
**Sujet :** Schéma de câblage physique officiel (Raspberry Pi 4 $\leftrightarrow$ Convertisseur logique 4 canaux $\leftrightarrow$ Prise DIN Minitel 2), repérage par code couleur des liaisons, procédure de déploiement sur carte SD et spécificités matérielles du Minitel 2.

---

### 1. Introduction

Ce document consigne la référence officielle du câblage physique entre la carte hôte **Raspberry Pi 4** (niveaux logiques 3.3V) et le terminal **Minitel 2** (niveaux logiques 5V TTL).

L'interfaçage est obligatoirement protégé par un **module convertisseur de niveau logique bidirectionnel 4 canaux** afin d'adapter les tensions sans risquer de détruire les entrées/sorties du SoC Broadcom BCM2711.

---

### 2. Tableau de Câblage Officiel et Repérage Couleur

Le raccordement utilise un code couleur précis pour identifier immédiatement les rails d'alimentation et les lignes de données série UART :

| Broche Physique Pi | Signal Pi / Rôle | Couleur du Fil | Broche Convertisseur | Destination Finale (Minitel 2) |
| :---: | :--- | :---: | :---: | :--- |
| **Pin 2** | Alimentation 5V | **Rouge** | **`HV`** | Alimentation du rail 5V de référence du convertisseur |
| **Pin 1** | Alimentation 3.3V | **Orange** | **`LV`** | Alimentation du rail 3.3V de référence du convertisseur |
| **Pin 6** | Masse (GND) | **Noir** | **`GND` (Côté LV)** | Masse commune (reliée à la Broche 2 DIN Minitel) |
| **Pin 8** | GPIO 14 (UART0 TX) | **Vert** | **`LV2`** $\leftrightarrow$ **`HV2`** | **Broche 1 DIN Minitel** (RX Minitel $\leftarrow$ Pi TX) |
| **Pin 10** | GPIO 15 (UART0 RX) | **Violet** | **`LV1`** $\leftrightarrow$ **`HV1`** | **Broche 3 DIN Minitel** (TX Minitel $\rightarrow$ Pi RX) |

[Source](https://pinout.xyz/)

---

### 3. Schéma Fonctionnel des Liaisons

```
[ Raspberry Pi 4 ]                      [ Convertisseur Logique ]                      [ Prise DIN Minitel 2 ]
Pin 2  (5V)        ───(Fil Rouge)───▶   HV
Pin 1  (3.3V)      ───(Fil Orange)──▶   LV
Pin 6  (GND)       ───(Fil Noir)────▶   GND (LV) ────(Masse commune)────────────────▶  Broche 2 (Masse GND)
Pin 8  (GPIO 14 TX)───(Fil Vert)────▶   LV2  ═════▶  HV2  ──────────────────────────▶  Broche 1 (RX Minitel)
Pin 10 (GPIO 15 RX)◀──(Fil Violet)───   LV1  ◀═════  HV1  ◀─────────────────────────   Broche 3 (TX Minitel)
```

---

### 4. Broche DIN 5 Broches (Vue Arrière Minitel 2)

À l'arrière du châssis du Minitel 2, la prise péri-informatique DIN 45326 femelle (5 broches) est agencée ainsi :

```
                    (Haut)
                 [Broche 2]  <--- Masse (GND) -> Reliée au GND commun (Fil Noir)
         [Broche 4]        [Broche 5]
    [Broche 1]                 [Broche 3]
  (RX Minitel)                (TX Minitel)
       ▲                            │
       │                            ▼
(Sortie HV2 / Vert)        (Entrée HV1 / Violet)
```

- **Broche 1 (RX)** : Reçoit les caractères émis par le noyau PerryTel OS via le canal vert (`HV2`).
- **Broche 2 (GND)** : Masse électrique de référence.
- **Broche 3 (TX)** : Émet les frappes du clavier vers le noyau via le canal violet (`HV1`).
- **Broches 4 & 5** : Laissées **vides et non connectées** (la broche 4 sort du 8.5V/12V interne non régulé).

---

### 5. Spécificités Matérielles du Minitel 2

Le passage à la génération **Minitel 2** apporte des avantages décisifs pour un système d'exploitation moderne :

1. **Étages d'Émission Actifs 5V (Push-Pull)** :
   Contrairement à certains Minitel 1 à collecteur ouvert nécessitant des résistances de tirage externes, le Minitel 2 délivre des fronts d'ondes TTL 5V francs et stables.
2. **Support des Hauts Débits (4800 / 9600 bauds)** :
   Le Minitel 2 est nativement capable de communiquer jusqu'à 4800 et 9600 bauds, éliminant la lenteur historique des 1200 bauds.
3. **Format de Trame Série (7E1)** :
   7 bits de données, Parité Paire (*Even*), 1 bit de stop.
4. **Bascule en Mode Terminal Téléinformatique (80 Colonnes)** :
   Pour utiliser PerryTel OS avec le confort d'un terminal ASCII standard :
   - Allumer le Minitel 2.
   - Appuyer sur la combinaison : **`Fnct`** + **`T`** puis **`A`** (passage en mode Téléinformatique / ASCII 80 colonnes).
   - Réglage de la vitesse : **`Fnct`** + **`P`** (ou menu de configuration selon modèle Telic ou Matra).

---

### 6. Procédure d'Installation et Déploiement sur Carte SD

1. **Préparation de la Carte SD (32 Go SDHC)** :
   - Table de partition : **MBR** (Master Boot Record).
   - Partition 1 : **FAT32** (nommée `bootfs`), taille minimale 256–512 Mo.
2. **Compilation du Noyau pour Matériel Réel** :
   À la racine du dépôt PerryTel :
   ```bash
   ./build.sh
   ```
   *(La compilation sans paramètre applique la cible matérielle physique sans l'indicateur `-DQEMU`)*.
3. **Fichiers Requis à la Racine de `bootfs`** :
   - `kernel8.img` (binaire du noyau PerryTel OS fraîchement généré).
   - `start4.elf` et `fixup4.dat` (firmware GPU officiel Raspberry Pi 4).
   - `bcm2711-rpi-4-b.dtb` (Device Tree Blob du Pi 4).
   - Dossier `overlays/` (pilotes matériels officiels).
   - `config.txt` configuré pour un boot 64-bit bare-metal :
     ```ini
     arm_64bit=1
     kernel=kernel8.img
     kernel_address=0x80000
     enable_uart=1
     disable_overscan=1
     dtoverlay=disable-bt
     ```
4. **Ordre d'Allumage pour le Premier Boot** :
   1. Allumer le **Minitel 2** en premier (polarisation des lignes 5V).
   2. Brancher l'alimentation USB-C du **Raspberry Pi 4**.
   3. L'écran affiche instantanément l'invite du Shell PerryTel OS.

---
*Document consigné pour la vault Obsidian - PerryTel OS Project.*
