ARCH = riscv
CROSS_COMPILE = riscv64-elf-

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -Wall -Wextra -O2 -fno-omit-frame-pointer -ggdb -gdwarf-2
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -fno-stack-protector

.PHONY: clean

test: test.o
	$(LD) $(LDFLAGS) -o $@ $<
%.dump: %
	$(OBJDUMP) -D $< > $@

clean:
	rm -f $(OBJS) *.dump *.d

-include *.d
