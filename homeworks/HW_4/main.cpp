#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <compare>

class CDate
{
public:
    CDate(int y, int m, int d)
        : m_Y(y)
        , m_M(m)
        , m_D(d)
    {
    }

    std::strong_ordering operator<=>(const CDate &other) const = default;

    friend std::ostream &operator<<(std::ostream &os, const CDate &d)
    {
        return os << d.m_Y << '-' << d.m_M << '-' << d.m_D;
    }

private:
    int m_Y;
    int m_M;
    int m_D;
};

enum class ESortKey
{
    NAME,
    BIRTH_DATE,
    ENROLL_YEAR
};
#endif /* __PROGTEST__ */

class CStudent
{
public:
    explicit CStudent(const std::shared_ptr<CStudent>& ptr);
    CStudent(const std::string &name, const CDate &born, int enrolled);
    bool operator==(const CStudent &other) const;
    bool operator!=(const CStudent &other) const;
    void setId(size_t id);
    static std::string normalizeName(const std::string& name);
    std::string getNameKey() const;
    std::string getName() const;
    CDate getDate() const;
    int getEnrolled() const;
    size_t getId() const;

private:
    std::string m_NameKey;
    std::string m_Name;
    CDate m_Born;
    int m_Enrolled;
    size_t m_Id;
};

CStudent::CStudent(const std::shared_ptr<CStudent>& ptr)
    : m_Name(ptr->getName()), m_Born(ptr->getDate()), m_Enrolled(ptr->getEnrolled()), m_Id(ptr->getId())
{
    m_NameKey = CStudent::normalizeName(ptr->getName());
}

CStudent::CStudent(const std::string &name, const CDate &born, int enrolled)
    : m_Name(name), m_Born(born), m_Enrolled(enrolled) 
{
    m_NameKey = CStudent::normalizeName(name);
}

bool CStudent::operator==(const CStudent &other) const {
    return other.m_Name == m_Name && other.m_Enrolled == m_Enrolled && (other.m_Born <=> m_Born) == std::strong_ordering::equal;
}

bool CStudent::operator!=(const CStudent &other) const {
    return !(*this == other);
}

void CStudent::setId(size_t id) {
    m_Id = id;
}

std::string CStudent::normalizeName(const std::string& name) {
    std::istringstream iss(name);
    std::vector<std::string> words(std::istream_iterator<std::string>{iss},
                                   std::istream_iterator<std::string>());
    std::sort(words.begin(), words.end());

    std::string sorted_str;
    for (auto& word : words) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        sorted_str += word + " ";
    }
        
    return sorted_str;
}

std::string CStudent::getNameKey() const {
    return m_NameKey;
}

std::string CStudent::getName() const {
    return m_Name;
}

CDate CStudent::getDate() const {
    return m_Born;
}

int CStudent::getEnrolled() const {
    return m_Enrolled;
}

size_t CStudent::getId() const {
    return m_Id;
}

class CFilter
{
public:
    CFilter();
    CFilter &name(const std::string &name);
    CFilter &bornBefore(const CDate &date);
    CFilter &bornAfter(const CDate &date);
    CFilter &enrolledBefore(int year);
    CFilter &enrolledAfter(int year);
    std::unordered_set<std::string> getNameFilters() const;
    CDate getDateFilterBefore() const;
    CDate getDateFilterAfter() const;
    int getEnrolledFilterBefore() const;
    int getEnrolledFilterAfter() const;

    bool m_NameFilter;
    bool m_DateFilter;
    bool m_EnrolledFilter;

private:
    std::unordered_set<std::string> m_NameFilters;
    CDate m_DateFiltersBefore;
    CDate m_DateFiltersAfter;
    int m_EnrolledBefore;
    int m_EnrolledAfter;
};

CFilter::CFilter()
    : m_NameFilter(false), m_DateFilter(false), m_EnrolledFilter(false)
    , m_DateFiltersBefore(CDate(0,0,0)), m_DateFiltersAfter(CDate(0,0,0)), m_EnrolledBefore(-1), m_EnrolledAfter(-1)
{

}

