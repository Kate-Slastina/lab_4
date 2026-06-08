#pragma once

#include <memory>
#include <functional>
#include <vector>
#include "core/Sequence.hpp"
#include "core/ArraySequence.hpp"
#include "core/Optional.hpp"
#include "core/Exceptions.hpp"
#include "IGenerator.hpp"

// -------------------------- Базовые генераторы --------------------------

template<typename T>
class EmptyGenerator : public IGenerator<T> {
public:
    T GetNext() override { throw EndOfSequence(); }
    bool HasNext() const override { return false; }
    Optional<T> TryGetNext() override { return Optional<T>(); }
};

template<typename T>
class FunctionGenerator : public IGenerator<T> {
    std::function<T()> func_;
public:
    FunctionGenerator(std::function<T()> func) : func_(func) {}
    T GetNext() override { return func_(); }
    bool HasNext() const override { return true; }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(func_()); }
        catch (...) { return Optional<T>(); }
    }
};

template<typename T>
class FiniteFunctionGenerator : public IGenerator<T> {
    std::function<T()> func_;
    size_t remaining_;
public:
    FiniteFunctionGenerator(std::function<T()> func, size_t count)
        : func_(func), remaining_(count) {}
    T GetNext() override {
        if (remaining_ == 0) throw EndOfSequence();
        --remaining_;
        return func_();
    }
    bool HasNext() const override { return remaining_ > 0; }
    Optional<T> TryGetNext() override {
        if (!HasNext()) return Optional<T>();
        --remaining_;
        return Optional<T>(func_());
    }
};

template<typename T>
class ArrayGenerator : public IGenerator<T> {
    std::shared_ptr<Sequence<T>> seq_;
    size_t index_;
public:
    ArrayGenerator(std::shared_ptr<Sequence<T>> seq) : seq_(seq), index_(0) {}
    T GetNext() override {
        if (index_ >= seq_->GetLength()) throw EndOfSequence();
        return seq_->Get(index_++);
    }
    bool HasNext() const override { return index_ < seq_->GetLength(); }
    Optional<T> TryGetNext() override {
        if (!HasNext()) return Optional<T>();
        return Optional<T>(seq_->Get(index_++));
    }
};


template<typename T>
class AppendGenerator : public IGenerator<T> {
    std::shared_ptr<IGenerator<T>> base_;
    T value_;
    bool baseExhausted_;
    bool valueReturned_;
public:
    AppendGenerator(std::shared_ptr<IGenerator<T>> base, const T& value)
        : base_(base), value_(value), baseExhausted_(false), valueReturned_(false) {}
    T GetNext() override {
        if (!baseExhausted_) {
            try {
                return base_->GetNext();
            } catch (const EndOfSequence&) {
                baseExhausted_ = true;
            }
        }
        if (!valueReturned_) {
            valueReturned_ = true;
            return value_;
        }
        throw EndOfSequence();
    }
    bool HasNext() const override {
        return (!baseExhausted_ && base_->HasNext()) || !valueReturned_;
    }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(GetNext()); }
        catch (...) { return Optional<T>(); }
    }
};

template<typename T>
class PrependGenerator : public IGenerator<T> {
    T value_;
    bool valueReturned_;
    std::shared_ptr<IGenerator<T>> base_;
public:
    PrependGenerator(std::shared_ptr<IGenerator<T>> base, const T& value)
        : value_(value), valueReturned_(false), base_(base) {}
    T GetNext() override {
        if (!valueReturned_) {
            valueReturned_ = true;
            return value_;
        }
        return base_->GetNext();
    }
    bool HasNext() const override { return !valueReturned_ || base_->HasNext(); }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(GetNext()); }
        catch (...) { return Optional<T>(); }
    }
};

template<typename T>
class InsertGenerator : public IGenerator<T> {
    std::shared_ptr<IGenerator<T>> base_;
    size_t insertIndex_;
    T value_;
    size_t currentIndex_;
    bool inserted_;
public:
    InsertGenerator(std::shared_ptr<IGenerator<T>> base, size_t index, const T& value)
        : base_(base), insertIndex_(index), value_(value), currentIndex_(0), inserted_(false) {}
    T GetNext() override {
        if (!inserted_ && currentIndex_ == insertIndex_) {
            inserted_ = true;
            ++currentIndex_;
            return value_;
        }
        T val = base_->GetNext();
        ++currentIndex_;
        return val;
    }
    bool HasNext() const override { return base_->HasNext() || !inserted_; }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(GetNext()); }
        catch (...) { return Optional<T>(); }
    }
};

