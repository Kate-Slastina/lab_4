#pragma once
#include "../my/my_function.hpp" // не нужно, просто заглушка
#include <stdexcept>

namespace my {
    template<typename T> class Optional; // объявление из my_optional (но у нас нет my_optional, определим здесь)
}

// Простейшая реализация Optional (можно взять из предыдущего проекта)
template<typename T>
class Optional {
private:
    bool hasValue_;
    T value_;
public:
    Optional() : hasValue_(false) {}
    Optional(const T& val) : hasValue_(true), value_(val) {}
    Optional(T&& val) : hasValue_(true), value_(std::move(val)) {}

    bool HasValue() const { return hasValue_; }
    T& GetValue() {
        if (!hasValue_) throw std::runtime_error("Optional has no value");
        return value_;
    }
    const T& GetValue() const {
        if (!hasValue_) throw std::runtime_error("Optional has no value");
        return value_;
    }
    T GetValueOrDefault(const T& def) const {
        return hasValue_ ? value_ : def;
    }

    explicit operator bool() const { return hasValue_; }
};