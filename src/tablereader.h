#pragma once
#include "record.h"
#include "blockhandle.h"
#include <list>

namespace dls{

class Disk;

class TableReader{

public:
    TableReader(Disk& disk);
    std::list<Record> get(uint64_t offset, const std::string& smallestKey, const std::string& largestKey);
    void get(const BlockHandle& handle, const std::string& smallestKey,
            const std::string& largestKey, std::list<Record>& records);
    std::string readBlock(const BlockHandle& blockhandler);

private:
    Disk& disk_;
};
    
} // namespace dls
