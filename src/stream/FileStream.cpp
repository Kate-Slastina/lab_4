#include "FileStream.hpp"
#include "core/Exceptions.hpp"
#include <sstream>

template<typename T>
FileStream<T>::FileStream(const std::string& filename, Deserializer<T> deser, bool seekable)
    : ReadOnlyStream<T>(), filename_(filename), deserializer_(deser),
      position_(0), fileSizeElements_(0), seekable_(seekable) {
}

template<typename T>
void FileStream<T>::BuildIndex() {
    if (!seekable_) return;
    linePositions_.clear();
    file_.clear();
    file_.seekg(0);
    std::streampos pos = file_.tellg();
    linePositions_.push_back(pos);
    std::string dummy;
    while (std::getline(file_, dummy)) {
        pos = file_.tellg();
        linePositions_.push_back(pos);
    }
    file_.clear();
    file_.seekg(0);
    if (!linePositions_.empty())
        fileSizeElements_ = linePositions_.size() - 1;
    else
        fileSizeElements_ = 0;
}

template<typename T>
void FileStream<T>::Open() {
    if (file_.is_open()) return;
    file_.open(filename_);
    if (!file_.is_open())
        throw LabException("Cannot open file: " + filename_);
    if (seekable_) {
        BuildIndex();   // один раз строим индекс позиций
    } else {
        fileSizeElements_ = 0;
    }
}

template<typename T>
bool FileStream<T>::IsEndOfStream() {
    if (!file_.is_open()) return true;
    return file_.peek() == EOF;
}

template<typename T>
T FileStream<T>::ReadFromFile() {
    std::string line;
    if (!std::getline(file_, line)) {
        throw EndOfSequence();
    }
    return deserializer_(line);
}

template<typename T>
T FileStream<T>::Read() {
    if (!file_.is_open()) throw LabException("FileStream is not open");
    if (IsEndOfStream()) throw EndOfSequence();
    T value = ReadFromFile();
    ++position_;
    return value;
}

template<typename T>
void FileStream<T>::Seek(size_t index) {
    if (!seekable_ || !file_.is_open())
        throw LabException("Seek not supported on this stream");
    if (index >= fileSizeElements_)
        throw IndexOutOfRange("Seek beyond end of file");
    file_.clear();
    file_.seekg(linePositions_[index]);
    position_ = index;
}

template<typename T>
void FileStream<T>::Close() {
    if (file_.is_open())
        file_.close();
    position_ = 0;
}

template<typename T>
FileStream<T>::~FileStream() {
    Close();
}

template class FileStream<int>;
template class FileStream<double>;
template class FileStream<std::string>;