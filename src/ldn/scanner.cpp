#include "scanner.hpp"
#include <chrono>
#include <thread>

namespace LDN {
    NetworkScanner::NetworkScanner(NetworkManager& mgr,
                                 const std::vector<uint8_t>& channels,
                                 float dwell)
        : mgr(mgr), channels(channels), dwellTime(dwell) {}

    std::vector<NetworkInfo> NetworkScanner::scan() {
        std::vector<NetworkInfo> networks;
        auto monitor = mgr.create_monitor("phy0", "ldn-scan");
        
        for(auto channel : channels) {
            monitor.set_channel(channel);
            auto start = std::chrono::steady_clock::now();
            
            while((std::chrono::steady_clock::now() - start) < 
                  std::chrono::milliseconds(static_cast<int>(dwellTime * 1000))) {
                try {
                    auto frame = monitor.receive();
                    AdvertisementFrame adv;
                    adv.decode(frame.data);
                    
                    NetworkInfo info;
                    info.parse(adv);
                    info.channel = channel;
                    info.address = frame.source;
                    
                    if(std::find_if(networks.begin(), networks.end(),
                        [&](const auto& n){ return n.address == info.address; }) == networks.end()) {
                        networks.push_back(info);
                    }
                } catch(const std::exception&) {}
            }
        }
        return networks;
    }
}