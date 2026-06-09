#ifndef MY_SHARED_PTR_HPP
#define MY_SHARED_PTR_HPP

#include <cstddef>

template<typename T>
class SharedPtr {
public:
    SharedPtr() : ptr(nullptr), refCount(nullptr) {}
    SharedPtr(T* p) : ptr(p), refCount(p ? new size_t(1) : nullptr) {}
    SharedPtr(const SharedPtr& other) : ptr(other.ptr), refCount(other.refCount) {
        if (refCount) ++(*refCount);
    }
    ~SharedPtr() { release(); }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            release();
            ptr = other.ptr;
            refCount = other.refCount;
            if (refCount) ++(*refCount);
        }
        return *this;
    }

    T* get() const { return ptr; }
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    operator bool() const { return ptr != nullptr; }

    void reset(T* p = nullptr) {
        release();
        ptr = p;
        refCount = p ? new size_t(1) : nullptr;
    }

    size_t use_count() const { return refCount ? *refCount : 0; }

private:
    void release() {
        if (refCount && --(*refCount) == 0) {
            delete ptr;
            delete refCount;
        }
        ptr = nullptr;
        refCount = nullptr;
    }

    T* ptr;
    size_t* refCount;
};

#endif