#pragma once
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace LDN {
    class Buffer {
        std::vector<uint8_t> data;
        size_t pos = 0;
        
    public:
        Buffer() = default;
        explicit Buffer(const std::vector<uint8_t>& data) : data(data) {}
        
        void write(const uint8_t* bytes, size_t size);
        void read(uint8_t* out, size_t size);
        
        template<typename T> T readBE();
        template<typename T> T readLE();
        template<typename T> void writeBE(T value);
        template<typename T> void writeLE(T value);
        
        void pad(size_t count, uint8_t value = 0);
        void align(size_t alignment);
        
        size_t tell() const { return pos; }
        size_t size() const { return data.size(); }
        const std::vector<uint8_t>& get() const { return data; }
    };

    // Template implementations
    template<typename T> T Buffer::readBE() {
        T value = 0;
        read(reinterpret_cast<uint8_t*>(&value), sizeof(T));
        if constexpr (sizeof(T) == 2) value = be16toh(value);
        else if constexpr (sizeof(T) == 4) value = be32toh(value);
        else if constexpr (sizeof(T) == 8) value = be64toh(value);
        return value;
    }

    template<typename T> void Buffer::writeBE(T value) {
        if constexpr (sizeof(T) == 2) value = htobe16(value);
        else if constexpr (sizeof(T) == 4) value = htobe32(value);
        else if constexpr (sizeof(T) == 8) value = htobe64(value);
        write(reinterpret_cast<uint8_t*>(&value), sizeof(T));
    }
}