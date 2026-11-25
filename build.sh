#!/bin/bash
set -e
EXTRA_CFLAGS="-Iinclude -mgeneral-regs-only -Idrivers/lwIP/include"
ARCH_DIR="arch/aarch64"
KERNEL_DIR="kernel"
DRIVER_DIR="drivers"
USER_LIB_DIR="user"
BUILD_DIR="build"
BIN_DIR="bin"

mkdir -p $BUILD_DIR
mkdir -p $BUILD_DIR/bin

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

for f in $(find $KERNEL_DIR -name "*.c"); do
    filename=$(basename "$f" .c)
    aarch64-linux-gnu-gcc -DAARCH64 $EXTRA_CFLAGS -std=gnu99 -ffreestanding -O2 -Wall -Wextra -c $f -o $BUILD_DIR/$filename.o
done

for f in $(find $DRIVER_DIR -name "*.c"); do
    filename=$(basename "$f" .c)
    aarch64-linux-gnu-gcc -DAARCH64 $EXTRA_CFLAGS -std=gnu99 -ffreestanding -O2 -Wall -Wextra -c $f -o $BUILD_DIR/$filename.o
done

for f in $(find $USER_LIB_DIR -name "*.c"); do
    filename=$(basename "$f" .c)
    aarch64-linux-gnu-gcc -DAARCH64 $EXTRA_CFLAGS -std=gnu99 -ffreestanding -O2 -Wall -Wextra -c $f -o $BUILD_DIR/$filename.o
done


for f in $(find $BIN_DIR -name "*.c"); do
    filename=$(basename "$f" .c)
    aarch64-linux-gnu-gcc -DAARCH64 $EXTRA_CFLAGS -static -no-pie -std=gnu99 -nostdlib -ffreestanding -O2 -Wall -Wextra -Wl,-Ttext,0x200000 -Wl,--build-id=none -Wl,-n -Wl,-e,main $f $BUILD_DIR/perrylib.o $BUILD_DIR/string.o -o $BUILD_DIR/bin/$filename.elf
    aarch64-linux-gnu-objcopy -O binary $BUILD_DIR/bin/$filename.elf $BUILD_DIR/bin/$filename

    if [ "$1" == "qemu" ]; then
      mmd -D s -i sd.img@@1M ::bin 2>/dev/null || true
      mcopy -o -i sd.img@@1M $BUILD_DIR/bin/$filename ::bin/$filename
    fi
done

# Linking (inclut tous les .o générés)
aarch64-linux-gnu-ld -T $ARCH_DIR/linker.ld --no-warn-rwx-segments -o $BUILD_DIR/kernel8.elf $BUILD_DIR/*.o
aarch64-linux-gnu-objcopy -O binary $BUILD_DIR/kernel8.elf $BUILD_DIR/kernel8.img

echo "Build completed. Output: $BUILD_DIR/kernel8.img"

if [ "$1" == "qemu" ]; then
    qemu-system-aarch64 -M raspi4b -kernel $BUILD_DIR/kernel8.img -serial stdio -serial pty -display none -drive file=sd.img,if=sd,format=raw
fi