#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_llvm.sh" "./musl.sh ./libnl3.sh"

pkg="hostapd-2.10"
URL="https://w1.fi/releases/$pkg.tar.gz"
SHA512="243baa82d621f859d2507d8d5beb0ebda15a75548a62451dc9bca42717dcc8607adac49b354919a41d8257d16d07ac7268203a79750db0cfb34b51f80ff1ce8f"
FILE_DEPENDENCIES="files/hostapd/.config"

recipe_start
rm -rf "./$pkg"; gzip -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

cp ../files/hostapd/.config hostapd/.config
arch=x86_64
export CC=clang
export CFLAGS="-target $arch-unknown-linux-musl --sysroot $SCRIPT_DIR/musl/$arch -I$SCRIPT_DIR/linux-headers/$arch/include"
export LDFLAGS="-target $arch-unknown-linux-musl --sysroot $SCRIPT_DIR/musl/$arch -static -L$SCRIPT_DIR/libnl3/lib"
make -C hostapd -j "$NUM_CPUS" install BINDIR= DESTDIR="$SCRIPT_DIR/hostapd" LIBNL_INC="$SCRIPT_DIR/libnl3/include/libnl3"
objcopy --strip-sections "$SCRIPT_DIR/hostapd/hostapd"

cd ..; rm -rf "./$pkg"
recipe_finish
