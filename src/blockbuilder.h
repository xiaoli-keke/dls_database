#pragma once
#include "options.h"
#include "blockhandle.h"
#include <vector>
#include <string>

namespace dls{

class BlockBuilder {
public:
    BlockBuilder(const Options& options);
    void reset();
    void add(const std::string& key, const std::string& value);
    const std::string& finish();
    bool empty() const { return buffer_.empty(); }

    size_t currentSizeEstimate() const;

private:
    const Options& options_;
    std::string buffer_;
    std::vector<uint32_t> restarts_;
    int counter_{0};
    bool finished_{false};
    std::string last_key_;
};
    
} // namespace dls
