# OperatingSystem
このリポジトリは[『ゼロからのOS自作入門』](https://www.amazon.co.jp/%E3%82%BC%E3%83%AD%E3%81%8B%E3%82%89%E3%81%AEOS%E8%87%AA%E4%BD%9C%E5%85%A5%E9%96%80-%E5%86%85%E7%94%B0-%E5%85%AC%E5%A4%AA/dp/4839975868)を参考にオペレーティング・システムを開発しているものです。本コードは上記技術書を参考にしたもので、写経したものではありません。本環境にはDockerを利用しており、基本的なLinux, Dockerの知識が必要になります。<br />
<br />
本OSのビルド手順は簡単なもので、dockerコンテナを立ち上げるだけです。

```bash
docker-compose up
```

上記コマンドを実行するだけで環境が立ち上がり、ブートローダ, カーネルのコンパイル及び[QEMU](https://www.qemu.org/)によるブートが行われます。
<br />

## 環境
Dockerfile内でインストールしている環境に関して説明します。
### EDK II
本OSのブートローダにはUEFI規格を採用しています。[EDK II](https://github.com/tianocore/edk2)はUEFIの開発に深く関わっているIntelが作成したSDKです。利用方法は[こちら](https://osdev-jp.readthedocs.io/ja/latest/2017/create-uefi-app-with-edk2.html)を参照ください。<br />
<br />
まずedk2リポジトリをダウンロードしてブランチを切り替えます。最新バージョンでは`nasm`のバージョンが合わずbuild時にエラーになってしまいます(2023/01/28現在)。そのため過去のブランチに切り替えています。

```bash
$ git clone https://github.com/tianocore/edk2.git
$ cd edk2
$ git checkout 38c8be123aced4cc8ad5c7e0da9121a181b94251
```

次にサブモジュールを初期化し、更新します。[サブモジュール](https://git-scm.com/book/ja/v2/Git-%E3%81%AE%E3%81%95%E3%81%BE%E3%81%96%E3%81%BE%E3%81%AA%E3%83%84%E3%83%BC%E3%83%AB-%E3%82%B5%E3%83%96%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB)とは、自分のリポジトリで他人のリポジトリをサブモジュールとして登録し、特定のcommitを参照する仕組みです。

```bash
$ git submodule init
$ git submodule update
```

最後にブートローダのコンパイルを行えるよう実行ファイルを作成します。

```bash
$ cd BaseTools/Source/C
$ make
```

なおブートローダは`edk2`ディレクトリ直下で環境変数を読み取り、buildすることでコンパイルを行っています。下記コマンドはedkをルートディレクトリ直下にcloneした場合の例です。
```bash
$ cd /edk2
$ source edksetup.sh
$ build
```
<br />

## 開発環境
環境| バージョン
--- | ---
CPU | AMD Ryzen 5 2600
ArchLinux(x86_64) | 6.1.7-arch1-1
Docker | 20.10.23
Docker Compose | 2.14.2