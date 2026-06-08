#pragma once
#include "LazySequence.hpp"

inline LazySequence<unsigned long long> FibonacciSequence() {
    return LazySequence<unsigned long long>(
        [prev = 0ULL, curr = 1ULL]() mutable -> unsigned long long {
            unsigned long long next = prev + curr;
            prev = curr;
            curr = next;
            return next;
        }
    );
}