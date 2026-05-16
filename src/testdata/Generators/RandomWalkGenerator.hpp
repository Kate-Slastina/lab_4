#pragma once
#include <random>
#include <functional>

template<typename T = double>
class RandomWalkGenerator {
private:
    T current_;
    std::function<double()> stepGenerator_;
    mutable std::mt19937 rng_;
    size_t index_;

public:
    RandomWalkGenerator(T startValue, std::unique_ptr<std::normal_distribution<double>> stepDist = nullptr, unsigned seed = std::random_device{}())
        : current_(startValue), rng_(seed), index_(0)
    {
        if (!stepDist) {
            stepGenerator_ = [dist = std::normal_distribution<double>(0.0, 1.0), rng = rng_]() mutable { return dist(rng); };
        } else {
            stepGenerator_ = [dist = std::move(*stepDist), rng = rng_]() mutable { return dist(rng); };
        }
    }

    RandomWalkGenerator(T startValue, std::function<double()> stepGen, unsigned seed = std::random_device{}())
        : current_(startValue), stepGenerator_(stepGen), rng_(seed), index_(0) {}

    T next() {
        if (index_ == 0) {
            ++index_;
            return current_;
        }
        double step = stepGenerator_();
        current_ += static_cast<T>(step);
        ++index_;
        return current_;
    }

    void reset(T newStartValue) {
        current_ = newStartValue;
        index_ = 0;
    }

    T operator()() { return next(); }
    size_t index() const { return index_; }
};