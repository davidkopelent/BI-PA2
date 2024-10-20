#ifndef __PROGTEST__
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <array>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <compare>
#include <algorithm>
#include <cassert>
#include <memory>
#include <iterator>
#include <functional>
#include <stdexcept>
using namespace std::literals;

class CDummy {
    public:
        CDummy(char c)
          : m_C(c)
        {
        }
        
        bool operator==(const CDummy &other) const = default;
        friend std::ostream &operator<<(std::ostream &os, const CDummy &x)
        {
            return os << '\'' << x.m_C << '\'';
        }
    private:
        char m_C;
};
#endif /* __PROGTEST__ */

#define TEST_EXTRA_INTERFACE

template <typename T_>
class CSelfMatch {
public:
    // constructor (initializer list)
    CSelfMatch(std::initializer_list<T_> initList)
        : m_Sequence(initList)
    {
        countSubstringOccurences();
    }

    // constructor (a container)
    template<typename Container>
    CSelfMatch(const Container & container)
        : m_Sequence(container.begin(), container.end())
    {
        countSubstringOccurences();
    }

    template<typename Iterator>
    CSelfMatch(Iterator begin, Iterator end)
        : m_Sequence(begin, end) 
    {
        countSubstringOccurences();
    }

    static bool compareBySecond(const std::pair<std::vector<T_>, std::vector<size_t>>& a, 
                        const std::pair<std::vector<T_>, std::vector<size_t>>& b) {
        return a.second.size() < b.second.size();
    }

    void sortOccurencesBySize() {
        std::sort(m_SequenceOccurences.begin(), m_SequenceOccurences.end(), compareBySecond);
    }

    // size_t sequenceLen(n) const
    size_t sequenceLen(size_t n) const {
        if (n == 0)
            throw std::invalid_argument("Invalid value of N (zero).");

        auto it = std::lower_bound(m_SequenceOccurences.begin(), m_SequenceOccurences.end(), n, [](const auto &pair, size_t n) {return pair.second.size() < n;});
        if (it >= m_SequenceOccurences.end())
            return 0;

        size_t max = 0;
        for (auto iter = it; iter != m_SequenceOccurences.end(); ++iter) {
            if (iter->first.size() >= max)
                max = iter->first.size();
        }

        return max;
    }

    // template<size_t N_> std::vector<std::array<size_t, N_>> findSequences() const
    template<size_t N_>
    std::vector<std::array<size_t, N_>> findSequences() const {
        if (N_ == 0)
            throw std::invalid_argument("Invalid value of N (zero).");

        size_t max = 0;
        for (const auto &it: m_SequenceOccurences) {
            if (it.second.size() >= N_ && it.first.size() > max) {
                max = it.first.size();
            }
        }

        std::vector<std::array<size_t, N_>> result;
        for (const auto &it: m_SequenceOccurences) {
            if (it.second.size() >= N_ && it.first.size() == max) {
                std::vector<size_t> values = it.second;
                std::vector<int> combination(N_, 0); 

                int size = values.size();
                int index = 0;
                
                while (index >= 0) {
                    if (combination[index] < size) { 
                        combination[index]++; 
                        if (index == N_ - 1) {
                            std::array<size_t, N_> positions;
                            for (size_t i = 0; i < N_; ++i) {
                                positions[i] = values[combination[i] - 1]; 
                            }
                            result.push_back(positions);
                        } else {
                            index++; 
                            combination[index] = combination[index - 1]; 
                        }
                    } else {
                        index--; 
                    }
                }
            }
        }
        
        return result;
    }
    
    void countSubstringOccurences() {
        for (size_t len = 1; len <= m_Sequence.size(); len++) {    
            for (size_t i = 0; i <= m_Sequence.size() - len; i++) {
                size_t j = i + len - 1;       
                
                std::vector<T_> val;
                for (size_t k = i; k <= j; k++) {
                    val.push_back(m_Sequence[k]);
                }

                bool found = false;
                for (size_t l = 0; l < m_SequenceOccurences.size(); l++) {
                    if (val == m_SequenceOccurences[l].first) {
                        m_SequenceOccurences[l].second.push_back(i);
                        found = true;
                        break;
                    }
                }
                
                if (!found)
                    m_SequenceOccurences.push_back({val, {i}});
            }
        }

        sortOccurencesBySize();
    }

    template <typename... Args> 
    void push_back(Args... args) {
        (m_Sequence.push_back(args), ...);
        m_SequenceOccurences.clear();
        countSubstringOccurences();
    }

private:
    std::vector<T_> m_Sequence;
    std::vector<std::pair<std::vector<T_>, std::vector<size_t>>> m_SequenceOccurences;
};

template<typename Container>
CSelfMatch(const Container & container) -> CSelfMatch<typename Container::value_type>;

template<typename Iterator>
CSelfMatch(Iterator begin, Iterator end) -> CSelfMatch<typename std::iterator_traits<Iterator>::value_type>;

#ifndef __PROGTEST__
template<size_t N_>
bool positionMatch(std::vector<std::array<size_t, N_>> a, std::vector<std::array<size_t, N_>> b) {
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());
    return a == b;
}

