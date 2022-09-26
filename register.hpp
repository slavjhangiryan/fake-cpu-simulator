#ifndef REGISTER_HPP
#define REGISTER_HPP

#include <string>

template <typename T>
class Register {
public:
    Register();
    Register(const std::string& registerName, T registerValue);
    Register(const std::string& registerName);

public:
    T read();
    void write(T newValue);

private:
    std::string name;
    T value;
};

#endif /* REGISTER_HPP */
