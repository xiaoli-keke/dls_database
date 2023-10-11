#pragma once
#include "def.h"

#include <mutex>
#include <condition_variable>
#include <map>
#include <memory>
#include <deque>
#include <thread>
#include <atomic>
#include <list>
#include "memtable.h"
#include "disk.h"
#include "options.h"
#include "metainfo.h"

namespace dls{

class DataBase{
public:
    DataBase(const std::string& catlog);
    ~DataBase();

    int put(KeyType&& key, ValueType && value, uint64_t timestamp);

    std::list<Record> get(const KeyType& key, uint64_t begin_ts, uint64_t end_ts);

    void exit();

    void writeLog();

private:
    Options options_;
    std::string catlog_;
    uint64_t offset_ {0};
    Disk disk_;
    std::atomic<bool> exist_ {false};
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread write_log_thread_;
    MemTable::ptr mem_; //current mem table, always not null;
    std::deque<MemTable::ptr> mems_; //for write file
    std::list<MetaInfo> meta_infos_;
};
}
