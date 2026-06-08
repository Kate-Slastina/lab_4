#include "Deserializer.hpp"
#include <sstream>
#include <stdexcept>

template<>
int Deserializer<int>::operator()(const std::string& str) const {
    std::istringstream iss(str);
    int value;
    if (!(iss >> value))
        throw std::runtime_error("Failed to deserialize int from: " + str);
    return value;
}

template<>
double Deserializer<double>::operator()(const std::string& str) const {
    std::istringstream iss(str);
    double value;
    if (!(iss >> value))
        throw std::runtime_error("Failed to deserialize double from: " + str);
    return value;
}

template<>
std::string Deserializer<std::string>::operator()(const std::string& str) const {
    return str;
}