#pragma once
#include <functional>
#include <vector>
#include <memory>

template<typename T = double>
class CompositeGenerator {
private:
    std::vector<std::function<T()>> generators_;
    std::function<T(T, T)> combiner_;
    size_t index_;

public:
    CompositeGenerator(std::vector<std::function<T()>> gens, std::function<T(T,T)> combine = [](T a, T b) { return a + b; })
        : generators_(std::move(gens)), combiner_(combine), index_(0) {}

    T next() {
        if (generators_.empty()) return T(0);
        T result = generators_[0]();
        for (size_t i = 1; i < generators_.size(); ++i) {
            result = combiner_(result, generators_[i]());
        }
        ++index_;
        return result;
    }

    T operator()() { return next(); }
    void reset() { index_ = 0; }
    size_t index() const { return index_; }
};