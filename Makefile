# Makefile -- stm32bringup
# Copyright © 2020-2022 Renaud Fivet

### Build environment selection

ifeq (linux, $(findstring linux, $(MAKE_HOST)))
 INSTALLDIR = $(HOME)/Packages
#REVDIR = gcc-arm-none-eabi-9-2019-q4-major
#REVDIR = gcc-arm-none-eabi-9-2020-q2-update
#REVDIR = gcc-arm-none-eabi-10-2020-q4-major
#REVDIR = gcc-arm-none-eabi-10.3-2021.07
#REVDIR = gcc-arm-none-eabi-10.3-2021.10
 REVDIR = gcc-arm-11.2-2022.02-x86_64-arm-none-eabi
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
#REVDIR = GNU Arm Embedded Toolchain/10 2020-q4-major
#REVDIR = GNU Arm Embedded Toolchain/10 2021.07
#REVDIR = GNU Arm Embedded Toolchain/10 2021.10
 REVDIR = GNU Arm Embedded Toolchain/gcc-arm-11.2-2022.02-mingw-w64-i686-arm-none-eabi
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

### Memory Models
# By default we use the memory mapping from linker script

# In RAM Execution, load and start by USART bootloader
# Bootloader uses first 2K of RAM, execution from bootloader
#FLASHSTART = 0x20000800
#FLASHSIZE  = 2K
#RAMSTART   = 0x20000000
#RAMSIZE    = 2K

# In RAM Execution, load and start via SWD
# 4K RAM available, execution via SWD
#FLASHSTART = 0x20000000
#FLASHSIZE  = 3K
#RAMSTART   = 0x20000C00
#RAMSIZE    = 1K

# In Flash Execution
# if FLASHSTART is not at beginning of FLASH: execution via bootloader or SWD
#FLASHSTART = 0x08000000
#FLASHSIZE  = 16K
#RAMSTART   = 0x20000000
#RAMSIZE    = 4K

# ISR vector copied and mapped to RAM when FLASHSTART != 0x08000000
ifdef FLASHSTART
 ifneq ($(FLASHSTART),0x08000000)
  ifeq ($(FLASHSTART),0x20000000)
   # Map isr vector in RAM
   RAMISRV := 1
  else
   # Copy and map isr vector in RAM
   RAMISRV := 2
  endif
 endif
 BINLOC  = $(FLASHSTART)
else
 BINLOC  = 0x08000000
endif

# build options
CRC32SIGN := 1


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
#SRCS = startup.ram.c txeie.c uptime.1.c
 SRCS = startup.crc.c txeie.c uptime.1.c
LIBSRCS = printf.c putchar.c puts.c # memset.c memcpy.c
ALLSRCS = $(SRCS) $(LIBSRCS)

CPU = -mthumb -mcpu=cortex-m0 --specs=nano.specs
ifdef RAMISRV
 CDEFINES = -DRAMISRV=$(RAMISRV)
endif
ifdef CRC32SIGN
 CDEFINES += -DCRC32SIGN=$(CRC32SIGN)
endif
WARNINGS=-pedantic -Wall -Wextra -Wstrict-prototypes
CFLAGS = $(CPU) -g $(WARNINGS) -Os $(CDEFINES)

LD_SCRIPT = generic.ld
ifdef FLASHSTART
 LDOPTS  =--defsym FLASHSTART=$(FLASHSTART) --defsym FLASHSIZE=$(FLASHSIZE)
 LDOPTS +=--defsym RAMSTART=$(RAMSTART) --defsym RAMSIZE=$(RAMSIZE)
endif
LDOPTS +=-Map=$(subst .elf,.map,$@) -cref --print-memory-usage
comma :=,
space :=$() # one space before the comment
LDFLAGS =-Wl,$(subst $(space),$(comma),$(LDOPTS))

### Build rules

.PHONY: clean all version

all: $(PROJECT).$(BINLOC).bin $(PROJECT).hex

version:
	@echo make $(MAKE_VERSION) $(MAKE_HOST)
	@echo PATH="$(PATH)"
	$(CC) --version

clean:
	@echo CLEAN
	@rm -f *.dep *.o *.elf *.map *.lst *.bin *.hex *.a

$(PROJECT).elf: $(SRCS:.c=.o) libstm32.a
boot.elf: boot.o
ledon.elf: ledon.o
blink.elf: blink.o
ledtick.elf: ledtick.o
cstartup.elf: cstartup.o

%.elf:
	@echo $@
	$(CC) $(CPU) -T$(LD_SCRIPT) $(LDFLAGS) -nostartfiles -o $@ $+
	$(SIZE) $@
	$(OBJDUMP) -hS $@ > $(subst .elf,.lst,$@)

%.bin: %.elf
	@echo $@
	$(OBJCOPY) -O binary $< $@

%.$(BINLOC).bin: %.elf
	@echo $@
	$(OBJCOPY) -O binary $< $@
ifdef CRC32SIGN
	crc32/sign32 $@
	mv signed.bin $@

%.hex: %.$(BINLOC).bin
	@echo $@
	$(OBJCOPY) --change-address=$(BINLOC) -I binary -O ihex $< $@
endif

%.hex: %.elf
	@echo $@
	$(OBJCOPY) -O ihex $< $@

libstm32.a: $(LIBSRCS:.c=.o)
	$(AR) rc $@ $?

%.dep: %.c
	@echo $@
	$(CC) $(CDEFINES) -MM $< > $@

ifneq ($(MAKECMDGOALS),clean)
include $(ALLSRCS:.c=.dep)
endif
