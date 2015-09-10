CURDIR:=./

define BuildDir
CURDIR:=$(CURDIR)/$(1)

define MODULE_DIR_$(1)_open
menu "$(1)"
endef
define MODULE_DIR_$(1)_close
endmenu
endef

export MODULE_DIR_$(1)_open
export MODULE_DIR_$(1)_close

MODULE_CONFIGS+=MODULE_DIR_$(1)_open
include $(CURDIR)/$(1)/Makefile
MODULE_CONFIGS+=MODULE_DIR_$(1)_close

CURDIR:=$(shell basename $(CURDIR))
endef

define BuildModule
obj-$(CONFIG_$(1)) += $(patsubst %.c,$(CURDIR)/%.o,$(2)); 

define MODULE_CONFIG_$(1)
config $(1)
	default n
	tristate $(1) 
endef

export MODULE_CONFIG_$(1)
MODULE_CONFIGS+=MODULE_CONFIG_$(1)

MAINTAINER:=
DESCRIPTION:=
endef

define BuildArch
$(eval $(call BuildDir,$(1)))
define ARCH_CONFIG_$(1)
config $(1)
	default n
	bool $(1) 
endef

export ARCH_CONFIG_$(1)
ARCH_CONFIGS+=ARCH_CONFIG_$(1)
endef
