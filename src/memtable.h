#pragma once
#include "def.h"
#include "record.h"
#include <memory>
#include <map>
#include <list>

namespace dls{

class MemTable {
public:
    using ptr = std::shared_ptr<MemTable>;
    int insert(KeyType&& key, ValueType && value, uint64_t timestamp);
    std::list<Record> get(const KeyType& key, uint64_t being_ts, uint64_t end_ts) const;
    void get(const KeyType& key, uint64_t begin_ts, uint64_t end_ts, std::list<Record> &records) const;
    std::string buildTable() const;
    uint64_t totalSize() const;
    uint64_t getSmallestTimestamp() const {
        return smallest_ts_;
    }
    uint64_t getLargestTimestamp() const {
        return largest_ts_;
    }
    const KeyType& getSmallestKey() const {
        return smallest_key_;
    }
    const KeyType& getLargestKey() const {
        return largest_key_;
    }

    bool matched(const KeyType& key, uint64_t begin_ts, uint64_t end_ts) const;
private:
    friend class DataBase;
    std::multimap<KeyType, ValueType> datas_;
    uint64_t size_ = 0;
    bool first_ = true;
    KeyType smallest_key_;
    KeyType largest_key_;
    uint64_t smallest_ts_ = 0;
    uint64_t largest_ts_ = 0;
};
} // namespace name
