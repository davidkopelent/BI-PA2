#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <string>
#include <stdexcept>
using namespace std;

struct CTimeTester;
#endif /* __PROGTEST__ */

class CTime
{
private:
    int m_Hour;
    int m_Minute;
    int m_Second;

public:
    // constructor, destructor
    CTime(int s = 0)
    {
        m_Hour = (s / 3600) % 24;
        m_Minute = (s % 3600) / 60;
        m_Second = s % 60;
    }

    CTime(int h, int m, int s = 0)
    {
        if (h > 23 || h < 0 || m > 59 || m < 0 || s < 0 || s > 59)
            throw std::invalid_argument("Invalid time!");
        m_Hour = h;
        m_Minute = m;
        m_Second = s;
    }

    CTime &operator=(const CTime &t)
    {
        m_Hour = t.m_Hour;
        m_Minute = t.m_Minute;
        m_Second = t.m_Second;
        return *this;
    }

    // arithmetic operators
    CTime &operator+=(const CTime &t)
    {
        *this = *this + t;
        return *this;
    }

    CTime &operator-=(const CTime &t)
    {
        *this = *this - t;
        return *this;
    }

    friend CTime operator+(CTime a, CTime b);
    friend CTime operator-(CTime a, int s);
    friend int operator-(CTime a, CTime b);

    CTime &operator++()
    {
        *this += 1;
        return *this;
    }

    CTime operator++(int)
    {
        CTime tmp(*this);
        *this += 1;
        return tmp;
    }

    CTime &operator--()
    {
        *this -= 1;
        return *this;
    }

    CTime operator--(int)
    {
        CTime tmp(*this);
        *this -= 1;
        return tmp;
    }

    // comparison operators
    friend bool operator<(const CTime &a, const CTime &b);
    friend bool operator<=(const CTime &a, const CTime &b);
    friend bool operator>(const CTime &a, const CTime &b);
    friend bool operator>=(const CTime &a, const CTime &b);
    friend bool operator!=(const CTime &a, const CTime &b);
    friend bool operator==(const CTime &a, const CTime &b);

    // output operator
    friend ostream &operator<<(ostream &out, const CTime &t);

    friend class ::CTimeTester;
};

CTime operator+(CTime a, CTime b)
{
    int totalSeconds = (a.m_Hour * 3600 + a.m_Minute * 60 + a.m_Second) + (b.m_Hour * 3600 + b.m_Minute * 60 + b.m_Second);
    int hour = (totalSeconds / 3600) % 24;
    int minute = (totalSeconds % 3600) / 60;
    int second = totalSeconds % 60;
    return CTime(hour, minute, second);
}

CTime operator-(CTime a, int s)
{
    int totalSeconds = (a.m_Hour * 3600 + a.m_Minute * 60 + a.m_Second) - s;
    int hour = (totalSeconds / 3600) % 24;
    int minute = (totalSeconds % 3600) / 60;
    int second = totalSeconds % 60;
    return CTime(hour, minute, second);
}

int operator-(CTime a, CTime b)
{
    int totalSeconds = (a.m_Hour * 3600 + a.m_Minute * 60 + a.m_Second) - (b.m_Hour * 3600 + b.m_Minute * 60 + b.m_Second);
    return totalSeconds;
}

// comparison operators
bool operator<(const CTime &a, const CTime &b)
{
    return (a.m_Hour * 3600 + a.m_Minute * 60 + a.m_Second) < (b.m_Hour * 3600 + b.m_Minute * 60 + b.m_Second);
}

bool operator<=(const CTime &a, const CTime &b)
{
    return a < b || a == b;
}

bool operator>(const CTime &a, const CTime &b)
{
    return b < a;
}

bool operator>=(const CTime &a, const CTime &b)
{
    return b < a || b == a;
}

bool operator!=(const CTime &a, const CTime &b)
{
    return a < b || b < a;
}

bool operator==(const CTime &a, const CTime &b)
{
    return !(a < b) && !(b < a);
}

// output operator
ostream &operator<<(ostream &out, const CTime &t)
{
    return out << t.m_Hour << (t.m_Hour <= 9 ? "0:" : ":") << t.m_Minute << (t.m_Minute <= 9 ? "0:" : ":") << t.m_Second << (t.m_Second <= 9 ? "0" : "");
}

#ifndef __PROGTEST__
struct CTimeTester
{
    static bool test(const CTime &time, int hour, int minute, int second)
    {
        return time.m_Hour == hour && time.m_Minute == minute && time.m_Second == second;
    }
};

#include <cassert>
#include <sstream>

int main()
{
    CTime a{12, 30};
    assert(CTimeTester::test(a, 12, 30, 0));

    CTime b{13, 30};
    assert(CTimeTester::test(b, 13, 30, 0));

    assert(b - a == 3600);

    assert(CTimeTester::test(a + 60, 12, 31, 0));
    assert(CTimeTester::test(a - 60, 12, 29, 0));

    assert(a < b);
    assert(a <= b);
    assert(a != b);
    assert(!(a > b));
    assert(!(a >= b));
    assert(!(a == b));

    while (++a != b);
    assert(a == b);

    std::ostringstream output;
    assert(static_cast<bool>(output << a));
    assert(output.str() == "13:30:00");

    assert(a++ == b++);
    assert(a == b);
    assert(--a == --b);
    assert(a == b);

    assert(a-- == b--);
    assert(a == b);

    return 0;
}
#endif /* __PROGTEST__ */