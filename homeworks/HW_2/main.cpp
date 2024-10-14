#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>
#include <compare>
#include <stdexcept>
#endif /* __PROGTEST__ */

class CBigInt {
public:
    // constructors
    CBigInt(int number = 0);
    CBigInt(const char *number);
    CBigInt(const std::string &number);

    // copying/assignment/destruction
    CBigInt& operator = (const CBigInt &number);

    // operator +=, any of {CBigInt/int/string}
    CBigInt& operator += (const CBigInt &number);

    // operator *=, any of {CBigInt/int/string}
    CBigInt& operator *= (const CBigInt &number);

    // comparison operators, any combination {CBigInt/int/string} {<,<=,>,>=,==,!=} {CBigInt/int/string}
    friend bool operator < (const CBigInt &l, const CBigInt &r);
    friend bool operator <= (const CBigInt &l, const CBigInt &r);
    friend bool operator > (const CBigInt &l, const CBigInt &r);
    friend bool operator >= (const CBigInt &l, const CBigInt &r);
    friend bool operator == (const CBigInt &l, const CBigInt &r);
    friend bool operator != (const CBigInt &l, const CBigInt &r);

    // operator +, any combination {CBigInt/int/string} + {CBigInt/int/string}
    friend CBigInt operator + (CBigInt l, CBigInt r);
    friend CBigInt operator - (CBigInt l, CBigInt r);

    // operator *, any combination {CBigInt/int/string} * {CBigInt/int/string}
    friend CBigInt operator * (CBigInt l, CBigInt r);

    // output operator <<
    friend std::ostream& operator << (std::ostream& out, const CBigInt &number);

    // input operator >>
    friend std::istream& operator >> (std::istream& in, CBigInt &number);

private:
    void initialize();
    static bool isValidNumber(const std::string &n);
    static bool isGreaterThan(const std::string &l, const std::string &r);

    std::string m_Number;
    char m_Sign;
};

void CBigInt::initialize() {
    if (m_Sign == '-')
        m_Number = m_Number.substr(1);

    if (!CBigInt::isValidNumber(m_Number))
        throw std::invalid_argument("Invalid argument");

    m_Number.erase(0, std::min(m_Number.find_first_not_of('0'), m_Number.size() - 1));

    if (m_Number == "0")
        m_Sign = '+';
}

CBigInt::CBigInt(int number): m_Number(std::to_string(number)) {
    m_Sign = (number < 0) ? '-' : '+';
    initialize();
}

CBigInt::CBigInt(const char *number) : m_Number(number) {
    m_Sign = (number[0] == '-') ? '-' : '+';
    initialize();
}

CBigInt::CBigInt(const std::string &number) : m_Number(number) {
    m_Sign = (number[0] == '-') ? '-' : '+';
    initialize();
}

CBigInt& CBigInt::operator = (const CBigInt &number) {
    m_Number = number.m_Number;
    m_Sign = number.m_Sign;
    return *this;
}

CBigInt& CBigInt::operator += (const CBigInt &number) {
    *this = *this + number;
    return *this;
}

CBigInt& CBigInt::operator *= (const CBigInt &number) {
    *this = *this * number;
    return *this;
}

bool operator < (const CBigInt &l, const CBigInt &r) {
    if (l.m_Sign != r.m_Sign)
        return l.m_Sign == '-' && r.m_Sign == '+';

    if (l.m_Number.length() != r.m_Number.length())
        return (l.m_Sign == '+' && l.m_Number.length() < r.m_Number.length()) ||
               (l.m_Sign == '-' && l.m_Number.length() > r.m_Number.length());

    for (size_t i = 0; i < l.m_Number.length(); i++) {
        if (l.m_Number[i] != r.m_Number[i])
            return (l.m_Sign == '+' && l.m_Number[i] < r.m_Number[i]) ||
                   (l.m_Sign == '-' && l.m_Number[i] > r.m_Number[i]);
    }

    return false;
}

bool operator <= (const CBigInt &l, const CBigInt &r) {
    return l < r || l == r;
}

bool operator > (const CBigInt &l, const CBigInt &r) {
    return r < l;
}

bool operator >= (const CBigInt &l, const CBigInt &r) {
    return r < l || l == r;
}

bool operator == (const CBigInt &l, const CBigInt &r) {
    return !(l < r) && !(r < l);
}

bool operator != (const CBigInt &l, const CBigInt &r) {
    return l < r || r < l;
}