int main() {
    CSelfMatch<char> x0 ( "aaaaaaaaaaa"s );
    assert ( x0 . sequenceLen ( 2 ) == 10 );
    assert ( positionMatch ( x0 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 1 } } ) );
    CSelfMatch<char> x1 ( "abababababa"s );
    assert ( x1 . sequenceLen ( 2 ) == 9 );
    assert ( positionMatch ( x1 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 2 } } ) );
    CSelfMatch<char> x2 ( "abababababab"s );
    assert ( x2 . sequenceLen ( 2 ) == 10 );
    assert ( positionMatch ( x2 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 2 } } ) );
    CSelfMatch<char> x3 ( "aaaaaaaaaaa"s );
    assert ( x3 . sequenceLen ( 3 ) == 9 );
    assert ( positionMatch ( x3 . findSequences<3> (), std::vector<std::array<size_t, 3> > { { 0, 1, 2 } } ) );
    CSelfMatch<char> x4 ( "abababababa"s );
    assert ( x4 . sequenceLen ( 3 ) == 7 );
    assert ( positionMatch ( x4 . findSequences<3> (), std::vector<std::array<size_t, 3> > { { 0, 2, 4 } } ) );
    CSelfMatch<char> x5 ( "abababababab"s );
    assert ( x5 . sequenceLen ( 3 ) == 8 );
    assert ( positionMatch ( x5 . findSequences<3> (), std::vector<std::array<size_t, 3> > { { 0, 2, 4 } } ) );
    CSelfMatch<char> x6 ( "abcdXabcd"s );
    assert ( x6 . sequenceLen ( 1 ) == 9 );
    assert ( positionMatch ( x6 . findSequences<1> (), std::vector<std::array<size_t, 1> > { { 0 } } ) );
    CSelfMatch<char> x7 ( "abcdXabcd"s );
    assert ( x7 . sequenceLen ( 2 ) == 4 );
    assert ( positionMatch ( x7 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 5 } } ) );
    CSelfMatch<char> x8 ( "abcdXabcdeYabcdZabcd"s );
    assert ( x8 . sequenceLen ( 2 ) == 4 );
    assert ( positionMatch ( x8 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 5 }, { 0, 11 }, { 0, 16 }, { 5, 11 }, { 5, 16 }, { 11, 16 } } ) );
    CSelfMatch<char> x9 ( "abcdXabcdYabcd"s );
    assert ( x9 . sequenceLen ( 3 ) == 4 );
    assert ( positionMatch ( x9 . findSequences<3> (), std::vector<std::array<size_t, 3> > { { 0, 5, 10 } } ) );
    CSelfMatch<char> x10 ( "abcdefghijklmn"s );
    assert ( x10 . sequenceLen ( 2 ) == 0 );
    assert ( positionMatch ( x10 . findSequences<2> (), std::vector<std::array<size_t, 2> > {  } ) );
    CSelfMatch<char> x11 ( "abcXabcYabcZdefXdef"s );
    assert ( x11 . sequenceLen ( 2 ) == 3 );
    assert ( positionMatch ( x11 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 4 }, { 0, 8 }, { 4, 8 }, { 12, 16 } } ) );
    CSelfMatch<int> x12 { 1, 2, 3, 1, 2, 4, 1, 2 };
    assert ( x12 . sequenceLen ( 2 ) == 2 );
    assert ( positionMatch ( x12 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 3 }, { 0, 6 }, { 3, 6 } } ) );
    assert ( x12 . sequenceLen ( 3 ) == 2 );
    assert ( positionMatch ( x12 . findSequences<3> (), std::vector<std::array<size_t, 3> > { { 0, 3, 6 } } ) );
    std::initializer_list<CDummy> init13 { 'a', 'b', 'c', 'd', 'X', 'a', 'b', 'c', 'd', 'Y', 'a', 'b', 'c', 'd' };
    CSelfMatch<CDummy> x13 ( init13 . begin (), init13 . end () );
    assert ( x13 . sequenceLen ( 2 ) == 4 );
    assert ( positionMatch ( x13 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 0, 5 }, { 0, 10 }, { 5, 10 } } ) );
    std::initializer_list<int> init14 { 1, 2, 1, 1, 2, 1, 0, 0, 1, 2, 1, 0, 1, 2, 0, 1, 2, 0, 1, 1, 1, 2, 0, 2, 0, 1, 2, 1, 0 };
    CSelfMatch<int> x14 ( init14 . begin (), init14 . end () );
    assert ( x14 . sequenceLen ( 2 ) == 5 );
    assert ( positionMatch ( x14 . findSequences<2> (), std::vector<std::array<size_t, 2> > { { 11, 14 }, { 7, 24 } } ) );
    std::initializer_list<int> init15 { 1, 2, 1, 1, 2, 1, 0, 0, 1, 2, 1, 0, 1, 2, 0, 1, 2, 0, 1, 1, 1, 2, 0, 2, 0, 1, 2, 1, 0 };
    CSelfMatch<int> x15 ( init15 . begin (), init15 . end () );
    assert ( x15 . sequenceLen ( 3 ) == 4 );
    assert ( positionMatch ( x15 . findSequences<3> (), std::vector<std::array<size_t, 3> > { { 3, 8, 25 } } ) );

    #ifdef TEST_EXTRA_INTERFACE
    CSelfMatch y0 ( "aaaaaaaaaaa"s );
    assert ( y0 . sequenceLen ( 2 ) == 10 );

    std::string s1 ( "abcd" );
    CSelfMatch y1 ( s1 . begin (), s1 . end () );
    assert ( y1 . sequenceLen ( 2 ) == 0 );

    CSelfMatch y2 ( ""s );
    y2 . push_back ( 'a', 'b', 'c', 'X' );
    y2 . push_back ( 'a' );
    y2 . push_back ( 'b', 'c' );
    assert ( y2 . sequenceLen ( 2 ) == 3 );
    #endif /* TEST_EXTRA_INTERFACE */
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
