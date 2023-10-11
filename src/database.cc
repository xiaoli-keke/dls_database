#include "database.h"
#include "tablebuilder.h"
#include <chrono>
#include <iostream>
#include "tablereader.h"
#include "logger.h"

namespace dls{

DataBase::DataBase(const std::string& catlog) 
: catlog_(catlog)
, disk_(catlog){
    mem_.reset(new MemTable());
    write_log_thread_ = std::thread([this](){
        writeLog();
    });
}

DataBase::~DataBase() {
    if(write_log_thread_.joinable()){
        write_log_thread_.join();
    }
}

int DataBase::put(KeyType&& key, KeyType&& value, uint64_t timestamp){
    std::lock_guard<std::mutex> lk(mutex_);
    int ret = mem_->insert(std::move(key), std::move(value), timestamp);
    if(ret != 0) {
        return ret;
    }
    // calculate the size
    //kell
    if(mem_->totalSize() >= options_.table_size) {
        mems_.emplace_back(std::move(mem_));
        mem_.reset(new MemTable());
        cv_.notify_all();
    }
    return 0;
}

std::list<Record> DataBase::get(const KeyType& key, uint64_t begin_ts, uint64_t end_ts) {
    
    std::list<MemTable::ptr> mem_tables;
    std::list<Record> records;
    std::list<MetaInfo> meta_infos;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        // 1. 从当前mem_中遍历
        records = mem_->get(key, begin_ts, end_ts);
        // 2 遍历 mems_, 选出可以用来search 的memtable
        for(size_t i = 0; i < mems_.size(); ++i) {
            MemTable::ptr mem_table = mems_[i];
            if(mem_table->matched(key, begin_ts, end_ts)) {
                mem_tables.emplace_back(std::move(mem_table));
            }   
        }
        // 读取meta info, 获取满足条件的offset list
        for(auto& meta_info : meta_infos_) {
            if(begin_ts > meta_info.end_ts || end_ts < meta_info.begin_ts) {
                continue;
            }
            meta_infos.push_back(meta_info);
        }
    }
    for(const auto& mem : mem_tables){
        mem->get(key, begin_ts, end_ts, records);
    }
    TableReader table_reader(disk_);
    KeyType key_with_begin_ts = key;
    KeyType key_with_end_ts = key;
    PutFixed64(key_with_begin_ts, begin_ts);
    PutFixed64(key_with_end_ts, end_ts);
    std::cout << "read from file=====================" << std::endl;
    for(const auto meta_info : meta_infos) {
        auto results = table_reader.get(meta_info.offset, key_with_begin_ts, key_with_end_ts);
        
        for(auto &r : results) {
            std::cout << r.format() << std::endl;
        }
    }
    std::cout << "read file end" << std::endl;
    return records;
}

void DataBase::exit() {
    exist_ = true;
    cv_.notify_all();
}

void DataBase::writeLog() {
    Options options;
    while(!exist_) {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait(lk, [this](){
            return exist_ == true || !mems_.empty();
        });
        while(!mems_.empty()) {
            auto mem = mems_.front();
            // 假设一次写入50ms, 写入log 和meta, 写入meta info
            lk.unlock();
            TableBuilder table_builder(disk_, offset_, options_);
            for(auto it = mem->datas_.begin(); it != mem->datas_.end(); it++){
                table_builder.add(it->first, it->second);
            }
            table_builder.finish();
            MetaInfo metainfo;
            offset_ = table_builder.getTotalSize();
            //LOG_DEBUG("offset=%ld", offset_);
            metainfo.begin_ts = mem->getSmallestTimestamp();
            metainfo.end_ts = mem->getLargestTimestamp();
            metainfo.offset = offset_;
            // 需要写入meta data
            lk.lock();
            meta_infos_.emplace_back(metainfo);
            mems_.pop_front();
        }
        // sync 200ms
        //disk_.sync();
        // std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    {
        TableBuilder table_builder(disk_, offset_, options_);
        std::lock_guard<std::mutex> lk(mutex_);
        for(auto it = mem_->datas_.begin(); it != mem_->datas_.end(); it++){
            table_builder.add(it->first, it->second);
        }
        table_builder.finish();
        offset_ = table_builder.getTotalSize();
        MetaInfo metainfo;
        metainfo.begin_ts = mem_->getSmallestTimestamp();
        metainfo.end_ts = mem_->getLargestTimestamp();
        metainfo.offset = offset_;
        //LOG_DEBUG("offset=%ld", offset_);
        meta_infos_.emplace_back(metainfo);
        mems_.clear();
    }
    disk_.sync();
    LOG_DEBUG("SYNC");
}

}