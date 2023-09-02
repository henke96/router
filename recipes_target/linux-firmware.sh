#!/bin/sh --
set -e
script_dir="$(cd -- "$(dirname -- "$0")" && pwd)"
. "$script_dir/../hc/bootstrap/recipe.sh"

URL="https://www.kernel.org/pub/linux/kernel/firmware/linux-firmware-20230625.tar.xz"
SHA256="87597111c0d4b71b31e53cb85a92c386921b84c825a402db8c82e0e86015500d"
DEPENDENCIES="../hc/bootstrap/xz.sh"

recipe_start
cd ..
mv linux-firmware-20230625 linux-firmware
cd linux-firmware
recipe_finish
