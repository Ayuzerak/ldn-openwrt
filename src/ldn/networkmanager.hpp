#pragma once
#include "frames.hpp"
#include "macaddress.hpp"
#include <linux/nl80211.h>
#include <netlink/socket.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <functional>
#include <memory>

namespace LDN {
    class NetworkManager {
        struct nl_sock* sock;
        int nl80211_id;
        
    public:
        NetworkManager();
        ~NetworkManager();
        
        // Interface control
        void create_interface(const std::string& phy, const std::string& name, int iftype);
        void delete_interface(int ifindex);
        void set_channel(int ifindex, uint8_t channel);
        
        // Frame operations
        void register_frame(int ifindex, uint16_t frame_type, const std::vector<uint8_t>& match);
        void send_frame(int ifindex, const std::vector<uint8_t>& frame);
        
        // Event handling
        void handle_events(const std::function<void(nl_msg*)>& callback);
    };

    class NetworkInterface {
    protected:
        NetworkManager& mgr;
        int ifindex;
        MACAddress address;
        std::string name;
        
    public:
        NetworkInterface(NetworkManager& mgr, int ifindex, const std::string& name);
        virtual ~NetworkInterface() = default;
        
        virtual void up();
        void set_mac(const MACAddress& addr);
        void disable_ipv6();
        
        int get_index() const { return ifindex; }
        MACAddress get_address() const { return address; }
    };

    class MonitorInterface : public NetworkInterface {
        uint8_t current_channel;
        
    public:
        MonitorInterface(NetworkManager& mgr, int ifindex, const std::string& name);
        
        void set_channel(uint8_t channel);
        void receive(std::function<void(const std::vector<uint8_t>&, uint8_t)> callback);
    };

    class APInterface : public NetworkInterface {
        uint8_t channel;
        std::vector<uint8_t> beacon_head;
        
    public:
        APInterface(NetworkManager& mgr, int ifindex, const std::string& name, 
                    const std::string& ssid, uint8_t channel);
        
        void start_ap(const std::vector<uint8_t>& key);
        void add_station(const MACAddress& addr, const std::vector<uint8_t>& key);
        void remove_station(const MACAddress& addr);
    };
}