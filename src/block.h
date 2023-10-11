#pragma once
#include <cstdint>
#include <string>
namespace dls{

class Iterator;
class Block{
public:
    Block(const std::string& buf);
    Block(const Block&) = delete;
    Block& operator=(const Block&) = delete;

    dls::Iterator* newIterator();

private:
    class Iter;
    uint32_t NumRestarts() const;
    
private:

    const char* data_;
    std::size_t size_;
    uint32_t restart_offset_;
};

} // namespace dls

