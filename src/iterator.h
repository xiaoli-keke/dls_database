#pragma once
#include "slice.h"

namespace dls{
class Iterator{
public:
    Iterator() {}
    Iterator(const Iterator&) = delete;
    virtual ~Iterator() {}

    virtual bool valid() const = 0;
    // seek smallest
    virtual void leftSeek(const Slice& slice) = 0;
    // seek largeset
    virtual void righSeek(const Slice& slice) = 0;

    virtual void seekToFirst() = 0;
    virtual void seekToLast() = 0;

    virtual void next() = 0;
    virtual void prev() = 0;
    
    virtual Slice key() const = 0;
    virtual Slice value() const = 0;

    virtual bool operator ==(const Iterator& other) const = 0;

};

    
} // namespace dls
