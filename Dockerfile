FROM ubuntu:20.04

# タイムゾーンの設定
RUN apt update && \
    apt -y install tzdata
ENV TZ=Asia/Tokyo

# 必要パッケージのインストール
RUN apt update && \
    apt -y install git qemu-system-x86 qemu-utils build-essential clang cmake lld llvm-dev nasm iasl uuid-dev python3-distutils dosfstools

# EDK IIのビルド
RUN git clone https://github.com/tianocore/edk2.git && \
    cd /edk2  && \
    git checkout 38c8be123aced4cc8ad5c7e0da9121a181b94251 && \
    git submodule init && \
    git submodule update && \
    cd /edk2/BaseTools/Source/C && \
    make && \
    cd /edk2 && \
    ./OvmfPkg/build.sh -a X64

# Newlibビルドに必要な変数定義
ARG BASEDIR=/
ARG PREFIX=/x86_64-elf
ARG COMMON_CFLAGS="-nostdlibinc -O2 -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS"
ARG CC=clang
ARG CXX=clang++
ARG TARGET_TRIPLE=x86_64-elf

# Nwelibビルド
RUN cd $BASEDIR && \
    git clone --depth 1 --branch fix-build https://github.com/uchan-nos/newlib-cygwin.git && \
    mkdir build_newlib && \
    cd build_newlib && \
    ../newlib-cygwin/newlib/configure CC=$CC CC_FOR_BUILD=$CC CFLAGS="-fPIC $COMMON_CFLAGS" --target=$TARGET_TRIPLE --prefix=$PREFIX --disable-multilib --disable-newlib-multithread && \
    make -j 4 && \
    make install

# libc++abiのビルド
RUN cd $BASEDIR && \
    git clone --depth 1 --branch llvmorg-10.0.0 https://github.com/llvm/llvm-project.git && \
    mkdir build_libcxxabi && \
    cd build_libcxxabi && \
    cmake -G "Unix Makefiles" \
        -DCMAKE_INSTALL_PREFIX=$PREFIX \
        -DCMAKE_CXX_COMPILER=$CXX \
        -DCMAKE_CXX_FLAGS="-I$PREFIX/include $COMMON_CFLAGS -D_LIBCPP_HAS_NO_THREADS" \
        -DCMAKE_C_COMPILER=$CC \
        -DCMAKE_C_FLAGS="-I$PREFIX/include $COMMON_CFLAGS -D_LIBCPP_HAS_NO_THREADS" \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
        -DCMAKE_BUILD_TYPE=Release \
        -DLIBCXXABI_LIBCXX_INCLUDES="$BASEDIR/llvm-project/libcxx/include" \
        -DLIBCXXABI_ENABLE_EXCEPTIONS=False \
        -DLIBCXXABI_ENABLE_THREADS=False \
        -DLIBCXXABI_TARGET_TRIPLE=$TARGET_TRIPLE \
        -DLIBCXXABI_ENABLE_SHARED=False \
        -DLIBCXXABI_ENABLE_STATIC=True \
        $BASEDIR/llvm-project/libcxxabi && \
    make -j4 && \
    make install

# libc++のビルド
RUN cd $BASEDIR && \
    mkdir build_libcxx && \
    cd build_libcxx && \
    cmake -G "Unix Makefiles" \
        -DCMAKE_INSTALL_PREFIX=$PREFIX \
        -DCMAKE_CXX_COMPILER=$CXX \
        -DCMAKE_CXX_FLAGS="-I$PREFIX/include $COMMON_CFLAGS" \
        -DCMAKE_CXX_COMPILER_TARGET=$TARGET_TRIPLE \
        -DCMAKE_C_COMPILER=$CC \
        -DCMAKE_C_FLAGS="-I$PREFIX/include $COMMON_CFLAGS" \
        -DCMAKE_C_COMPILER_TARGET=$TARGET_TRIPLE \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
        -DCMAKE_BUILD_TYPE=Release \
        -DLIBCXX_CXX_ABI=libcxxabi \
        -DLIBCXX_CXX_ABI_INCLUDE_PATHS="$BASEDIR/llvm-project/libcxxabi/include" \
        -DLIBCXX_CXX_ABI_LIBRARY_PATH="$PREFIX/lib" \
        -DLIBCXX_ENABLE_EXCEPTIONS=False \
        -DLIBCXX_ENABLE_FILESYSTEM=False \
        -DLIBCXX_ENABLE_MONOTONIC_CLOCK=False \
        -DLIBCXX_ENABLE_RTTI=False \
        -DLIBCXX_ENABLE_THREADS=False \
        -DLIBCXX_ENABLE_SHARED=False \
        -DLIBCXX_ENABLE_STATIC=True \
        $BASEDIR/llvm-project/libcxx && \
    make -j4 && \
    make install

# 後片付け
RUN cd $BASEDIR && \
    rm -r build_newlib newlib-cygwin llvm-project build_libcxxabi build_libcxx