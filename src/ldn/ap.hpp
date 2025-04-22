#pragma once
#include "networkmanager.hpp"
#include "eventqueue.hpp"

namespace LDN {
    class APNetwork {
        NetworkManager& mgr;
        RouteManager& route;
        EventQueue& events;
        std::unique_ptr<APInterface> interface;
        NetworkInfo network;
        
    public:
        APNetwork(NetworkManager& mgr, RouteManager& route, EventQueue& eq);
        
        void create(const CreateParams& params);
        void update_participants();
        void broadcast_advertisement();
        void process_events();
    };

    struct CreateParams {
        std::string phy;
        std::string ifname;
        uint64_t comm_id;
        uint16_t game_mode;
        uint8_t max_players;
        std::vector<uint8_t> app_data;
        std::string password;
    };
}