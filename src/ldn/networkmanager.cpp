#include "networkmanager.hpp"
#include <linux/if_arp.h>
#include <linux/if.h>
#include <fcntl.h>
#include <unistd.h>

namespace LDN {
    // NetworkManager implementation
    NetworkManager::NetworkManager() {
        sock = nl_socket_alloc();
        nl_connect(sock, NETLINK_GENERIC);
        nl80211_id = genl_ctrl_resolve(sock, "nl80211");
    }

    NetworkManager::~NetworkManager() {
        nl_close(sock);
        nl_socket_free(sock);
    }

    void NetworkManager::create_interface(const std::string& phy, const std::string& name, int iftype) {
        struct nl_msg* msg = nlmsg_alloc();
        genlmsg_put(msg, 0, 0, nl80211_id, 0, 0, NL80211_CMD_NEW_INTERFACE, 0);
        
        nla_put_u32(msg, NL80211_ATTR_IFTYPE, iftype);
        nla_put_string(msg, NL80211_ATTR_IFNAME, name.c_str());
        
        struct nl_cb* cb = nl_cb_alloc(NL_CB_DEFAULT);
        nl_send_auto_complete(sock, msg);
        nlmsg_free(msg);
        nl_cb_put(cb);
    }

    void NetworkManager::set_channel(int ifindex, uint8_t channel) {
        struct nl_msg* msg = nlmsg_alloc();
        genlmsg_put(msg, 0, 0, nl80211_id, 0, 0, NL80211_CMD_SET_CHANNEL, 0);
        
        nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex);
        nla_put_u32(msg, NL80211_ATTR_WIPHY_FREQ, Channels[channel]);
        
        struct nl_cb* cb = nl_cb_alloc(NL_CB_DEFAULT);
        nl_send_auto_complete(sock, msg);
        nlmsg_free(msg);
        nl_cb_put(cb);
    }

    // MonitorInterface implementation
    void MonitorInterface::set_channel(uint8_t channel) {
        if(channel != current_channel) {
            mgr.set_channel(ifindex, channel);
            current_channel = channel;
        }
    }

    // APInterface implementation
    APInterface::APInterface(NetworkManager& mgr, int ifindex, 
                            const std::string& name, const std::string& ssid, uint8_t channel)
        : NetworkInterface(mgr, ifindex, name), channel(channel) 
    {
        BeaconFrame beacon;
        beacon.source = address;
        beacon.beacon_interval = 100;
        beacon.capability_information = 0x511;
        beacon.elements[WLAN_EID_SSID] = ssid;
        beacon_head = beacon.encode();
    }

    void APInterface::start_ap(const std::vector<uint8_t>& key) {
        struct nl_msg* msg = nlmsg_alloc();
        genlmsg_put(msg, 0, 0, mgr.nl80211_id, 0, 0, NL80211_CMD_START_AP, 0);
        
        nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifindex);
        nla_put(msg, NL80211_ATTR_BEACON_HEAD, beacon_head.size(), beacon_head.data());
        nla_put_u32(msg, NL80211_ATTR_BEACON_INTERVAL, 100);
        nla_put_u32(msg, NL80211_ATTR_DTIM_PERIOD, 3);
        nla_put_u32(msg, NL80211_ATTR_HIDDEN_SSID, NL80211_HIDDEN_SSID_ZERO_CONTENTS);
        
        if(!key.empty()) {
            struct nlattr* key_attr = nla_nest_start(msg, NL80211_ATTR_KEY);
            nla_put_u8(msg, NL80211_KEY_IDX, 1);
            nla_put(msg, NL80211_KEY_DATA, key.size(), key.data());
            nla_put_u32(msg, NL80211_KEY_CIPHER, WLAN_CIPHER_SUITE_CCMP);
            nla_nest_end(msg, key_attr);
        }
        
        nl_send_auto_complete(mgr.sock, msg);
        nlmsg_free(msg);
    }
}