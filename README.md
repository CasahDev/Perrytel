# Perrytel
Système d'exploitation pour Raspberry PI 4 ayant pour but de communiquer des information sur un minitel 1b

## Arborescence de fichiers
Perrytel/
 ├── arch/
 │   └── aarch64/          # Code spécifique au processeur (Boot, Linker)
 ├── drivers/              # Drivers matériels (indépendants de l'OS)
 ├── include/              # Headers (.h)
 │   ├── drivers/
 │   └── kernel/           # Futurs headers de l'OS
 ├── kernel/               # Logique de l'OS (Architecture-indépendante)
 │   └── main.c
 └── build.sh              # Script de build mis à jour