template<typename T>
class SkipGenerator : public IGenerator<T> {
    std::shared_ptr<IGenerator<T>> base_;
    size_t skipStart_;
    size_t skipEnd_;
    size_t currentIndex_;
    bool skippingDone_;
public:
    SkipGenerator(std::shared_ptr<IGenerator<T>> base, size_t start, size_t end)
        : base_(base), skipStart_(start), skipEnd_(end), currentIndex_(0), skippingDone_(false) {}
    T GetNext() override {
        if (!skippingDone_) {
            while (currentIndex_ < skipStart_ && base_->HasNext()) {
                base_->GetNext();
                ++currentIndex_;
            }
            skippingDone_ = true;
        }
        if (currentIndex_ > skipEnd_ || !base_->HasNext())
            throw EndOfSequence();
        T val = base_->GetNext();
        ++currentIndex_;
        return val;
    }
    bool HasNext() const override {
        if (!skippingDone_) {
            if (!base_->HasNext()) return false;
            return true;
        }
        if (currentIndex_ > skipEnd_) return false;
        return base_->HasNext();
    }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(GetNext()); }
        catch (...) { return Optional<T>(); }
    }
};

template<typename T>
class ConcatGenerator : public IGenerator<T> {
    std::shared_ptr<IGenerator<T>> first_;
    std::shared_ptr<IGenerator<T>> second_;
    bool firstDone_;
public:
    ConcatGenerator(std::shared_ptr<IGenerator<T>> first, std::shared_ptr<IGenerator<T>> second)
        : first_(first), second_(second), firstDone_(false) {}
    T GetNext() override {
        if (!firstDone_) {
            try {
                return first_->GetNext();
            } catch (const EndOfSequence&) {
                firstDone_ = true;
                return second_->GetNext();
            }
        }
        return second_->GetNext();
    }
    bool HasNext() const override {
        if (!firstDone_) return first_->HasNext() || second_->HasNext();
        return second_->HasNext();
    }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(GetNext()); }
        catch (...) { return Optional<T>(); }
    }
};

template<typename T, typename U>
class MapGenerator : public IGenerator<U> {
    std::shared_ptr<IGenerator<T>> base_;
    std::function<U(T)> mapper_;
public:
    MapGenerator(std::shared_ptr<IGenerator<T>> base, std::function<U(T)> mapper)
        : base_(base), mapper_(mapper) {}
    U GetNext() override { return mapper_(base_->GetNext()); }
    bool HasNext() const override { return base_->HasNext(); }
    Optional<U> TryGetNext() override {
        auto opt = base_->TryGetNext();
        if (opt.HasValue()) return Optional<U>(mapper_(opt.GetValue()));
        return Optional<U>();
    }
};

template<typename T>
class FilterGenerator : public IGenerator<T> {
    std::shared_ptr<IGenerator<T>> base_;
    std::function<bool(T)> predicate_;
    mutable Optional<T> cache_;
    void fillCache() const {
        if (cache_.HasValue()) return;
        while (base_->HasNext()) {
            T val = base_->GetNext();
            if (predicate_(val)) {
                cache_ = Optional<T>(val);
                return;
            }
        }
    }
public:
    FilterGenerator(std::shared_ptr<IGenerator<T>> base, std::function<bool(T)> pred)
        : base_(base), predicate_(pred) {}
    T GetNext() override {
        if (!HasNext()) throw EndOfSequence();
        T res = cache_.GetValue();
        cache_ = Optional<T>();
        return res;
    }
    bool HasNext() const override {
        fillCache();
        return cache_.HasValue();
    }
    Optional<T> TryGetNext() override {
        if (!HasNext()) return Optional<T>();
        T res = cache_.GetValue();
        cache_ = Optional<T>();
        return Optional<T>(res);
    }
};


template<typename T>
class LazySequence {
private:
    std::vector<T> cache_;
    std::shared_ptr<IGenerator<T>> generator_;
    bool isFinite_;
    size_t knownLength_;

public:
    // Пустая последовательность
    LazySequence()
        : generator_(std::make_shared<EmptyGenerator<T>>()),
          isFinite_(true),
          knownLength_(0) {}

    // Из массива
    LazySequence(const T* items, size_t count)
        : cache_(items, items + count),
          generator_(std::make_shared<ArrayGenerator<T>>(
              std::make_shared<ArraySequence<T>>(items, count))),
          isFinite_(true),
          knownLength_(count) {}

    // Из существующей последовательности (копирование)
    explicit LazySequence(Sequence<T>* seq)
        : cache_(),
          generator_(std::make_shared<ArrayGenerator<T>>(
              std::shared_ptr<Sequence<T>>(seq))),
          isFinite_(true),
          knownLength_(seq->GetLength()) {
        for (size_t i = 0; i < knownLength_; ++i)
            cache_.push_back(seq->Get(i));
    }

    explicit LazySequence(std::unique_ptr<Sequence<T>> seq)
        : LazySequence(seq.release()) {}

    // Из генератора
    LazySequence(std::shared_ptr<IGenerator<T>> gen, bool finite = false, size_t length = 0)
        : generator_(gen ? gen : std::make_shared<EmptyGenerator<T>>()),
          isFinite_(finite),
          knownLength_(finite ? length : static_cast<size_t>(-1)) {}

    // Из функции с опциональной длиной (0 = бесконечная)
    LazySequence(std::function<T()> func, size_t length = 0)
        : LazySequence(length > 0
                       ? std::static_pointer_cast<IGenerator<T>>(
                           std::make_shared<FiniteFunctionGenerator<T>>(func, length))
                       : std::static_pointer_cast<IGenerator<T>>(
                           std::make_shared<FunctionGenerator<T>>(func)),
                       length > 0, length) {}

