#!/bin/sh --
set -eax
cd -- "${0%/*}/"
. ../hc/bootstrap/recipe.sh
recipe_init "../hc/bootstrap/make.sh ../hc/bootstrap/xz.sh ../hc/bootstrap/cmake.sh ../hc/bootstrap/python.sh ./host_llvm.sh" "./musl.sh ./linux-headers.sh"

pkg="llvm-project-16.0.6.src"
URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.6/$pkg.tar.xz"
SHA512="89a67ebfbbc764cc456e8825ecfa90707741f8835b1b2adffae0b227ab1fe5ca9cce75b0efaffc9ca8431cae528dc54fd838867a56a2b645344d9e82d19ab1b7"

recipe_start
rm -rf "./$pkg"; xz -d -c "$DOWNLOAD" | tar xf -; cd "./$pkg"

arch=x86_64
cmake -S runtimes -B libcxxbuild -G "Unix Makefiles" -Wno-dev \
-DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/$RECIPE_NAME" \
-DCMAKE_MAKE_PROGRAM=make -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang \
-DCMAKE_SYSROOT="$SCRIPT_DIR/musl/$arch" \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_DOCS=OFF \
-DLLVM_HOST_TRIPLE="x86_64-unknown-linux-musl" \
-DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
-DLIBCXX_ENABLE_STATIC=ON \
-DLIBCXX_ENABLE_SHARED=OFF \
-DLIBCXX_HAS_MUSL_LIBC=ON \
-DLIBCXX_ADDITIONAL_COMPILE_FLAGS="-I$SCRIPT_DIR/linux-headers/$arch" \
-DLIBCXX_INCLUDE_BENCHMARKS=OFF \
-DLIBCXX_CXX_ABI="libcxxabi" \
-DLIBCXX_ENABLE_STATIC_ABI_LIBRARY=ON \
-DLIBCXXABI_ENABLE_STATIC=ON \
-DLIBCXXABI_ENABLE_SHARED=OFF \
-DLIBCXXABI_USE_LLVM_UNWINDER=ON \
-DLIBCXXABI_ENABLE_STATIC_UNWINDER=ON \
-DLIBCXXABI_USE_COMPILER_RT=ON \
-DLIBUNWIND_ENABLE_STATIC=ON \
-DLIBUNWIND_ENABLE_SHARED=OFF \
-DLIBUNWIND_USE_COMPILER_RT=ON
make -C libcxxbuild -j "$NUM_CPUS" install

# https://github.com/llvm/llvm-project/issues/64014
cmake -S llvm -B build -G "Unix Makefiles" -Wno-dev \
-DCMAKE_SYSTEM_NAME="Linux" \
-DLLVM_HOST_TRIPLE="x86_64-unknown-linux-musl" \
-DLLVM_NATIVE_TOOL_DIR="$SCRIPT_DIR/host_llvm/bin" \
-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
-DLLVM_BUILD_STATIC=ON \
-DCMAKE_MAKE_PROGRAM=make -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/$RECIPE_NAME" \
-DCMAKE_BUILD_TYPE=Release \
-DLLVM_APPEND_VC_REV=OFF \
-DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_DOCS=OFF -DLLVM_INCLUDE_UTILS=OFF \
-DLLVM_ENABLE_DOXYGEN=OFF -DLLVM_ENABLE_FFI=OFF -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_CURL=OFF -DLLVM_ENABLE_LIBEDIT=OFF -DLLVM_ENABLE_LIBPFM=OFF -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_BINDINGS=OFF -DLLVM_ENABLE_UNWIND_TABLES=OFF -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_HAVE_LIBXAR=OFF -DCLANG_ENABLE_ARCMT=OFF \
-DLLVM_ENABLE_PROJECTS="clang;lld" \
-DLLVM_TARGETS_TO_BUILD="X86" \
-DLLVM_DEFAULT_TARGET_TRIPLE="x86_64-unknown-linux-musl" \
-DDEFAULT_SYSROOT="/TODO" -DCLANG_DEFAULT_LINKER="lld" -DCLANG_DEFAULT_RTLIB="compiler-rt" \
-DCMAKE_SYSROOT="$SCRIPT_DIR/musl/$arch" \
-DCMAKE_CXX_FLAGS="-I$SCRIPT_DIR/$RECIPE_NAME/include -I$SCRIPT_DIR/$RECIPE_NAME/include/c++/v1" \
-DCMAKE_EXE_LINKER_FLAGS="-L$SCRIPT_DIR/$RECIPE_NAME/lib"
make -C build -j "$NUM_CPUS" install-clang install-lld

llvm-objcopy --strip-sections "../$RECIPE_NAME/bin/clang-16"
llvm-objcopy --strip-sections "../$RECIPE_NAME/bin/lld"

cd ..; rm -rf "./$pkg"
recipe_finish
