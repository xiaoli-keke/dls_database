#include "tablereader.h"
#include "disk.h"
#include "footer.h"
#include "block.h"
#include "iterator.h"
namespace dls{

TableReader::TableReader(Disk& disk)
: disk_(disk) {

}

std::list<Record> TableReader::get(uint64_t offset, const std::string& smallestKey, const std::string& largestKey) {
    std::list<Record> records;
    Footer footer;
    char footer_space[Footer::kEncodedLength];
    disk_.readData(offset - Footer::kEncodedLength, Footer::kEncodedLength, footer_space);
    Slice FooterSlice(footer_space, Footer::kEncodedLength);
    footer.DecodeFrom(&FooterSlice);
    std::string index_buf = readBlock(footer.index_handle());
    Block index_block(index_buf);
    Iterator* left_index_iter = static_cast<Iterator*>(index_block.newIterator());
    if(left_index_iter == nullptr) {
        return records;
    }
    left_index_iter->seekToFirst();
    if(!left_index_iter->valid()) {
        delete left_index_iter;
        return records;
    }
    Iterator* right_index_iter = static_cast<Iterator*>(index_block.newIterator());
    right_index_iter->seekToLast();
    if(!right_index_iter->valid() || right_index_iter->key().compare(Slice(smallestKey)) < 0) {
        delete right_index_iter;
        delete left_index_iter;
        return records;
    }
    left_index_iter->leftSeek(smallestKey);
    // while(left_index_iter->valid()) {
    //     Slice key = left_index_iter->key();
    //     BlockHandle handle;
    //     Slice value = left_index_iter->value();
    //     handle.DecodeFrom(&value);
    //     get(handle, smallestKey, largestKey, records);
    //     if(key.compare(largestKey) > 0) {
    //         break;
    //     }
    //     left_index_iter->next();
    // }
    right_index_iter->righSeek(largestKey);
    while(true) {
        if(!left_index_iter->valid()){
            break;
        }
        BlockHandle handle;
        Slice value = left_index_iter->value();
        handle.DecodeFrom(&value);
        get(handle, smallestKey, largestKey, records);
        if(*left_index_iter == *right_index_iter) {
            // 这里需要再向下取一次，因为index的指向的是data中最后一个，所以
            // 可能存在部分数据在next上.
            left_index_iter->next();
            if(left_index_iter->valid()) {
                value = left_index_iter->value();
                handle.DecodeFrom(&value);
                get(handle, smallestKey, largestKey, records);
            }
            break;
        }
        left_index_iter->next();
    }
    delete left_index_iter;
    delete right_index_iter;
    return records;
}

void TableReader::get(const BlockHandle& handle, const std::string& smallestKey,
            const std::string& largestKey, std::list<Record>& records){
    std::string buf = readBlock(handle);
    Block block(buf);
    Iterator* left_iter = static_cast<Iterator*>(block.newIterator());
    left_iter->seekToFirst();
    if(!left_iter->valid()) {
        delete left_iter;
        return;
    }
    if(left_iter->key().compare(Slice(largestKey)) > 0) {
        delete left_iter;
        return;
    }
    Iterator* right_iter = static_cast<Iterator*>(block.newIterator());
    right_iter->seekToLast();
    if(!right_iter->valid() || right_iter->key().compare(Slice(smallestKey)) < 0) {
        delete right_iter;
        return;
    }
    left_iter->leftSeek(smallestKey);
    // while(left_iter->valid()) {
    //     Slice value = left_iter->value();
    //     Slice key = left_iter->key();
    //     if(key.compare(largestKey) > 0) {
    //         break;
    //     }
    //     Record r;
    //     uint64_t timestamp = DecodeFixed64(key.data() + key.size() - sizeof(uint64_t));
    //     r.format_timestamp = std::to_string(timestamp);
    //     r.value.assign(value.data(), value.size());
    //     records.emplace_back(std::move(r));
    //     left_iter->next();
    // }
    right_iter->righSeek(largestKey);
    if(!right_iter->valid()){
        delete left_iter;
        delete right_iter;
        return;
    }
    while(true) {
        if(!left_iter->valid()){
            break;
        }
        Slice value = left_iter->value();
        Slice key = left_iter->key();
        Record r;
        uint64_t timestamp = DecodeFixed64(key.data() + key.size() - sizeof(uint64_t));
        r.format_timestamp = std::to_string(timestamp);
        r.value.assign(value.data(), value.size());
        records.emplace_back(std::move(r));
        if(*left_iter == *right_iter) {
            break;
        }
        left_iter->next();
    }
    delete left_iter;
    delete right_iter;

}


std::string TableReader::readBlock(const BlockHandle& handle) {
    size_t n = static_cast<size_t>(handle.size());
    std::string buf;
    buf.resize(n);
    disk_.readData(handle.offset(), n, buf.data());
    return buf;
}


    
} // namespace dls
