#pragma once
#include <stdint.h>
#include <string>
#include "slice.h"

namespace dls{

char* EncodeVarint32(char* dst, uint32_t v);
char* EncodeVarint64(char* dst, uint64_t v);

// bool GetVarint32(Slice* input, uint32_t* value);
bool GetVarint64(Slice* input, uint64_t* value);

const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v);
const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* value);

void PutVarint32(std::string& dst, uint32_t v);
void PutVarint64(std::string& dst, uint64_t v);

void PutFixed32(std::string& dst, uint32_t value);
void PutFixed64(std::string& dst, uint64_t value);

inline void EncodeFixed64(char* dst, uint64_t value) {
    uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);
    // Recent clang and gcc optimize this to a single mov / str instruction.
    buffer[0] = static_cast<uint8_t>(value >> 56);
    buffer[1] = static_cast<uint8_t>(value >> 48);
    buffer[2] = static_cast<uint8_t>(value >> 40);
    buffer[3] = static_cast<uint8_t>(value >> 32);
    buffer[4] = static_cast<uint8_t>(value >> 24);
    buffer[5] = static_cast<uint8_t>(value >> 16);
    buffer[6] = static_cast<uint8_t>(value >> 8);
    buffer[7] = static_cast<uint8_t>(value);
}

inline uint64_t DecodeFixed64(const char* ptr) {
    const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);
    // Recent clang and gcc optimize this to a single mov / ldr instruction.
    return (static_cast<uint64_t>(buffer[7])) |
        (static_cast<uint64_t>(buffer[6]) << 8) |
         (static_cast<uint64_t>(buffer[5]) << 16) |
         (static_cast<uint64_t>(buffer[4]) << 24) |
         (static_cast<uint64_t>(buffer[3]) << 32) |
         (static_cast<uint64_t>(buffer[2]) << 40) |
         (static_cast<uint64_t>(buffer[1]) << 48) |
         (static_cast<uint64_t>(buffer[0]) << 56);
}

inline void EncodeFixed32(char* dst, uint32_t value) {
  uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);
  buffer[0] = static_cast<uint8_t>(value >> 24);
  buffer[1] = static_cast<uint8_t>(value >> 16);
  buffer[2] = static_cast<uint8_t>(value >> 8);
  buffer[3] = static_cast<uint8_t>(value);
}

inline uint32_t DecodeFixed32(const char* ptr) {
  const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);

  // Recent clang and gcc optimize this to a single mov / ldr instruction.
  return (static_cast<uint32_t>(buffer[3])) |
         (static_cast<uint32_t>(buffer[2]) << 8) |
         (static_cast<uint32_t>(buffer[1]) << 16) |
         (static_cast<uint32_t>(buffer[0]) << 24);
}

} // namespace dls
