#!/bin/sh --
set -e
cd -- "$(dirname -- "$0")"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ../hc/bootstrap/cmake.sh ../hc/bootstrap/python.sh ../recipes_target/musl-headers.sh"

URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.6/llvm-project-16.0.6.src.tar.xz"
SHA256="ce5e71081d17ce9e86d7cbcfa28c4b04b9300f8fb7e78422b1feb6bc52c3028e"

recipe_start
cmake -S llvm -B build -G "Unix Makefiles" -Wno-dev \
-DCMAKE_MAKE_PROGRAM=make -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
-DCMAKE_INSTALL_RPATH="$SCRIPT_DIR/$RECIPE_NAME/lib" -DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/$RECIPE_NAME" -DCMAKE_BUILD_TYPE=Release -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
-DLLVM_APPEND_VC_REV=OFF \
-DLLVM_ENABLE_PROJECTS="clang;lld" \
-DLLVM_TARGETS_TO_BUILD="X86" \
-DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-unknown-linux-musl" \
-DDEFAULT_SYSROOT="/path/that/doesnt/exist" -DCLANG_DEFAULT_LINKER="lld" -DCLANG_DEFAULT_RTLIB="compiler-rt" \
-DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF \
-DLLVM_ENABLE_DOXYGEN=OFF -DLLVM_ENABLE_FFI=OFF -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_CURL=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_BINDINGS=OFF -DLLVM_ENABLE_UNWIND_TABLES=OFF -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_HAVE_LIBXAR=OFF -DCLANG_ENABLE_ARCMT=OFF \
-DLLVM_ENABLE_RUNTIMES="compiler-rt" \
-DLLVM_BUILTIN_TARGETS="x86_64-unknown-linux-musl" \
-DLLVM_RUNTIME_TARGETS="x86_64-unknown-linux-musl" \
-DBUILTINS_x86_64-unknown-linux-musl_CMAKE_SYSROOT="$SCRIPT_DIR/../recipes_target/musl-headers/x86_64" \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_SCUDO_STANDALONE_BUILD_SHARED=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_GWP_ASAN=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_ORC=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_MEMPROF=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_PROFILE=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_LIBFUZZER=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_XRAY=OFF \
-DRUNTIMES_x86_64-unknown-linux-musl_COMPILER_RT_BUILD_SANITIZERS=OFF
make -C build -j "$NUM_CPUS" install
recipe_finish
