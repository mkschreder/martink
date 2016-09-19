# $(1): package name 

define PackageSetup 
endef

# 1: package directory path
# 2: folder name
define RegisterPackage 
$(eval PKG_DIR:=$(1)$(2))
$(eval MAKE_TARGET=all)
$(eval -include $(1)$(2)/Makefile.ritter)
endef

define PackageAddDepend
$(eval $(1)_PKG_DEPENDS+=$(2)/install)
endef

# $(1): package name
define BuildPackage 

define Package/install
endef
define Package/config
endef
define Package/info
endef
define Package/Build
endef

DEPENDS:=
$(eval $(1)_PKG_DIR:=$$(PKG_DIR))
$(eval $(1)_PKG_BUILD_DIR=$$(BUILD_DIR)/$(1)-$$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_VERSION=$(PKG_SOURCE_VERSION))
$(eval $(1)_PKG_SOURCE_URL=$(PKG_SOURCE_URL))
$(eval $(1)_MAKE_TARGET=$$(MAKE_TARGET))
$(eval $(call Package/info))
$(foreach dep,$(DEPENDS),$(eval $(call PackageAddDepend,$(1),$(dep))))
$$($(1)_PKG_BUILD_DIR):
	$(Q)mkdir -p $(dir $$($(1)_PKG_BUILD_DIR))
	$(Q)echo "Checking out source code from $$($(1)_PKG_SOURCE_URL) to '$$($(1)_PKG_BUILD_DIR)'"
	$(Q)[ -e $$($(1)_PKG_BUILD_DIR) ] || git clone $$($(1)_PKG_SOURCE_URL) $$($(1)_PKG_BUILD_DIR)
	(cd $$($(1)_PKG_BUILD_DIR); git checkout $$($(1)_PKG_SOURCE_VERSION))
tmp/KConfig-$(1): 
	@mkdir -p tmp/
	if [ -f $(1)/KConfig ]; then cp $(1)/KConfig tmp/KConfig-$(1); fi
$$($(1)_PKG_BUILD_DIR)/Makefile:
	(cd $$($(1)_PKG_BUILD_DIR); [ -e Makefile.am ] || autoreconf -fi && ./configure --host=$$(ARCH) --build=x86 CFLAGS="--specs=nosys.specs") 
package/$(1)/prereq: 
	@if [ "$$(BUILD_DIR)" = "" ]; then echo "Packages can not be built individually! Must be built as part of a firmware!"; exit 1; fi
	@if [ "$$(ARCH)" = "" ]; then echo "Firmware ARCH not set. This has to be set in firmware makefile!"; exit 1; fi
package/$(1)/compile: package/$(1)/prereq $$($(1)_PKG_DEPENDS) $$($(1)_PKG_BUILD_DIR) $$($(1)_PKG_BUILD_DIR)/Makefile tmp/KConfig-$(1)
	$(call Package/Build)
	$(Q)echo "CC:=$$(CC) AS=$$(AS)"
	$(Q)echo "CFLAGS=$$(CFLAGS)"
	$(Q)echo "LDFLAGS=$$(LDFLAGS)"
	$(Q)echo "Package dependencies $$($(1)_PKG_DEPENDS)"
	$(Q)echo "building package $(1)"
	cd $$($(1)_PKG_BUILD_DIR); \
		$(call package/$(1)/build)
	$(MAKE) -C "$$($(1)_PKG_BUILD_DIR)" $$($(1)_MAKE_TARGET) 
	#cd $$($(1)_PKG_BUILD_DIR) && $(MAKE) $$($(1)_MAKE_TARGET) CC=$$(CC) CCAS=$$(CC) CCASFLAGS="$$(CPU_CFLAGS)" CFLAGS="$$(CFLAGS)" CXXFLAGS:="$$(CXXFLAGS)" LDFLAGS="$$(LDFLAGS)" V1=
package/$(1)/install: package/$(1)/compile
	$(Q)echo "Installing package $(1)"
	$(call Package/install,$$($(1)_PKG_BUILD_DIR),$$($(1)_PKG_STAGING_DIR))
PHONY+=$$($(1)_PKG_BUILD_DIR)
PHONY+=package/$(1)/compile
PHONY+=package/$(1)/install
endef

