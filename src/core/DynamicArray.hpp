#pragma once
#include <cstddef>
#include <stdexcept>

template<typename T>
class DynamicArray {
private:
    T* data_;
    size_t size_;
    size_t capacity_;

    void resize(size_t new_capacity) {
        T* new_data = new T[new_capacity];
        for (size_t i = 0; i < size_; ++i)
            new_data[i] = data_[i];
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}

    explicit DynamicArray(size_t size) : data_(new T[size]), size_(size), capacity_(size) {}

    DynamicArray(const DynamicArray& other)
        : data_(new T[other.size_]), size_(other.size_), capacity_(other.size_) {
        for (size_t i = 0; i < size_; ++i)
            data_[i] = other.data_[i];
    }

    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    ~DynamicArray() {
        delete[] data_;
    }

    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            capacity_ = other.size_;
            data_ = new T[size_];
            for (size_t i = 0; i < size_; ++i)
                data_[i] = other.data_[i];
        }
        return *this;
    }

    T& operator[](size_t index) {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data_[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data_[index];
    }

    size_t GetSize() const { return size_; }

    void Append(const T& value) {
        if (size_ >= capacity_) {
            resize(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = value;
    }

    void RemoveLast() {
        if (size_ > 0) --size_;
    }
};