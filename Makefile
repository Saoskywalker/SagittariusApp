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

#extren library makefile
include lib/FreeRTOS-Kernel/FreeRTOS.mk

#define
DEFINES	+= #-D_USER_DEBUG 

#include path
INCDIRS	+= \
	-Iarch/$(ARCH)/$(MACH)/port/include \

#library path
LIBDIRS	+=

#library
LIBS += 

#c source path
SRCDIRS_C += 
SRC_C += $(wildcard $(foreach n, $(SRCDIRS_C), $(n)/*.c))

#c source files
# port src
SRC_C += \
	arch/$(ARCH)/$(MACH)/port/MTF_io.c \
	arch/$(ARCH)/$(MACH)/port/file_operate_hal.c \
	arch/$(ARCH)/$(MACH)/port/delay.c \
	arch/$(ARCH)/$(MACH)/port/framebuffer_port.c \
	arch/$(ARCH)/$(MACH)/port/audio_port.c \
	arch/$(ARCH)/$(MACH)/port/touch_port.c \
	arch/$(ARCH)/$(MACH)/port/PWM_port.c \
	arch/$(ARCH)/$(MACH)/port/GPIO_port.c \
	arch/$(ARCH)/$(MACH)/port/ROM_port.c \
	arch/$(ARCH)/$(MACH)/port/uart_port.c \
	arch/$(ARCH)/$(MACH)/port/timer_port.c \
	arch/$(ARCH)/$(MACH)/port/system_port.c \

#hal src
SRC_C += \
	hal/beep.c \

#usr src
SRC_C += \
	# demo_mingw.c \

#include path
INCDIRS	+= \
	-ISagittariusUI/include \
	-Ihal/include \
	-Ilib \
	-Ilib/UI \
	-Ilib/PICTURE \
	-Ilib/TOUCH \
	-Ilib/TEXT \
	-Ilib/crypto \
	-Ilib/FreeModbus_Slave_Master_v16/port \
	-Ilib/FreeModbus_Slave_Master_v16/modbus/rtu \
	-Ilib/ComPort \
	-Ilib/music_library \
	-Ilib/debug_library \

# usr library src
SRC_C += \
	lib/PICTURE/bmp.c \
	lib/PICTURE/gif.c \
	lib/PICTURE/lodepng.c \
	lib/PICTURE/piclib.c \
	lib/TOUCH/touch.c \
	lib/TOUCH/ts_calibrate_common.c \
	lib/TEXT/text.c \
	lib/TEXT/text_rect.c \
	lib/TEXT/font.c \
	lib/UI/UI_engine.c \
	lib/UI/lcd.c \
	lib/crypto/crc32.c \
	lib/crypto/sha256.c \
	lib/ComPort/dgus2Com.c \
	lib/ComPort/ComPort.c \
	lib/ComPort/MTF_ComProtocol.c \
	lib/ComPort/T5UIC2_agreement.c \
	lib/FreeModbus_Slave_Master_v16/modbus/rtu/mbcrc.c \
	lib/music_library/music_play.c \
	lib/debug_library/my_assert.c \

#hal src
SRC_C += \
	# hal/beep.c \
	# hal/dma_pool.c \

#usr src
SRC_C += \
	SagittariusUI/main.c \
	SagittariusUI/Sagittarius_global.c \
	SagittariusUI/Sagittarius_timer.c \
	SagittariusUI/cJSON.c \
	SagittariusUI/cJSON_extend.c \
	SagittariusUI/licence.c \
	SagittariusUI/TestBoard.c \
	SagittariusUI/MTF_HMI.c \
	SagittariusUI/file_type.c \

#c++ source path
SRCDIRS_CXX += 
SRC_CXX += $(wildcard $(foreach n, $(SRCDIRS_CXX), $(n)/*.cpp))

#c++ source files
SRC_CXX += 

#asm source files
SRC_ASM += 

OBJ = $(addprefix $(BUILD)/, $(SRC_C:.c=.o)) $(addprefix $(BUILD)/, $(SRC_ASM:.S=.o)) $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))

-include $(addprefix $(BUILD)/, $(SRC_C:.c=.d)) $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.d))

define compile_c
$(ECHO) "CC $<"
$(Q)$(CC) $(INCDIRS) $(MCFLAGS) $(CFLAGS) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@# Regex adjusted from the above to play better with Windows paths, etc.
@$(CP) $(@:.o=.d) $(@:.o=.P); \
  $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P);
endef

# define compile_c
# $(ECHO) "CC $<"
# $(CC) $(INCDIRS) $(MCFLAGS) $(CFLAGS) -c -MD -o $@ $<
# @# The following fixes the dependency file.
# @# See http://make.paulandlesley.org/autodep.html for details.
# @# Regex adjusted from the above to play better with Windows paths, etc.
# @$(CP) $(@:.o=.d) $(@:.o=.P); \
#   $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
#       -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
#   $(RM) -f $(@:.o=.d)
# endef

define compile_cxx
$(ECHO) "CXX $<"
$(Q)$(CXX) $(INCDIRS) $(MCFLAGS) $(CXXFLAGS) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@# Regex adjusted from the above to play better with Windows paths, etc.
@$(CP) $(@:.o=.d) $(@:.o=.P); \
  $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P);
endef

$(BUILD)/%.o: %.c
	$(call compile_c)

$(BUILD)/%.o: %.cpp
	$(call compile_cxx)

$(BUILD)/%.o: %.S
	$(ECHO) "AS $<"
	$(Q)$(AS) $(MCFLAGS) $(ASFLAGS) -c -o $@ $<
	
$(BUILD)/%.o: %.s
	$(ECHO) "AS $<"
	$(Q)$(AS) $(MCFLAGS) $(CFLAGS) -o $@ $<

OBJ_DIRS = $(sort $(dir $(OBJ)))
$(OBJ): | $(OBJ_DIRS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

$(BUILD)/$(addprefix $(PROG), $(EXTENSION)): $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) -Wl,--cref,-Map=$@.map -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/$(addprefix $(PROG), .bin): $(BUILD)/$(addprefix $(PROG), $(EXTENSION))
	$(Q)$(OBJCOPY) -v -O binary $^ $@
	$(ECHO) Make header information for brom booting
	@$(PACK_TOOL) $@