CBigInt operator+(CBigInt l, CBigInt r) {
    if (l.m_Sign != r.m_Sign)
        return l - r;

    if (l.m_Number.length() < r.m_Number.length())
        return r + l;

    int carry = 0;
    std::string result;
    int i = l.m_Number.length() - 1;
    int j = r.m_Number.length() - 1;

    while (i >= 0) {
        int sum = carry;
        if (j >= 0) sum += r.m_Number[j--] - '0';
        sum += l.m_Number[i--] - '0';

        carry = sum / 10;
        result.push_back('0' + sum % 10);
    }

    if (carry)
        result.push_back('0' + carry);

    std::reverse(result.begin(), result.end());

    if (l.m_Sign == '-' && r.m_Sign == '-')
        return "-" + result;
    return result;
}

CBigInt operator-(CBigInt l, CBigInt r) {
    std::string result;
    std::string longer = l.m_Number;
    std::string shorter = r.m_Number;
    char resultSign = l.m_Sign;

    if (CBigInt::isGreaterThan(r.m_Number, l.m_Number)) {
        longer = r.m_Number;
        shorter = l.m_Number;
        resultSign = r.m_Sign;
    }

    int borrow = 0;
    int i = longer.length() - 1;
    int j = shorter.length() - 1;

    while (i >= 0) {
        int diff = (longer[i--] - '0') - borrow;
        if (j >= 0)
            diff -= shorter[j--] - '0';
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.push_back('0' + diff);
    }

    std::reverse(result.begin(), result.end());
    result.erase(0, std::min(result.find_first_not_of('0'), result.size() - 1));
    result = (resultSign == '-' ? "-" : "") + result;
    return result;
}

CBigInt operator*(CBigInt l, CBigInt r) {
    int m = l.m_Number.length() - 1;
    int n = r.m_Number.length() - 1;
    int carry = 0;
    std::string result;

    for (int i=0; i <= m + n; ++i) {
        for (int j = std::max(0, i - n); j <= std::min(i, m); ++j)
            carry += (l.m_Number[m-j] - '0') * (r.m_Number[n-i+j] - '0');

        result += carry % 10 + '0';
        carry /= 10;
    }

    if (carry != 0)
        result += carry % 10 + '0';

    std::reverse(result.begin(), result.end());
    if (l.m_Sign != r.m_Sign)
        return "-" + result;
    return result;
}

std::ostream& operator << (std::ostream& out, const CBigInt &number) {
    std::string sign = (number.m_Sign == '-' && number.m_Number != "0") ? "-" : "";
    return out << sign << number.m_Number;
}

std::istream& operator >> (std::istream& in, CBigInt &number) {
    while (std::isspace(in.peek()))
        in.get();

    std::string result;
    bool status = false;
    char sign = number.m_Sign;

    if (in.peek() == '-') {
        number.m_Sign = '-';
        in.get();
    } else {
        number.m_Sign = '+';
    }

    while (in.peek() == '0')
        in.get();

    while (std::isdigit(in.peek())) {
        result += in.get();
        status = true;
    }

    if (status) {
        number.m_Number = result;
    } else {
        in.setstate(std::ios::failbit);
        number.m_Sign = sign;
    }

    return in;
}

bool CBigInt::isValidNumber(const std::string &n) {
    for (size_t i = 0; i < n.length(); i++) {
        if (!std::isdigit(n[i]))
            return false;
    }
    return true;
}

bool CBigInt::isGreaterThan(const std::string &l, const std::string &r) {
    int leftLength = l.length();
    int rightLength = r.length();

    if (leftLength > rightLength)
        return true;

    if (leftLength < rightLength)
        return false;
    
    if (leftLength == rightLength) {
        for (int i = 0; i < leftLength; i++) {
            if (l[i] != r[i])
                return l[i] > r[i];
        }
    }

    return false;
}

