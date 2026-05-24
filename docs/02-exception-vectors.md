# Journal de bord - Perrytel OS

## 02. Mise en place de la table des vecteurs d'exceptions

**Date :** 24 Mai 2026
**Sujet :** Création de `vectors.S` et configuration de `VBAR_EL1`.

---

### 1. Théorie : La table AArch64
Contrairement aux architectures plus simples, l'ARMv8 (64-bit) impose une structure de table de vecteurs très stricte pour gérer les interruptions et les erreurs (exceptions) :
- **Taille totale** : 16 entrées.
- **Espacement** : Chaque entrée doit être espacée exactement de **128 octets**.
- **Alignement global** : La table doit commencer sur une adresse multiple de **2048 octets** ($2^{11}$).

### 2. Implémentation technique (`arch/aarch64/vectors.S`)

#### La Macro `ventry`
Pour garantir l'espacement de 128 octets sans calcul manuel, une macro a été utilisée :
```assembly
.macro ventry label
    b \label        // Saut vers le gestionnaire (4 octets)
    .align 7        // Remplit de zéros jusqu'au prochain multiple de 128 (2^7)
.endm
```

#### Structure de la table
La table `vectors` appelle 16 fois la macro, couvrant les 4 groupes d'états du CPU (Current EL avec SP0, Current EL avec SPx, Lower EL AArch64, Lower EL AArch32).
Pour l'instant, toutes les entrées pointent vers un gestionnaire d'erreur (`invalid_entry`), sauf l'entrée n°6 dédiée aux **IRQ** survenant dans le noyau (`irq_handler_el1h`).

### 3. Activation du mécanisme (`arch/aarch64/boot.S`)
La table est "enregistrée" auprès du processeur au démarrage via le registre système `VBAR_EL1` :
```assembly
ldr x0, =vectors
msr vbar_el1, x0
```

### 4. Automatisation du Build
Le script `build.sh` a été modifié pour compiler automatiquement tout nouveau fichier `.S` présent dans `arch/aarch64/`. Cela facilite l'ajout de briques assembleur sans toucher à la configuration de compilation.

### 5. État actuel
- [x] Table des vecteurs alignée et complétée (16 entrées).
- [x] Enregistrement dans `VBAR_EL1` effectué.
- [ ] Sauvegarde du contexte (registres) lors d'une exception.
- [ ] Gestionnaire IRQ en C.

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
