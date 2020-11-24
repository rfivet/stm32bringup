# Makefile -- stm32bringup
# Copyright (c) 2020 Renaud Fivet

### Build environment selection

ifeq (linux, $(findstring linux, $(MAKE_HOST)))
#GCCDIR = ~/Packages/gcc-arm-none-eabi-9-2019-q4-major
 GCCDIR = ~/Packages/gcc-arm-none-eabi-9-2020-q2-update
else
#GCCDIR = "D:/Program Files (x86)/GNU Tools ARM Embedded/9 2019-q4-major"
 GCCDIR = "D:/Program Files (x86)/GNU Arm Embedded Toolchain/9 2020-q2-update"
endif

BINPFX  = @$(GCCDIR)/bin/arm-none-eabi-
CC      = $(BINPFX)gcc
LD      = $(BINPFX)ld
OBJCOPY = $(BINPFX)objcopy
OBJDUMP = $(BINPFX)objdump
SIZE    = $(BINPFX)size

### STM32F030F4P6 based board

PROJECT = f030f4
#SRCS = boot.c
#SRCS = ledon.c
#SRCS = blink.c
#SRCS = ledtick.c
#SRCS = cstartup.c
SRCS = startup.c init.c success.c
OBJS = $(SRCS:.c=.o)
CPU = -mthumb -mcpu=cortex-m0
CFLAGS = $(CPU) -g -Wall -Wextra -Os
LD_SCRIPT = $(PROJECT).ld

### Build rules

.PHONY: clean all

all: $(PROJECT).hex $(PROJECT).bin

clean:
	@echo CLEAN
	@rm -f *.o *.elf *.map *.lst *.bin *.hex

$(PROJECT).elf: $(OBJS)
	@echo $@
	$(LD) -T$(LD_SCRIPT) -Map=$(PROJECT).map -cref -o $@ $(OBJS)
	$(SIZE) $@
	$(OBJDUMP) -hS $@ > $(PROJECT).lst

%.elf: %.o
	@echo $@
	$(LD) -T$(LD_SCRIPT) -Map=$*.map -cref -o $@ $<
	$(SIZE) $@
	$(OBJDUMP) -hS $@ > $*.lst

%.bin: %.elf
	@echo $@
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	@echo $@
	$(OBJCOPY) -O ihex $< $@
