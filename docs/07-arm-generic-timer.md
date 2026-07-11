# 07. Implémentation du Timer Générique ARM

Ce document retrace l'intégration du timer générique ARM (Virtual Timer) dans PerryTel OS.

## 1. Interaction avec les registres (`timer.c`)
Le timer générique d'ARM est contrôlé via des registres systèmes internes. L'accès a été implémenté en langage C grâce à de l'assembleur "inline" (`asm volatile`) :
- `cntfrq_el0` (lecture) : Fréquence du timer (nombre de ticks par seconde).
- `cntv_tval_el0` (écriture) : Valeur du compte à rebours. Lorsque cette valeur arrive à 0, l'interruption se déclenche.
- `cntv_ctl_el0` (écriture) : Registre de contrôle permettant d'activer le timer en passant son bit 0 à 1.

Le timer est initialisé en lisant la fréquence du système et en l'injectant dans `cntv_tval_el0` pour obtenir un délai d'exactement 1 seconde.

## 2. Configuration du GIC (`gic.c`)
Sur BCM2838/AArch64, le Virtual Timer est physiquement lié à l'interruption locale **PPI 27**.
Pour que le processeur soit notifié, cette interruption spécifique a été activée dans le distributeur GIC via l'activation du bit 27 du registre `ISENABLER[0]`.

## 3. Handler et réarmement (`exception.c`)
La fonction `timer_handler` a été enregistrée pour réagir à l'interruption 27.
**Point crucial :** sur l'architecture ARM, lorsque l'interruption du timer se déclenche, elle reste pendante tant que le registre `cntv_tval_el0` n'est pas modifié. Le handler a donc été conçu pour **réarmer le timer** à la volée avec une nouvelle valeur. Sans cela, le système serait resté bloqué dans une boucle infinie d'interruptions.
