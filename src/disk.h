#pragma once
#include <fstream>
#include <string>
namespace dls{

class Disk {
public:

    Disk(const std::string& catlog);
    ~Disk();

    int writeData(const char* data, size_t length);
    int readData(uint64_t offset, size_t length, char* data);
    int sync();

private:
    size_t getDataFileSize() const;
private:
    std::fstream data_io_;
    std::fstream read_data_io_;
    std::fstream meta_io_;

    std::string data_file_name_;
    std::string meta_file_name_;

};
    
} // namespace dls