CFilter &CFilter::name(const std::string &name) {
    if (m_NameFilters.find(name) == m_NameFilters.end()) {
        m_NameFilters.insert(CStudent::normalizeName(name));
        m_NameFilter = true;
    }
    return *this;
}

CFilter &CFilter::bornBefore(const CDate &date) {
    if (m_DateFiltersBefore == CDate(0,0,0) || date <=> m_DateFiltersBefore == std::strong_ordering::less) {
        m_DateFiltersBefore = date;
        m_DateFilter = true;
    }
    return *this;
}

CFilter &CFilter::bornAfter(const CDate &date) {
    if (m_DateFiltersAfter == CDate(0,0,0) || date <=> m_DateFiltersAfter == std::strong_ordering::greater) {
        m_DateFiltersAfter = date;
        m_DateFilter = true;
    }
    return *this;
}

CFilter &CFilter::enrolledBefore(int year) {
    if (m_EnrolledBefore == 0 || year < m_EnrolledBefore) {
        m_EnrolledBefore = year;
        m_EnrolledFilter = true;
    }
    return *this;
}

CFilter &CFilter::enrolledAfter(int year) {
    if (m_EnrolledAfter == 0 || year > m_EnrolledAfter) {
        m_EnrolledAfter = year;
        m_EnrolledFilter = true;
    }
    return *this;
}

std::unordered_set<std::string> CFilter::getNameFilters() const {
    return m_NameFilters;
}

CDate CFilter::getDateFilterBefore() const {
    return m_DateFiltersBefore;
}

CDate CFilter::getDateFilterAfter() const {
    return m_DateFiltersAfter;
}

int CFilter::getEnrolledFilterBefore() const {
    return m_EnrolledBefore;
}

int CFilter::getEnrolledFilterAfter() const {
    return m_EnrolledAfter;
}

class CSort
{
public:
    CSort();
    CSort &addKey(ESortKey key, bool ascending);
    bool operator() (const CStudent& a, const CStudent& b) const;

private:
    std::vector<std::pair<ESortKey, bool>> m_SortKeys;
};

CSort::CSort() {}

CSort &CSort::addKey(ESortKey key, bool ascending) {
    if (std::find_if(m_SortKeys.begin(), m_SortKeys.end(), [key](const std::pair<ESortKey, bool>& k){return key == k.first;}) == m_SortKeys.end())
        m_SortKeys.push_back({key, ascending});
    return *this;
}

bool CSort::operator() (const CStudent& a, const CStudent& b) const {
    for (size_t i = 0; i < m_SortKeys.size(); i++) { 
    	if (m_SortKeys[i].first == ESortKey::BIRTH_DATE) { 
            CDate dateA = a.getDate();
            CDate dateB = b.getDate();
            auto comparison = dateA <=> dateB;

            if (comparison != 0)
                return (comparison == std::strong_ordering::greater) ^ m_SortKeys[i].second;
        } 
        else if (m_SortKeys[i].first == ESortKey::ENROLL_YEAR) {
            if (!(a.getEnrolled() == b.getEnrolled()))
    			return (a.getEnrolled() > b.getEnrolled()) ^ m_SortKeys[i].second;
        } 
        else if (m_SortKeys[i].first == ESortKey::NAME) {
            std::string x = a.getName();
            std::string y = b.getName();	

            if (!(x == y))
                return (x > y) ^ m_SortKeys[i].second;
        }
    }

    return a.getId() < b.getId(); 
}

class CStudyDept
{
public:
    CStudyDept();
    bool addStudent(const CStudent &x);
    bool delStudent(const CStudent &x);
    std::list<CStudent> search(const CFilter &flt, const CSort &sortOpt) const;
    std::set<std::string> suggest(const std::string &name) const;

private:
    struct CStudentCompare {
        bool operator()(const std::shared_ptr<CStudent>& lhs, const std::shared_ptr<CStudent>& rhs) const {
            if (lhs->getName() != rhs->getName())
                return lhs->getName() < rhs->getName();
            if (lhs->getEnrolled() != rhs->getEnrolled())
                return lhs->getEnrolled() < rhs->getEnrolled();
            return lhs->getDate() <=> rhs->getDate() == std::strong_ordering::less;
        }
    };

