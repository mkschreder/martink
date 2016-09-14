# $(1): package name 

define PackageSetup 
$(eval $(1)_PKG_BUILD_DIR:=$(BUILD_DIR)/$(1)-$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_VERSION:=$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_URL:=$(PKG_SOURCE_URL))
endef

define BuildPackage 
$(eval $(call PackageSetup,$(1)))
endef

define PackageAddDepend
$(eval $(1)_PKG_DEPENDS+=$(2)/install)
endef

# $(1): package path
# $(2): package name
define RegisterPackage 

define Package/install
endef
define Package/config
endef
define Package/info
endef
DEPENDS:=

$(eval -include $(1)$(2)/Makefile.ritter)
$(eval $(call PackageSetup,$(2)))
$(eval $(call Package/info))
$(foreach dep,$(DEPENDS),$(eval $(call PackageAddDepend,$(2),$(dep))))
$$($(2)_PKG_BUILD_DIR):
	$(Q)mkdir -p $(dir $($(2)_PKG_BUILD_DIR))
	$(Q)echo "Checking out source code from $$($(2)_PKG_SOURCE_DIR), $$($(2)_PKG_SOURCE_URL)"
	$(Q)git clone $$($(2)_PKG_SOURCE_URL) $$($(2)_PKG_BUILD_DIR)
	(cd $$($(2)_PKG_BUILD_DIR); git checkout $$($(2)_PKG_SOURCE_VERSION))
tmp/KConfig-$(2): 
	@mkdir -p tmp/
	if [ -f $(1)$(2)/KConfig ]; then cp $(1)$(2)/KConfig tmp/KConfig-$(2); fi
$$($(2)_PKG_BUILD_DIR)/Makefile:
	(cd $$($(2)_PKG_BUILD_DIR); autoreconf -fi; ./configure --host=x86 --build=x86 --target=$(ARCH))
$(1)$(2)/compile: $$($(2)_PKG_DEPENDS) $$($(2)_PKG_BUILD_DIR) $$($(2)_PKG_BUILD_DIR)/Makefile tmp/KConfig-$(2)
	$(Q)echo "CC=$$(CC) AS=$$(AS)"
	$(Q)echo "CFLAGS=$$(CFLAGS)"
	$(Q)echo "LDFLAGS=$$(LDFLAGS)"
	$(Q)echo "Package dependencies $$($(2)_PKG_DEPENDS)"
	$(Q)echo "building package $(1)$(2)"
	$(Q)make -C $$($(2)_PKG_BUILD_DIR) CC=$$(TARGET_CC) CCAS=$$(TARGET_CC) CCASFLAGS="$$(CPU_CFLAGS)" CFLAGS="$$(CFLAGS)" LDFLAGS="$$(LDFLAGS) $$(CPU_LDFLAGS)"
$(1)$(2)/install: $(1)$(2)/compile
	$(Q)echo "Installing package $(2)"
	$(call Package/install,$$($(2)_PKG_BUILD_DIR),$(STAGING_DIR))
PHONY+=$(1)$(2)/compile
PHONY+=$(1)$(2)/install
endef

