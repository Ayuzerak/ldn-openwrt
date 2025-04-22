#pragma once
#include "networkmanager.hpp"
#include "frames.hpp"
#include "eventqueue.hpp"
#include <memory>

namespace LDN {
    class StationNetwork {
        NetworkManager& mgr;
        RouteManager& route;
        EventQueue& events;
        
        std::unique_ptr<NetworkInterface> interface;
        NetworkInfo network;
        uint8_t participant_id;
        std::vector<uint8_t> auth_key;
        
    public:
        StationNetwork(NetworkManager& mgr, RouteManager& route, EventQueue& eq);
        
        void connect(const ConnectParams& params);
        void disconnect();
        void process_events();
        
        const NetworkInfo& get_network() const { return network; }
    };

    struct ConnectParams {
        std::string phy;
        std::string ifname;
        NetworkInfo network;
        std::string password;
        std::string node_name;
        uint16_t app_version;
        bool enable_challenge = true;
        uint64_t device_id;
    };
}