    struct CDateCompare {
        bool operator()(const std::shared_ptr<CStudent>& lhs, const std::shared_ptr<CStudent>& rhs) const {
            return lhs->getDate() <=> rhs->getDate() == std::strong_ordering::less;
        }
    };

    static bool containsWord(const std::string& name, const std::string& word);
    static bool dateInRange(const CDate& a, const CDate& b, const CDate& date);
    static bool enrolledInRange(int a, int b, int enrolled);
    std::list<CStudent> filterByName(const CFilter &flt, const CSort &sortOpt) const;
    std::list<CStudent> filterByDate(const CFilter &flt, const CSort &sortOpt) const;
    std::list<CStudent> filterByEnrolled(const CFilter &flt, const CSort &sortOpt) const;

    size_t m_NextVal;
    std::set<std::shared_ptr<CStudent>, CStudentCompare> m_Students; 
    std::unordered_map<std::string, std::set<std::shared_ptr<CStudent>, CStudentCompare>> m_NameIndex; 
    std::map<CDate, std::set<std::shared_ptr<CStudent>, CDateCompare>> m_BirthDateIndex; 
    std::map<int, std::set<std::shared_ptr<CStudent>, CStudentCompare>> m_EnrollYearIndex; 
};

CStudyDept::CStudyDept() 
    : m_NextVal(0)
{
}

bool CStudyDept::addStudent(const CStudent &x) {
    auto student = std::make_shared<CStudent>(x);
    auto it = m_Students.find(student);
    if (it != m_Students.end())
        return false;   

    student->setId(m_NextVal);
    m_Students.insert(student);
    m_NameIndex[student->getNameKey()].insert(student);
    m_BirthDateIndex[student->getDate()].insert(student);
    m_EnrollYearIndex[student->getEnrolled()].insert(student);
    m_NextVal++;
    return true;
}

bool CStudyDept::delStudent(const CStudent &x) {
    auto student = std::make_shared<CStudent>(x);
    auto it = m_Students.find(student);
    if (it == m_Students.end())
        return false;

    m_NameIndex[student->getNameKey()].erase(student);
    m_BirthDateIndex[student->getDate()].erase(student);
    m_EnrollYearIndex[student->getEnrolled()].erase(student);
    m_Students.erase(student);
    return true;
}

bool CStudyDept::dateInRange(const CDate& a, const CDate& b, const CDate& date) {
    if (a != CDate(0,0,0) && b != CDate(0,0,0)) {
        return (date <=> a == std::strong_ordering::greater && date <=> b == std::strong_ordering::less);
    } else if (a != CDate(0,0,0)) {
        return (date <=> a == std::strong_ordering::greater);
    } else if (b != CDate(0,0,0)) {
        return (date <=> b == std::strong_ordering::less);
    }
    return false;
}

bool CStudyDept::enrolledInRange(int a, int b, int enrolled) {
    if (a != -1 && b != -1) {
        return (enrolled > a && enrolled < b);
    } else if (a != -1) {
        return (enrolled > a);
    } else if (b != -1) {
        return (enrolled < b);
    }
    return false;
}

