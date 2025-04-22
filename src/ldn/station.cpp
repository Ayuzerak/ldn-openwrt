#include "station.hpp"
#include "crypto.hpp"

namespace LDN {
    StationNetwork::StationNetwork(NetworkManager& mgr, 
                                 RouteManager& route, 
                                 EventQueue& eq)
        : mgr(mgr), route(route), events(eq) {}
    
    void StationNetwork::connect(const ConnectParams& params) {
        // Create station interface
        interface = std::make_unique<NetworkInterface>(mgr, params.phy, params.ifname);
        interface->up();
        
        // Set WLAN channel
        mgr.set_channel(interface->get_index(), params.network.channel);
        
        // Perform authentication
        AuthenticationFrame auth_frame;
        auth_frame.version = params.network.version;
        auth_frame.header = params.network.session;
        auth_frame.networkKey = params.network.key;
        auth_frame.authKey = GenerateRandomBytes(16);
        
        AuthenticationRequest auth_req;
        auth_req.username = params.node_name;
        auth_req.appVersion = params.app_version;
        
        if(params.enable_challenge) {
            ChallengeRequest challenge;
            challenge.token = params.network.challenge;
            challenge.nonce = GenerateRandomU64();
            challenge.deviceId = params.device_id;
            auth_req.challenge = challenge.encode();
        }
        
        // Send authentication frame
        mgr.send_frame(interface->get_index(), auth_frame.encode());
        
        // Process authentication response
        while(true) {
            auto event = events.pop();
            if(auto frame = std::get_if<DataFrameEvent>(&event)) {
                AuthenticationFrame response;
                response.decode(frame->data);
                
                if(response.statusCode != AUTH_SUCCESS) {
                    throw AuthError(response.statusCode);
                }
                
                network = params.network;
                participant_id = response.participant_id;
                break;
            }
        }
        
        // Configure network
        route.add_address(interface->get_index(), 
                         network.participants[participant_id].ipAddress,
                         24);
        
        for(const auto& participant : network.participants) {
            if(participant.connected) {
                route.add_neighbor(interface->get_index(),
                                 participant.ipAddress,
                                 participant.macAddress);
            }
        }
    }
}