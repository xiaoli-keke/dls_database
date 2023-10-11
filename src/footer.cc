#include "footer.h"
#include "coding.h"
#include <cassert>

static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;

namespace dls{
void Footer::EncodeTo(std::string& dst) const {
  const size_t original_size = dst.size();
  metaindex_handle_.EncodeTo(dst);
  index_handle_.EncodeTo(dst);
  dst.resize(2 * BlockHandle::kMaxEncodedLength);  // Padding
  PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber & 0xffffffffu));
  PutFixed32(dst, static_cast<uint32_t>(kTableMagicNumber >> 32));
  assert(dst.size() == original_size + kEncodedLength);
  (void)original_size;  // Disable unused variable warning.
}

bool Footer::DecodeFrom(Slice* input) {
  const char* magic_ptr = input->data() + kEncodedLength - 8;
  const uint32_t magic_lo = DecodeFixed32(magic_ptr);
  const uint32_t magic_hi = DecodeFixed32(magic_ptr + 4);
  const uint64_t magic = ((static_cast<uint64_t>(magic_hi) << 32) |
                          (static_cast<uint64_t>(magic_lo)));
  if (magic != kTableMagicNumber) {
    return false;
  }

  bool result = metaindex_handle_.DecodeFrom(input);
  if (result) {
    result = index_handle_.DecodeFrom(input);
  }
  if (result) {
    // We skip over any leftover data (just padding for now) in "input"
    const char* end = magic_ptr + 8;
    *input = Slice(end, input->data() + input->size() - end);
  }
  return result;
}
    
} // namespace dls
