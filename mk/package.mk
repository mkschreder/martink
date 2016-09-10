STAGING_DIR:=staging_dir

INSTALL_DIR:=mkdir -p
CP:=cp -Rp

CFLAGS+=-I$(STAGING_DIR)/usr/include/
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/

define BuildPackage 
$(eval $(1)_PKG_BUILD_DIR:=$(BUILD_DIR)/$(1)-$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_VERSION:=$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_URL:=$(PKG_SOURCE_URL))
$($(1)_PKG_BUILD_DIR): 
	mkdir -p $(dir $(dir $(1)_PKG_BUILD_DIR))
	git clone $($(1)_PKG_SOURCE_URL) $($(1)_PKG_BUILD_DIR)
	(cd $($(1)_PKG_BUILD_DIR); git checkout $($(1)_PKG_SOURCE_VERSION))
$($(1)_PKG_BUILD_DIR)/.configured:
	(cd $($(1)_PKG_BUILD_DIR); autoreconf -fi; ./configure --host=x86 --target=$(ARCH) CC=$(CC))
	touch $($(1)_PKG_BUILD_DIR)/.configured
package/$(1)/compile: $($(1)_PKG_BUILD_DIR) $($(1)_PKG_BUILD_DIR)/.configured
	make -C $($(1)_PKG_BUILD_DIR) CC=$(CC) CFLAGS="$(CFLAGS)"
package/$(1)/install: package/$(1)/compile
	$(call Package/install,$($(1)_PKG_BUILD_DIR),$(STAGING_DIR))
PHONY+=package/$(1)/compile
endef

define RegisterPackage 
$(1)$(2): 
	@echo "adding package $(1)$(2)"
PHONY+=$(1)$(2)
-include $(1)$(2)/Makefile.ritter
endef

$(foreach package,$(wildcard package/*),$(eval $(call RegisterPackage,$(dir $(package)),$(notdir $(basename $(package))))))
