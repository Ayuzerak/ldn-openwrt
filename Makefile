include $(TOPDIR)/rules.mk

PKG_NAME:=ldn
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Your Name <you@example.com>
PKG_LICENSE:=MIT

include $(INCLUDE_DIR)/package.mk

define Package/ldn
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Nintendo LDN Protocol Implementation
  DEPENDS:=+libopenssl +libnl-core +libnl-genl +libubus +libubox
endef

define Package/ldn/description
  Implementation of the Nintendo LDN local wireless protocol for OpenWrt
endef

define Build/Prepare
    mkdir -p $(PKG_BUILD_DIR)
    $(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
    $(MAKE) -C $(PKG_BUILD_DIR) \
        $(TARGET_CONFIGURE_OPTS) \
        CFLAGS="$(TARGET_CFLAGS)" \
        LDFLAGS="$(TARGET_LDFLAGS)" \
        all
endef

define Package/ldn/install
    $(INSTALL_DIR) $(1)/usr/bin
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/examples/host $(1)/usr/bin/ldn-host
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/examples/join $(1)/usr/bin/ldn-join
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/examples/scan $(1)/usr/bin/ldn-scan
    
    $(INSTALL_DIR) $(1)/usr/libexec/ldn
    $(INSTALL_BIN) $(PKG_BUILD_DIR)/ldn-daemon $(1)/usr/libexec/ldn/
    
    $(INSTALL_DIR) $(1)/etc/init.d
    $(INSTALL_BIN) ./src/openwrt/ldn.init $(1)/etc/init.d/ldn
    
    $(INSTALL_DIR) $(1)/etc/config
    $(INSTALL_CONF) ./src/openwrt/ldn.config $(1)/etc/config/ldn
    
    $(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
    $(INSTALL_DATA) ./luci-app-ldn/root/usr/lib/lua/luci/controller/ldn.lua $(1)/usr/lib/lua/luci/controller/
    
    $(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
    $(INSTALL_DATA) ./luci-app-ldn/root/usr/lib/lua/luci/model/cbi/ldn.lua $(1)/usr/lib/lua/luci/model/cbi/
endef

$(eval $(call BuildPackage,ldn))