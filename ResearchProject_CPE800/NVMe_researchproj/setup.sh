#!/bin/bash

# Configuration
ARCH="riscv64"
CROSS="riscv64-linux-gnu-"
BB_VER="1.36.1"
TP_DIR="$(pwd)/thirdparties/non-permanent"
ROOTFS="$TP_DIR/rootfs"

echo "--- 1. Cleaning up thirdparties ---"
# Unmount if necessary to avoid 'Device busy'
sudo umount -l $ROOTFS/dev $ROOTFS/proc $ROOTFS/sys 2>/dev/null || true

echo "--- 2. Building BusyBox in $TP_DIR ---"
cd $TP_DIR

if [ ! -d "busybox-$BB_VER" ]; then
    wget -qO- https://busybox.net/downloads/busybox-$BB_VER.tar.bz2 | tar -xjf -
    mv busybox-$BB_VER busybox_src
fi

cd busybox_src
# Force static linking so we don't need shared libs in the initrd
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc)
make ARCH=$ARCH CROSS_COMPILE=$CROSS -j$(nproc) install CONFIG_PREFIX=$ROOTFS
     
cd ../..

echo "--- 3. Creating Initrd with Folder Sharing ---"
mkdir -p $ROOTFS/dev
mkdir -p $ROOTFS/proc
mkdir -p $ROOTFS/sys
mkdir -p $ROOTFS/etc
mkdir -p $ROOTFS/src

#copy the init script inside rootfs/init
cat <<EOF > $ROOTFS/init
#!/bin/sh
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev

# Mount the root project dir to /src
mount -t 9p -o trans=virtio,version=9p2000.L project_root /src

echo "--- RISC-V RESEARCH ENV READY ---"
echo "Project root is shared at /src"
cd /src
/bin/sh
EOF

chmod +x $ROOTFS/init

echo "--- 4. Packing thirdparties/rootfs.cpio ---"
cd $ROOTFS
find . | cpio -H newc -o > $TP_DIR/../rootfs.cpio
cd ../../../

echo "--- 5. Compile kernel on RISC-V target ---"

cd thirdparties/permanent/linux
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- menuconfig
make ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc)
riscv64-linux-gnu-objcopy -O binary vmlinux Image
mv Image ../../
cd ../../../

echo "--- 6. Generate NVMe disk ---"

qemu-img create -f raw thirdparties/nvme_disk.img 1G

echo "--- DONE ---"