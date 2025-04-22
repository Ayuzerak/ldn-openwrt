#pragma once
#include "frames.hpp"

namespace LDN {
    struct NetworkInfo {
        MACAddress address;
        uint8_t channel;
        SessionInfo session;
        uint8_t version;
        std::array<uint8_t, 16> key;
        uint16_t securityLevel;
        uint8_t acceptPolicy;
        uint8_t maxParticipants;
        uint8_t numParticipants;
        std::array<ParticipantInfo, 8> participants;
        std::vector<uint8_t> applicationData;
        uint64_t challenge;
        std::array<uint8_t, 4> nonce;

        bool check(const NetworkInfo& other) const;
        void parse(const AdvertisementFrame& frame);
        AdvertisementFrame build() const;
    };
}