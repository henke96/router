#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
DEBUG_SUFFIX=-debug
. files/bootloader/base_recipe.sh
