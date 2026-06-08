#pragma once
#include "ReadOnlyStream.hpp"
#include "Deserializer.hpp"
#include <fstream>
#include <string>
#include <vector>

template<typename T>
class FileStream : public ReadOnlyStream<T> {
private:
    std::string filename_;
    std::ifstream file_;
    Deserializer<T> deserializer_;
    size_t position_;
    size_t fileSizeElements_;
    bool seekable_;
    std::vector<std::streampos> linePositions_;   // для быстрого Seek

    T ReadFromFile();
    void BuildIndex();

public:
    FileStream(const std::string& filename, Deserializer<T> deser, bool seekable = true);

    bool IsEndOfStream() override;
    T Read() override;
    size_t GetPosition() const override { return position_; }
    bool CanSeek() const override { return seekable_ && file_.is_open(); }
    void Seek(size_t index) override;
    void Open() override;
    void Close() override;

    ~FileStream();
};