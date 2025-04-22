#include "ap.hpp"
#include "crypto.hpp"

namespace LDN {
    APNetwork::APNetwork(NetworkManager& mgr, 
                        RouteManager& route, 
                        EventQueue& eq)
        : mgr(mgr), route(route), events(eq) {}

    void APNetwork::create(const CreateParams& params) {
        // Generate network parameters
        network.session.localCommunicationId = params.comm_id;
        network.session.gameMode = params.game_mode;
        network.session.ssid = GenerateRandomBytes(16);
        network.key = GenerateDataKey(params.password);
        network.channel = SelectOptimalChannel();
        network.nonce = GenerateRandomBytes(4);
        
        // Create AP interface
        interface = std::make_unique<APInterface>(mgr, params.phy, params.ifname,
                                                 network.session.ssid, 
                                                 network.channel);
        interface->start_ap(network.key);
        
        // Configure host participant
        ParticipantInfo host;
        host.ipAddress = "169.254.1.1";
        host.macAddress = interface->get_address();
        host.name = "Host";
        host.connected = true;
        network.participants[0] = host;
        
        // Set up routing
        route.add_address(interface->get_index(), host.ipAddress, 24);
        route.add_neighbor(interface->get_index(), host.ipAddress, host.macAddress);
        
        // Start advertisement thread
        std::thread([this]{
            while(running) {
                broadcast_advertisement();
                std::this_thread::sleep_for(100ms);
            }
        }).detach();
    }

    void APNetwork::broadcast_advertisement() {
        AdvertisementFrame frame;
        frame.header = network.session;
        frame.version = 3;
        frame.encryption = network.securityLevel == 3 ? 1 : 2;
        frame.nonce = network.nonce;
        frame.info = network.toAdvertisementInfo();
        
        mgr.send_frame(interface->get_index(), frame.encode());
    }
}