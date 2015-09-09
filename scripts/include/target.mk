define add-target
TARGET_DIR:=$(1)
TARGET:=$$(BUILD_DIR)/image-$(subst /,-,$(1))
include $(1)/Makefile
$$(TARGET): $$(obj-y) 
	$$(Q)$$(CC) -o $$(TARGET) $$(LDFLAGS) $$(obj-y) 
$(1)/compile: $$(TARGET) ; 
.PHONY += $(1)/compile
endef
