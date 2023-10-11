#pragma once
#include "options.h"
#include "blockbuilder.h"

#include <string>

namespace dls{
class Disk;

class TableBuilder{
public:
    TableBuilder(Disk& disk, uint64_t offset, const Options& options);
    void add(const std::string& key, const std::string& value);

    void flush();
    void finish();

    void writeBlock(BlockBuilder& block, BlockHandle& handle);

    uint64_t getTotalSize() const {
        return total_size_;
    }

private:
    Disk& disk_;
    uint64_t offset_ {0};
    const Options& options_;
    BlockBuilder data_block_;
    BlockBuilder index_block_;  // save index block
    int64_t num_entries_;     //
    bool pending_index_entry_{false};
    BlockHandle pending_handle_;
    std::string last_key_;
    uint64_t total_size_ {0};

};
    
} // namespace dls
