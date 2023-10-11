#include "blockbuilder.h"
#include "coding.h"
#include <cassert>

namespace dls
{

BlockBuilder::BlockBuilder(const Options& options)
: options_(options){}

void BlockBuilder::reset(){
    buffer_.clear();
    restarts_.clear();
    restarts_.push_back(0); // First restart point is at offset 0
    counter_ = 0;
    finished_ = false;
    last_key_.clear();
  }

const std::string& BlockBuilder::finish(){
    // Append restart array
    for (size_t i = 0; i < restarts_.size(); i++)
    {
      PutFixed32(buffer_, restarts_[i]);
    }
    PutFixed32(buffer_, restarts_.size());
    finished_ = true;
    return buffer_;
  }

void BlockBuilder::add(const std::string &key, const std::string &value){
    assert(counter_ <= options_.block_restart_interval);
    size_t shared = 0;
    if (counter_ < options_.block_restart_interval)
    {
      const size_t min_length = std::min(last_key_.length(), key.length());
      while ((shared < min_length) && last_key_[shared] == key[shared])
      {
        shared++;
      }
    }
    else
    {
      restarts_.push_back(buffer_.length());
      counter_ = 0;
    }

    const size_t non_shared = key.size() - shared;
    // Add "<shared><non_shared><value_size>" to buffer_
    PutVarint32(buffer_, shared);
    PutVarint32(buffer_, non_shared);
    PutVarint32(buffer_, value.size());
    buffer_.append(key.data() + shared, non_shared);
    buffer_.append(value.data(), value.size());
    last_key_.resize(shared);
    last_key_.append(key.data() + shared, non_shared);
    counter_++;
  }

size_t BlockBuilder::currentSizeEstimate() const{
    return (buffer_.size() +                      // Raw data buffer
            restarts_.size() * sizeof(uint32_t) + // Restart array
            sizeof(uint32_t));                    // Restart array length
  }

} // namespace dls
