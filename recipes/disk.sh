#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "./bootloader.sh ./host_mtools.sh"

recipe_start
dd if=/dev/zero of=disk.img bs=1048576 count=8
mformat -i disk.img -N 0 -v ROUTER ::
mmd -i disk.img ::/efi
mmd -i disk.img ::/efi/boot
cp disk.img debug.disk.img
mcopy -i disk.img ../bootloader/x86_64/bootloader.efi ::/efi/boot/bootx64.efi
mcopy -i debug.disk.img ../bootloader/x86_64/debug.bootloader.efi ::/efi/boot/bootx64.efi

mkdir ../$RECIPE_NAME
mv disk.img ../$RECIPE_NAME/
mv debug.disk.img ../$RECIPE_NAME/
recipe_finish

