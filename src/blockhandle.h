#pragma once
#include <cstdint>
#include <cassert>
#include "coding.h"

namespace dls{
class BlockHandle {
public:
  // Maximum encoding length of a BlockHandle
  enum { kMaxEncodedLength = 10 + 10 };

  BlockHandle() {}

  // The offset of the block in the file.
  uint64_t offset() const { return offset_; }
  void set_offset(uint64_t offset) { offset_ = offset; }

  // The size of the stored block
  uint64_t size() const { return size_; }
  void set_size(uint64_t size) { size_ = size; }

  void EncodeTo(std::string& dst) const {
    assert(offset_ != ~static_cast<uint64_t>(0));
    assert(size_ != ~static_cast<uint64_t>(0));
    PutVarint64(dst, offset_);
    PutVarint64(dst, size_);
  }
  bool DecodeFrom(Slice* input){
if (GetVarint64(input, &offset_) && GetVarint64(input, &size_)) {
    return true;
  } else {
    return false;
  }
  }

 private:
  uint64_t offset_ = 0;
  uint64_t size_ = 0;
};
    
} // namespace dls
