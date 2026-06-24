#!/bin/bash

EXTRA_CFLAGS="-Iinclude"
ARCH_DIR="arch/aarch64"
KERNEL_DIR="kernel"
DRIVER_DIR="drivers"
BUILD_DIR="build"

mkdir -p $BUILD_DIR

if [ "$1" == "qemu" ]; then
    echo "Compilation pour QEMU..."
    EXTRA_CFLAGS="$EXTRA_CFLAGS -DQEMU"
else
    echo "Compilation pour vrai Raspberry Pi..."
fi

# Compilation AArch64 (tous les fichiers .S)
for f in $ARCH_DIR/*.S; do
    filename=$(basename "$f" .S)
    aarch64-linux-gnu-gcc -DAARCH64 -c "$f" -o "$BUILD_DIR/$filename.o"
done

for f in $KERNEL_DIR/*.c; do
    filename=$(basename "$f" .c)
    aarch64-linux-gnu-gcc -DAARCH64 $EXTRA_CFLAGS -std=gnu99 -ffreestanding -O2 -Wall -Wextra -c $f -o $BUILD_DIR/$filename.o
done

for f in $DRIVER_DIR/*.c; do
    filename=$(basename "$f" .c)
    aarch64-linux-gnu-gcc -DAARCH64 $EXTRA_CFLAGS -std=gnu99 -ffreestanding -O2 -Wall -Wextra -c $f -o $BUILD_DIR/$filename.o
done

# Linking (inclut tous les .o générés)
aarch64-linux-gnu-ld -T $ARCH_DIR/linker.ld --no-warn-rwx-segments -o $BUILD_DIR/kernel8.elf $BUILD_DIR/*.o
aarch64-linux-gnu-objcopy -O binary $BUILD_DIR/kernel8.elf $BUILD_DIR/kernel8.img

echo "Build completed. Output: $BUILD_DIR/kernel8.img"

if [ "$1" == "qemu" ]; then
    ./qemu-build/qemu-9.0.2/build/qemu-system-aarch64 -M raspi4b -kernel $BUILD_DIR/kernel8.img -serial stdio -display none
fi
