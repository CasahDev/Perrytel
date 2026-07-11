# Documentation des Registres du GIC (Generic Interrupt Controller)

Ce document répertorie et détaille le fonctionnement des registres du GIC au fur et à mesure de leur utilisation dans le projet Perrytel.

## 1. SGIR (Software Generated Interrupt Register)
**Emplacement** : GIC Distributor (GICD)
**Rôle** : Permet de générer des interruptions logicielles (SGI - Software Generated Interrupts). Ces interruptions (ID 0 à 15) sont généralement utilisées pour la communication inter-processeurs (IPI - Inter-Processor Interrupts).
**Structure (32 bits)** :
*   **[25:24] TargetListFilter** : Détermine le ciblage de l'interruption :
    *   `0b00` : Envoi aux CPU spécifiés dans `CPUTargetList`.
    *   `0b01` : Envoi à tous les CPU **sauf** celui qui déclenche l'interruption.
    *   `0b10` : Envoi **uniquement** au CPU qui déclenche l'interruption (Self-target).
*   **[23:16] CPUTargetList** : Masque de bits indiquant les processeurs cibles (si le filtre vaut `0b00`). Le bit 16 correspond au CPU 0, le bit 17 au CPU 1, etc.
*   **[15] SATT** : Attribut de sécurité (si applicable).
*   **[3:0] SGIINTID** : L'ID de l'interruption SGI générée (valeur entre 0 et 15).
*   *(Les autres bits sont réservés).*

## 2. ISENABLER (Interrupt Set-Enable Register)
**Emplacement** : GIC Distributor (GICD)
**Rôle** : Permet d'activer (Enable) la transmission des interruptions vers les CPU interfaces.
**Fonctionnement** : 
Le GIC gère des centaines d'interruptions (jusqu'à 1020). Les registres `ISENABLER` forment un tableau de registres de 32 bits, où **chaque bit correspond à une interruption spécifique**. 
*   Pour activer l'interruption `N`, on écrit un `1` au bit `N % 32` du registre `ISENABLER[N / 32]`.
*   Écrire un `1` active l'interruption.
*   Écrire un `0` n'a **aucun effet** (pour désactiver une interruption, il faut utiliser un autre registre : `ICENABLER`).
*   **Exemple** : `gicd->ISENABLER[0] = 1 << 1;` active l'interruption numéro 1.

## 3. CTLR (Control Register)
*(Note : C'est probablement le registre mentionné sous le nom "CLRLT")*
**Emplacement** : GIC Distributor (GICD) et GIC CPU Interface (GICC)
**Rôle** : C'est le registre de contrôle principal. Il sert de "bouton marche/arrêt" global.
**Fonctionnement** :
*   **Au niveau du Distributor (`gicd->CTLR`)** : Le bit `0` (EnableGrp1) ou `Enable` permet d'activer globalement la distribution des interruptions matérielles du distributeur vers les interfaces CPU. S'il vaut 0, aucune interruption n'est transmise (à part certaines SGI spécifiques).
*   **Au niveau du CPU Interface (`gicc->CTLR`)** : Le bit `0` (EnableGrp1) permet au CPU d'accepter les interruptions signalées par le Distributeur. S'il vaut 0, le CPU ignore les signaux du GIC.
*   **Exemple** : `gicd->CTLR = 0x01;` et `gicc->CTLR = 0x01;` activent le routage et la réception des interruptions.

## 4. IAR (Interrupt Acknowledge Register)
**Emplacement** : GIC CPU Interface (GICC)
**Rôle** : Permet au processeur de prendre en compte (Acknowledge) l'interruption et d'obtenir son numéro.
**Fonctionnement** :
*   Lorsqu'une interruption survient, le processeur doit **lire** ce registre.
*   Cette lecture a un **effet de bord matériel** : elle signale au GIC que l'interruption est en cours de traitement (elle passe de l'état "Pending" à "Active"). Il est donc crucial de ne lire ce registre qu'**une seule fois** par interruption.
*   **Structure (32 bits)** :
    *   **[9:0] Interrupt ID** : Le numéro de l'interruption qui vient de se déclencher (0 à 1019).
    *   **[12:10] CPUID** : Dans le cas d'une SGI, indique le processeur qui a généré l'interruption.
*   **Exemple** : `const uint32_t iar = gicc->IAR;` valide l'interruption et stocke son ID dans `iar`. Si l'ID vaut 1023, c'est une "Spurious Interrupt" (fausse alerte), ce qui signifie qu'aucune interruption valide n'est à traiter.
