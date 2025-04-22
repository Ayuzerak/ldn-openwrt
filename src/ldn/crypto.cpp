#include "crypto.hpp"
#include <algorithm>

namespace LDN {
    void AES::ECB_Decrypt(const uint8_t* key, const uint8_t* in, uint8_t* out) {
        AES_KEY aesKey;
        AES_set_decrypt_key(key, 128, &aesKey);
        AES_ecb_encrypt(in, out, &aesKey, AES_DECRYPT);
    }

    void AES::CTR_Crypt(const uint8_t* key, const uint8_t* nonce, const uint8_t* in, uint8_t* out, size_t len) {
        AES_KEY aesKey;
        AES_set_encrypt_key(key, 128, &aesKey);
        
        uint8_t counter[16] = {0};
        std::copy_n(nonce, 4, counter);
        uint32_t* ctr = reinterpret_cast<uint32_t*>(counter + 4);
        
        uint8_t ecount[16] = {0};
        unsigned int num = 0;
        AES_ctr128_encrypt(in, out, len, &aesKey, counter, ecount, &num);
    }

    void DeriveKey(const std::vector<uint8_t>& input, const std::vector<uint8_t>& source, std::vector<uint8_t>& output) {
        std::vector<uint8_t> temp(16);
        
        // First decryption with MASTER_KEY
        AES::ECB_Decrypt(MASTER_KEY.data(), AES_KEK_GENERATION_SOURCE.data(), temp.data());
        
        // Subsequent decryption steps
        AES::ECB_Decrypt(temp.data(), source.data(), temp.data());
        AES::ECB_Decrypt(temp.data(), AES_KEY_GENERATION_SOURCE.data(), temp.data());
        
        // SHA-256 hash of input
        std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
        SHA256(input.data(), input.size(), hash.data());
        
        // Final decryption
        std::vector<uint8_t> keyMaterial(hash.begin(), hash.begin() + 16);
        AES::ECB_Decrypt(temp.data(), keyMaterial.data(), output.data());
    }

    void GenerateDataKey(const std::vector<uint8_t>& key, const std::string& password, std::vector<uint8_t>& output) {
        const std::vector<uint8_t> source = {0xf1, 0xe7, 0x01, 0x84, 0x19, 0xa8, 0x4f, 0x71, 
                                           0x1d, 0xa7, 0x14, 0xc2, 0xcf, 0x91, 0x9c, 0x9c};
        std::vector<uint8_t> input(key);
        input.insert(input.end(), password.begin(), password.end());
        DeriveKey(input, source, output);
    }

    void HMAC_SHA256(const uint8_t* key, size_t key_len, const uint8_t* data, size_t data_len, uint8_t* output) {
        HMAC_CTX* ctx = HMAC_CTX_new();
        HMAC_Init_ex(ctx, key, key_len, EVP_sha256(), nullptr);
        HMAC_Update(ctx, data, data_len);
        HMAC_Final(ctx, output, nullptr);
        HMAC_CTX_free(ctx);
    }
}