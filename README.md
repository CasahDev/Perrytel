# Perrytel
Système d'exploitation pour Raspberry PI 4 ayant pour but de communiquer des information sur un minitel 1b

## Features
- [x] Intéruptions
- [x] Timers
- [ ] Drivers clavier minitel
- [ ] Multi processes
- [ ] EMMC/SD Host
- [ ] Système de fichiers
- [ ] Drivers ethernets

## Arborescence de fichiers
```
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
```

## Format des commits:
- 🤖: Patch de bugs
- 💫: Ajout de features
- 🧾: Ajout de documentation

## Initialisation carte SD virtuelle

```shell
qemu-img create sd.img 64M
parted -s sd.img mklabel msdos
parted -s sd.img mkpart primary fat32 2048s 100%
```