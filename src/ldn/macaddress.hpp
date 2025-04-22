#pragma once
#include <array>
#include <string>
#include <cstdint>

namespace LDN {
    class MACAddress {
        std::array<uint8_t, 6> bytes;
        
    public:
        MACAddress();
        explicit MACAddress(const std::array<uint8_t, 6>& data);
        explicit MACAddress(const std::string& str);
        
        bool operator==(const MACAddress& other) const;
        std::string toString() const;
        std::array<uint8_t, 6> encode() const;
        void decode(const uint8_t* data);
        
        static MACAddress random();
    };
}