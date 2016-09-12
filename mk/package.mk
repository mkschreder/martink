
INSTALL_DIR:=mkdir -p
CP:=cp -Rp

CFLAGS+=-I$(STAGING_DIR)/usr/include/
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/

# $(1): package name 

define PackageSetup 
$(eval $(1)_PKG_BUILD_DIR:=$(BUILD_DIR)/$(1)-$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_VERSION:=$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_URL:=$(PKG_SOURCE_URL))
endef

define BuildPackage 
$(eval $(call PackageSetup,$(1)))
endef

# $(1): package path
# $(2): package name
define RegisterPackage 
$(eval -include $(1)$(2)/Makefile.ritter)
$(eval $(call PackageSetup,$(2)))
$$($(2)_PKG_BUILD_DIR):
	mkdir -p $(dir $($(2)_PKG_BUILD_DIR))
	@echo "Checking out source code from $$($(2)_PKG_SOURCE_DIR), $$($(2)_PKG_SOURCE_URL)"
	git clone $$($(2)_PKG_SOURCE_URL) $$($(2)_PKG_BUILD_DIR)
	(cd $$($(2)_PKG_BUILD_DIR); git checkout $$($(2)_PKG_SOURCE_VERSION))
$$($(2)_PKG_BUILD_DIR)/.configured:
	(cd $$($(2)_PKG_BUILD_DIR); autoreconf -fi; ./configure --host=x86 --target=$(ARCH) TARGET_CC=$(TARGET_CC))
	touch $$($(2)_PKG_BUILD_DIR)/.configured
$(1)$(2)/compile: $$($(2)_PKG_BUILD_DIR) $$($(2)_PKG_BUILD_DIR)/.configured
	echo "building package $(1)$(2)"
	make -C $$($(2)_PKG_BUILD_DIR) CC=$(CC) CFLAGS="$(CFLAGS)"
$(1)$(2)/install: $(1)$(2)/compile
	$(call Package/install,$$($(2)_PKG_BUILD_DIR),$(STAGING_DIR))
PHONY+=$(1)$(2)/compile
PHONY+=$(1)$(2)/install
endef

$(foreach package,$(wildcard apps/*),$(eval $(call RegisterPackage,$(dir $(package)),$(notdir $(basename $(package))))))
$(foreach package,$(wildcard package/*),$(eval $(call RegisterPackage,$(dir $(package)),$(notdir $(basename $(package))))))
