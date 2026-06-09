#pragma once
#include "IGenerator.hpp"
#include "core/Sequence.hpp"
#include "core/MutableArraySequence.hpp"
#include "../my/my_function.hpp"
#include "../my/my_shared_ptr.hpp"

template<typename T>
class DefaultGenerator : public IGenerator<T> {
private:
      Function<T(const Sequence<T>&)> recurrence_;
      SharedPtr<MutableArraySequence<T>> previous_;
    size_t arity_;
    bool isInfinite_;

public:
    DefaultGenerator(  Function<T(const Sequence<T>&)> recurrence, size_t arity, bool infinite = true)
        : recurrence_(recurrence), arity_(arity), isInfinite_(infinite) {
        previous_ =   SharedPtr<MutableArraySequence<T>>(new MutableArraySequence<T>());
    }

    void SetInitial(const Sequence<T>& init) {
        for (size_t i = 0; i < init.GetLength() && i < arity_; ++i) {
            previous_->AppendInPlace(init.Get(i));
        }
    }

    T GetNext() override {
        if (!HasNext())
            throw EndOfSequence("No more elements in the generator");
        T next = recurrence_(*previous_);
        auto newSeq = static_cast<MutableArraySequence<T>*>(previous_->Append(next));
        previous_.reset(newSeq);
        if (previous_->GetLength() > arity_) {
            auto sub = previous_->GetSubsequence(previous_->GetLength() - arity_, previous_->GetLength() - 1);
            previous_.reset(static_cast<MutableArraySequence<T>*>(sub));
        }
        return next;
    }

    bool HasNext() const override {
        return isInfinite_ || (previous_->GetLength() < arity_);
    }

    Optional<T> TryGetNext() override {
        try { return Optional<T>(GetNext()); }
        catch (...) { return Optional<T>(); }
    }
};