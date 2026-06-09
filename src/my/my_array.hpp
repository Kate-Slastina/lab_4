#ifndef MY_ARRAY_HPP
#define MY_ARRAY_HPP

#include <cstddef>
#include <algorithm>

template<typename T>
class Array {
public:
    using iterator = T*;
    using const_iterator = const T*;

    Array() : data(nullptr), sz(0), cap(0) {}
    Array(size_t size) : sz(size), cap(size) {
        data = new T[cap];
    }
    Array(const T* begin, const T* end) : sz(end - begin), cap(sz) {
        data = new T[cap];
        for (size_t i = 0; i < sz; ++i) data[i] = begin[i];
    }
    Array(const Array& other) : sz(other.sz), cap(other.cap) {
        data = new T[cap];
        for (size_t i = 0; i < sz; ++i) data[i] = other.data[i];
    }
    ~Array() { delete[] data; }

    Array& operator=(const Array& other) {
        if (this != &other) {
            delete[] data;
            sz = other.sz;
            cap = other.cap;
            data = new T[cap];
            for (size_t i = 0; i < sz; ++i) data[i] = other.data[i];
        }
        return *this;
    }

    void push_back(const T& val) {
        if (sz >= cap) reserve(cap == 0 ? 1 : cap * 2);
        data[sz++] = val;
    }

    void reserve(size_t newCap) {
        if (newCap <= cap) return;
        T* newData = new T[newCap];
        T* src = data;
        T* dst = newData;
        T* end = data + sz;
        while (src != end) *dst++ = *src++;
        delete[] data;
        data = newData;
        cap = newCap;
    }

    size_t size() const { return sz; }
    size_t capacity() const { return cap; }
    bool empty() const { return sz == 0; }

    T& operator[](size_t idx) { return data[idx]; }
    const T& operator[](size_t idx) const { return data[idx]; }

    iterator begin() { return data; }
    iterator end() { return data + sz; }
    const_iterator begin() const { return data; }
    const_iterator end() const { return data + sz; }

private:
    T* data;
    size_t sz, cap;
};

#endif