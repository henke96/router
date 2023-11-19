#!/bin/sh --
set -ex
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ./host_rsync.sh" ""

pkg="linux-6.6"
URL="https://www.kernel.org/pub/linux/kernel/v6.x/$pkg.tar.xz"
SHA512="458b2c34d46206f9b4ccbac54cc57aeca1eaecaf831bc441e59701bac6eadffc17f6ce24af6eadd0454964e843186539ac0d63295ad2cc32d112b60360c39a35"

recipe_start
rm -rf "./$pkg"; xz -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

export ARCH=x86_64
make -j "$NUM_CPUS" headers_install INSTALL_HDR_PATH="$SCRIPT_DIR/$RECIPE_NAME/$ARCH" HOSTCC="$CC"

cd ..; rm -rf "./$pkg"
recipe_finish

