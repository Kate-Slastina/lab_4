#pragma once
#include <stdexcept>
#include <string>

class LabException : public std::runtime_error {
public:
    explicit LabException(const std::string& msg) : std::runtime_error(msg) {}
};

class IndexOutOfRange : public LabException {
public:
    explicit IndexOutOfRange(const std::string& msg = "Index out of range") : LabException(msg) {}
};

class EndOfSequence : public LabException {
public:
    explicit EndOfSequence(const std::string& msg = "End of sequence reached") : LabException(msg) {}
};

class EmptySequence : public LabException {
public:
    explicit EmptySequence(const std::string& msg = "Sequence is empty") : LabException(msg) {}
};

class InfiniteSequence : public LabException {
public:
    explicit InfiniteSequence(const std::string& msg = "Operation not supported on infinite sequence") : LabException(msg) {}
};