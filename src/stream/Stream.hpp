#pragma once
#include <cstddef>
#include "core/Exceptions.hpp"

template<typename T>
class Stream {
public:
    virtual bool IsEndOfStream() = 0;          
    virtual T Read() = 0;
    virtual size_t GetPosition() const = 0;
    virtual bool CanSeek() const = 0;
    virtual void Seek(size_t index) = 0;
    virtual void Open() = 0;
    virtual void Close() = 0;
    virtual ~Stream() = default;
};