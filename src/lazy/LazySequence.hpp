#pragma once
#include "../my/my_array.hpp"
#include "../my/my_shared_ptr.hpp"
#include "../my/my_function.hpp"
#include "IGenerator.hpp"
#include "core/Exceptions.hpp"

// -------------------------- Базовые генераторы (без изменений) --------------------------
template<typename T>
class EmptyGenerator : public IGenerator<T> {
public:
    T GetNext() override { throw EndOfSequence(); }
    bool HasNext() const override { return false; }
    Optional<T> TryGetNext() override { return Optional<T>(); }
};

template<typename T>
class FunctionGenerator : public IGenerator<T> {
    Function<T()> func_;
public:
    FunctionGenerator(Function<T()> func) : func_(func) {}
    T GetNext() override { return func_(); }
    bool HasNext() const override { return true; }
    Optional<T> TryGetNext() override {
        try { return Optional<T>(func_()); }
        catch (...) { return Optional<T>(); }
    }
};

template<typename T>
class FiniteFunctionGenerator : public IGenerator<T> {
    Function<T()> func_;
    size_t remaining_;
public:
    FiniteFunctionGenerator(Function<T()> func, size_t count)
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
    Array<T> data_;
    size_t index_;
public:
    ArrayGenerator(const T* items, size_t count) : data_(items, items + count), index_(0) {}
    T GetNext() override {
        if (index_ >= data_.size()) throw EndOfSequence();
        return data_[index_++];
    }
    bool HasNext() const override { return index_ < data_.size(); }
    Optional<T> TryGetNext() override {
        if (!HasNext()) return Optional<T>();
        return Optional<T>(data_[index_++]);
    }
};

template<typename T>
class AppendGenerator : public IGenerator<T> {
    SharedPtr<IGenerator<T>> base_;
    T value_;
    bool baseExhausted_;
    bool valueReturned_;
public:
    AppendGenerator(SharedPtr<IGenerator<T>> base, const T& value)
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
    SharedPtr<IGenerator<T>> base_;
public:
    PrependGenerator(SharedPtr<IGenerator<T>> base, const T& value)
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
    SharedPtr<IGenerator<T>> base_;
    size_t insertIndex_;
    T value_;
    size_t currentIndex_;
    bool inserted_;
public:
    InsertGenerator(SharedPtr<IGenerator<T>> base, size_t index, const T& value)
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
    SharedPtr<IGenerator<T>> base_;
    size_t skipStart_;
    size_t skipEnd_;
    size_t currentIndex_;
    bool skippingDone_;
public:
    SkipGenerator(SharedPtr<IGenerator<T>> base, size_t start, size_t end)
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
    SharedPtr<IGenerator<T>> first_;
    SharedPtr<IGenerator<T>> second_;
    bool firstDone_;
public:
    ConcatGenerator(SharedPtr<IGenerator<T>> first, SharedPtr<IGenerator<T>> second)
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
    SharedPtr<IGenerator<T>> base_;
    Function<U(T)> mapper_;
public:
    MapGenerator(SharedPtr<IGenerator<T>> base, Function<U(T)> mapper)
        : base_(base), mapper_(mapper) {}
    U GetNext() override { return mapper_(base_->GetNext()); }
    bool HasNext() const override { return base_->HasNext(); }
    Optional<U> TryGetNext() override {
        auto opt = base_->TryGetNext();
        if (opt.HasValue()) return Optional<U>(mapper_(opt.GetValue()));
        return Optional<U>();
    }
};

// Исправленный FilterGenerator с ограничением попыток
template<typename T>
class FilterGenerator : public IGenerator<T> {
    SharedPtr<IGenerator<T>> base_;
    Function<bool(T)> predicate_;
    mutable Optional<T> cache_;
    mutable size_t attempts_;
    static constexpr size_t MAX_ATTEMPTS = 10'000'000;

    void fillCache() const {
        if (cache_.HasValue()) return;
        attempts_ = 0;
        while (base_->HasNext()) {
            T val = base_->GetNext();
            ++attempts_;
            if (predicate_(val)) {
                cache_ = Optional<T>(val);
                return;
            }
            if (attempts_ >= MAX_ATTEMPTS) {
                throw EndOfSequence("No matching element found after " + std::to_string(MAX_ATTEMPTS) + " attempts");
            }
        }
    }
public:
    FilterGenerator(SharedPtr<IGenerator<T>> base, Function<bool(T)> pred)
        : base_(base), predicate_(pred), attempts_(0) {}
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

// -------------------------- LazySequence (с поддержкой фабрики) --------------------------

template<typename T>
class LazySequence {
private:
    mutable Array<T> cache_;
    SharedPtr<IGenerator<T>> generator_;
    bool isFinite_;
    size_t knownLength_;
    Function<T()> factory_;   // для бесконечных последовательностей – исходная функция
    static constexpr size_t MAX_CACHE_SIZE = 1000000;

public:
    // Пустая последовательность
    LazySequence()
        : generator_(SharedPtr<IGenerator<T>>(new EmptyGenerator<T>())),
          isFinite_(true), knownLength_(0) {}

