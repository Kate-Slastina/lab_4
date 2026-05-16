#pragma once
#include "IGenerator.hpp"
#include <memory>

template<typename T>
class SkipGenerator : public IGenerator<T> {
    std::shared_ptr<IGenerator<T>> base;
    size_t skipCount;
    size_t skipped;
public:
    SkipGenerator(std::shared_ptr<IGenerator<T>> b, size_t count)
        : base(b), skipCount(count), skipped(0) {}
    T GetNext() override {
        while (skipped < skipCount && base->HasNext()) {
            base->GetNext();
            ++skipped;
        }
        if (!base->HasNext()) throw EndOfSequence();
        return base->GetNext();
    }
    bool HasNext() const override {
        if (skipped < skipCount && !base->HasNext()) return false;
        return base->HasNext();
    }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(GetNext()); } catch (...) { return Optional<T>(); }
    }
};