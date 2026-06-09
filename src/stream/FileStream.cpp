// Реализации шаблонов уже в .hpp, поэтому cpp может быть пустым, либо с инстанцированием
// Но для избежания проблем линковки оставим явные инстанцирования.

#include "FileStream.hpp"

template class FileStream<int>;
template class FileStream<double>;
template class FileStream<std::string>;