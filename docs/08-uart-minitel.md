# 08. Configuration de l'UART pour Minitel (7E1)

Ce document retrace la reconfiguration de l'UART (port série) de PerryTel OS pour dialoguer avec un Minitel physique via la prise péri-informatique.

## 1. Découplage de la configuration
Pour permettre un changement dynamique de la vitesse (baud rate), la configuration a été extraite de `uart_init()` vers une nouvelle fonction `uart_set_baudrate(uint32_t baud)`.

## 2. Calcul du Baud Rate (Entiers)
La puce UART PL011 (horloge à 3 MHz) utilise des diviseurs (entier et fractionnaire).
Afin d'éviter l'usage de nombres flottants (complexes pour un noyau bas niveau), le calcul a été implémenté en arithmétique entière :
- `divisor = 12000000 / baud`
- `ibrd = divisor / 64`
- `fbrd = divisor % 64`

Cela permet de basculer facilement de 1200 bauds (défaut Minitel) à 4800 bauds (Minitel 1B).

## 3. Format de trame Minitel (7E1)
Le Minitel exige un format 7 bits de données, Parité Paire (Even), 1 bit de stop.
Le registre `UART0_LCRH` a été configuré avec `(1 << 6) | (1 << 4) | (1 << 2) | (1 << 1)`. 
**Point de vigilance :** L'écriture dans `LCRH` doit toujours être effectuée en dernier (après `IBRD` et `FBRD`) car elle sert de loquet matériel pour appliquer les nouvelles vitesses.
