srctree		:= $(CURDIR)

Q:=@
ifneq ($(V),)
	Q:=
endif

all: config fixdirs check default_target
	@echo "\033[32;40m [LD] $(ARCH)-$(CPU)-$(BOARD)\033[m"
	
VPATH := arch:boards:build:crypto:disp:hid:io:motors:net:radio:rfid:sensors:tty

-include .config

INSTALL_DIR:=mkdir -p
CP:=cp -Rp

include mk/package.mk
include mk/firmware.mk

#BUILD_DIR := build_dir/target-$(ARCH)-$(CPU)-$(BOARD)
#STAGING_DIR := $(CURDIR)/staging_dir/target-$(ARCH)-$(CPU)-$(BOARD)
#CFLAGS+=-I$(STAGING_DIR)/usr/include/
#LDFLAGS+=-L$(STAGING_DIR)/usr/lib/

# define defaults that can be added to in submakefiles
INCLUDES += -I. -Iinclude -Iinclude/c++ -Ikernel
COMMON_CFLAGS += -ffunction-sections -fdata-sections -Os
TARGET_CFLAGS += -Wall -Wno-format-y2k -W -Wno-strict-prototypes -Wmissing-prototypes \
-Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings -Wswitch \
-Wshadow -Wcast-align -Wchar-subscripts -Winline \
-Wnested-externs -Wredundant-decls -Wmissing-field-initializers -Wextra \
-Wformat=2 -Wno-format-nonliteral -Wpointer-arith -Wno-missing-braces \
-Wno-unused-parameter -Wno-unused-variable -Wno-inline
TARGET_CXXFLAGS += -Wall -Wno-format-y2k -W \
-Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings -Wswitch \
-Wcast-align -Wchar-subscripts -Wredundant-decls


$(foreach fw,$(wildcard firmware/*),$(eval $(call RegisterFirmware,$(dir $(fw)),$(notdir $(basename $(fw))))))
$(foreach package,$(wildcard apps/*),$(eval $(call RegisterPackage,$(dir $(package)),$(notdir $(basename $(package))))))
$(foreach package,$(wildcard package/*),$(eval $(call RegisterPackage,$(dir $(package)),$(notdir $(basename $(package))))))


define check-set 
$(if $(value $1),,$(warning $1 is not set correctly!))
endef 

CONFIG := .config
CONFIG_H := include/configs/$(ARCH).h

obj-y := $(patsubst %, $(BUILD_DIR)/%, $(obj-y))
TARGET:=$(ARCH)-$(CPU)-$(BOARD)

ktree := martink
#$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

#include $(obj-y:.o=.d)

#$(eval $(call BuildDir,block))

# append flags defined in arch/
COMMON_CFLAGS += -I$(srctree) -I$(srctree)/include $(CPU_CFLAGS) 

# add includes to the make
CFLAGS 		+= $(CFLAGS-y) $(INCLUDES) $(COMMON_CFLAGS) -std=gnu99  
CXXFLAGS 	+= -Ilib/stlport-avr $(INCLUDES) $(COMMON_CFLAGS) -fpermissive  -std=c++11 
LDFLAGS 	+= -Wl,--relax,--gc-sections $(LDFLAGS-y) $(CPU_LDFLAGS) 
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

# Basic helpers built in scripts/
PHONY += scripts_basic defconfig
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic


# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;

# needed for menuconfig
%config: scripts_basic   	
	$(Q)make $(build)=scripts/kconfig $@
	$(Q)if [ $(SAVECONFIG) ]; then cp .config $(CONFIG); fi

config: 
	$(Q)if [ ! -f $(CONFIG) ]; then echo "Unknown config $(CONFIG)!"; exit 1; fi
	$(Q)if [ ! -e .config ]; then cp $(CONFIG) .config; fi
	@echo "toolchain: $(CC)"
	
#fixdep: 
#	$(Q)find build -type f -iname '*.d' -exec sh -c 'scripts/basic/fixdep "$${1%.*}.d" "$${1%.*}.o" "" > $${1%.*}.cmd' convert {} \;

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

#$(TARGET): $(obj-y)
#	$(Q)$(CC) -o $@.elf  $(obj-y) -Wl,-Map,$@.map $(LDFLAGS)
#	@echo "Finalizing image.."
#	$(call target/image/finalize,$(TARGET).elf,$(TARGET))

flash: 
	$(call target/image/flash,$(TARGET))

default_target: $(TARGET)
	@echo ""
		
#$(patsubst %, $(BUILD_DIR)/%, $(obj-y))

docs: 
	@cat README-intro.md $(docs-y) > README.md
	@pandoc -V geometry:margin=1in --toc README.md -o Reference-Manual.pdf
	
$(BUILD_DIR)/%.o: %.cpp .config 
	$(Q)mkdir -p `dirname $@`
	$(Q)$(CXX) -c $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c .config 
	@echo "\033[33;40m [CC] $(subst $(BUILD_DIR)/,,$@)\033[m"
	$(Q)mkdir -p `dirname $@`
	$(Q)$(CC) -c $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/%.o: %.S .config 
	$(Q)mkdir -p `dirname $@`
	$(Q)$(AS) -c  $< -o $@

$(BUILD_DIR)/%.d: %.c
	@mkdir -p `dirname $@`
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

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

-include $(obj-y:%.o=%.d)

install: 
	mkdir -p $(DESTDIR)/usr/lib/
	cp -Rp $(TARGET) $(DESTDIR)/usr/lib/

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable se we can use it in if_changed and friends.
.PHONY: $(PHONY) directories

