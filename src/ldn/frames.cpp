#include "frames.hpp"
#include "crypto.hpp"
#include <arpa/inet.h>

namespace LDN {
    // SessionInfo implementation
    void SessionInfo::encode(Buffer& buf, bool bigEndian) {
        if(bigEndian) {
            buf.writeBE(localCommunicationId);
            buf.writeBE<uint16_t>(0); // Padding
            buf.writeBE(gameMode);
            buf.writeBE<uint32_t>(0); // Padding
        } else {
            buf.writeLE(localCommunicationId);
            buf.writeLE<uint16_t>(0);
            buf.writeLE(gameMode);
            buf.writeLE<uint32_t>(0);
        }
        buf.write(ssid.data(), ssid.size());
    }

    void SessionInfo::decode(Buffer& buf, bool bigEndian) {
        if(bigEndian) {
            localCommunicationId = buf.readBE<uint64_t>();
            buf.readBE<uint16_t>(); // Padding
            gameMode = buf.readBE<uint16_t>();
            buf.readBE<uint32_t>(); // Padding
        } else {
            localCommunicationId = buf.readLE<uint64_t>();
            buf.readLE<uint16_t>();
            gameMode = buf.readLE<uint16_t>();
            buf.readLE<uint32_t>();
        }
        buf.read(ssid.data(), ssid.size());
    }

    // ParticipantInfo implementation
    void ParticipantInfo::encode(Buffer& buf) {
        in_addr ip;
        inet_pton(AF_INET, ipAddress.c_str(), &ip);
        buf.write(reinterpret_cast<uint8_t*>(&ip), 4);
        
        auto mac = macAddress.encode();
        buf.write(mac.data(), mac.size());
        
        buf.writeBE<uint8_t>(connected ? 1 : 0);
        buf.writeBE<uint8_t>(0); // Padding
        
        std::array<uint8_t, 32> nameBuf{};
        std::copy_n(name.begin(), std::min(name.size(), 31ul), nameBuf.begin());
        buf.write(nameBuf.data(), nameBuf.size());
        
        buf.writeBE(appVersion);
        buf.pad(10);
    }

    void ParticipantInfo::decode(Buffer& buf) {
        in_addr ip;
        buf.read(reinterpret_cast<uint8_t*>(&ip), 4);
        ipAddress = inet_ntoa(ip);
        
        std::array<uint8_t, 6> mac;
        buf.read(mac.data(), mac.size());
        macAddress.decode(mac.data());
        
        connected = buf.readBE<uint8_t>() != 0;
        buf.readBE<uint8_t>(); // Padding
        
        std::array<uint8_t, 32> nameBuf;
        buf.read(nameBuf.data(), nameBuf.size());
        name = std::string(reinterpret_cast<char*>(nameBuf.data()));
        name.erase(std::find(name.begin(), name.end(), '\0'), name.end());
        
        appVersion = buf.readBE<uint16_t>();
        buf.pad(10);
    }

    // AdvertisementInfo implementation
    void AdvertisementInfo::encode(Buffer& buf) {
        buf.write(key.data(), key.size());
        buf.writeBE(securityLevel);
        buf.writeBE(stationAcceptPolicy);
        buf.pad(3);
        buf.writeBE(maxParticipants);
        buf.writeBE(numParticipants);
        
        for(auto& p : participants) {
            p.encode(buf);
        }
        
        buf.pad(2);
        buf.writeBE<uint16_t>(applicationData.size());
        buf.write(applicationData.data(), applicationData.size());
        buf.pad(384 - applicationData.size(), 0);
        buf.pad(412);
        buf.writeBE(challenge);
    }

    void AdvertisementInfo::decode(Buffer& buf) {
        buf.read(key.data(), key.size());
        securityLevel = buf.readBE<uint16_t>();
        stationAcceptPolicy = buf.readBE<uint8_t>();
        buf.pad(3);
        maxParticipants = buf.readBE<uint8_t>();
        numParticipants = buf.readBE<uint8_t>();
        
        for(auto& p : participants) {
            p.decode(buf);
        }
        
        buf.pad(2);
        uint16_t dataSize = buf.readBE<uint16_t>();
        applicationData.resize(dataSize);
        buf.read(applicationData.data(), dataSize);
        buf.pad(384 - dataSize);
        buf.pad(412);
        challenge = buf.readBE<uint64_t>();
    }

    // AdvertisementFrame implementation
    void AdvertisementFrame::encrypt(const std::vector<uint8_t>& data, std::vector<uint8_t>& out) {
        if(encryption == 1) {
            out = data;
            return;
        }
        
        const std::array<uint8_t, 16> source = {
            0x19, 0x18, 0x84, 0x74, 0x3e, 0x24, 0xc7, 0x7d, 
            0x87, 0xc6, 0x9e, 0x42, 0x07, 0xd0, 0xc4, 0x38
        };
        
        Buffer headerBuf;
        header.encode(headerBuf, true);
        std::vector<uint8_t> key(16);
        DeriveKey(headerBuf.get(), source, key);
        
        out.resize(data.size());
        AES::CTR_Crypt(key.data(), nonce.data(), data.data(), out.data(), data.size());
    }

    void AdvertisementFrame::decode(Buffer& buf) {
        if(buf.readBE<uint8_t>() != 0x7F) throw std::runtime_error("Invalid vendor frame");
        if(buf.readBE<uint24_t>() != 0x0022AA) throw std::runtime_error("Invalid OUI");
        if(buf.readBE<uint8_t>() != 4) throw std::runtime_error("Not LDN frame");
        
        buf.readBE<uint8_t>(); // Padding
        if(buf.readBE<uint16_t>() != 0x101) throw std::runtime_error("Not advertisement frame");
        buf.pad(4);
        
        Buffer headerBuf(buf.get().data() + buf.tell(), 32);
        header.decode(headerBuf, true);
        buf.skip(32);
        
        version = buf.readBE<uint8_t>();
        encryption = buf.readBE<uint8_t>();
        if(buf.readBE<uint16_t>() != 0x500) throw std::runtime_error("Invalid size field");
        buf.read(nonce.data(), 4);
        
        std::vector<uint8_t> encrypted(buf.remaining());
        buf.read(encrypted.data(), encrypted.size());
        
        std::vector<uint8_t> decrypted;
        decrypt(encrypted, decrypted);
        
        Buffer infoBuf(decrypted);
        info.decode(infoBuf);
    }
}