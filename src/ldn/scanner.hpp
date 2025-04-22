#pragma once
#include "networkmanager.hpp"
#include "frames.hpp"
#include <vector>

namespace LDN {
    class NetworkScanner {
        NetworkManager& mgr;
        std::vector<uint8_t> channels;
        float dwellTime;
        
    public:
        NetworkScanner(NetworkManager& mgr, 
                      const std::vector<uint8_t>& channels = {1, 6, 11},
                      float dwell = 0.110);
        
        std::vector<NetworkInfo> scan();
    };
}