#pragma once
#include <iostream>
class membuf : public std::basic_streambuf<char> {
public:
    membuf(const char* p, size_t l) {
        setg((char*)p, (char*)p, (char*)p + l);
    }
};
class memstream : public std::istream {
public:
    memstream(const char* p, size_t l) :
        std::istream(&_buffer),
        _buffer(p, l) {
        rdbuf(&_buffer);
    }

private:
    membuf _buffer;
};