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
# include lib/FreeRTOS-Kernel/FreeRTOS.mk

#define
DEFINES	+= -D_USER_DEBUG 

#include path
INCDIRS	+= \
	-Iarch/$(ARCH)/$(MACH)/port/include \
	-Ilib/music_library \
	-Ilib/debug_library \

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
	arch/$(ARCH)/$(MACH)/port/audio_port.c \
	arch/$(ARCH)/$(MACH)/port/MTF_io.c \

# usr library src
SRC_C += \
	lib/music_library/music_play.c \
	lib/debug_library/my_assert.c \

#hal src
SRC_C += \

#usr src
SRC_C += \
	demo_mingw.c \

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
