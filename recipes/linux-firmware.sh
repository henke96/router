#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/xz.sh"

URL="https://www.kernel.org/pub/linux/kernel/firmware/linux-firmware-20230625.tar.xz"
SHA512="0e48aa7f63495485426d37491c7cb61843165625bd47f912c5d83628c6de871759f1a78be3af3d651f7c396bd87dff07e21ba7afc47896c1c143106d5f16d351"

recipe_start
cd ..
mv "$SOURCE_DIR_NAME" "$RECIPE_NAME"
cd "$RECIPE_NAME"
recipe_finish
