# targets are used for flashing boards and setting up board specific settings

# $(1): target path
# $(2): target name
define RegisterTarget
$(eval -include $(1)$(2)/Makefile.ritter)
$(eval $(call Target/defaults))
$(eval $(call Target/info))
$(foreach dep,$(DEPENDS),$(eval $(call PackageAddDepend,$(2),$(dep))))
$(1)$(2)/image: $$($(2)_PKG_DEPENDS) $$(APP)/install
	$(Q)echo "Creating image $(1)$(2)... $$($(2)_PKG_DEPENDS)"; 
	$(call Target/image,$(APP),$(notdir $(APP)))
$(1)$(2)/flash: $(1)$(2)/image 
	$(Q)echo "Flashing target $(1)$(2)"; 
	$(call Target/flash,$(APP),$(notdir $(APP)))
PHONY+=$(1)$(2)/flash
endef

