srctree		:= $(CURDIR)

Q:=@
ifneq ($(V),)
	Q:=
endif

include scripts/include/target.mk

VPATH := arch:boards:build:crypto:disp:hid:io:motors:net:radio:rfid:sensors:tty

# define defaults that can be added to in submakefiles
INCLUDES := -I. -Iinclude -Iinclude/c++ -Ikernel
COMMON_FLAGS := -MD -ffunction-sections -fdata-sections -Wall -Werror -Os
CFLAGS += -Wall -fPIC -Wno-format-y2k -W -Wstrict-prototypes -Wmissing-prototypes \
-Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings -Wswitch \
-Wshadow -Wcast-align -Wchar-subscripts -Winline \
-Wnested-externs -Wredundant-decls -Wmissing-field-initializers -Wextra \
-Wformat=2 -Wno-format-nonliteral -Wpointer-arith -Wno-missing-braces \
-Wno-unused-parameter
#-Wpedantic
CXXFLAGS += -Wall -Wno-format-y2k -W \
-Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings -Wswitch \
-Wcast-align -Wchar-subscripts -Wredundant-decls
LDFLAGS := -Wl,--relax,--gc-sections
EXTRALIBS := 

ifneq ($(BUILD),)
include configs/$(BUILD).config
else
include .config
endif

BUILD_DIR := build/$(BUILD)
CONFIG := configs/$(BUILD).config
CONFIG_H := include/configs/$(BUILD).h
ARCH := $(firstword $(subst -, ,$(BUILD)))
CPU = $(word 2,$(subst -, ,$(BUILD)))

ktree := martink
#$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

include Makefile.build 
ifdef BUILD
	include configs/$(BUILD).config
else
	include .config
endif

#$(eval $(call BuildDir,block))

# append flags defined in arch/
BUILD_DEFINE := $(subst -,_,$(BUILD))
COMMON_FLAGS += -I$(srctree) -I$(srctree)/include -DBUILD_$(BUILD_DEFINE) $(CPU_FLAGS) 

# add includes to the make
CFLAGS 		+= $(INCLUDES) $(COMMON_FLAGS) -std=gnu99 
CXXFLAGS 	+= -Ilib/stlport-avr $(INCLUDES) $(COMMON_FLAGS) -fpermissive  -std=c++11 
LDFLAGS 	:= $(COMMON_FLAGS) $(LDFLAGS)
OUTDIRS := build build/crypto/aes
#TARGET := kernel-$(BUILD)

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

all: config fixdirs fixdep check $(obj-y) default_target
	@echo $$'\e[32;40mLinking target $(TARGET)\e[m'
	
# Basic helpers built in scripts/
PHONY += scripts_basic defconfig
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic

obj-y := $(patsubst %, $(BUILD_DIR)/%, $(obj-y))

# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;

# needed for menuconfig
%config: scripts_basic FORCE 	
	$(Q)if [ ! -e .config ]; then cp $(CONFIG) .config; fi
	$(Q)make $(build)=scripts/kconfig $@
	$(Q)if [ $(SAVECONFIG) ]; then cp .config $(CONFIG); fi

config: 
	$(Q)if [ ! -f $(CONFIG) ]; then echo "Unknown config $(CONFIG)!"; exit 1; fi
	$(Q)if [ ! -e .config ]; then cp $(CONFIG) .config; fi
	@echo "toolchain: $(CC)"
	
fixdep: 
	$(Q)find build -type f -iname '*.d' -exec sh -c 'scripts/basic/fixdep "$${1%.*}.d" "$${1%.*}.o" "" > $${1%.*}.cmd' convert {} \;

#echo "#include \"include/configs/$(BUILD).h\"">config.h

fixdirs: 
	@mkdir -p build

saveconfig: 
ifdef BUILD
		cp include/autoconf.h include/configs/$(BUILD).h
		cp .config configs/$(BUILD).config
else
		echo "Please specify BUILD you want to save to!"
endif

default_target: 
	$(Q)rm -f $(TARGET)
	$(Q)$(CC) -o $(TARGET) $(LDFLAGS) $(obj-y) 
	

#$(patsubst %, $(BUILD_DIR)/%, $(obj-y))

buildall: 
	make -C . BUILD=arm-stm32f100mdvl
	make -C . BUILD=arm-stm32f103
	make -C . BUILD=avr-atmega328p
	make -C . BUILD=arm-stm32f100mdvl build-fst6-demo
	make -C . BUILD=arm-stm32f103 build-cc3d-demo
	make -C . BUILD=avr-atmega328p build-mwii-demo
	make -C . docs 
	
docs: 
	@cat README-intro.md $(docs-y) > README.md
	@pandoc -V geometry:margin=1in --toc README.md -o Reference-Manual.pdf
	
$(BUILD_DIR)/%.o: %.cpp .config 
	$(Q)mkdir -p `dirname $@`
	$(Q)$(CXX) -c $(CXXFLAGS) $< -o $@

#splint -unrecogcomments -predboolint -exportlocal -noeffect -fcnuse +matchanyintegral -boolops +boolint -D__GNUC__ -DBUILD_arm_stm32f103 -DSTM32F10X_MD -Iarch/arm/stm32/CMSIS -I./ -I./include $<
#scan-build -enable-checker alpha.core.BoolAssignment -enable-checker alpha.core.CastToStruct -enable-checker alpha.core.IdenticalExpr -enable-checker alpha.core.PointerArithm -enable-checker alpha.core.PointerSub -enable-checker alpha.core.SizeofPtr -enable-checker alpha.core.TestAfterDivZero -enable-checker alpha.security.ArrayBoundV2 -enable-checker alpha.security.ReturnPtrRange -enable-checker security.FloatLoopCounter -enable-checker security.insecureAPI.strcpy --use-cc=$(CC) $(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c .config 
	@echo $$'\e[33;40m$(subst $(BUILD_DIR)/,,$@)\e[m'
	$(Q)mkdir -p `dirname $@`
	$(Q)$(CC) -c $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/%.o: %.S .config 
	$(Q)mkdir -p `dirname $@`
	$(Q)$(AS) -c  $< -o $@

check: GCC-exists
	
GCC-exists: ; @which $(CC) > /dev/null

clean: 
	rm -rf build
	@find . \( -name '*.o' -o -name '*.ko' -o -name '*.cmd' \
		-o -name '*.d' -o -name '.*.tmp'  \) \
		-type f -print | xargs rm -f	
	@find . \( -name 'docproc' -o -name 'split-include' \
		  \
		-o -name 'qconf' -o -name 'gconf' -o -name 'kxgettext' \
		-o -name 'mconf' -o -name 'conf' -o -name 'lxdialog' \) \
		-type f -print | xargs rm -f	

-include $(obj-y:%.o=$(BUILD_DIR)/%.d)

PHONY += FORCE
FORCE:

install: 
	mkdir -p $(DESTDIR)/usr/lib/
	cp -Rp $(TARGET) $(DESTDIR)/usr/lib/

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable se we can use it in if_changed and friends.
.PHONY: $(PHONY) directories

$(eval $(call add-target,target/linux))
