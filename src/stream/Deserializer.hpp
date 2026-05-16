#pragma once
#include <string>
#include <sstream>
#include <stdexcept>

template<typename T>
class Deserializer {
public:
    T operator()(const std::string& str) const;
};

template<>
inline int Deserializer<int>::operator()(const std::string& str) const {
    std::istringstream iss(str);
    int value;
    if (!(iss >> value))
        throw std::runtime_error("Failed to deserialize int from: " + str);
    return value;
}

template<>
inline double Deserializer<double>::operator()(const std::string& str) const {
    std::istringstream iss(str);
    double value;
    if (!(iss >> value))
        throw std::runtime_error("Failed to deserialize double from: " + str);
    return value;
}

template<>
inline std::string Deserializer<std::string>::operator()(const std::string& str) const {
    return str;
}