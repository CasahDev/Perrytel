# 06. Interruptions SGI, Transition EL1 et Alignements

Ce document résume la résolution des problèmes complexes liés au déclenchement des interruptions générées par logiciel (SGI) via le GIC, aux alignements mémoires, et au changement de niveau d'exécution (Exception Level).

## 1. La macro `ventry` et l'alignement à 128 octets
La table des vecteurs d'exception (Exception Vector Table) en AArch64 exige que chaque entrée soit alignée sur 128 octets (`.align 7`). 
L'inclusion des macros de sauvegarde et de restauration de contexte (`kernel_entry` et `kernel_exit`) directement à l'intérieur de la macro `ventry` faisait dépasser cette limite de 128 octets.
*   **Solution :** La macro `ventry` a été simplifiée pour ne contenir qu'une instruction de saut (`b \label`). Le `kernel_entry` et le `kernel_exit` sont désormais appelés explicitement dans le handler (`irq_handler_el1h`), ce qui garantit que la table des vecteurs est parfaitement alignée.

## 2. Transition de l'EL2 vers l'EL1
QEMU lance les noyaux AArch64 au niveau d'exception EL2 (Hyperviseur) par défaut. Bien qu'il soit possible de manipuler des interruptions en EL2, il est architecturalement plus propre (et nécessaire pour la suite du développement de l'OS) de fonctionner en EL1.
La transition a été implémentée dans `boot.S` en simulant un retour d'exception :
1.  **HCR_EL2 :** Configuration à 64-bits (bit `RW` à 1).
2.  **SPSR_EL2 :** Configuration de l'état ciblé (EL1h) et masquage temporaire des interruptions (`0x3C5`).
3.  **ELR_EL2 :** Adresse de la prochaine instruction en EL1 (`kernel_main`).
4.  **SP_EL1 & VBAR_EL1 :** Initialisation de la pile et de la table des vecteurs pour l'EL1.
5.  **`eret` :** Bascule effective vers l'EL1.
*   **Conséquence :** Les macros de sauvegarde de contexte (`vectors.S`) ont dû être mises à jour pour utiliser `elr_el1` et `spsr_el1` au lieu de `elr_el2` et `spsr_el2`.

## 3. Emplacement physique du GIC sous QEMU
Sur le Raspberry Pi 4 physique, le GIC se trouve dans le *High Peripheral Space* (à l'adresse `0x400040000`). Cependant, l'émulation de QEMU (`bcm2838`) ne réplique pas fidèlement cet emplacement et ancre le GIC dans le *Legacy Peripheral Space*.
*   **Solution :** La définition de l'adresse de base du GIC a dû être maintenue à `0xFF840000` pour éviter des erreurs matérielles (Data Aborts synchrones `0x10`) lors des accès.

## 4. Alignement de la structure `gic_distributor`
Bien que le contrôleur GIC fût correctement initialisé, l'écriture dans le registre `SGIR` (pour déclencher l'interruption logicielle) était silencieusement ignorée.
La raison venait d'un manque de padding (remplissage) dans la structure C `gic_distributor` représentant les registres. L'offset de la variable `SGIR` tombait à `0xD00` au lieu des `0xF00` exigés par la spécification ARM GICv2.
*   **Solution :** Ajout de 512 octets de padding (`uint32_t _reserved1[128];`) entre les registres `ICFGR` et `SGIR`. L'offset est maintenant correct, et l'interruption `gic_test_sgi()` déclenche parfaitement le callback avec l'identifiant 1.
