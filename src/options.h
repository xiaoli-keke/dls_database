#pragma once
#include <cstdint>

namespace dls{

struct Options {
    int block_restart_interval = 32;
    std::size_t table_size = 16 * 1024;
    std::size_t block_size = 1024 * 8;
    bool sync = false;
};
    
} // namespace dls
