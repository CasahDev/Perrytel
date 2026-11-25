# Journal de bord - Perrytel OS

## 09. Décodage du Clavier Minitel

**Date :** 12 Juillet 2026
**Sujet :** Parsing des séquences clavier spécifiques au Minitel via une Machine à États.

---

### 1. Concept de la Machine à États
Le clavier du Minitel envoie des caractères ASCII standards pour les lettres et les chiffres. Cependant, les touches de fonction spécifiques (à droite du clavier) envoient une séquence d'échappement composée de deux octets : l'octet `0x13` (Séparateur/SEP) suivi d'un octet identifiant la touche.

Pour éviter de bloquer le noyau en attendant le second octet, le traitement est délégué à une **Machine à états** asynchrone exécutée dans la boucle principale (`Bottom Half`) de l'OS.

### 2. Référentiel des touches Minitel
Voici la correspondance des codes envoyés et leur sémantique conseillée pour l'OS :

| Touche Minitel | Séquence Hexa | Équivalent PC | Rôle dans l'OS (Sémantique) |
| :--- | :--- | :--- | :--- |
| **Envoi** | `0x13 0x41` | Entrée (Valider) | Valider un formulaire, exécuter une commande. |
| **Retour** | `0x13 0x42` | Précédent (Back) | Remonter au menu parent ou annuler le sous-menu actuel. |
| **Répétition** | `0x13 0x43` | F5 (Rafraîchir) | Recharger la page ou redessiner l'écran. |
| **Guide** | `0x13 0x44` | F1 (Aide) | Afficher le manuel ou la légende des commandes. |
| **Annulation** | `0x13 0x45` | Échap / Ctrl+C | Interrompre une action ou vider un champ de saisie. |
| **Sommaire** | `0x13 0x46` | Home (Accueil) | Retour immédiat au menu principal (Index). |
| **Correction** | `0x13 0x47` | Backspace | Effacer le dernier caractère tapé. |
| **Suite** | `0x13 0x48` | Page Suivante | Faire défiler un long texte ou afficher la page 2. |

### 3. Gestion des spécificités PC (Émulateur)
Pour maintenir la compatibilité lors des tests sous QEMU avec un clavier de PC standard, la touche "Entrée" classique (`\r` ou `\n`) est interceptée dans l'état normal (0) de la machine à états et associée manuellement à un retour à la ligne (`\r\n`).

### 4. État actuel
- [x] Machine à états implémentée (`kernel/keyboard.c`).
- [x] Interception du séparateur `0x13`.
- [x] Compatibilité croisée clavier PC (`\r`) / Minitel (`Envoi`).

---
*Note générée pour la vault Obsidian - Perrytel OS Project.*
