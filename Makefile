# Makefile -- stm32bringup
# Copyright (c) 2020-2021 Renaud Fivet

### Build environment selection

ifeq (linux, $(findstring linux, $(MAKE_HOST)))
 INSTALLDIR = $(HOME)/Packages
#REVDIR = gcc-arm-none-eabi-9-2019-q4-major
#REVDIR = gcc-arm-none-eabi-9-2020-q2-update
 REVDIR = gcc-arm-none-eabi-10-2020-q4-major
else
 DRIVE = d
ifeq (cygwin, $(findstring cygwin, $(MAKE_HOST)))
 OSDRIVE = /cygdrive/$(DRIVE)
else ifeq (msys, $(findstring msys, $(MAKE_HOST)))
 OSDRIVE = /$(DRIVE)
else
 OSDRIVE = $(DRIVE):
endif
 INSTALLDIR = $(OSDRIVE)/Program Files (x86)
#REVDIR = GNU Tools ARM Embedded/5.4 2016q2
#REVDIR = GNU Tools ARM Embedded/6 2017-q2-update
#REVDIR = GNU Tools ARM Embedded/7 2017-q4-major
#REVDIR = GNU Tools ARM Embedded/7 2018-q2-update
#REVDIR = GNU Tools ARM Embedded/9 2019-q4-major
#REVDIR = GNU Arm Embedded Toolchain/9 2020-q2-update
 REVDIR = GNU Arm Embedded Toolchain/10 2020-q4-major
endif

GCCDIR = $(INSTALLDIR)/$(REVDIR)
export PATH := $(GCCDIR)/bin:$(PATH)

BINPFX  = @arm-none-eabi-
AR      = $(BINPFX)ar
CC      = $(BINPFX)gcc
OBJCOPY = $(BINPFX)objcopy
OBJDUMP = $(BINPFX)objdump
SIZE    = $(BINPFX)size

### STM32F030F4P6 based board

PROJECT = f030f4

# In RAM Execution
# Bootloader uses first 2K of RAM, execution from bootloader
#FLASHSTART = 0x20000800
#FLASHSIZE  = 2K
#RAMSTART   = 0x20000000
#RAMSIZE    = 2K

# In Flash Execution
# if FLASHSTART is not at beginning of FLASH: execution from bootloader
FLASHSTART = 0x08000000
FLASHSIZE  = 16K
RAMSTART   = 0x20000000
RAMSIZE    = 4K

# ISR vector copied and mapped to RAM if FLASHSTART != 0x08000000
ifneq ($(FLASHSTART),0x08000000)
 RAMISRV := 1
endif

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
#SRCS = startup.txeie.c adc.c adccalib.c ds18b20.c
 SRCS = startup.ram.c txeie.c uptime.1.c
OBJS = $(SRCS:.c=.o)
LIBOBJS = printf.o putchar.o puts.o memset.o memcpy.o

CPU = -mthumb -mcpu=cortex-m0
ifdef RAMISRV
 CDEFINES = -DRAMISRV=$(RAMISRV)
endif
WARNINGS=-pedantic -Wall -Wextra -Wstrict-prototypes -Wno-unused-parameter
CFLAGS = $(CPU) -g $(WARNINGS) -Os $(CDEFINES)

LD_SCRIPT = generic.ld
LDOPTS  =--defsym FLASHSTART=$(FLASHSTART) --defsym FLASHSIZE=$(FLASHSIZE)
LDOPTS +=--defsym RAMSTART=$(RAMSTART) --defsym RAMSIZE=$(RAMSIZE)
LDOPTS +=-Map=$(subst .elf,.map,$@) -cref --print-memory-usage
comma :=,
space :=$() # one space before the comment
LDFLAGS =-Wl,$(subst $(space),$(comma),$(LDOPTS))

### Build rules

.PHONY: clean all version

all: $(PROJECT).hex $(PROJECT).$(FLASHSTART).bin

version:
	@echo make $(MAKE_VERSION) $(MAKE_HOST)
	@echo PATH="$(PATH)"
	$(CC) --version

clean:
	@echo CLEAN
	@rm -f *.o *.elf *.map *.lst *.bin *.hex *.a

$(PROJECT).elf: $(OBJS) libstm32.a
boot.elf: boot.o
ledon.elf: ledon.o
blink.elf: blink.o
ledtick.elf: ledtick.o
cstartup.elf: cstartup.o

%.elf:
	@echo $@
	$(CC) $(CPU) -T$(LD_SCRIPT) $(LDFLAGS) -nostartfiles -o $@ $+
	$(SIZE) -G $@
	$(OBJDUMP) -hS $@ > $(subst .elf,.lst,$@)

%.bin: %.elf
	@echo $@
	$(OBJCOPY) -O binary $< $@

%.$(FLASHSTART).bin: %.elf
	@echo $@
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	@echo $@
	$(OBJCOPY) -O ihex $< $@

libstm32.a: $(LIBOBJS)
	$(AR) rc $@ $?
