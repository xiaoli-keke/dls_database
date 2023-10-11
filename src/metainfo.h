#pragma once
#include <cstdint>
namespace dls{

struct MetaInfo {
    uint64_t begin_ts;
    uint64_t end_ts;
    uint64_t offset;
};
    
} // namespace dls
