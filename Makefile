srctree		:= $(CURDIR)

VPATH := arch:boards:build:crypto:disp:hid:io:motors:net:radio:rfid:sensors:tty

# define defaults that can be added to in submakefiles
INCLUDES := -I. -Iinclude -Iinclude/c++ -Ikernel
COMMON_FLAGS := -MD -ffunction-sections -Wall -Wuninitialized -Werror=implicit-function-declaration -Werror=strict-prototypes -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -fdata-sections -Os -Wl,--relax,--gc-sections
CFLAGS := 
CXXFLAGS := 
LDFLAGS := 
EXTRALIBS := 
BUILD_DIR := build
ktree := martink
#$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

ifdef BUILD
	include configs/$(BUILD).config
else
	include .config
endif

include Makefile.build 

# append flags defined in arch/
BUILD_DEFINE := $(subst -,_,$(BUILD))
COMMON_FLAGS += -DBUILD_$(subst -,_,$(BUILD)) -DBUILD=$(BUILD) $(CPU_FLAGS)

# add includes to the make
CFLAGS 		+= $(INCLUDES) $(COMMON_FLAGS) -std=gnu99 
CXXFLAGS 	+= -Ilib/stlport-avr $(INCLUDES) $(COMMON_FLAGS) -fpermissive  -std=c++11 
LDFLAGS 	:= $(COMMON_FLAGS) $(LDFLAGS)
OUTDIRS := build build/crypto/aes
APPNAME := libk-$(ARCH).a

# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)
	  
# Look for make include files relative to root of kernel src
MAKEFLAGS += --include-dir=$(srctree)

HOSTCC  	= gcc
HOSTCXX  	= g++
HOSTCFLAGS	:=
HOSTCXXFLAGS	:=

# We need some generic definitions
include $(srctree)/scripts/Kbuild.include

HOSTCFLAGS	+= $(call hostcc-option,-Wall -Wstrict-prototypes -O2 -fomit-frame-pointer,)
HOSTCXXFLAGS	+= -O2

# For maximum performance (+ possibly random breakage, uncomment
# the following)

MAKEFLAGS += -rR

export ktree srctree CONFIG_SHELL HOSTCC HOSTCFLAGS HOSTCXX HOSTCXXFLAGS 
export quiet Q KBUILD_VERBOSE

all: build; 

# Basic helpers built in scripts/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic

obj-y := $(patsubst %, $(BUILD_DIR)/%, $(obj-y))

# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;

# needed for menuconfig
%config: scripts_basic FORCE 	
	make $(build)=scripts/kconfig $@

app: build
	make -C $(APP) build

fixdep: 
	find build -type f -iname '*.d' -exec sh -c 'scripts/basic/fixdep "$${1%.*}.d" "$${1%.*}.o" "" > $${1%.*}.cmd' convert {} \;
	#echo "#include \"include/configs/$(BUILD).h\"">config.h
	
fixdirs: 
	mkdir -p build

saveconfig: 
ifdef BUILD
		cp include/autoconf.h include/configs/$(BUILD).h
		cp .config configs/$(BUILD).config
else
		echo "Please specify BUILD you want to save to!"
endif
	
build: fixdirs fixdep check $(obj-y) 
	rm -f $(APPNAME)
	ar rs $(APPNAME) $(obj-y)

$(BUILD_DIR)/%.o: %.cpp .config
	mkdir -p `dirname $@`
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c .config
	mkdir -p `dirname $@`
	$(CC) -c $(CFLAGS) $< -o $@

check: GCC-exists
	
GCC-exists: ; @which $(CC) > /dev/null

clean: 
	@find . \( -name '*.o' -o -name '*.ko' -o -name '*.cmd' \
		-o -name '*.d' -o -name '.*.tmp'  \) \
		-type f -print | xargs rm -f	
	@find . \( -name 'docproc' -o -name 'split-include' \
		  \
		-o -name 'qconf' -o -name 'gconf' -o -name 'kxgettext' \
		-o -name 'mconf' -o -name 'conf' -o -name 'lxdialog' \) \
		-type f -print | xargs rm -f	

-include $(obj-y:%.o=%.cmd)

PHONY += FORCE
FORCE:


# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable se we can use it in if_changed and friends.
.PHONY: $(PHONY) directories