std::list<CStudent> CStudyDept::filterByName(const CFilter &flt, const CSort &sortOpt) const {
    std::list<CStudent> results;

    for (const auto& nameFilter : flt.getNameFilters()) {
        auto it = m_NameIndex.find(CStudent::normalizeName(nameFilter));

        if (it != m_NameIndex.end()) {
            for (const auto &date: it->second) {
                if (flt.m_DateFilter && !flt.m_EnrolledFilter) {
                    if (dateInRange(flt.getDateFilterAfter(), flt.getDateFilterBefore(), date->getDate())) {
                        results.push_back(*date);
                    }
                } else if (flt.m_DateFilter && flt.m_EnrolledFilter) {
                    if (dateInRange(flt.getDateFilterAfter(), flt.getDateFilterBefore(), date->getDate())
                        && enrolledInRange(flt.getEnrolledFilterAfter(), flt.getEnrolledFilterBefore(), date->getEnrolled())) {
                        results.push_back(*date);
                    }
                } else if (!flt.m_DateFilter && flt.m_EnrolledFilter) {
                    if (enrolledInRange(flt.getEnrolledFilterAfter(), flt.getEnrolledFilterBefore(), date->getEnrolled())) {
                        results.push_back(*date);
                    }
                } else {
                    results.push_back(*date);
                }
            }
        }
    }

    if (results.empty()) return results;
    results.sort(sortOpt);
    return results;
}

std::list<CStudent> CStudyDept::filterByDate(const CFilter &flt, const CSort &sortOpt) const {
    std::list<CStudent> results;
    auto it = m_BirthDateIndex.lower_bound(flt.getDateFilterAfter());
    
    while (it != m_BirthDateIndex.end() && it->first <=> flt.getDateFilterBefore() == std::strong_ordering::less) {
        for (const auto &date: it->second) {
            if (flt.m_EnrolledFilter) {
                if (enrolledInRange(flt.getEnrolledFilterAfter(), flt.getEnrolledFilterBefore(), date->getEnrolled())) {
                    results.push_back(*date);
                }
            } else {
                results.push_back(*date);
            }
        }

        ++it;
    }

    if (results.empty()) return results;
    results.sort(sortOpt);
    return results;
}

std::list<CStudent> CStudyDept::filterByEnrolled(const CFilter &flt, const CSort &sortOpt) const {
    std::list<CStudent> results;
    auto it = m_EnrollYearIndex.lower_bound(flt.getEnrolledFilterAfter());

    while (it != m_EnrollYearIndex.end() && it->first < flt.getEnrolledFilterBefore()) {
        for (const auto &enrolled: it->second)
            results.push_back(*enrolled);
        ++it;
    }

    if (results.empty()) return results;
    results.sort(sortOpt);
    return results;
}

std::list<CStudent> CStudyDept::search(const CFilter &flt, const CSort &sortOpt) const {
    bool noFilter = !flt.m_NameFilter && !flt.m_DateFilter && !flt.m_EnrolledFilter;

    if (noFilter) {
        std::list<CStudent> results(m_Students.begin(), m_Students.end());
        results.sort(sortOpt);
        return results;
    }

    // Filter by name
    if (flt.m_NameFilter) {
        return filterByName(flt, sortOpt);
    }

    // Filter by birth date
    if (flt.m_DateFilter) {
        return filterByDate(flt, sortOpt);
    }

    // Filter by enrollment year
    if (flt.m_EnrolledFilter) {
        return filterByEnrolled(flt, sortOpt);
    }

    return std::list<CStudent>{};
}

bool CStudyDept::containsWord(const std::string& name, const std::string& word) {
    std::stringstream ss(name);
    std::string token;
    while (ss >> token) {
        if (token == word)
            return true;
    }
    return false;
}

std::set<std::string> CStudyDept::suggest(const std::string &name) const {
    std::set<std::string> results;
    std::set<std::string> words;
    std::string word;

    for (char ch : name) {
        if (!std::isspace(ch)) {
            word += std::tolower(ch);
        } else if (!word.empty()) {
            words.insert(word);
            word.clear();
        }
    }

    if (!word.empty())
        words.insert(word);

    for (const auto &it: m_Students) {
        std::string n = it->getName();
        std::transform(n.begin(), n.end(), n.begin(), ::tolower);
        bool acceptable = true;

        for (const auto &w: words) {
            if (!containsWord(n, w))
                acceptable = false;
        }

        if (acceptable && results.find(n) == results.end())
            results.insert(it->getName());
    }

    return results;
}

