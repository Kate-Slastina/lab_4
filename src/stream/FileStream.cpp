#include "FileStream.hpp"
#include "core/Exceptions.hpp"
#include <sstream>

template<typename T>
FileStream<T>::FileStream(const std::string& filename, Deserializer<T> deser, bool seekable)
    : ReadOnlyStream<T>(), filename_(filename), deserializer_(deser),
      position_(0), fileSizeElements_(0), seekable_(seekable) {
}

template<typename T>
void FileStream<T>::Open() {
    if (file_.is_open()) return;
    file_.open(filename_);
    if (!file_.is_open())
        throw LabException("Cannot open file: " + filename_);

    if (seekable_) {
        std::streampos original = file_.tellg();
        file_.seekg(0, std::ios::end);
        std::streamsize totalBytes = file_.tellg();
        file_.seekg(original);
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
    file_.clear();
    file_.seekg(0);
    position_ = 0;
    for (size_t i = 0; i < index; ++i) {
        if (IsEndOfStream()) throw IndexOutOfRange("Seek beyond end of file");
        Read();
    }
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