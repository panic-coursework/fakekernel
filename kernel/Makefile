ARCH = riscv
CROSS_COMPILE = riscv64-elf-

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -O2
CFLAGS += -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany -ffreestanding -fno-common -nostdlib -mno-relax -fno-stack-protector

SRCS = $(shell find . -name '*.c' -or -name '*.s' -or -name '*.S')
OBJS_C = $(SRCS:.c=.o)
OBJS_S = $(OBJS_C:.s=.o)
OBJS = $(OBJS_S:.S=.o)

kernel: $(OBJS) kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS) 

CPUS = 1
QEMU = qemu-system-riscv64
QEMUOPTS = -machine virt -bios none -kernel kernel -m 128M -smp $(CPUS) -nographic
QEMUOPTS += -global virtio-mmio.force-legacy=false
QEMUOPTS += -device loader,file=../user/test,force-raw=on,addr=0x800f0000
QEMUOPTS_RECORD = -icount shift=auto,rr=record,rrfile=replay.bin,rrsnapshot=init -net none -drive file=empty.qcow2,if=none,id=rr
QEMUOPTS_REPLAY = -icount shift=auto,rr=replay,rrfile=replay.bin,rrsnapshot=init -net none -drive file=empty.qcow2,if=none,id=rr

qemu: kernel.dump
	$(QEMU) $(QEMUOPTS)
qemugdb: kernel.dump
	$(QEMU) $(QEMUOPTS) -s -S
qemurr : kernel.dump
	$(QEMU) $(QEMUOPTS) $(QEMUOPTS_RECORD)
qemureplay : kernel.dump replay.bin
	$(QEMU) $(QEMUOPTS) $(QEMUOPTS_REPLAY) -s -S

kernel.dump: kernel
	$(OBJDUMP) -S -D $< > $@
%.dump: %.o
	$(OBJDUMP) -D $< > $@

.PHONY: all clean

all: kernel.dump
clean:
	rm -f kernel kernel.dump $(OBJS) *.dump *.d

-include *.d
