include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ldn
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_MAINTAINER:=Your Name <you@example.com>
PKG_LICENSE:=MIT

LUCI_TITLE:=Nintendo LDN Protocol Web Interface
LUCI_DEPENDS:=+ldn +luci-lib-json
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

define Package/luci-app-ldn/conffiles
/etc/config/ldn
endef

define Package/luci-app-ldn/install
    $(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
    $(INSTALL_DATA) ./files/root/usr/lib/lua/luci/controller/ldn.lua $(1)/usr/lib/lua/luci/controller/
    
    $(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
    $(INSTALL_DATA) ./files/root/usr/lib/lua/luci/model/cbi/ldn.lua $(1)/usr/lib/lua/luci/model/cbi/
    
    $(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
    $(INSTALL_DATA) ./po/en/ldn.po $(1)/usr/lib/lua/luci/i18n/
    
    $(INSTALL_DIR) $(1)/www/luci-static/resources/ldn
    $(INSTALL_DATA) ./files/htdocs/* $(1)/www/luci-static/resources/ldn/
endef

$(eval $(call BuildPackage,luci-app-ldn))