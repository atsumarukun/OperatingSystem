# OperatingSystem
このリポジトリは[『ゼロからのOS自作入門』](https://www.amazon.co.jp/%E3%82%BC%E3%83%AD%E3%81%8B%E3%82%89%E3%81%AEOS%E8%87%AA%E4%BD%9C%E5%85%A5%E9%96%80-%E5%86%85%E7%94%B0-%E5%85%AC%E5%A4%AA/dp/4839975868)を参考にオペレーティング・システムを開発しているものです。本コードは上記技術書を参考にしたもので、写経したものではありません。本環境にはDockerを利用しており、基本的なLinux, Dockerの知識が必要になります。<br />
<br />
本OSのビルド手順は簡単なもので、dockerコンテナを立ち上げるだけです。<br />

```bash
docker-compose up
```

上記コマンドを実行するだけで環境が立ち上がり、ブートローダ, カーネルのコンパイル及び[QEMU](https://www.qemu.org/)によるブートが行われます。<br />
<br />

## 環境
Dockerfile内でインストールしている環境に関して説明します。Docker環境を利用しない場合はこちらを参照してください。<br />
### EDK II
本OSのブートローダにはUEFI規格を採用しています。[EDK II](https://github.com/tianocore/edk2)はUEFIの開発に深く関わっているIntelが作成したSDKです。利用方法は[こちら](https://osdev-jp.readthedocs.io/ja/latest/2017/create-uefi-app-with-edk2.html)を参照ください。<br />
<br />
まずedk2リポジトリをダウンロードしてブランチを切り替えます。最新バージョンでは`nasm`のバージョンが合わずbuild時にエラーになってしまいます(2023/01/28現在)。そのため過去のブランチに切り替えています。<br />

```bash
$ git clone https://github.com/tianocore/edk2.git
$ cd edk2
$ git checkout 38c8be123aced4cc8ad5c7e0da9121a181b94251
```

次にサブモジュールを初期化し、更新します。[サブモジュール](https://git-scm.com/book/ja/v2/Git-%E3%81%AE%E3%81%95%E3%81%BE%E3%81%96%E3%81%BE%E3%81%AA%E3%83%84%E3%83%BC%E3%83%AB-%E3%82%B5%E3%83%96%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB)とは、自分のリポジトリで他人のリポジトリをサブモジュールとして登録し、特定のcommitを参照する仕組みです。<br />

```bash
$ git submodule init
$ git submodule update
```

最後にブートローダのコンパイルを行えるよう実行ファイルを作成します。<br />

```bash
$ cd BaseTools/Source/C
$ make
```

なおブートローダは`edk2`ディレクトリ直下で環境変数を読み取り、buildすることでコンパイルを行っています。下記コマンドはedkをルートディレクトリ直下にcloneした場合の例です。<br />
```bash
$ cd /edk2
$ source edksetup.sh
$ build
```
<br />

### Newlib
組み込みシステムでの使用を意図して作成された標準Cライブラリです。開発に標準C++ライブラリである`libc++`を利用しているのですが、標準Cライブラリに依存しているためNewlibをインストールする必要があります。<br />
まずはインストールの際に必要な変数を定義します。<br />

```bash
$ BASEDIR=/
$ PREFIX=/x86_64-elf
$ COMMON_CFLAGS="-nostdlibinc -O2 -D__ELF__ -D_LDBL_EQ_DBL -D_GNU_SOURCE -D_POSIX_TIMERS"
$ CC=clang
$ CXX=clang++
$ TARGET_TRIPLE=x86_64-elf
```

実際にNewlibをインストールしていきます。<br />

```bash
$ cd $BASEDIR
$ git clone --depth 1 --branch fix-build https://github.com/uchan-nos/newlib-cygwin.git && \
$ mkdir build_newlib
$ cd build_newlib
$ ../newlib-cygwin/newlib/configure CC=$CC CC_FOR_BUILD=$CC CFLAGS="-fPIC $COMMON_CFLAGS" --target=$TARGET_TRIPLE --prefix=$PREFIX --disable-multilib --disable-newlib-multithread
$ make -j 4
$ make install
```
<br />

### libc++/libc++abi
標準C++ライブラリであるlibc++と、アーキテクチャの一部機能を提供しているlibc++abiをインストールします。<br />
まずはlibc++abiからインストールしていきます。<br />

```bash
$ cd $BASEDIR
$ git clone --depth 1 --branch llvmorg-10.0.0 https://github.com/llvm/llvm-project.git
$ mkdir build_libcxxabi
$ cd build_libcxxabi
$ cmake -G "Unix Makefiles" \
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
    $BASEDIR/llvm-project/libcxxabi
$ make -j4
$ make install
```

上記でGitHubからCloneしている`llvm-project`のバージョンはコンパイラ(clang)のバージョンにあわせています。<br />
次にlibc++をインストールします。<br />

```bash
$ cd $BASEDIR
$ mkdir build_libcxx
$ cd build_libcxx
$ cmake -G "Unix Makefiles" \
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
    $BASEDIR/llvm-project/libcxx
$ make -j4
$ make install
```

<br />

## 開発環境
環境| バージョン
--- | ---
CPU | AMD Ryzen 5 2600
ArchLinux(x86_64) | 6.1.7-arch1-1
Docker | 20.10.23
Docker Compose | 2.14.2
<br />

## 参考文献
1. 内田公太(2021).[『ゼロからのOS自作入門』](https://www.amazon.co.jp/%E3%82%BC%E3%83%AD%E3%81%8B%E3%82%89%E3%81%AEOS%E8%87%AA%E4%BD%9C%E5%85%A5%E9%96%80-%E5%86%85%E7%94%B0-%E5%85%AC%E5%A4%AA/dp/4839975868).マイナビ
2. osdev.jp[「libc++ のビルド」](https://osdev.jp/wiki/building-libcxx).osdev.jp(2023/01/29)