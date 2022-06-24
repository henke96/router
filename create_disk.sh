#!/bin/sh
# Usage: ./create_disk.sh [INSTALL_COMMAND]
set -e

cleanup() {
    set +e
    umount mnt/boot
    umount mnt/primary
    losetup -d $dev
}

dd if=/dev/zero of=disk.img bs=512 count=204800

dev="$(losetup --show -f disk.img)"
trap cleanup EXIT

# Create partitions and filesystems.
parted -s $dev \
mklabel gpt \
mkpart EFI 1Mib 34Mib set 1 esp on \
mkpart Primary 34Mib 99Mib

mkfs -t fat -F 32 ${dev}p1
mkfs -t ext4 ${dev}p2

# Mount the disk.
mkdir -p mnt/boot
mount ${dev}p1 mnt/boot
mkdir -p mnt/primary
mount ${dev}p2 mnt/primary
echo "Disk device is: $dev"

# Install kernel.
mkdir -p mnt/boot/EFI/BOOT
cp linux/linux*/arch/x86/boot/bzImage mnt/boot/EFI/BOOT/BOOTX64.EFI

# Install directories.
mkdir -p mnt/primary/dev
mkdir -p mnt/primary/proc
mkdir -p mnt/primary/bin

# Install binaries.
cp src/init/release.bin mnt/primary/bin/init
cp src/router/release.bin mnt/primary/bin/router

if test -n "$1"
then
    eval "$1"
else
    echo "No install command given, entering shell. Use Ctrl-D when done."
    $SHELL
fi
