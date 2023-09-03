. ../hc/bootstrap/recipe.sh
recipe_init "./bootloader$DEBUG_SUFFIX.sh ../recipes_host/mtools.sh"

FILE_DEPENDENCIES="files/disk/base_recipe.sh"

recipe_start
dd if=/dev/zero of=disk.img bs=1048576 count=8
mformat -i disk.img -N 0 -v ROUTER ::
mmd -i disk.img ::/efi
mmd -i disk.img ::/efi/boot
mcopy -i disk.img ../bootloader$DEBUG_SUFFIX/x86_64/${DEBUG_PREFIX}bootloader.efi ::/efi/boot/bootx64.efi

mkdir ../$RECIPE_NAME
mv disk.img ../$RECIPE_NAME/disk.img
recipe_finish
