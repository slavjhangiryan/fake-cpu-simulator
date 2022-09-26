#include "register.hpp"
#include <iostream>

template <typename T>
Register<T>::Register() {
    this->name = "";
    this->value = T{};
}

template <typename T>
Register<T>::Register(const std::string& registerName, T registerValue) {
    this->name = registerName;
    this->value = registerValue;
}

template <typename T>
Register<T>::Register(const std::string& registerName) {
    this->name = registerName;
    this->value = T{};
}

template <typename T>
T Register<T>::read() {
    return value;
}

template <typename T>
void Register<T>::write(T newValue) {
    this->value = newValue;
}
