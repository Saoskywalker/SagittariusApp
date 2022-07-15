########################################
# this file use to set target board
# because IC driver usually fixed, for APP plant easily, 
# design this file, and APP usually use ubuntu/linux by standard.
########################################

#
# Get platform information about ARCH and MACH from PLATFORM variable.
#
PLATFORM ?= arm32-f1c100s
ifeq ($(words $(subst -, , $(PLATFORM))), 2)
ARCH			:= $(word 1, $(subst -, , $(PLATFORM)))
MACH			:= mach_$(word 2, $(subst -, , $(PLATFORM)))
else
ARCH			?= x64
MACH			?= mingw
endif

#compile environment makefile
include arch/$(ARCH)/$(MACH)/mkenv.mk

#define
DEFINES	+= -DLCD_MODEL_4_3_IPS \

INCDIRS	+= \
	-Ilib/FATFS/src \
	-Ilib/FATFS/src/option \
	-Ilib/FATFS/exfuns \
	-Ilib/FLASH \
	-Iboard/lion/include\
	-Ilib/USB \

SRC_C += \
	lib/FATFS/src/diskio.c \
	lib/FATFS/src/ff.c \
	lib/FATFS/src/option/cc936.c \
	lib/FLASH/w25qxx.c \
	hal/dma_pool.c \
	lib/USB/usb_cdc.c \
	lib/USB/usb_desc.c \
	lib/USB/usb_dev.c \
	board/lion/system_board.c \
	board/lion/licence.c \
