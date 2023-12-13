#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ../hc/bootstrap/cmake.sh ../hc/bootstrap/python.sh" "./musl-headers.sh"

pkg="llvm-project-16.0.6.src"
URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.6/$pkg.tar.xz"
SHA512="89a67ebfbbc764cc456e8825ecfa90707741f8835b1b2adffae0b227ab1fe5ca9cce75b0efaffc9ca8431cae528dc54fd838867a56a2b645344d9e82d19ab1b7"

recipe_start
rm -rf "./$pkg"; xz -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

cmake -S llvm -B build -G "Unix Makefiles" -Wno-dev \
-DCMAKE_MAKE_PROGRAM=make -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" \
-DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/$RECIPE_NAME" \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_APPEND_VC_REV=OFF \
-DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_DOCS=OFF -DLLVM_INCLUDE_UTILS=OFF \
-DLLVM_ENABLE_DOXYGEN=OFF -DLLVM_ENABLE_FFI=OFF -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_CURL=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_BINDINGS=OFF -DLLVM_ENABLE_UNWIND_TABLES=OFF -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_HAVE_LIBXAR=OFF -DCLANG_ENABLE_ARCMT=OFF \
-DLLVM_ENABLE_PROJECTS="clang;lld" \
-DLLVM_TARGETS_TO_BUILD="X86" \
-DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-unknown-linux-musl" \
-DDEFAULT_SYSROOT="/path/that/doesnt/exist" -DCLANG_DEFAULT_LINKER="lld" -DCLANG_DEFAULT_RTLIB="compiler-rt" -DCLANG_DEFAULT_CXX_STDLIB="libc++" \
-DLLVM_ENABLE_RUNTIMES="compiler-rt" \
-DLLVM_BUILTIN_TARGETS="x86_64-unknown-linux-musl" \
-DLLVM_RUNTIME_TARGETS="x86_64-unknown-linux-musl" \
-DBUILTINS_x86_64-unknown-linux-musl_CMAKE_SYSROOT="$SCRIPT_DIR/musl-headers/x86_64" \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_SCUDO_STANDALONE_BUILD_SHARED=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_GWP_ASAN=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_ORC=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_MEMPROF=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_PROFILE=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_LIBFUZZER=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_XRAY=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_SANITIZERS=OFF
make -C build -j "$NUM_CPUS" install-runtimes install-x86-resource-headers/fast install-utility-resource-headers/fast install-builtins-x86_64-unknown-linux-musl/fast install-clang/fast install-lld/fast install-llvm-ar/fast install-llvm-ranlib/fast install-llvm-nm/fast install-llvm-strip/fast install-llvm-objcopy/fast install-llvm-objdump/fast install-llvm-readobj/fast install-llvm-readelf/fast install-llvm-tblgen/fast install-clang-tblgen/fast

cd ..; rm -rf "./$pkg"
recipe_finish
