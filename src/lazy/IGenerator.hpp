#pragma once
#include "../core/Optional.hpp"

template<typename T>
class IGenerator {
public:
    virtual T GetNext() = 0;
    virtual bool HasNext() const = 0;
    virtual Optional<T> TryGetNext() = 0;
    virtual void Reset() {}
    virtual ~IGenerator() = default;
};