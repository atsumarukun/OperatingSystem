#!/bin/bash

DISK_IMG=disk.img
BASE_DIR=/os
EDK2_DIR=/edk2
MOUNT_POINT=$BASE_DIR/mnt

# build boot loader
/edk2/build.sh

# sestting qemu
cd $BASE_DIR
if [ ! -d $MOUNT_POINT ]
then
    mkdir $MOUNT_POINT
fi

rm -f $BASE_DIR/$DISK_IMG

qemu-img create -f raw $BASE_DIR/$DISK_IMG 200M
mkfs.fat -n 'OperatingSystem' -s 2 -f 2 -R 32 -F 32 $BASE_DIR/$DISK_IMG

mount -o loop $BASE_DIR/$DISK_IMG $MOUNT_POINT
mkdir -p $MOUNT_POINT/EFI/BOOT

cp $EDK2_DIR/Build/LoaderPkgX64/DEBUG_CLANG38/X64/Loader.efi $MOUNT_POINT/EFI/BOOT/BOOTX64.EFI
umount $MOUNT_POINT

# run qemu
qemu-system-x86_64 \
    -m 1G \
    -drive if=pflash,format=raw,file=$EDK2_DIR/Build/OvmfX64/DEBUG_GCC5/FV/OVMF_CODE.fd \
    -drive if=pflash,format=raw,file=$EDK2_DIR/Build/OvmfX64/DEBUG_GCC5/FV/OVMF_VARS.fd \
    -drive if=ide,index=0,media=disk,format=raw,file=$BASE_DIR/$DISK_IMG \
    -device nec-usb-xhci,id=xhci \
    -device usb-mouse \
    -device usb-kbd \
    -monitor stdio $QEMU_OPTS

# reset qemu
rm -r $BASE_DIR/$DISK_IMG $MOUNT_POINT