#include <iostream>
#include <cstring>
#include <string>

template<size_t SIZE>
class FixedString {
public:
    FixedString() {
        std::memset(data, 0, SIZE + 1);
    }

    FixedString(const char* str) {
        safeCopy(str);
    }

    FixedString(const std::string& str) {
        safeCopy(str.c_str());
    }

    const char* c_str() const {
        return data;
    }

    size_t length() const {
        return std::strlen(data);
    }

    // Implicit conversion to const char*
    operator const char*() const {
        return data;
    }

    // Implicit conversion to std::string
    operator std::string() const {
        return std::string(data);
    }

    // Assignment from const char*
    FixedString<SIZE>& operator=(const char* str) {
        safeCopy(str);
        return *this;
    }

    // Assignment from std::string
    FixedString<SIZE>& operator=(const std::string& str) {
        safeCopy(str.c_str());
        return *this;
    }

private:
    char data[SIZE + 1];  // +1 for the null terminator

    void safeCopy(const char* str) {
        size_t length = std::strlen(str);
        if (length > SIZE) {
            length = SIZE;
        }
        std::strncpy(data, str, length);
        // Null-terminate the remaining part of the array if necessary
        std::memset(data + length, 0, SIZE + 1 - length);
    }
};
