include mkflags.mk

all: run

# Command Execution
run:
		sudo /usr/lib/qemu/virtiofsd --socket-path=/tmp/vfsd.sock --shared-dir=./project-src --sandbox none &
		sudo $(QEMU_BIN) -M $(QEMU_MACHINE) \
		-cpu $(QEMU_CPU) \
		-m $(MEM) \
		-smp $(SMP) \
		-display none -serial stdio \
		-kernel $(VMLINUX) \
		-initrd $(INITRD) \
		-bios default \
		-object memory-backend-memfd,id=mem,size=$(MEM),share=on \
        -numa node,memdev=mem \
        -chardev socket,id=char0,path=/tmp/vfsd.sock \
        -device vhost-user-fs-pci,queue-size=1024,chardev=char0,tag=hostshare,addr=03.0 \
		-append "console=ttyS0 root=/dev/ram0 rw vfio_iommu_type1.allow_unsafe_interrupts=1" \
		-device riscv-iommu-pci,addr=01.0 \
		-drive format=raw,file=$(NVME_DISK),if=none,id=nvm0 \
		-device nvme,drive=nvm0,serial=deadbeef,addr=02.0

clean:
	rm -rdf thirdparties/non-permanent/*