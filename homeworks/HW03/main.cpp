#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#endif /* __PROGTEST__ */

class CPatchStr
{
public:
    CPatchStr();
    CPatchStr(const char *str);
    // copy constructor
    CPatchStr(const CPatchStr& src);
    // destructor
    ~CPatchStr();
    // operator =
    CPatchStr& operator=(const CPatchStr& src);
    CPatchStr subStr(size_t from, size_t len) const;
    CPatchStr &append(const CPatchStr &src);

    CPatchStr &insert(size_t pos, const CPatchStr &src);
    CPatchStr &remove(size_t from, size_t len);
    char *toStr() const;

private:
    void realloc();

    char *data;
    size_t len; 
    size_t capacity;
    static const size_t MULTIPLIER = 2;
};

CPatchStr::CPatchStr()
    : data(nullptr)
    , len(0)
    , capacity(100)
{
}

CPatchStr::CPatchStr(const char *str) {
    len = strlen(str);
    capacity = len + 1;
    data = new char[capacity];
    strcpy(data, str);
}

CPatchStr::CPatchStr(const CPatchStr &src)
    : data(new char[src.capacity])
    , len(src.len)
    , capacity(src.len + 1)
{
    strncpy(data, src.data, src.len);
}

CPatchStr::~CPatchStr() {
    delete [] data;
}

CPatchStr& CPatchStr::operator=(const CPatchStr &src) {
    if (&src == this) return *this;
    delete [] data;
    len = src.len;
    capacity = src.len + 1;
    data = new char[capacity];
    if (src.data != nullptr)
        strncpy(data, src.data, src.len);
    return *this;
}

CPatchStr CPatchStr::subStr(size_t from, size_t len) const {
    if (from + len > this->len) throw std::out_of_range("Index out of range!");
    char *substring = new char[len + 1];
    for (size_t i = from; i < from + len; i++) substring[i - from] = data[i];
    substring[len] = '\0';
    CPatchStr sub = substring;
    delete [] substring;
    return sub;
}

CPatchStr &CPatchStr::append(const CPatchStr &src) {
    if (capacity <= len + src.len) realloc();
    for (size_t i = len; i < len + src.len; i++) data[i] = src.data[i-len];
    len += src.len;
    return *this;
}

CPatchStr &CPatchStr::insert(size_t pos, const CPatchStr &src) {
    if (pos > len) throw std::out_of_range("Index out of range!");
    if (capacity <= len + src.len) realloc();
    memmove(data + pos + src.len, data + pos, len - pos + 1); 
    memcpy(data + pos, src.data, src.len); 
    len += src.len;
    return *this;
}

CPatchStr &CPatchStr::remove(size_t from, size_t len) {
    if (from + len > this->len) throw std::out_of_range("Index out of range!");
    memmove(data + from, data + from + len, this->len - from - len + 1); 
    this->len -= len;
    return *this;
}

char *CPatchStr::toStr() const {
    char *str = new char[len+1];
    strncpy(str, data, len);
    str[len] = '\0';
    return str;
}

void CPatchStr::realloc() {
    size_t newCapacity = 2*capacity + 100;
    char* new_data = new char[newCapacity];
    for (size_t i = 0; i < len; i++) new_data[i] = data[i];
    delete[] data;
    data = new_data;
    capacity = newCapacity;
} 

#ifndef __PROGTEST__
bool stringMatch(char *str,
                 const char *expected)
{
    bool res = std::strcmp(str, expected) == 0;
    delete[] str;
    return res;
}

int main()
{
    char tmpStr[100];

    CPatchStr a("test");
    assert(stringMatch(a.toStr(), "test"));
    std::strncpy(tmpStr, " da", sizeof(tmpStr) - 1);
    a.append(tmpStr);
    assert(stringMatch(a.toStr(), "test da"));
    std::strncpy(tmpStr, "ta", sizeof(tmpStr) - 1);
    a.append(tmpStr);
    assert(stringMatch(a.toStr(), "test data"));
    std::strncpy(tmpStr, "foo text", sizeof(tmpStr) - 1);
    CPatchStr b(tmpStr);
    assert(stringMatch(b.toStr(), "foo text"));
    CPatchStr c(a);
    assert(stringMatch(c.toStr(), "test data"));
    CPatchStr d(a.subStr(3, 5));
    assert(stringMatch(d.toStr(), "t dat"));
    d.append(b);
    assert(stringMatch(d.toStr(), "t datfoo text"));
    d.append(b.subStr(3, 4));
    assert(stringMatch(d.toStr(), "t datfoo text tex"));
    c.append(d);
    assert(stringMatch(c.toStr(), "test datat datfoo text tex"));
    c.append(c);
    assert(stringMatch(c.toStr(), "test datat datfoo text textest datat datfoo text tex"));
    d.insert(2, c.subStr(6, 9));
    assert(stringMatch(d.toStr(), "t atat datfdatfoo text tex"));
    b = "abcdefgh";  
    assert(stringMatch(b.toStr(), "abcdefgh"));
    assert(stringMatch(d.toStr(), "t atat datfdatfoo text tex"));
    assert(stringMatch(d.subStr(4, 8).toStr(), "at datfd"));
    assert(stringMatch(b.subStr(2, 6).toStr(), "cdefgh"));
    try
    {
        b.subStr(2, 7).toStr();
        assert("Exception not thrown" == nullptr);
    }
    catch (const std::out_of_range &e)
    {
    }
    catch (...)
    {
        assert("Invalid exception thrown" == nullptr);
    }
    a.remove(3, 5);
    assert(stringMatch(a.toStr(), "tesa"));
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */