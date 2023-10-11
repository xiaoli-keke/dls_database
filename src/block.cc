#include "block.h"
#include "coding.h"
#include "iterator.h"
#include "logger.h"
#include <cassert>

namespace dls{

static inline const char* DecodeEntry(const char* p, const char* limit,
                                        uint32_t* shared, uint32_t* non_shared,
                                        uint32_t* value_length){
    if(limit -p < 3) return nullptr;
    *shared = reinterpret_cast<const uint8_t*>(p)[0];
    *non_shared = reinterpret_cast<const uint8_t*>(p)[1];
    *value_length = reinterpret_cast<const uint8_t*>(p)[2];
    if ((*shared | *non_shared | *value_length) < 128) {
    // Fast path: all three values are encoded in one byte each
        p += 3;
    } else {
        if ((p = GetVarint32Ptr(p, limit, shared)) == nullptr) return nullptr;
        if ((p = GetVarint32Ptr(p, limit, non_shared)) == nullptr) return nullptr;
        if ((p = GetVarint32Ptr(p, limit, value_length)) == nullptr) return nullptr;
    }
    if (static_cast<uint32_t>(limit - p) < (*non_shared + *value_length)) {
    return nullptr;
  }
  return p;
}

class Block::Iter : public Iterator {
private:
    const char* const data_;    // underlying block contents
    uint32_t const restarts_;   // Offset of restart array (list of fixed32)
    uint32_t const num_restarts_; // Number of uint32_t entries in restart array

    // current_ is offset in data_ of current entry.  >= restarts_ if !Valid
    uint32_t current_;
    uint32_t restart_index_;  // Index of restart block in which current_ falls
    std::string key_;
    Slice value_;

    inline uint32_t NextEntryOffset() const {
        return (value_.data() + value_.size()) - data_;
    }

    uint32_t GetRestartPoint(uint32_t index) {
        assert(index < num_restarts_);
        return DecodeFixed32(data_ + restarts_ + index * sizeof(uint32_t));
    }

public:
    Iter(const char* data, uint32_t restarts,
        uint32_t num_restarts)
        : data_(data),
        restarts_(restarts),
        num_restarts_(num_restarts),
        current_(restarts_),
        restart_index_(num_restarts_) {
    assert(num_restarts_ > 0);
  }

    bool valid() const override { return current_ < restarts_; }
    
    Slice key() const override {
        assert(valid());
        return key_;
    }

    Slice value() const override {
        assert(valid());
        return value_;
    }

    void prev() override {
        assert(valid());
        const uint32_t original = current_;
        // restart index 是当前的data的restart 的offset ,理论上只有等于，没有大于当前的offset
        while(GetRestartPoint(restart_index_) >= original) {
            if(restart_index_ == 0) {
                //no more entries
                current_ = restarts_;
                restart_index_ = num_restarts_;
                return;
            }
            restart_index_--;
        }
        seekToRestartPoint(restart_index_);
        // 到这里restart_index_ 一定是小于当前的，第一次的parseNextKey是解析当前restart的部分
        do {
            // Loop until end of current entry hits the start of original entry
        }while (parseNextKey() && NextEntryOffset() < original);
    }

    void next() override {
        assert(valid());
        parseNextKey();
    }

