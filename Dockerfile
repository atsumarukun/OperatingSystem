FROM ubuntu:20.04

RUN apt update && \
    apt -y install tzdata
ENV TZ=Asia/Tokyo

RUN apt update && \
    apt -y install git qemu-system-x86 qemu-utils build-essential clang lld llvm-dev nasm iasl uuid-dev python3-distutils dosfstools

RUN git clone https://github.com/tianocore/edk2.git && \
    cd /edk2  && \
    git checkout 38c8be123aced4cc8ad5c7e0da9121a181b94251 && \
    git submodule init && \
    git submodule update && \
    cd /edk2/BaseTools/Source/C && \
    make && \
    cd /edk2 && \
    ./OvmfPkg/build.sh -a X64