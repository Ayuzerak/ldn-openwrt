#include "buffer.hpp"
#include <endian.h>
#include <algorithm>
#include <stdexcept>

namespace LDN {

void Buffer::write(const uint8_t* bytes, size_t size) {
    if (pos + size > data.size()) {
        data.resize(pos + size);
    }
    std::copy(bytes, bytes + size, data.begin() + pos);
    pos += size;
}

void Buffer::read(uint8_t* out, size_t size) {
    if (pos + size > data.size()) {
        throw std::out_of_range("Buffer read overflow");
    }
    std::copy(data.begin() + pos, data.begin() + pos + size, out);
    pos += size;
}

template<typename T>
T Buffer::readBE() {
    T value;
    read(reinterpret_cast<uint8_t*>(&value), sizeof(T));
    
    if constexpr (sizeof(T) == 2) {
        return be16toh(value);
    } else if constexpr (sizeof(T) == 4) {
        return be32toh(value);
    } else if constexpr (sizeof(T) == 8) {
        return be64toh(value);
    } else {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8, 
                     "Unsupported type size");
    }
}

template<typename T>
T Buffer::readLE() {
    T value;
    read(reinterpret_cast<uint8_t*>(&value), sizeof(T));
    
    if constexpr (sizeof(T) == 2) {
        return le16toh(value);
    } else if constexpr (sizeof(T) == 4) {
        return le32toh(value);
    } else if constexpr (sizeof(T) == 8) {
        return le64toh(value);
    } else {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                     "Unsupported type size");
    }
}

template<typename T>
void Buffer::writeBE(T value) {
    T be_value;
    
    if constexpr (sizeof(T) == 2) {
        be_value = htobe16(value);
    } else if constexpr (sizeof(T) == 4) {
        be_value = htobe32(value);
    } else if constexpr (sizeof(T) == 8) {
        be_value = htobe64(value);
    } else {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                     "Unsupported type size");
    }
    
    write(reinterpret_cast<uint8_t*>(&be_value), sizeof(T));
}

template<typename T>
void Buffer::writeLE(T value) {
    T le_value;
    
    if constexpr (sizeof(T) == 2) {
        le_value = htole16(value);
    } else if constexpr (sizeof(T) == 4) {
        le_value = htole32(value);
    } else if constexpr (sizeof(T) == 8) {
        le_value = htole64(value);
    } else {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                     "Unsupported type size");
    }
    
    write(reinterpret_cast<uint8_t*>(&le_value), sizeof(T));
}

void Buffer::pad(size_t count, uint8_t value) {
    const size_t new_pos = pos + count;
    if (new_pos > data.size()) {
        data.resize(new_pos, value);
    } else {
        std::fill_n(data.begin() + pos, count, value);
    }
    pos = new_pos;
}

void Buffer::align(size_t alignment) {
    const size_t remainder = pos % alignment;
    if (remainder != 0) {
        pad(alignment - remainder);
    }
}

// Explicit template instantiation
template uint16_t Buffer::readBE<uint16_t>();
template uint32_t Buffer::readBE<uint32_t>();
template uint64_t Buffer::readBE<uint64_t>();
template uint16_t Buffer::readLE<uint16_t>();
template uint32_t Buffer::readLE<uint32_t>();
template uint64_t Buffer::readLE<uint64_t>();

template void Buffer::writeBE<uint16_t>(uint16_t);
template void Buffer::writeBE<uint32_t>(uint32_t);
template void Buffer::writeBE<uint64_t>(uint64_t);
template void Buffer::writeLE<uint16_t>(uint16_t);
template void Buffer::writeLE<uint32_t>(uint32_t);
template void Buffer::writeLE<uint64_t>(uint64_t);

} // namespace LDN