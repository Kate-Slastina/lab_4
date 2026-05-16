#pragma once
#include "core/Optional.hpp"
#include <iterator>

template<typename T>
class LazySequence;

template<typename T>
class LazySequenceIterator {
private:
    LazySequence<T>* sequence_;
    size_t currentIndex_;

public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    explicit LazySequenceIterator(LazySequence<T>* seq, size_t startIndex = 0)
        : sequence_(seq), currentIndex_(startIndex) {}

    T operator*() const {
        return sequence_->Get(currentIndex_);
    }

    LazySequenceIterator& operator++() {
        ++currentIndex_;
        return *this;
    }

    LazySequenceIterator operator++(int) {
        LazySequenceIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const LazySequenceIterator& other) const {
        return sequence_ == other.sequence_ && currentIndex_ == other.currentIndex_;
    }

    bool operator!=(const LazySequenceIterator& other) const {
        return !(*this == other);
    }

    size_t GetIndex() const { return currentIndex_; }
};

template<typename T>
class ILazySequenceIterator {
public:
    virtual size_t GetCurrentIndex() const = 0;
    virtual T GetNext() = 0;
    virtual bool HasNext() const = 0;
    virtual Optional<T> TryGetNext() = 0;
    virtual ~ILazySequenceIterator() = default;
};

template<typename T>
class SequenceIteratorAdapter : public ILazySequenceIterator<T> {
private:
    LazySequenceIterator<T> it_;
public:
    SequenceIteratorAdapter(LazySequence<T>* seq, size_t start = 0)
        : it_(seq, start) {}

    size_t GetCurrentIndex() const override {
        return it_.GetIndex();
    }

    T GetNext() override {
        T val = *it_;
        ++it_;
        return val;
    }

    bool HasNext() const override {
        return true;
    }

    Optional<T> TryGetNext() override {
        try {
            return Optional<T>(GetNext());
        } catch (...) {
            return Optional<T>();
        }
    }
};