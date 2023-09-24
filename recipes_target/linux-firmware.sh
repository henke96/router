#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/xz.sh"

URL="https://www.kernel.org/pub/linux/kernel/firmware/linux-firmware-20230625.tar.xz"
SHA256="87597111c0d4b71b31e53cb85a92c386921b84c825a402db8c82e0e86015500d"

recipe_start
cd ..
mv "$SOURCE_DIR_NAME" "$RECIPE_NAME"
cd "$RECIPE_NAME"
recipe_finish
