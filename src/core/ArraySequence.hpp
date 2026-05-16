#pragma once
#include "Sequence.hpp"
#include "DynamicArray.hpp"
#include <initializer_list>

template<typename T>
class ArraySequence : public Sequence<T> {
private:
    DynamicArray<T> data_;

public:
    ArraySequence() = default;

    ArraySequence(const T* items, size_t count) {
        for (size_t i = 0; i < count; ++i)
            data_.Append(items[i]);
    }

    ArraySequence(std::initializer_list<T> list) {
        for (const T& item : list)
            data_.Append(item);
    }

    ArraySequence(const DynamicArray<T>& arr) : data_(arr) {}

    ArraySequence(const ArraySequence& other) : data_(other.data_) {}

    T GetFirst() const override {
        if (data_.GetSize() == 0) throw EmptySequence();
        return data_[0];
    }

    T GetLast() const override {
        if (data_.GetSize() == 0) throw EmptySequence();
        return data_[data_.GetSize() - 1];
    }

    T Get(size_t index) const override {
        if (index >= data_.GetSize()) throw IndexOutOfRange();
        return data_[index];
    }

    Sequence<T>* GetSubsequence(size_t startIndex, size_t endIndex) const override {
        if (startIndex > endIndex || endIndex >= data_.GetSize()) throw IndexOutOfRange();
        auto* sub = new ArraySequence<T>();
        for (size_t i = startIndex; i <= endIndex; ++i)
            sub->Append(data_[i]);
        return sub;
    }

    Sequence<T>* Append(const T& item) const override {
        auto* copy = new ArraySequence<T>(*this);
        copy->data_.Append(item);
        return copy;
    }

    Sequence<T>* Prepend(const T& item) const override {
        auto* copy = new ArraySequence<T>();
        copy->data_.Append(item);
        for (size_t i = 0; i < data_.GetSize(); ++i)
            copy->data_.Append(data_[i]);
        return copy;
    }

    Sequence<T>* InsertAt(const T& item, size_t index) const override {
        if (index > data_.GetSize()) throw IndexOutOfRange();
        auto* copy = new ArraySequence<T>();
        for (size_t i = 0; i < index; ++i)
            copy->data_.Append(data_[i]);
        copy->data_.Append(item);
        for (size_t i = index; i < data_.GetSize(); ++i)
            copy->data_.Append(data_[i]);
        return copy;
    }

    Sequence<T>* Concat(const Sequence<T>* other) const override {
        auto* copy = new ArraySequence<T>(*this);
        for (size_t i = 0; i < other->GetLength(); ++i)
            copy->data_.Append(other->Get(i));
        return copy;
    }

    size_t GetLength() const override { return data_.GetSize(); }
    bool IsEmpty() const override { return data_.GetSize() == 0; }

    void AppendInPlace(const T& item) { data_.Append(item); }
};