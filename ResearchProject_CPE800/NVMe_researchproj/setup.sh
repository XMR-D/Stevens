#!/bin/bash

# Configuration
ARCH="riscv64"
CROSS="riscv64-linux-gnu-"
BB_VER="1.36.1"
TP_DIR="$(pwd)/thirdparties/non-permanent"
ROOTFS="$TP_DIR/rootfs"

echo "--- 1. Cleaning up thirdparties ---"
sudo umount -l $ROOTFS/dev $ROOTFS/proc $ROOTFS/sys 2>/dev/null || true

echo "--- 2. Building BusyBox in $TP_DIR ---"
cd $TP_DIR
rm -rdf rootfs/

if [ ! -d "busybox-$BB_VER" ]; then
    wget -qO- https://busybox.net/downloads/busybox-$BB_VER.tar.bz2 | tar -xjf -
fi

cd busybox-$BB_VER
sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config
sed -i 's/CONFIG_PIE=y/# CONFIG_PIE is not set/' .config
sed -i 's/CONFIG_TC=y/# CONFIG_TC is not set/' .config

make ARCH=riscv CROSS_COMPILE=$CROSS -j$(nproc)
make ARCH=$ARCH CROSS_COMPILE=$CROSS -j$(nproc) install CONFIG_PREFIX=$ROOTFS
cd ../..

echo "--- 3. Creating Initrd with Folder Sharing ---"
mkdir -p $ROOTFS/dev $ROOTFS/proc $ROOTFS/sys $ROOTFS/etc $ROOTFS/src $ROOTFS/lib

# Revised /init script
cat <<EOF > $ROOTFS/init
#!/bin/sh
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev

# --- Mount only your source/projects directory ---
# 'projectshare' is the tag we will use in the QEMU command
mount -t virtiofs projectshare /src

echo "------------------------------------------------"
echo "NVMe Research Environment Loaded"
echo "Shared projects are available in /src"
echo "------------------------------------------------"

cd src/
./nvme_setup.sh

exec /bin/sh
EOF

chmod +x $ROOTFS/init

echo "--- 4. Packing rootfs  ---"

# Cpio packing, pack the Busybox
cd $ROOTFS
find . -mindepth 1 | cpio -H newc -o | gzip -9 > $TP_DIR/../rootfs.cpio.gz
cd ../../../

echo "--- 5. Compile kernel on RISC-V target ---"


IMAGE_PATH="thirdparties/Image"
if [ ! -f "$IMAGE_PATH" ]; then
    echo "Kernel Image not found. Building..."
    cd thirdparties/permanent/linux
    make ARCH=riscv CROSS_COMPILE=$CROSS -j$(nproc)
    riscv64-linux-gnu-objcopy -O binary vmlinux Image
    mv Image ../../
    cd ../../../
else
    echo "Kernel Image already exists."
fi

echo "--- 6. Generate NVMe disk ---"
qemu-img create -f raw thirdparties/nvme_disk.img 1G

echo "--- DONE ---"