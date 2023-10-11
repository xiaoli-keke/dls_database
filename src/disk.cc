#include "disk.h"
#include "logger.h"
#include "exception.h"
#include <iostream>
#include <sys/stat.h>


namespace dls{
Disk::Disk(const std::string& catlog){
    data_file_name_ = catlog + "/log.data";
    meta_file_name_ = catlog + "/log.meta";

    data_io_.open(data_file_name_, std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
    if(!data_io_.is_open()) {
        data_io_.clear();
        data_io_.open(data_file_name_, std::ios::binary | std::ios::trunc | std::ios::out | std::ios::in);
        if(!data_io_.is_open()) {
            throw Exception("can't open data file");
        }
    }
    meta_io_.open(meta_file_name_, std::ios::binary | std::ios::in | std::ios::app | std::ios::out);
    if(!meta_io_.is_open()) {
        meta_io_.clear();
        meta_io_.open(meta_file_name_, std::ios::binary | std::ios::trunc | std::ios::out | std::ios::in);
        if(!meta_io_.is_open()) {
            throw Exception("can't open meta file");
        }  
    }

    read_data_io_.open(data_file_name_, std::ios::binary | std::ios::out);
}

Disk::~Disk() {
    sync();
}

int Disk::writeData(const char* data, size_t length) {
    data_io_.seekg(0, std::ios_base::end);
    data_io_.write(data, length);
    if (data_io_.bad()) {
    std::cout << "I/O error while writing" << std::endl;
    }
    return 0;
}

int Disk::readData(uint64_t offset, size_t length, char* data) {
    if(offset + length > getDataFileSize()) {
        LOG_DEBUG("I/O error reading past end of file");
        return -1;
    } else {
        data_io_.seekp(offset);
        data_io_.read(data, length);
        if (data_io_.bad()) {
            LOG_DEBUG("I/O error while reading");
        }
        return -1;
    }
    return 0;
}

int Disk::sync() {
    if(data_io_.is_open()){
        data_io_.flush();
    }
    return 0;
}

size_t Disk::getDataFileSize() const{
    struct stat stat_buf;
    int rc = stat(data_file_name_.data(), &stat_buf);
    return rc == 0 ? static_cast<size_t>(stat_buf.st_size) : -1;
}
    
} // namespace dls
