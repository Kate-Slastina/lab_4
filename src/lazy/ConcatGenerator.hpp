#pragma once
#include "IGenerator.hpp"
#include "../my/my_shared_ptr.hpp"

template<typename T>
class ConcatGenerator : public IGenerator<T> {
      SharedPtr<IGenerator<T>> first_;
      SharedPtr<IGenerator<T>> second_;
    bool firstExhausted_;

public:
    ConcatGenerator(  SharedPtr<IGenerator<T>> first,   SharedPtr<IGenerator<T>> second)
        : first_(first), second_(second), firstExhausted_(false) {}

    T GetNext() override {
        if (!firstExhausted_) {
            try {
                return first_->GetNext();
            } catch (const EndOfSequence&) {
                firstExhausted_ = true;
                return second_->GetNext();
            }
        } else {
            return second_->GetNext();
        }
    }

    bool HasNext() const override {
        if (!firstExhausted_) {
            return first_->HasNext() || second_->HasNext();
        } else {
            return second_->HasNext();
        }
    }

    Optional<T> TryGetNext() override {
        if (!firstExhausted_) {
            auto val = first_->TryGetNext();
            if (val.HasValue()) return val;
            firstExhausted_ = true;
        }
        return second_->TryGetNext();
    }
};