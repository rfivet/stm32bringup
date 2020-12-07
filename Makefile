# Makefile -- stm32bringup
# Copyright (c) 2020 Renaud Fivet

### Build environment selection

ifeq (linux, $(findstring linux, $(MAKE_HOST)))
#GCCDIR = ~/Packages/gcc-arm-none-eabi-9-2019-q4-major
#GCCDIR = ~/Packages/gcc-arm-none-eabi-9-2020-q2-update
 GCCDIR = $(HOME)/Packages/gcc-arm-none-eabi-9-2020-q2-update
else
#GCCDIR = "D:/Program Files (x86)/GNU Tools ARM Embedded/9 2019-q4-major"
 GCCDIR = "D:/Program Files (x86)/GNU Arm Embedded Toolchain/9 2020-q2-update"
endif

BINPFX  = @$(GCCDIR)/bin/arm-none-eabi-
AR      = $(BINPFX)ar
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
#SRCS = startup.c init.c success.c
#SRCS = startup.c board.c success.c
#SRCS = startup.c usart1tx.c hello.c
#SRCS = startup.c uplow.1.c uptime.1.c
#SRCS = startup.c uplow.2.c uptime.c printf.c putchar.c
#SRCS = startup.c uplow.2.c uptime.c
#SRCS = startup.c uplow.2.c hello.c
SRCS = startup.c clocks.c uptime.c
OBJS = $(SRCS:.c=.o)
LIBOBJS = printf.o putchar.o puts.o
CPU = -mthumb -mcpu=cortex-m0
CFLAGS = $(CPU) -g -Wall -Wextra -Os
LD_SCRIPT = $(PROJECT).ld
LIBDIR  = $(GCCDIR)/lib/gcc/arm-none-eabi/9.3.1/thumb/v6-m/nofp
LIB_PATHS = -L. -L$(LIBDIR)
LIBSTEM = stm32
LIBS = -l$(LIBSTEM) -lgcc

### Build rules

.PHONY: clean all

all: $(PROJECT).hex $(PROJECT).bin

clean:
	@echo CLEAN
	@rm -f *.o *.elf *.map *.lst *.bin *.hex *.a

$(PROJECT).elf: $(OBJS) lib$(LIBSTEM).a
	@echo $@
	$(LD) -T$(LD_SCRIPT) $(LIB_PATHS) -Map=$(PROJECT).map -cref -o $@ $(OBJS) $(LIBS)
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

lib$(LIBSTEM).a: $(LIBOBJS)
	$(AR) rc $@ $?
