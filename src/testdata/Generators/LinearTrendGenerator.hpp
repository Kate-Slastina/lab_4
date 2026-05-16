#pragma once
#include <random>
#include <functional>
#include <cmath>

template<typename T = double>
class LinearTrendGenerator {
private:
    double a_;
    double b_;
    double noiseSigma_;
    mutable std::mt19937 rng_;
    mutable std::normal_distribution<double> noiseDist_;
    size_t index_;

public:
    LinearTrendGenerator(double intercept, double slope, double noiseSigma = 0.0, unsigned seed = std::random_device{}())
        : a_(intercept), b_(slope), noiseSigma_(noiseSigma), rng_(seed), noiseDist_(0.0, noiseSigma), index_(0) {}

    T next() {
        double value = a_ + b_ * index_;
        if (noiseSigma_ > 0.0) {
            value += noiseDist_(rng_);
        }
        ++index_;
        return static_cast<T>(value);
    }

    void reset() { index_ = 0; }

    T operator()() { return next(); }

    size_t index() const { return index_; }
};