    // Из функции от индекса (бесконечная)
    LazySequence(std::function<T(size_t)> func)
        : LazySequence([func, idx = size_t(0)]() mutable { return func(idx++); }) {}

    // Получение элемента (материализует при необходимости)
    static constexpr size_t MAX_CACHE_SIZE = 1'000'000; // 1млн

T Get(size_t index) {
    if (knownLength_ != static_cast<size_t>(-1) && index >= knownLength_)
        throw IndexOutOfRange();
    if (index < cache_.size())
        return cache_[index];
    // Если последовательность бесконечная, но кэш уже слишком велик,
    // предотвращаем дальнейший рост (генерируем on‑fly без сохранения)
    if (!isFinite_ && cache_.size() >= MAX_CACHE_SIZE) {
        // достраиваем до нужного индекса, не сохраняя в cache_
        size_t need = index - cache_.size() + 1;
        for (size_t i = 0; i < need; ++i) {
            if (!generator_->HasNext())
                throw IndexOutOfRange();
            T next = generator_->GetNext();
            // не добавляем в cache_, если превышен лимит
            if (i == need-1) return next;
        }
    }
    while (cache_.size() <= index) {
        if (!generator_->HasNext())
            throw IndexOutOfRange();
        T next = generator_->GetNext();
        cache_.push_back(next);
    }
    return cache_[index];
}

    size_t GetLength() const { return knownLength_; }
    bool IsFinite() const { return isFinite_; }
    bool IsEmpty() const { return knownLength_ == 0; }

    T GetFirst() {
        if (cache_.empty()) {
            if (generator_->HasNext())
                return Get(0);
            else
                throw EmptySequence();
        }
        return cache_[0];
    }

    T GetLast() {
        if (knownLength_ == 0) throw EmptySequence();
        if (knownLength_ == static_cast<size_t>(-1))
            throw InfiniteSequence();
        return Get(knownLength_ - 1);
    }

    // Операции, возвращающие новые LazySequence
    LazySequence<T>* Append(const T& item) const {
        auto newGen = std::make_shared<AppendGenerator<T>>(generator_, item);
        size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ + 1;
        return new LazySequence<T>(newGen, isFinite_, newLen);
    }

    LazySequence<T>* Prepend(const T& item) const {
        auto newGen = std::make_shared<PrependGenerator<T>>(generator_, item);
        size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ + 1;
        return new LazySequence<T>(newGen, isFinite_, newLen);
    }

    LazySequence<T>* InsertAt(size_t index, const T& item) const {
        if (knownLength_ != static_cast<size_t>(-1) && index > knownLength_)
            throw IndexOutOfRange();
        auto newGen = std::make_shared<InsertGenerator<T>>(generator_, index, item);
        size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ + 1;
        return new LazySequence<T>(newGen, isFinite_, newLen);
    }

    LazySequence<T>* Concat(const LazySequence<T>* other) const {
        auto concatGen = std::make_shared<ConcatGenerator<T>>(generator_, other->generator_);
        bool finite = isFinite_ && other->isFinite_;
        size_t len = finite ? knownLength_ + other->knownLength_ : static_cast<size_t>(-1);
        return new LazySequence<T>(concatGen, finite, len);
    }

    LazySequence<T>* SkipFirst(size_t count) const {
        if (count >= knownLength_ && knownLength_ != static_cast<size_t>(-1))
            return new LazySequence<T>();
        auto skipGen = std::make_shared<SkipGenerator<T>>(generator_, count, static_cast<size_t>(-1));
        size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ - count;
        return new LazySequence<T>(skipGen, isFinite_, newLen);
    }

    LazySequence<T>* Skip(size_t start, size_t end) const {
        if (start > end) throw std::invalid_argument("start must be <= end");
        if (knownLength_ != static_cast<size_t>(-1) && end >= knownLength_)
            throw IndexOutOfRange();
        auto skipGen = std::make_shared<SkipGenerator<T>>(generator_, start, end);
        size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : end - start + 1;
        return new LazySequence<T>(skipGen, isFinite_, newLen);
    }

    template<typename U>
    LazySequence<U>* Map(std::function<U(T)> f) const {
        auto mapGen = std::make_shared<MapGenerator<T, U>>(generator_, f);
        return new LazySequence<U>(mapGen, isFinite_, knownLength_);
    }

    T Reduce(std::function<T(T,T)> f) {
        if (knownLength_ == 0) throw EmptySequence();
        if (knownLength_ == static_cast<size_t>(-1))
            throw InfiniteSequence();
        T result = Get(0);
        for (size_t i = 1; i < knownLength_; ++i)
            result = f(result, Get(i));
        return result;
    }

    LazySequence<T>* Where(std::function<bool(T)> pred) const {
        auto filterGen = std::make_shared<FilterGenerator<T>>(generator_, pred);
        return new LazySequence<T>(filterGen, false, static_cast<size_t>(-1));
    }



    ~LazySequence() = default;
};