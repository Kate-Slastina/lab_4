#ifndef MY_RANDOM_HPP
#define MY_RANDOM_HPP

#include <cstdint>

class LCG {
public:
    using result_type = uint32_t;
    LCG(result_type seed = 42) : state(seed) {}
    void seed(result_type s) { state = s; }
    result_type operator()() {
        state = state * 1103515245 + 12345;
        return (state >> 16) & 0x7FFF;
    }
    double nextDouble() {
        return static_cast<double>((*this)()) / 32768.0;
    }
private:
    result_type state;
};

#endif