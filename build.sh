#!/bin/bash

cd /edk2/LoaderPkg

if [ ! -e memorymap.hpp ]; then
  ln -s /os/kernel/memory/memorymap.hpp
  echo test
fi

if [ ! -e elf.hpp ]; then
  ln -s /os/kernel/file/elf.hpp
fi

cd /edk2
source ./edksetup.sh
build