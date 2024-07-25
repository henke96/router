#!/bin/sh --
set -e
script_dir="$(cd -- "${0%/*}/" && pwd)"
OUT="$(cd -- "$OUT" && pwd)"

for pkg in bash e2fsprogs ncurses nano utilitiesExtra; do
    "$script_dir/recipesExtra/$pkg"
    if test -d "$OUT/$pkg/bin"; then
        export PATH="$OUT/$pkg/bin:$PATH"
    fi
    if test -d "$OUT/$pkg/sbin"; then
        export PATH="$OUT/$pkg/sbin:$PATH"
    fi
done

for pkg in make xz; do
    "$script_dir/recipes/$pkg"
    if test -d "$OUT/$pkg/bin"; then
        export PATH="$OUT/$pkg/bin:$PATH"
    fi
    if test -d "$OUT/$pkg/sbin"; then
        export PATH="$OUT/$pkg/sbin:$PATH"
    fi
done

if test -n "$EXTRA_ALL"; then
    for pkg in bc bison bzip2 cmake flex llvm m4 perl python; do
        "$script_dir/recipes/$pkg"
        if test -d "$OUT/$pkg/bin"; then
            export PATH="$OUT/$pkg/bin:$PATH"
        fi
        if test -d "$OUT/$pkg/sbin"; then
            export PATH="$OUT/$pkg/sbin:$PATH"
        fi
    done
fi

exec setsid "$OUT/bash/bin/bash" --rcfile "$OUT/bash/bashrc"