#ifndef __PROGTEST__
static bool equal ( const CBigInt & x, const char val [] )
{
  std::ostringstream oss;
  oss << x;
  return oss . str () == val;
}
static bool equalHex ( const CBigInt & x, const char val [] )
{
  return true; // hex output is needed for bonus tests only
  // std::ostringstream oss;
  // oss << std::hex << x;
  // return oss . str () == val;
}
int main ()
{
    CBigInt a, b;
    std::istringstream is;
    a = 10;
    a += 20;
    assert ( equal ( a, "30" ) );
    a *= 5;
    assert ( equal ( a, "150" ) );
    b = a + 3;
    assert ( equal ( b, "153" ) );
    b = a * 7;
    assert ( equal ( b, "1050" ) );
    assert ( equal ( a, "150" ) );
    assert ( equalHex ( a, "96" ) );

    a = 10;
    a += -20;
    assert ( equal ( a, "-10" ) );
    a *= 5;
    assert ( equal ( a, "-50" ) );
    b = a + 73;
    assert ( equal ( b, "23" ) );
    b = a * -7;
    assert ( equal ( b, "350" ) );
    assert ( equal ( a, "-50" ) );
    assert ( equalHex ( a, "-32" ) );

    a = "12345678901234567890";
    a += "-99999999999999999999";
    assert ( equal ( a, "-87654321098765432109" ) );
    a *= "54321987654321987654";
    assert ( equal ( a, "-4761556948575111126880627366067073182286" ) );
    a *= 0;
    assert ( equal ( a, "0" ) );
    a = 10;
    b = a + "400";
    assert ( equal ( b, "410" ) );
    b = a * "15";
    assert ( equal ( b, "150" ) );
    assert ( equal ( a, "10" ) );
    assert ( equalHex ( a, "a" ) );

    is . clear ();
    is . str ( " 1234" );
    assert ( is >> b );
    assert ( equal ( b, "1234" ) );
    is . clear ();
    is . str ( " 12 34" );
    assert ( is >> b );
    assert ( equal ( b, "12" ) );
    is . clear ();
    is . str ( "999z" );
    assert ( is >> b );
    assert ( equal ( b, "999" ) );
    is . clear ();
    is . str ( "abcd" );
    assert ( ! ( is >> b ) );
    is . clear ();
    is . str ( "- 758" );
    assert ( ! ( is >> b ) );
    a = 42;
    try
    {
        a = "-xyz";
        assert ( "missing an exception" == nullptr );
    }
    catch ( const std::invalid_argument & e )
    {
        assert ( equal ( a, "42" ) );
    }

    a = "73786976294838206464";
    assert ( equal ( a, "73786976294838206464" ) );
    assert ( equalHex ( a, "40000000000000000" ) );
    assert ( a < "1361129467683753853853498429727072845824" );
    assert ( a <= "1361129467683753853853498429727072845824" );
    assert ( ! ( a > "1361129467683753853853498429727072845824" ) );
    assert ( ! ( a >= "1361129467683753853853498429727072845824" ) );
    assert ( ! ( a == "1361129467683753853853498429727072845824" ) );
    assert ( a != "1361129467683753853853498429727072845824" );
    assert ( ! ( a < "73786976294838206464" ) );
    assert ( a <= "73786976294838206464" );
    assert ( ! ( a > "73786976294838206464" ) );
    assert ( a >= "73786976294838206464" );
    assert ( a == "73786976294838206464" );
    assert ( ! ( a != "73786976294838206464" ) );
    assert ( a < "73786976294838206465" );
    assert ( a <= "73786976294838206465" );
    assert ( ! ( a > "73786976294838206465" ) );
    assert ( ! ( a >= "73786976294838206465" ) );
    assert ( ! ( a == "73786976294838206465" ) );
    assert ( a != "73786976294838206465" );
    a = "2147483648";
    assert ( ! ( a < -2147483648 ) );
    assert ( ! ( a <= -2147483648 ) );
    assert ( a > -2147483648 );
    assert ( a >= -2147483648 );
    assert ( ! ( a == -2147483648 ) );
    assert ( a != -2147483648 );
    a = "-12345678";
    assert ( ! ( a < -87654321 ) );
    assert ( ! ( a <= -87654321 ) );
    assert ( a > -87654321 );
    assert ( a >= -87654321 );
    assert ( ! ( a == -87654321 ) );
    assert ( a != -87654321 );

    b = "-0";
    assert ( equal ( b, "0" ) );
    std::istringstream is2;
    is2 . clear ();
    is2 . str ( " 1 2     34" );
    assert ( is2 >> b );
    assert ( equal ( b, "1" ) );
    is2 . clear ();
    is2 . str ( " 00000002" );
    assert ( is2 >> b );
    assert ( equal ( b, "2" ) );

    a = "-0";
    b = "0";
    assert( a >= b);
    assert(a == b);

    a = "0000000";
    b = "-000000001";
    assert(a >= b);
    assert(a != b);
    assert(b <= a);
    assert(b < a + 2);

    assert(a <= b * b);
    assert(a != b);
    assert(b <= a);
    assert(b < a + 2);

    a = b;
    b = "0";
    a *= "2147483648";
    assert(a < b);

    a += "232424244242424242";
    b = "232424244242424242";
    assert(a <= b);

    a = "-11111111111111111111111111";
    b = "-1111111111111111111111111111";
    assert(a >= b);
    assert(a != b);
    assert(b != a);
    assert(b * (-1) * (-1) < a);

    a = "2";
    b = "3";
    assert(a < b);
    assert( a + 1 == b);

    a = "-23";
    b = "-23";
    assert((-1) * a == (-1) * b);
    b += "0";
    assert((-1) * a == (-1) * b);
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
