#pragma once
#include <string>

template<typename T>
class Deserializer {
public:
    T operator()(const std::string& str) const;
};

template<> int Deserializer<int>::operator()(const std::string& str) const;
template<> double Deserializer<double>::operator()(const std::string& str) const;
template<> std::string Deserializer<std::string>::operator()(const std::string& str) const;