#!/bin/sh
set -e

cleanup() {
    set +e
    umount mnt
    losetup -d $dev
}

dev="$(losetup --show -P -f disk.img)"
trap cleanup EXIT

mkdir -p mnt/
mount ${dev}p1 mnt/

echo "Disk device is: $dev"
echo "Entering shell, use Ctrl-D when done."
$SHELL
