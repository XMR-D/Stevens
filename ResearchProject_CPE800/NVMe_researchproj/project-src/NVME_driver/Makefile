include mkflags.mk

src-y:=src/main.c

all: run_term

build:
	$(CC) $(CFLAGS) $(src-y) -o test

#THIS RECEIPE NEEDS A PROPER README EXPLAINATION
run_fullscreen:
	$(MAKE) build
	sudo killall qemu-system-riscv64 virtiofsd || true
	sudo rm -f /tmp/vfsd.sock /tmp/vfsd.sock.pid /tmp/qemu_pty /tmp/qemu_stop
	
	sudo /usr/lib/qemu/virtiofsd --socket-path=/tmp/vfsd.sock --shared-dir=. --sandbox none &
	
	@# On modifie la boucle pour qu'elle sorte du bash proprement
	gnome-terminal --window-with-profile='XMRDEV' \
		--geometry +1920+0 \
		--full-screen \
		--zoom=0.7 \
		--hide-menubar \
		--title="RISC_V_CONSOLE" \
		-- bash -c "tty > /tmp/qemu_pty; while [ ! -f /tmp/qemu_stop ]; do sleep 1; done; exit" & \
	\
	echo "Waiting for PTY..."; \
	while [ ! -s /tmp/qemu_pty ]; do sleep 0.1; done; \
	PTY_VAL=$$(cat /tmp/qemu_pty); \
	echo "Using : $$PTY_VAL"; \
	sudo chmod 666 $$PTY_VAL; \
	\
	sudo $(QEMU_BIN) -M $(QEMU_MACHINE) \
		-cpu $(QEMU_CPU) \
		-m $(MEM) \
		-smp $(SMP) \
		-display none \
		-chardev serial,id=serial0,path=$$PTY_VAL \
		-serial chardev:serial0 \
		-kernel $(VMLINUX) \
		-initrd $(INITRD) \
		-bios default \
		-object memory-backend-memfd,id=mem,size=$(MEM),share=on \
		-numa node,memdev=mem \
		-chardev socket,id=char0,path=/tmp/vfsd.sock \
		-device vhost-user-fs-pci,queue-size=1024,chardev=char0,tag=projectshare,addr=03.0 \
		-append "console=ttyS0 root=/dev/ram0 rw vfio_iommu_type1.allow_unsafe_interrupts=1" \
		-device riscv-iommu-pci,addr=01.0 \
		-drive format=raw,file=$(NVME_DISK),if=none,id=nvm0 \
		-device nvme,drive=nvm0,serial=deadbeef,addr=02.0; 
	
	echo "QEMU closed, signaling terminal to exit..."; \
	touch /tmp/qemu_stop; \
	sleep 1; \
	rm -f /tmp/qemu_pty /tmp/qemu_stop

run_term:
	make build
	sudo killall qemu-system-riscv64 virtiofsd || true
	sudo rm -f /tmp/vfsd.sock /tmp/vfsd.sock.pid /tmp/qemu_pty

	sudo /usr/lib/qemu/virtiofsd --socket-path=/tmp/vfsd.sock --shared-dir=. --sandbox none &

	sudo $(QEMU_BIN) -M $(QEMU_MACHINE) \
		-cpu $(QEMU_CPU) \
		-m $(MEM) \
		-smp $(SMP) \
		-display none \
		-serial stdio \
		-kernel $(VMLINUX) \
		-initrd $(INITRD) \
		-bios default \
		-object memory-backend-memfd,id=mem,size=$(MEM),share=on \
		-numa node,memdev=mem \
		-chardev socket,id=char0,path=/tmp/vfsd.sock \
		-device vhost-user-fs-pci,queue-size=1024,chardev=char0,tag=projectshare,addr=03.0 \
		-append "console=ttyS0 root=/dev/ram0 rw vfio_iommu_type1.allow_unsafe_interrupts=1" \
		-device riscv-iommu-pci,addr=01.0 \
		-drive format=raw,file=$(NVME_DISK),if=none,id=nvm0 \
		-device nvme,drive=nvm0,serial=deadbeef,addr=02.0


clean:
	rm -rdf *.o test

kill:
	sudo killall qemu-system-riscv64 virtiofsd || true
	rm -f /tmp/qemu_pty