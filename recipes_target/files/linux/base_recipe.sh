. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ../recipes_host/llvm.sh ../recipes_host/bc.sh ../recipes_host/flex.sh ../recipes_host/bison.sh ../recipes_host/elfutils.sh ./linux-firmware.sh ./init.sh ./router.sh"

URL="https://www.kernel.org/pub/linux/kernel/v6.x/linux-6.1.48.tar.xz"
SHA256="c606cbd0353e677df6fae73cc16ba3c9244b98372ed7771d551024016f55ac31"
FILE_DEPENDENCIES="files/linux/initramfs files/linux/.config files/linux/regulatory.db files/linux/base_recipe.sh"

recipe_start
cp ../files/linux/.config .config
export KBUILD_BUILD_TIMESTAMP="@" KBUILD_BUILD_USER="@" KBUILD_BUILD_HOST="@" ARCH=x86_64
make -j "$NUM_CPUS" CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm STRIP=llvm-strip OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump READELF=llvm-readelf HOSTCFLAGS="-I$SCRIPT_DIR/../recipes_host/elfutils/include" HOSTLDFLAGS="-Wl,-rpath,$SCRIPT_DIR/../recipes_host/elfutils/lib -L$SCRIPT_DIR/../recipes_host/elfutils/lib"
mkdir "../$RECIPE_NAME"
mv arch/x86/boot/bzImage "../$RECIPE_NAME/"
recipe_finish
