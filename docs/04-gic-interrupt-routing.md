# Journal de bord - Perrytel OS

## 04. GIC : Architecture et Routage Logiciel des Interruptions

**Sujet :** Gestion des adresses mémoires du Generic Interrupt Controller (GIC) et mise en place d'un système de callback (table de routage).

---

### 1. Organisation Mémoire du GIC (RPi4)
Le GIC sur l'architecture BCM2711 est un GIC-400 divisé en deux blocs principaux :
- **GIC Distributor (GICD) :** Gère la priorité et le routage global. Situé à l'offset `0x1000` de l'adresse de base.
- **GIC CPU Interface (GICC) :** Assure la liaison directe avec le cœur CPU. Situé à l'offset `0x2000` de l'adresse de base.

**Piège évité (Arithmétique des pointeurs) :** 
Lors de l'addition d'offsets à l'adresse de base du GIC, il a fallu faire attention au type du pointeur. Additionner `0x1000` à un pointeur d'entiers (4 octets) multiplie l'offset par 4. La solution implémentée consiste à utiliser un cast `(char *)` avant l'addition pour imposer un calcul direct en octets.

### 2. Le Routage par Tableau de Pointeurs de Fonctions
L'implémentation d'un grand `switch/case` pour distribuer l'interruption à chaque périphérique n'étant pas scalable, une approche de type **Callback** a été choisie :
- Création d'un type personnalisé via `typedef` pour les pointeurs de fonction.
- Mise en place d'un tableau global de 1024 entrées représentant l'annuaire des interruptions matérielles.
- Méthodes de type "register" et "get" pour associer dynamiquement des fonctions à un ID matériel.

### 3. La Chorégraphie du `irq_handler`
Lorsqu'une exception est interceptée par la table vectorielle, la fonction C `irq_handler` déroule une séquence stricte de MMIO :
1. **Acknowledge :** Lecture du registre `IAR` du `GICC` pour valider la prise en charge et récupérer l'ID de l'interruption. Ce registre doit être lu **une seule fois** pour ne pas altérer l'état matériel côté GIC.
2. **Spurious Interrupt :** Si l'ID est `1023` (interruption fantôme / annulée), le traitement est immédiatement abandonné (pas de callback, pas de validation de fin).
3. **Dispatch :** Appel du gestionnaire correspondant depuis le tableau (l'ID inconnu tombant dans le handler par défaut).
4. **End of Interrupt (EOIR) :** Écriture de l'ID initial dans le registre `EOIR` du `GICC` pour autoriser le matériel à envoyer la prochaine interruption.

### 4. État actuel
- [x] Mapping mémoire des blocs GICD et GICC.
- [x] Architecture du callback et tableau de routage (C).
- [x] Séquence matérielle `IAR` -> `Dispatch` -> `EOIR` implémentée.
- [ ] Initialisation matérielle du GIC (activation des registres de contrôle).
- [ ] Configuration d'une interruption spécifique (Timer, etc.).
- [ ] Démasquage des IRQ au niveau du processeur ARM (bit I du registre DAIF).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
