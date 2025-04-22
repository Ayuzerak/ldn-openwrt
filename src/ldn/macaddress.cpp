#include "macaddress.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <random>

namespace LDN {
    MACAddress::MACAddress() : bytes{0} {}
    
    MACAddress::MACAddress(const std::array<uint8_t, 6>& data) : bytes(data) {}
    
    MACAddress::MACAddress(const std::string& str) {
        unsigned int values[6];
        if (sscanf(str.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
                   &values[0], &values[1], &values[2],
                   &values[3], &values[4], &values[5]) != 6) {
            throw std::invalid_argument("Invalid MAC address format");
        }
        for (int i = 0; i < 6; i++) bytes[i] = static_cast<uint8_t>(values[i]);
    }
    
    bool MACAddress::operator==(const MACAddress& other) const {
        return bytes == other.bytes;
    }
    
    std::string MACAddress::toString() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (size_t i = 0; i < bytes.size(); ++i) {
            if (i > 0) oss << ":";
            oss << std::setw(2) << static_cast<int>(bytes[i]);
        }
        return oss.str();
    }
    
    std::array<uint8_t, 6> MACAddress::encode() const { return bytes; }
    
    void MACAddress::decode(const uint8_t* data) {
        std::copy(data, data + 6, bytes.begin());
    }
    
    MACAddress MACAddress::random() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::array<uint8_t, 6> addr;
        for (auto& b : addr) b = static_cast<uint8_t>(dis(gen));
        addr[0] &= 0xFE; // Unicast
        addr[0] |= 0x02; // Locally administered
        return MACAddress(addr);
    }
}