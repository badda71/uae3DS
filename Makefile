ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

DFLAGS = -DUSE_SDL -DGCCCONSTFUNC="__attribute__((const))" -DUSE_UNDERSCORE -fno-exceptions -DUNALIGNED_PROFITABLE -DREGPARAM="__attribute__((regparm(3)))" -DOPTIMIZED_FLAGS -D__inline__=__inline__ -DSHM_SUPPORT_LINKS=0 -DOS_WITHOUT_MEMORY_MANAGEMENT -DVKBD_ALWAYS 

DFLAGS+= -DVERSION3DS=\"0.1\"
DFLAGS+= -DEMULATED_JOYSTICK
#DFLAGS+= -DMENU_MUSIC
#DFLAGS+= -DNO_SOUND

DFLAGS+= -DDEBUG_UAE4ALL -DUAE_CONSOLE -DDOUBLEBUFFER

#DFLAGS+= -DUSE_AUTOCONFIG
DFLAGS+= -DUAE_CONSOLE
DFLAGS+= -DGP2X
DFLAGS+= -DUSE_ZFILE
#DFLAGS+= -DUAE4ALL_NO_USE_RESTRICT
DFLAGS+= -DNO_THREADS
#DFLAGS+= -DDEBUG_TIMESLICE
DFLAGS+= -DFAME_INTERRUPTS_PATCH
#DFLAGS+= -DFAME_INTERRUPTS_SECURE_PATCH
#DFLAGS+= -DFAME_GLOBAL_CONTEXT

#DFLAGS+= -DUAE_MEMORY_ACCESS
#DFLAGS+= -DSAFE_MEMORY_ACCESS
#DFLAGS+= -DERROR_WHEN_MEMORY_OVERRUN

#DFLAGS+= -DDEBUG_FILE=\"stdout.txt\"
##DFLAGS+= -DDEBUG_UAE4ALL_FFLUSH
#DFLAGS+= -DDEBUG_M68K
#DFLAGS+= -DDEBUG_INTERRUPTS
#DFLAGS+= -DDEBUG_CIA
###DFLAGS+= -DDEBUG_SOUND
#DFLAGS+= -DDEBUG_MEMORY
###DFLAGS+= -DDEBUG_MAPPINGS
#DFLAGS+= -DDEBUG_DISK
#DFLAGS+= -DDEBUG_CUSTOM
###DFLAGS+= -DDEBUG_EVENTS
###DFLAGS+= -DDEBUG_GFX -DDEBUG_BLITTER
###DFLAGS+= -DDEBUG_FRAMERATE
###DFLAGS+= -DAUTO_FRAMERATE=1400
###DFLAGS+= -DMAX_AUTO_FRAMERATE=4400
###DFLAGS+= -DAUTO_FRAMERATE_SOUND
#DFLAGS+= -DSTART_DEBUG=999999
#DFLAGS+= -DMAX_AUTOEVENTS=999990
#DFLAGS+= -DAUTO_RUN

#DFLAGS+= -DPROFILER_UAE4ALL
DFLAGS+=-DUSE_FAME_CORE -DUSE_FAME_CORE_C -DFAME_INLINE_LOOP -DFAME_IRQ_CLOCKING -DFAME_CHECK_BRANCHES -DFAME_EMULATE_TRACE -DFAME_DIRECT_MAPPING -DFAME_BYPASS_TAS_WRITEBACK -DFAME_ACCURATE_TIMING -DFAME_GLOBAL_CONTEXT -DFAME_FETCHBITS=8 -DFAME_DATABITS=8 -DFAME_GOTOS -DFAME_EXTRA_INLINE=__inline__ -DFAME_NO_RESTORE_PC_MASKED_BITS

SRCS := $(shell find src -name \*.cpp -o -name \*.c)
INCLUDES = $(addprefix -I,$(shell find -L src -type d 2> /dev/null))
INCLUDES += -I/opt/devkitpro/portlibs/3ds/include \
	-I/opt/devkitpro/libctru/include
GCCFLAGS = -Wno-switch -fpermissive -fomit-frame-pointer -Wno-unused -Wno-format -g -Wall -O2 -Wshadow -fdata-sections -ffunction-sections -march=armv6k -mfloat-abi=hard -mtp=soft -mtune=mpcore -mword-relocations -DARM11 -D_3DS

CFLAGS = $(GCCFLAGS) $(DFLAGS) $(INCLUDES)

LDFLAGS = -specs=3dsx.specs -g -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

LIBS = -L/opt/devkitpro/portlibs/3ds/lib\
	-L/opt/devkitpro/portlibs/3ds\
	-L/opt/devkitpro/libctru/lib\
	-lmikmod\
	-lSDL\
	-lSDL_mixer\
	-lz\
	-logg\
	-lmad\
	-lvorbisidec\
	-lctru\
	-lcitro3d\
	-lm

CXX = arm-none-eabi-g++
ODIR = obj
RDIR = resources
NAME = uae3DS
VERSION = 0.1

OBJ = $(addsuffix .o,$(addprefix $(ODIR)/,$(subst /,.,$(basename $(SRCS)))))
$(shell mkdir -p $(ODIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(ODIR)/$*.Td
POSTCOMPILE = @mv -f $(ODIR)/$*.Td $(ODIR)/$*.d && touch $@

all: $(NAME).3dsx

$(NAME).3dsx: $(NAME).elf $(RDIR)/icon.png
	smdhtool --create "uae3DS" "Amiga 500 emulator" "badda71" $(RDIR)/icon.png $(ODIR)/$(NAME).smdh
	3dsxtool $(NAME).elf $(NAME).3dsx --romfs=romfs --smdh=$(ODIR)/$(NAME).smdh

$(NAME).elf: $(OBJ)
	$(CXX) $(LDFLAGS) -Wl,--start-group $(OBJ) $(LIBS) -Wl,--end-group -o $@

$(ODIR)/%.d: ;
.PRECIOUS: $(ODIR)/%.d

.SECONDEXPANSION:
$(ODIR)/%.o: $$(subst .,/,$$*).cpp $(ODIR)/%.d
	$(CXX) $(CFLAGS) $(DEPFLAGS) -g -c -o $@ $<
	$(POSTCOMPILE)

include $(wildcard $(patsubst %,%.d,$(basename $(OBJ))))

.PHONY: clean
clean:
	rm -rf $(ODIR) $(NAME).3dsx $(NAME).elf
