#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# ROMFS is the directory containing data to be added to RomFS, relative to the Makefile (Optional)
#
# NO_ICON: if set to anything, do not use icon.
# NO_NACP: if set to anything, no .nacp file is generated.
# APP_TITLE is the name of the app stored in the .nacp file (Optional)
# APP_AUTHOR is the author of the app stored in the .nacp file (Optional)
# APP_VERSION is the version of the app stored in the .nacp file (Optional)
# APP_TITLEID is the titleID of the app stored in the .nacp file (Optional)
# ICON is the filename of the icon (.jpg), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.jpg
#     - icon.jpg
#     - <libnx folder>/default_icon.jpg
#
# CONFIG_JSON is the filename of the NPDM config file (.json), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.json
#     - config.json
#   If a JSON file is provided or autodetected, an ExeFS PFS0 (.nsp) is built instead
#   of a homebrew executable (.nro). This is intended to be used for sysmodules.
#   NACP building is skipped as well.
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	NXKit
DATA		:=	data
INCLUDES	:=	NXKit
ROMFS		:=	resource

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
INCLUDES := $(INCLUDES) \
			NXKit/TestControllers/ViewController \
			NXKit/TestControllers/ListViewController \
			NXKit/TestControllers/TableViewTestController \
			NXKit/TestControllers/TableView2TestController \
			NXKit/TestControllers/AnimationTestViewController \
			NXKit/TestControllers/ControlTestViewController \
			NXKit/TestControllers/TouchTestViewController

SOURCES  := $(SOURCES) \
			NXKit/TestControllers/ViewController \
			NXKit/TestControllers/ListViewController \
			NXKit/TestControllers/TableViewTestController \
			NXKit/TestControllers/TableView2TestController \
			NXKit/TestControllers/AnimationTestViewController \
			NXKit/TestControllers/ControlTestViewController \
			NXKit/TestControllers/TouchTestViewController


INCLUDES := $(INCLUDES) \
			NXKit/lib \
			NXKit/lib/extern/include \
			NXKit/lib/extern/include/lunasvg \
			NXKit/lib/extern/lib/lunasvg/3rdparty/plutovg \
			NXKit/lib/extern/lib/lunasvg/3rdparty/software \
			NXKit/lib/extern/include/nanovg-gl/ \
			NXKit/lib/extern/lib/yoga/src

SOURCES  := $(SOURCES) \
			NXKit/lib/Platforms/Switch \
			NXKit/lib/Platforms/Switch/Input \
			NXKit/lib/Platforms/Universal \
			NXKit/lib/Core/Driver/Video \
			NXKit/lib/Core/Application \
			NXKit/lib/Core/Geometry \
			NXKit/lib/Core/NXAffineTransform \
			NXKit/lib/Core/NXTransform3D \
			NXKit/lib/Core/UIActionsView \
			NXKit/lib/Core/UIAlertController \
			NXKit/lib/Core/UIAppDelegate \
			NXKit/lib/Core/UIBlurView \
			NXKit/lib/Core/UIBottomSheetController \
			NXKit/lib/Core/UIButton \
			NXKit/lib/Core/UIColor \
			NXKit/lib/Core/UIControl \
			NXKit/lib/Core/UIEdgeInsets \
			NXKit/lib/Core/UIEvent \
			NXKit/lib/Core/UIGestureRecognizer \
			NXKit/lib/Core/UIImage \
			NXKit/lib/Core/UIImageView \
			NXKit/lib/Core/UILabel \
			NXKit/lib/Core/UINavigationController \
			NXKit/lib/Core/UIPanGestureRecognizer \
			NXKit/lib/Core/UIResponder \
			NXKit/lib/Core/UIScrollView \
			NXKit/lib/Core/UISelectorViewController \
			NXKit/lib/Core/UIStackView \
			NXKit/lib/Core/UITabBarController \
			NXKit/lib/Core/UITableView \
			NXKit/lib/Core/UITableView2 \
			NXKit/lib/Core/UITableViewCell \
			NXKit/lib/Core/UITableViewDefaultCell \
			NXKit/lib/Core/UITableViewRadioCell \
			NXKit/lib/Core/UITableViewSwitchCell \
			NXKit/lib/Core/UITapGestureRecognizer \
			NXKit/lib/Core/UITouch \
			NXKit/lib/Core/UITraitCollection \
			NXKit/lib/Core/UIView \
			NXKit/lib/Core/UIViewController \
			NXKit/lib/Core/UIWindow \
			NXKit/lib/Core/Utils/Animation/AnimationContext \
			NXKit/lib/Core/Utils/Animation/Core \
			NXKit/lib/Core/Utils/FontManager \
			NXKit/lib/Core/Utils/GaussianBlur \
			NXKit/lib/Core/Utils/GroupTask \
			NXKit/lib/Core/Utils/Literals \
			NXKit/lib/Core/Utils/Tools \
			NXKit/lib/extern/lib/nanovg-gl/ \
			NXKit/lib/extern/lib/glad \
			NXKit/lib/extern/lib/lunasvg \
			NXKit/lib/extern/lib/lunasvg/3rdparty/plutovg \
			NXKit/lib/extern/lib/lunasvg/3rdparty/software \
			NXKit/lib/extern/lib/libretro-common/compat \
			NXKit/lib/extern/lib/libretro-common/encodings \
			NXKit/lib/extern/lib/libretro-common/features \
			NXKit/lib/extern/lib/yoga/src/yoga \
			NXKit/lib/extern/lib/yoga/src/yoga/event \
			NXKit/lib/extern/lib/yoga/src/yoga/internal

ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

DEFINES	:=	-DYG_ENABLE_EVENTS

CFLAGS	:=	-g -Wall -O2 -ffunction-sections \
			$(ARCH) $(DEFINES)

CFLAGS	+=	$(INCLUDE) -D__SWITCH__

CXXFLAGS	:= $(CFLAGS) -std=c++1z -O2

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:=  -lglfw3 -lEGL -lglapi -ldrm_nouveau -lnx

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(LIBNX)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

ifeq ($(strip $(CONFIG_JSON)),)
	jsons := $(wildcard *.json)
	ifneq (,$(findstring $(TARGET).json,$(jsons)))
		export APP_JSON := $(TOPDIR)/$(TARGET).json
	else
		ifneq (,$(findstring config.json,$(jsons)))
			export APP_JSON := $(TOPDIR)/config.json
		endif
	endif
else
	export APP_JSON := $(TOPDIR)/$(CONFIG_JSON)
endif

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

ifneq ($(APP_TITLEID),)
	export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
	export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
ifeq ($(strip $(APP_JSON)),)
	@rm -fr $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf
else
	@rm -fr $(BUILD) $(TARGET).nsp $(TARGET).nso $(TARGET).npdm $(TARGET).elf
endif


#---------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
ifeq ($(strip $(APP_JSON)),)

all	:	$(OUTPUT).nro

ifeq ($(strip $(NO_NACP)),)
$(OUTPUT).nro	:	$(OUTPUT).elf $(OUTPUT).nacp
else
$(OUTPUT).nro	:	$(OUTPUT).elf
endif

else

all	:	$(OUTPUT).nsp

$(OUTPUT).nsp	:	$(OUTPUT).nso $(OUTPUT).npdm

$(OUTPUT).nso	:	$(OUTPUT).elf

endif

$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
