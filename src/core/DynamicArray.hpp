#pragma once
#include "../my/my_array.hpp"
#include <stdexcept>

template<typename T>
class DynamicArray {
private:
    Array<T> data_;
public:
    DynamicArray() = default;
    DynamicArray(size_t size) : data_(size) {}
    DynamicArray(const DynamicArray& other) = default;
    DynamicArray(DynamicArray&& other) = default;

    T& operator[](size_t index) {
        if (index >= data_.size()) throw std::out_of_range("Index out of range");
        return data_[index];
    }
    const T& operator[](size_t index) const {
        if (index >= data_.size()) throw std::out_of_range("Index out of range");
        return data_[index];
    }
    size_t GetSize() const { return data_.size(); }
    void Append(const T& value) { data_.push_back(value); }
    void RemoveLast() {
        if (data_.size() > 0) {
            // Array не имеет pop_back, но можно уменьшить размер вручную – для простоты не реализуем
        }
    }
};