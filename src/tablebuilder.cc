#include "tablebuilder.h"
#include "disk.h"
#include "footer.h"
namespace dls{

TableBuilder::TableBuilder(Disk& disk, uint64_t offset, const Options& options)
: disk_(disk)
, offset_(offset)
, options_(options)
, data_block_(options)
, index_block_(options) {
    data_block_.reset();
    index_block_.reset();
}

void TableBuilder::add(const std::string& key, const std::string& value){
    data_block_.add(key, value);
    last_key_ = key;
    num_entries_++;
    const size_t estimated_block_size = data_block_.currentSizeEstimate();
    if (estimated_block_size >= options_.block_size) {
        flush();
    }
}

void TableBuilder::flush(){
    if(data_block_.empty()) {
        return;
    }
    writeBlock(data_block_, pending_handle_);
    std::string handle_encoding;
    pending_handle_.EncodeTo(handle_encoding);
    index_block_.add(last_key_, handle_encoding);
}
void TableBuilder::finish() {
    flush();
    BlockHandle index_block_handle, metaindex_block_handle;
    writeBlock(index_block_, index_block_handle);
    Footer footer;
    footer.set_metaindex_handle(metaindex_block_handle);
    footer.set_index_handle(index_block_handle);
    std::string footer_encoding;
    footer.EncodeTo(footer_encoding);
    disk_.writeData(footer_encoding.data(), footer_encoding.size());
    total_size_ = offset_ + footer_encoding.size();
}

void TableBuilder::writeBlock(BlockBuilder& block, BlockHandle& handle) {
    const std::string& raw = block.finish();
    handle.set_offset(offset_);
    handle.set_size(raw.size());
    disk_.writeData(raw.data(), raw.length());
    offset_ += raw.size();
    block.reset();
}
} // namespace dls
