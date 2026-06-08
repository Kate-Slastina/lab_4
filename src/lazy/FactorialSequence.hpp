#pragma once
#include "LazySequence.hpp"

inline LazySequence<unsigned long long> FactorialSequence() {
    return LazySequence<unsigned long long>(
        [n = 1ULL, fact = 1ULL]() mutable -> unsigned long long {
            fact *= n;  
            ++n;
            return fact;
        }
    );
}