    void leftSeek(const Slice& target) override{
        seekToFirst();
        if(!valid()) {
            return;
        }
        if(Slice(key_).compare(target) >= 0) {
            // 如果最小的key 仍然大于或者等于target 则，该范围都满足
            return;
        }
        uint32_t left = 0;
        uint32_t right = num_restarts_ - 1;
        while(left < right) {
            uint32_t mid = (left + right + 1) / 2;
            uint32_t region_offset = GetRestartPoint(mid);
            uint32_t shared, non_shared, value_length;
            const char* key_ptr =
                DecodeEntry(data_ + region_offset, data_ + restarts_, &shared,
                      &non_shared, &value_length);
            Slice mid_key(key_ptr, non_shared);
            if(mid_key.compare(target) < 0) {
                left = mid;
            } else {
                // Key at "mid" is >= "target".
                right = mid - 1;
            }
        }
        seekToRestartPoint(left);
        while (true){
            if(!parseNextKey()) {
                return;
            }
            // 从最小的里面找到一个最大的key, 如果 >= target ，则符合条件
            if(Slice(key_).compare(target) >= 0) {
                return;
            }
        }
    }
    // seek largeset
    void righSeek(const Slice& target) override {
        seekToLast();
        if(!valid()) {
            return;
        }
        if(Slice(key_).compare(target) < 0){
            return;
        }
        uint32_t left = 0;
        uint32_t right = num_restarts_ - 1;
        while(left < right) {
            uint32_t mid = (left + right + 1) / 2;
            uint32_t region_offset = GetRestartPoint(mid);
            uint32_t shared, non_shared, value_length;
            const char* key_ptr =
                DecodeEntry(data_ + region_offset, data_ + restarts_, &shared,
                      &non_shared, &value_length);
            Slice mid_key(key_ptr, non_shared);
            if(mid_key.compare(target) > 0) {
                right = (right == mid ? mid - 1 : mid);
            } else {
                // Key at "mid" is < "target".
                left = mid + 1;
            }
        }
        seekToRestartPoint(right);
         while (parseNextKey() && NextEntryOffset() < restarts_) {
            // Keep skipping
        }
        while (true){
            if(!valid()){
                return;
            }
            // 从最大到小的里面找到一个最小的key, 如果 <= target ，则符合条件
            if(Slice(key_).compare(target) <= 0) {
                return;
            }
            prev();
        }
    }

    void seekToFirst() override {
        seekToRestartPoint(0);
        parseNextKey();
    }
    void seekToLast() override {
        seekToRestartPoint(num_restarts_ - 1);
        while (parseNextKey() && NextEntryOffset() < restarts_) {
            // Keep skipping
        }
    }
    uint32_t getRestartPoint(uint32_t index) {
        assert(index < num_restarts_);
        return DecodeFixed32(data_ + restarts_ + index * sizeof(uint32_t));
    }

    void seekToRestartPoint(uint32_t index) {
        key_.clear();
        restart_index_ = index;
        uint32_t offset = getRestartPoint(index);
        value_ = Slice(data_ + offset, 0);
    }

    bool parseNextKey() {
        current_ = NextEntryOffset();
        const char* p = data_ + current_;
        const char* limit = data_ + restarts_;
        if(p >= limit) {
            current_ = restarts_;
            restart_index_ = num_restarts_;
            return false;
        }
        uint32_t shared, non_shared, value_length;
        p = DecodeEntry(p, limit, &shared, &non_shared, &value_length);
        if(p == nullptr || key_.size() < shared) {
            LOG_ERROR("decode entry");
            return false;
        } else {
            key_.resize(shared);
            key_.append(p, non_shared);
            value_ = Slice(p + non_shared, value_length);
            while(restart_index_ + 1 < num_restarts_ &&
                GetRestartPoint(restart_index_ + 1) < current_) {
                ++restart_index_;
            }
            return true;
        }
    }

    bool operator ==(const Iterator& other) const override {
        if(this == &other) {
            return true;
        }
        const Iter* otherIter = dynamic_cast<const Iter*>(&other);
        if(otherIter == nullptr) {
            return false;
        }
        return (data_ == otherIter->data_ && current_ == otherIter->current_);
    }
};

inline uint32_t Block::NumRestarts() const{
    assert(size_ >= sizeof(uint32_t));
    return DecodeFixed32(data_ + size_ - sizeof(uint32_t));
}

Block::Block(const std::string& buf)
: data_(buf.data())
, size_(buf.size()){
    if(size_ < sizeof(uint32_t)) {
        size_ = 0;
    } else {
        size_t max_restarts_allowed = (size_ - sizeof(uint32_t)) / sizeof(uint32_t);
        if (NumRestarts() > max_restarts_allowed) {
        // The size is too small for NumRestarts()
            size_ = 0;
        } else {
            restart_offset_ = size_ - (1 + NumRestarts()) * sizeof(uint32_t);
        }
    }
}

dls::Iterator* Block::newIterator() {
    if (size_ < sizeof(uint32_t)) {
    return nullptr;
    }
    const uint32_t num_restarts = NumRestarts();
    if (num_restarts == 0) {
        return nullptr;
    } else {
        Iter *it = new Iter(data_, restart_offset_, num_restarts);
        return static_cast<Iterator*>(it);

    }
}   
} // namespace dls
