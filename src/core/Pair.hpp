#pragma once

template<typename T1, typename T2>
struct Pair {
    T1 first;
    T2 second;

    Pair() = default;
    Pair(const T1& f, const T2& s) : first(f), second(s) {}
    Pair(T1&& f, T2&& s) : first(std::move(f)), second(std::move(s)) {}

    bool operator==(const Pair& other) const {
        return first == other.first && second == other.second;
    }

};