    // Конечная последовательность из массива
    LazySequence(const T* items, size_t count)
        : cache_(items, items + count),
          generator_(SharedPtr<IGenerator<T>>(new ArrayGenerator<T>(items, count))),
          isFinite_(true), knownLength_(count) {}

    // БЕСКОНЕЧНАЯ последовательность из функции (без длины) – сохраняем фабрику
    LazySequence(Function<T()> func)
        : generator_(SharedPtr<IGenerator<T>>(new FunctionGenerator<T>(func))),
          isFinite_(false), knownLength_(static_cast<size_t>(-1)),
          factory_(func) {}

    // КОНЕЧНАЯ последовательность из функции + длина – фабрику не сохраняем
    LazySequence(Function<T()> func, size_t length)
        : generator_(SharedPtr<IGenerator<T>>(new FiniteFunctionGenerator<T>(func, length))),
          isFinite_(true), knownLength_(length) {}

    // Внутренний конструктор от генератора (используется при операциях)
    LazySequence(SharedPtr<IGenerator<T>> gen, bool finite, size_t length)
        : generator_(gen), isFinite_(finite), knownLength_(length) {}

    T Get(size_t index) {
        if (knownLength_ != static_cast<size_t>(-1) && index >= knownLength_)
            throw IndexOutOfRange();
        if (index < cache_.size())
            return cache_[index];
        if (!isFinite_ && cache_.size() >= MAX_CACHE_SIZE) {
            size_t need = index - cache_.size() + 1;
            for (size_t i = 0; i < need; ++i) {
                if (!generator_->HasNext()) throw IndexOutOfRange();
                T next = generator_->GetNext();
                if (i == need-1) return next;
            }
        }
        while (cache_.size() <= index) {
            if (!generator_->HasNext()) throw IndexOutOfRange();
            cache_.push_back(generator_->GetNext());
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
        if (!isFinite_) throw InfiniteSequence();
        return Get(knownLength_ - 1);
    }

    // --- Операции, которые возвращают новую последовательность ---
    // Для бесконечных последовательностей используем фабрику, чтобы начать с начала

    SharedPtr<LazySequence<T>> Append(const T& item) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Бесконечная из функции: новая последовательность – конкатенация исходной (с начала) и одного элемента
            // Но конкатенация бесконечной и конечной – бесконечная, начинающаяся с исходной. Однако Append не имеет смысла для бесконечной, т.к. элемент никогда не будет достигнут.
            // Поэтому просто возвращаем копию.
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(factory_));
        } else {
            auto newGen = SharedPtr<IGenerator<T>>(new AppendGenerator<T>(generator_, item));
            size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ + 1;
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(newGen, isFinite_, newLen));
        }
    }

    SharedPtr<LazySequence<T>> Prepend(const T& item) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Бесконечная из функции: новая последовательность – Prepend item + исходная бесконечная (с начала)
            auto newGen = SharedPtr<IGenerator<T>>(new PrependGenerator<T>(SharedPtr<IGenerator<T>>(new FunctionGenerator<T>(factory_)), item));
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(newGen, false, static_cast<size_t>(-1)));
        } else {
            auto newGen = SharedPtr<IGenerator<T>>(new PrependGenerator<T>(generator_, item));
            size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ + 1;
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(newGen, isFinite_, newLen));
        }
    }

    SharedPtr<LazySequence<T>> InsertAt(size_t index, const T& item) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Для бесконечной последовательности вставка по индексу – создаём новый генератор на основе фабрики
            auto baseGen = SharedPtr<IGenerator<T>>(new FunctionGenerator<T>(factory_));
            auto newGen = SharedPtr<IGenerator<T>>(new InsertGenerator<T>(baseGen, index, item));
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(newGen, false, static_cast<size_t>(-1)));
        } else {
            if (knownLength_ != static_cast<size_t>(-1) && index > knownLength_)
                throw IndexOutOfRange();
            auto newGen = SharedPtr<IGenerator<T>>(new InsertGenerator<T>(generator_, index, item));
            size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ + 1;
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(newGen, isFinite_, newLen));
        }
    }

    SharedPtr<LazySequence<T>> Concat(const LazySequence<T>* other) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Бесконечная из функции + другая последовательность: результат – бесконечная, начинающаяся с исходной (с начала)
            // Concat бесконечной с любой другой даёт бесконечную, равную исходной
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(factory_));
        } else {
            auto concatGen = SharedPtr<IGenerator<T>>(new ConcatGenerator<T>(generator_, other->generator_));
            bool finite = isFinite_ && other->isFinite_;
            size_t len = finite ? knownLength_ + other->knownLength_ : static_cast<size_t>(-1);
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(concatGen, finite, len));
        }
    }

    SharedPtr<LazySequence<T>> SkipFirst(size_t count) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Бесконечная из функции: создаём генератор, который пропускает count элементов
            struct SkipGen {
                Function<T()> func;
                size_t skip;
                mutable size_t pos = 0;
                T operator()() const {
                    while (pos < skip) { func(); ++pos; }
                    ++pos;
                    return func();
                }
            };
            SkipGen sg{factory_, count};
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(Function<T()>(sg)));
        } else {
            if (count >= knownLength_ && knownLength_ != static_cast<size_t>(-1))
                return SharedPtr<LazySequence<T>>(new LazySequence<T>());
            auto skipGen = SharedPtr<IGenerator<T>>(new SkipGenerator<T>(generator_, count, static_cast<size_t>(-1)));
            size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : knownLength_ - count;
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(skipGen, isFinite_, newLen));
        }
    }

    SharedPtr<LazySequence<T>> Skip(size_t start, size_t end) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Бесконечная из функции: создаём генератор, который выдаёт подпоследовательность [start, end]
            struct RangeGen {
                Function<T()> func;
                size_t start, end;
                mutable size_t pos = 0;
                T operator()() const {
                    while (pos < start) { func(); ++pos; }
                    if (pos > end) throw EndOfSequence();
                    ++pos;
                    return func();
                }
            };
            RangeGen rg{factory_, start, end};
            size_t len = end - start + 1;
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(Function<T()>(rg), len));
        } else {
            if (start > end) throw std::invalid_argument("start must be <= end");
            if (knownLength_ != static_cast<size_t>(-1) && end >= knownLength_)
                throw IndexOutOfRange();
            auto skipGen = SharedPtr<IGenerator<T>>(new SkipGenerator<T>(generator_, start, end));
            size_t newLen = (knownLength_ == static_cast<size_t>(-1)) ? static_cast<size_t>(-1) : end - start + 1;
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(skipGen, isFinite_, newLen));
        }
    }

    template<typename U>
    SharedPtr<LazySequence<U>> Map(Function<U(T)> f) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Бесконечная из функции: создаём новый генератор на основе фабрики и применяем отображение
            auto newGen = SharedPtr<IGenerator<U>>(new MapGenerator<T, U>(SharedPtr<IGenerator<T>>(new FunctionGenerator<T>(factory_)), f));
            return SharedPtr<LazySequence<U>>(new LazySequence<U>(newGen, false, static_cast<size_t>(-1)));
        } else {
            auto mapGen = SharedPtr<IGenerator<U>>(new MapGenerator<T, U>(generator_, f));
            return SharedPtr<LazySequence<U>>(new LazySequence<U>(mapGen, isFinite_, knownLength_));
        }
    }

    T Reduce(Function<T(T,T)> f) {
        if (knownLength_ == 0) throw EmptySequence();
        if (!isFinite_) throw InfiniteSequence();
        T result = Get(0);
        for (size_t i = 1; i < knownLength_; ++i)
            result = f(result, Get(i));
        return result;
    }

    SharedPtr<LazySequence<T>> Where(Function<bool(T)> pred) const {
        if (!isFinite_ && factory_.operator bool()) {
            // Бесконечная из функции: создаём новый FilterGenerator на основе фабрики
            auto baseGen = SharedPtr<IGenerator<T>>(new FunctionGenerator<T>(factory_));
            auto filterGen = SharedPtr<IGenerator<T>>(new FilterGenerator<T>(baseGen, pred));
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(filterGen, false, static_cast<size_t>(-1)));
        } else {
            auto filterGen = SharedPtr<IGenerator<T>>(new FilterGenerator<T>(generator_, pred));
            return SharedPtr<LazySequence<T>>(new LazySequence<T>(filterGen, false, static_cast<size_t>(-1)));
        }
    }

    ~LazySequence() = default;
};