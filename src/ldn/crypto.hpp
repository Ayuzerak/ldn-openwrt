#pragma once
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <cstdint>
#include <array>
#include <vector>

namespace LDN {
    constexpr std::array<uint8_t, 16> AES_KEK_GENERATION_SOURCE = {0x4d, 0x87, 0x09, 0x86, 0xc4, 0x5d, 0x20, 0x72, 0x2f, 0xba, 0x10, 0x53, 0xda, 0x92, 0xe8, 0xa9};
    constexpr std::array<uint8_t, 16> AES_KEY_GENERATION_SOURCE = {0x89, 0x61, 0x5e, 0xe0, 0x5c, 0x31, 0xb6, 0x80, 0x5f, 0xe5, 0x8f, 0x3d, 0xa2, 0x4f, 0x7a, 0xa8};
    constexpr std::array<uint8_t, 16> MASTER_KEY = {0xc2, 0xca, 0xaf, 0xf0, 0x89, 0xb9, 0xae, 0xd5, 0x56, 0x94, 0x87, 0x60, 0x55, 0x27, 0x1c, 0x7d};

    class AES {
    public:
        static void ECB_Decrypt(const uint8_t* key, const uint8_t* in, uint8_t* out);
        static void CTR_Crypt(const uint8_t* key, const uint8_t* nonce, const uint8_t* in, uint8_t* out, size_t len);
    };

    void DeriveKey(const std::vector<uint8_t>& input, const std::vector<uint8_t>& source, std::vector<uint8_t>& output);
    void GenerateDataKey(const std::vector<uint8_t>& key, const std::string& password, std::vector<uint8_t>& output);
    void HMAC_SHA256(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t* output);
}