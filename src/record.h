#pragma once
#include "def.h"

namespace dls{

struct Record {
    std::string format_timestamp;
    ValueType value;

    std::string format() const{
        return format_timestamp + ": " + value;
    }

    uint64_t totalSize() const {
        return format_timestamp.length() + 2 + value.size();
    }
};
    
} // namespace dls

