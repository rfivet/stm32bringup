# Makefile -- stm32bringup
# Copyright (c) 2020-2021 Renaud Fivet

### Build environment selection

ifeq (linux, $(findstring linux, $(MAKE_HOST)))
 INSTALLDIR = $(HOME)/Packages
#GCCDIR = $(INSTALLDIR)/gcc-arm-none-eabi-9-2019-q4-major
#GCCDIR = $(INSTALLDIR)/gcc-arm-none-eabi-9-2020-q2-update
 GCCDIR = $(INSTALLDIR)/gcc-arm-none-eabi-10-2020-q4-major
else
ifeq (cygwin, $(findstring cygwin, $(MAKE_HOST)))
 DRIVE = /cygdrive/d
else ifeq (msys, $(findstring msys, $(MAKE_HOST)))
 DRIVE = /d
else
 DRIVE = D:
endif
 INSTALLDIR = $(DRIVE)/Program Files (x86)
#GCCDIR = $(INSTALLDIR)/GNU Tools ARM Embedded/9 2019-q4-major
#GCCDIR = $(INSTALLDIR)/GNU Arm Embedded Toolchain/9 2020-q2-update
 GCCDIR = $(INSTALLDIR)/GNU Arm Embedded Toolchain/10 2020-q4-major
endif

export PATH := $(GCCDIR)/bin:$(PATH)

BINPFX  = @arm-none-eabi-
AR      = $(BINPFX)ar
CC      = $(BINPFX)gcc
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
#SRCS = startup.c clocks.c uptime.c
#SRCS = startup.txeie.c txeie.c uptime.c
#SRCS = startup.txeie.c gpioa.c dht11main.c dht11.c
#SRCS = startup.txeie.c gpioa.c ds18b20main.c ds18b20.c
#SRCS = startup.txeie.c adc.c adcmain.c
SRCS = startup.txeie.c adc.c adccalib.c ds18b20.c
OBJS = $(SRCS:.c=.o)
LIBOBJS = printf.o putchar.o puts.o memset.o
CPU = -mthumb -mcpu=cortex-m0
CFLAGS = $(CPU) -g -Wall -Wextra -Os
LD_SCRIPT = $(PROJECT).ld
LIBSTEM = stm32
LIBS = -l$(LIBSTEM)

### Build rules

.PHONY: clean all

all: $(PROJECT).hex $(PROJECT).bin

clean:
	@echo CLEAN
	@rm -f *.o *.elf *.map *.lst *.bin *.hex *.a

$(PROJECT).elf: $(OBJS) lib$(LIBSTEM).a
	@echo $@
	$(CC) $(CPU) -T$(LD_SCRIPT) -L. -Wl,-Map=$(PROJECT).map,-cref \
		-nostartfiles -o $@ $(OBJS) $(LIBS)
	$(SIZE) $@
	$(OBJDUMP) -hS $@ > $(PROJECT).lst

%.elf: %.o lib$(LIBSTEM).a
	@echo $@
	$(CC) $(CPU) -T$(LD_SCRIPT) -L. -Wl,-Map=$*.map,-cref -nostartfiles \
		-o $@ $< $(LIBS)
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
