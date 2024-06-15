#!/bin/sh --
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"

for pkg in bash e2fsprogs ncurses nano; do
    "$script_dir/recipesExtra/$pkg"
    export PATH="$OUT/$pkg/bin:$PATH"
done

for pkg in make xz; do
    "$script_dir/recipes/$pkg"
    export PATH="$OUT/$pkg/bin:$PATH"
done

if test -n "$EXTRA_ALL"; then
    for pkg in bc bison bzip2 cmake flex llvm m4 perl python; do
        "$script_dir/recipes/$pkg"
        export PATH="$OUT/$pkg/bin:$PATH"
    done
fi

echo "Entering bash"
exec bash
