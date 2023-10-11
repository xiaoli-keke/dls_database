#include "memtable.h"
#include "coding.h"
#include <cassert>
#include <cstring>
#include <iostream>

namespace dls{
    
int MemTable::insert(KeyType&& key, KeyType&& value, uint64_t timestamp){
    if(first_ == true) {
        first_ = false;
        smallest_ts_ = timestamp;
        smallest_key_ = key;
        largest_key_ = key;
    } else {
        if(timestamp < smallest_ts_) {
            return -1;
        }
    }
    largest_ts_ = timestamp;
    if(key < smallest_key_) {
        smallest_key_ = key;
    }
    if(key > largest_key_) {
        largest_key_ = key;
    }
    KeyType key_with_ts = std::move(key);
    PutFixed64(key_with_ts, timestamp);
    size_ += (key.size() + value.size());
    datas_.emplace(std::make_pair(std::move(key_with_ts), std::move(value)));
    return 0;
}

std::list<Record> MemTable::get(const KeyType& key, uint64_t begin_ts, uint64_t end_ts) const {
    std::list<Record> records;
    //  for(const auto& item : datas_) {
    //     std::cout << item.first << " " << item.second << std::endl;
    // }
    // if(end_ts < smallest_ts_  || begin_ts > largest_ts_) {
    //     return results;
    // }
    // if(key < smallest_key_ || key > largest_key_) {
    //     return results;
    // }
    if(matched(key, begin_ts, end_ts) == false) {
        return records;
    }
    get(key, begin_ts, end_ts, records);
    return records;
}

void MemTable::get(const KeyType& key, uint64_t begin_ts, uint64_t end_ts, std::list<Record> &records) const{
    KeyType key_with_begin_ts = key;
    KeyType key_with_end_ts = key;
    PutFixed64(key_with_begin_ts, begin_ts);
    PutFixed64(key_with_end_ts, end_ts);
    auto lowerIt = datas_.lower_bound(key_with_begin_ts);
    if(lowerIt == datas_.end()){
        return;
    }
    auto upperIt = datas_.upper_bound(key_with_end_ts);
    for(auto it = lowerIt; it != upperIt; it++) {
        const KeyType& key = it->first;
        const ValueType& value = it->second;
        //parse time
        size_t key_length = key.length();
        assert(key_length >= sizeof(uint64_t));
        uint64_t timestamp = DecodeFixed64(key.data() + key_length - sizeof(uint64_t));
        Record r;
        r.format_timestamp = std::to_string(timestamp);
        r.value = value;
        records.emplace_back(std::move(r));
    }
    return;
}

std::string MemTable::buildTable() const {
    std::string result = "";
    result.reserve(1024);
    for(auto& item: datas_) {
        result.append(item.first);
        result.append(item.second);
    }
    return result;
}

bool MemTable::matched(const KeyType& key, uint64_t begin_ts, uint64_t end_ts) const {
    if(end_ts < smallest_ts_  || begin_ts > largest_ts_) {
        return false;
    }
    if(key < smallest_key_ || key > largest_key_) {
        return false;
    }
    return true;
}

uint64_t MemTable::totalSize() const{
    return size_;
}
} // namespace dl

