#pragma once
#include <cstddef>
#include "Exceptions.hpp"

template<typename T>
class Sequence {
public:
    virtual T GetFirst() const = 0;
    virtual T GetLast() const = 0;
    virtual T Get(size_t index) const = 0;
    virtual Sequence<T>* GetSubsequence(size_t startIndex, size_t endIndex) const = 0;

    virtual Sequence<T>* Append(const T& item) const = 0;
    virtual Sequence<T>* Prepend(const T& item) const = 0;
    virtual Sequence<T>* InsertAt(const T& item, size_t index) const = 0;
    virtual Sequence<T>* Concat(const Sequence<T>* other) const = 0;

    virtual size_t GetLength() const = 0;
    virtual bool IsEmpty() const = 0;

    virtual ~Sequence() = default;
};