#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ./host_bzip2.sh ./host_m4.sh" ""

pkg="elfutils-0.189"
URL="https://sourceware.org/elfutils/ftp/0.189/$pkg.tar.bz2"
SHA512="93a877e34db93e5498581d0ab2d702b08c0d87e4cafd9cec9d6636dfa85a168095c305c11583a5b0fb79374dd93bc8d0e9ce6016e6c172764bcea12861605b71"
FILE_DEPENDENCIES="files/elfutils/dummylib.c files/elfutils/libintl.h"

recipe_start
rm -rf "./$pkg"; bzip2 -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

# Trick configure script to avoid unneeded dependencies.
"$CC" -shared -o ./libfts.so ../files/elfutils/dummylib.c
cp ./libfts.so ./libargp.so
cp ./libfts.so ./libobstack.so
cp ./libfts.so ./libz.so

# Remove libz dependency.
sed -e 's/elf_compress.c elf_compress_gnu.c//; s/elf_compress.$(OBJEXT)//; s/elf_compress_gnu.$(OBJEXT)//; s/ -lz//' ./libelf/Makefile.in > ./sed.temp
mv ./sed.temp ./libelf/Makefile.in
sed -e 's/__libelf_decompress_elf (strscn, &zsize, &zalign)/NULL/' ./libelf/elf_strptr.c > ./sed.temp
mv ./sed.temp ./libelf/elf_strptr.c

./configure --prefix="$SCRIPT_DIR/$RECIPE_NAME" --disable-dependency-tracking --disable-demangler --disable-nls --without-valgrind --without-bzlib --without-lzma --without-zstd --without-libiconv-prefix --without-libintl-prefix --disable-debuginfod --disable-libdebuginfod --disable-symbol-versioning \
CFLAGS="-Wno-error" LDFLAGS="-L$(pwd)"
cp ../files/elfutils/libintl.h ./libelf/libintl.h
ar r ./lib/libeu.a
make -C libelf -j "$NUM_CPUS" install

cd ..; rm -rf "./$pkg"
recipe_finish
