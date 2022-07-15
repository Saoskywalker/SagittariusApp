########################################
# this file use to set target board
# because IC driver usually fixed, for APP plant easily, 
# design this file, and APP usually use ubuntu/linux by standard.
########################################

#
# Get platform information about ARCH and MACH from PLATFORM variable.
#
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
DEFINES	+= -DLCD_MODEL_TM080SDH01 \

INCDIRS	+= \
	-Iboard/pc/include\

SRC_C += \
	board/pc/system_board.c \
	board/pc/licence.c \
