#!/bin/sh
# Usage: ./build_disk.sh [INSTALL_COMMAND]
set -e

# Build everything in src/
src/bootloader/build.sh
src/init/build.sh
src/router/build.sh

cleanup() {
    set +e
    umount mnt/
    losetup -d $dev
}

dd if=/dev/zero of=disk.img bs=1048576 count=8
dd if=/dev/zero of=disk2.img bs=1048576 count=16

dev="$(losetup --show -f disk.img)"
trap cleanup EXIT

# Create partitions and filesystems.
parted -s $dev \
mklabel gpt \
mkpart Router 2048s 100% set 1 esp on

mkfs -t fat -F 12 ${dev}p1

# Mount the disk.
mkdir -p mnt/
mount ${dev}p1 mnt/
echo "Disk device is: $dev"

# Install kernel.
mkdir -p mnt/EFI/BOOT
cp src/bootloader/bootloader.efi mnt/EFI/BOOT/BOOTX64.EFI

# Install directories.
mkdir -p mnt/dev
mkdir -p mnt/proc
mkdir -p mnt/sys
mkdir -p mnt/mnt
mkdir -p mnt/bin

# Install binaries.
cp src/init/release.bin mnt/bin/init
cp src/router/release.bin mnt/bin/router

if test -n "$1"
then
    eval "$1"
else
    echo "No install command given, entering shell. Use Ctrl-D when done."
    $SHELL
fi
