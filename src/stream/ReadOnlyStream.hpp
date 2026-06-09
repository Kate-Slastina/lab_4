#pragma once
#include "Stream.hpp"
#include "lazy/LazySequence.hpp"
#include "core/Sequence.hpp"
#include "../my/my_shared_ptr.hpp"

template<typename T>
class ReadOnlyStream : public Stream<T> {
private:
      SharedPtr<LazySequence<T>> source_;
    size_t position_;
    bool isOpen_;

public:
    ReadOnlyStream() : source_(nullptr), position_(0), isOpen_(false) {}

    ReadOnlyStream(  SharedPtr<LazySequence<T>> seq)
        : source_(seq), position_(0), isOpen_(false) {}

    ReadOnlyStream(LazySequence<T>* seq)
        : source_(seq), position_(0), isOpen_(false) {}

    ReadOnlyStream(Sequence<T>* seq)
        : source_(  SharedPtr<LazySequence<T>>(new LazySequence<T>(seq))), position_(0), isOpen_(false) {}

    ReadOnlyStream(const ReadOnlyStream&) = delete;
    ReadOnlyStream& operator=(const ReadOnlyStream&) = delete;

    ReadOnlyStream(ReadOnlyStream&&) = default;
    ReadOnlyStream& operator=(ReadOnlyStream&&) = default;

    bool IsEndOfStream() override {
        if (!isOpen_) return true;
        if (source_->IsFinite() && position_ >= source_->GetLength())
            return true;
        return false;
    }

    T Read() override {
        if (!isOpen_) throw LabException("Stream is not open");
        if (IsEndOfStream()) throw EndOfSequence("End of stream reached");
        T value = source_->Get(position_);
        ++position_;
        return value;
    }

    size_t GetPosition() const override { return position_; }
    bool CanSeek() const override { return true; }

    void Seek(size_t index) override {
        if (!isOpen_) throw LabException("Stream is not open");
        if (source_->IsFinite() && index >= source_->GetLength())
            throw IndexOutOfRange("Seek position out of range");
        position_ = index;
    }

    void Open() override { isOpen_ = true; }

    void Close() override {
        source_.reset();
        isOpen_ = false;
    }

    ~ReadOnlyStream() override { Close(); }
};