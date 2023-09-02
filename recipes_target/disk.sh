#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

DEPENDENCIES="./bootloader.sh ../recipes_host/mtools.sh"

recipe_start
dd if=/dev/zero of=disk.img bs=1048576 count=8
mformat -i disk.img -N 0 -v ROUTER ::
mmd -i disk.img ::/efi
mmd -i disk.img ::/efi/boot
mcopy -i disk.img ../bootloader/x86_64/bootloader.efi ::/efi/boot/bootx64.efi

mkdir ../disk
mv disk.img ../disk/disk.img
recipe_finish
