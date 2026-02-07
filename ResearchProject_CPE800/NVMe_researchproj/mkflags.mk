# Path Configuration
QEMU_BIN       = thirdparties/permanent/qemu/build/qemu-system-riscv64
VMLINUX        = thirdparties/Image
INITRD         = thirdparties/rootfs.cpio
NVME_DISK      = thirdparties/nvme_disk.img

# Machine & CPU Configuration
# 'aia=aplic-imsic' is the modern RISC-V interrupt standard
# 'cpu=max' ensures all experimental extensions are available
QEMU_MACHINE   = virt,aia=aplic-imsic
QEMU_CPU       = max
MEM            = 4G
SMP            = 4