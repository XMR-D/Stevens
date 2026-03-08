
# Path Configuration
QEMU_BIN       = ../../thirdparties/permanent/qemu/build/qemu-system-riscv64
VMLINUX        = ../../thirdparties/Image
INITRD         = ../../thirdparties/rootfs.cpio.gz
NVME_DISK      = ../../thirdparties/nvme_disk.img

# Machine & CPU Configuration
# 'aia=aplic-imsic' is the modern RISC-V interrupt standard
# 'cpu=max' ensures all experimental extensions are available
QEMU_MACHINE   = virt,aia=aplic-imsic
QEMU_CPU       = max
MEM            = 4G
SMP            = 4

# ON HOST COMPILATION FLAGS
CC = ../../thirdparties/permanent/riscv64-lp64d--musl--stable-2025.08-1/bin/riscv64-buildroot-linux-musl-gcc
AR=$(CROSS-COMPILE)ar
INCLUDE=-Iinclude -Ikernel_headers/include 
CFLAGS = -static -march=rv64gc -march=rv64gc_zihintpause -mabi=lp64d -O0 -g 