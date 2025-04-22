#pragma once
#include "buffer.hpp"
#include "macaddress.hpp"
#include <cstdint>
#include <vector>
#include <array>
#include <string>

namespace LDN {
    // Constants
    enum StationAcceptPolicy : uint8_t {
        ACCEPT_ALL = 0,
        ACCEPT_NONE = 1,
        ACCEPT_BLACKLIST = 2,
        ACCEPT_WHITELIST = 3
    };

    enum AuthStatusCode : uint8_t {
        AUTH_SUCCESS = 0,
        AUTH_DENIED_BY_POLICY = 1,
        AUTH_MALFORMED_REQUEST = 2,
        AUTH_INVALID_VERSION = 4,
        AUTH_UNEXPECTED = 5,
        AUTH_CHALLENGE_FAILURE = 6
    };

    // Protocol structures
    struct SessionInfo {
        uint64_t localCommunicationId;
        uint16_t gameMode;
        std::array<uint8_t, 16> ssid;

        void encode(Buffer& buf, bool bigEndian);
        void decode(Buffer& buf, bool bigEndian);
    };

    struct ParticipantInfo {
        std::string ipAddress;
        MACAddress macAddress;
        bool connected = false;
        std::string name;
        uint16_t appVersion = 0;

        void encode(Buffer& buf);
        void decode(Buffer& buf);
        void reset();
    };

    struct AdvertisementInfo {
        std::array<uint8_t, 16> key;
        uint16_t securityLevel;
        uint8_t stationAcceptPolicy;
        uint8_t maxParticipants;
        uint8_t numParticipants;
        std::array<ParticipantInfo, 8> participants;
        std::vector<uint8_t> applicationData;
        uint64_t challenge;

        void encode(Buffer& buf);
        void decode(Buffer& buf);
    };

    class AdvertisementFrame {
        SessionInfo header;
        uint8_t version;
        uint8_t encryption;
        std::array<uint8_t, 4> nonce;
        AdvertisementInfo info;

    public:
        void encrypt(const std::vector<uint8_t>& data, std::vector<uint8_t>& out);
        void decrypt(const std::vector<uint8_t>& data, std::vector<uint8_t>& out);
        
        void encode(Buffer& buf);
        void decode(Buffer& buf);
        
        // Getters/Setters
        AdvertisementInfo& getInfo() { return info; }
        SessionInfo& getHeader() { return header; }
        uint8_t getVersion() const { return version; }
    };

    struct ChallengeRequest {
        uint64_t token;
        uint64_t nonce;
        uint64_t deviceId;
        std::vector<uint64_t> params1;
        std::vector<uint64_t> params2;

        void encode(Buffer& buf);
        void decode(Buffer& buf);
    };

    struct ChallengeResponse {
        uint64_t nonce;
        uint64_t deviceId;
        uint64_t deviceIdHost;

        void encode(Buffer& buf);
        void decode(Buffer& buf);
    };

    struct AuthenticationRequest {
        std::string username;
        uint16_t appVersion;
        std::vector<uint8_t> challenge;

        void encode(Buffer& buf, uint8_t version);
        void decode(Buffer& buf, uint8_t version);
    };

    struct AuthenticationResponse {
        std::vector<uint8_t> challenge;

        void encode(Buffer& buf, uint8_t version);
        void decode(Buffer& buf, uint8_t version);
    };

    class AuthenticationFrame {
        uint8_t version;
        uint8_t statusCode;
        SessionInfo header;
        std::array<uint8_t, 16> networkKey;
        std::array<uint8_t, 16> authKey;
        bool isResponse;
        union {
            AuthenticationRequest request;
            AuthenticationResponse response;
        } payload;

    public:
        void encode(Buffer& buf);
        void decode(Buffer& buf);
    };

    class DisconnectFrame {
        uint8_t reason;
        
    public:
        void encode(Buffer& buf);
        void decode(Buffer& buf);
    };
}