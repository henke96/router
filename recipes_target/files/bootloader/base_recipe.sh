. ../hc/bootstrap/recipe.sh
recipe_init "../recipes_host/llvm.sh ./hc.sh ./linux$DEBUG_SUFFIX.sh"

FILE_DEPENDENCIES="$(find files/bootloader -name "*.c" -print -o -name "*.h" -print) files/bootloader/base_recipe.sh"

recipe_start
export NO_AARCH64=1
export FLAGS="-I$SCRIPT_DIR/linux$DEBUG_SUFFIX"
../../hc/tools/build/efi.sh ../files/bootloader bootloader
mkdir ../$RECIPE_NAME
mv ../files/bootloader/x86_64 ../$RECIPE_NAME/
recipe_finish
