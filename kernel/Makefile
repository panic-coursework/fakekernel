ARCH = riscv
CROSS_COMPILE = riscv64-elf-

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -Wall -O2 -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -fno-stack-protector

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

qemu: kernel.dump
	$(QEMU) $(QEMUOPTS)
qemugdb: kernel.dump
	$(QEMU) $(QEMUOPTS) -s -S

kernel.dump: kernel
	$(OBJDUMP) -D $< > $@
%.dump: %.o
	$(OBJDUMP) -d $< > $@

.PHONY: all clean

all: kernel.dump
clean:
	rm -f kernel kernel.dump $(OBJS) *.dump *.d

-include *.d