#ifndef __PROGTEST__
int main(void)
{
    CStudyDept x0;
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1980, 4, 11), 2010));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1980, 4, 11), 2010)));
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1980, 4, 11), 2010));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("Peter Peterson", CDate(1980, 4, 11), 2010)));
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1997, 6, 17), 2010));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1997, 6, 17), 2010)));
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1980, 4, 11), 2016));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1980, 4, 11), 2016)));
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1980, 4, 11), 2016));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("Peter Peterson", CDate(1980, 4, 11), 2016)));
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1997, 6, 17), 2010));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("Peter Peterson", CDate(1997, 6, 17), 2010)));
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("James Bond", CDate(1997, 6, 17), 2016));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("James Bond", CDate(1997, 6, 17), 2016)));
    assert(CStudent("James Bond", CDate(1980, 4, 11), 2010) != CStudent("Peter Peterson", CDate(1997, 6, 17), 2016));
    assert(!(CStudent("James Bond", CDate(1980, 4, 11), 2010) == CStudent("Peter Peterson", CDate(1997, 6, 17), 2016)));
    assert(x0.addStudent(CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014)));
    assert(x0.addStudent(CStudent("John Taylor", CDate(1981, 6, 30), 2012)));
    assert(x0.addStudent(CStudent("Peter Taylor", CDate(1982, 2, 23), 2011)));
    assert(x0.addStudent(CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017)));
    assert(x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));
    assert(x0.addStudent(CStudent("James Bond", CDate(1982, 7, 16), 2013)));
    assert(x0.addStudent(CStudent("James Bond", CDate(1981, 8, 16), 2013)));
    assert(x0.addStudent(CStudent("James Bond", CDate(1981, 7, 17), 2013)));
    assert(x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2012)));
    assert(x0.addStudent(CStudent("Bond James", CDate(1981, 7, 16), 2013)));
    assert(x0.search(CFilter(), CSort()) == (std::list<CStudent>{
                                            CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                                            CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                                            CStudent("Peter Taylor", CDate(1982, 2, 23), 2011),
                                            CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                                            CStudent("James Bond", CDate(1981, 7, 16), 2013),
                                            CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                            CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                            CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                            CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                            CStudent("Bond James", CDate(1981, 7, 16), 2013)}));
    assert(x0.search(CFilter(), CSort().addKey(ESortKey::NAME, true)) == (std::list<CStudent>{
                                                                            CStudent("Bond James", CDate(1981, 7, 16), 2013),
                                                                            CStudent("James Bond", CDate(1981, 7, 16), 2013),
                                                                            CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                            CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                            CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                            CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                                                            CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                                                                            CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                                                                            CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                                                                            CStudent("Peter Taylor", CDate(1982, 2, 23), 2011)}));
    assert(x0.search(CFilter(), CSort().addKey(ESortKey::NAME, false)) == (std::list<CStudent>{
                                                                            CStudent("Peter Taylor", CDate(1982, 2, 23), 2011),
                                                                            CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                                                                            CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                                                                            CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                                                                            CStudent("James Bond", CDate(1981, 7, 16), 2013),
                                                                            CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                            CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                            CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                            CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                                                            CStudent("Bond James", CDate(1981, 7, 16), 2013)}));
    assert(x0.search(CFilter(), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(ESortKey::NAME, true)) == (std::list<CStudent>{
                                                                                                                                                    CStudent("Peter John Taylor", CDate(1984, 1, 17), 2017),
                                                                                                                                                    CStudent("John Peter Taylor", CDate(1983, 7, 13), 2014),
                                                                                                                                                    CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                                                                                                    CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                                                                                                    CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                                                                                                    CStudent("Bond James", CDate(1981, 7, 16), 2013),
                                                                                                                                                    CStudent("James Bond", CDate(1981, 7, 16), 2013),
                                                                                                                                                    CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                                                                                                                                    CStudent("John Taylor", CDate(1981, 6, 30), 2012),
                                                                                                                                                    CStudent("Peter Taylor", CDate(1982, 2, 23), 2011)}));
    assert(x0.search(CFilter().name("james bond"), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(ESortKey::NAME, true)) == (std::list<CStudent>{
                                                                                                                                                                        CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                                                                                                                        CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                                                                                                                        CStudent("Bond James", CDate(1981, 7, 16), 2013),
                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 16), 2013),
                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 16), 2012)}));
    assert(x0.search(CFilter().bornAfter(CDate(1980, 4, 11)).bornBefore(CDate(1983, 7, 13)).name("John Taylor").name("james BOND"), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(ESortKey::NAME, true)) == (std::list<CStudent>{
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                                                                                                                                                                                                        CStudent("Bond James", CDate(1981, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                                                                                                                                                                                                                                        CStudent("John Taylor", CDate(1981, 6, 30), 2012)}));
    assert(x0.search(CFilter().name("james"), CSort().addKey(ESortKey::NAME, true)) == (std::list<CStudent>{}));

    assert(!x0.addStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));
    assert(x0.delStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));
    assert(x0.search(CFilter().bornAfter(CDate(1980, 4, 11)).bornBefore(CDate(1983, 7, 13)).name("John Taylor").name("james BOND"), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(ESortKey::NAME, true)) == (std::list<CStudent>{
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                                                                                                                                                                                                        CStudent("Bond James", CDate(1981, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                                                                                                                                                                                                                                        CStudent("John Taylor", CDate(1981, 6, 30), 2012)}));
    

    assert(x0.search(CFilter().bornAfter(CDate(1980, 4, 11)).bornBefore(CDate(1983, 7, 13)).name("John Taylor").name("james BOND"), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(ESortKey::NAME, true)) == (std::list<CStudent>{
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                                                                                                                                                                                                        CStudent("Bond James", CDate(1981, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                                                                                                                                                                                                                                        CStudent("John Taylor", CDate(1981, 6, 30), 2012)}));



    assert(x0.search(CFilter().bornAfter(CDate(1980, 4, 11)).bornBefore(CDate(1983, 7, 13)).name("John Taylor").name("james BOND").name("John Taylor").name("John Taylor").name("John Taylor").name("John Taylor").name("John Taylor").name("John Taylor"), CSort().addKey(ESortKey::ENROLL_YEAR, false).addKey(ESortKey::BIRTH_DATE, false).addKey(ESortKey::NAME, true)) == (std::list<CStudent>{
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1982, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 8, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 17), 2013),
                                                                                                                                                                                                                                                        CStudent("Bond James", CDate(1981, 7, 16), 2013),
                                                                                                                                                                                                                                                        CStudent("James Bond", CDate(1981, 7, 16), 2012),
                                                                                                                                                                                                                                                        CStudent("John Taylor", CDate(1981, 6, 30), 2012)}));
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
    assert(x0.suggest("peter") == (std::set<std::string>{
                                      "John Peter Taylor",
                                      "Peter John Taylor",
                                      "Peter Taylor"}));
    assert(x0.suggest("bond") == (std::set<std::string>{
                                     "Bond James",
                                     "James Bond"}));
    assert(x0.suggest("peter joHn") == (std::set<std::string>{
                                           "John Peter Taylor",
                                           "Peter John Taylor"}));
    assert(x0.suggest("peter joHn bond") == (std::set<std::string>{}));
    assert(x0.suggest("pete") == (std::set<std::string>{}));
    assert(x0.suggest("peter joHn PETER") == (std::set<std::string>{
                                                 "John Peter Taylor",
                                                 "Peter John Taylor"}));
    
    
    assert(!x0.delStudent(CStudent("James Bond", CDate(1981, 7, 16), 2013)));

    CStudyDept x1;
    x1.addStudent(CStudent("David KopeLENT", CDate(2002, 2, 18), 2022));
    assert(x1.search(CFilter().name("KopeLENt DaviD").bornAfter(CDate(2000, 2, 18)).enrolledAfter(2019), CSort()) == std::list<CStudent>{CStudent("David KopeLENT", CDate(2002, 2, 18), 2022)});
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */