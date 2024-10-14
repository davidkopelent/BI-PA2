#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <array>
#include <cassert>
using namespace std;
#endif /* __PROGTEST__ */

class CTimeStamp
{
public:
    unsigned long long int date;
    CTimeStamp(int year, int month, int day, int hour, int minute, int second)
    {
        date = year * 10000000000 + month * 100000000 + day * 1000000 + hour * 10000 + minute * 100 + second;
    }
};

class CContact
{
public:
    unsigned long long int date;
    int phone_1;
    int phone_2;

    CContact(CTimeStamp stamp, int phone_a, int phone_b)
    {
        date = stamp.date;
        phone_1 = phone_a;
        phone_2 = phone_b;
    }
};

class CEFaceMask
{
private:
    int total;
    struct Contact
    {
        unsigned long long int date;
        int contact_1;
        int contact_2;
    };
    vector<Contact> contacts;

public:
    CEFaceMask()
    {
        total = 0;
    }

    CEFaceMask &addContact(CContact contact)
    {
        Contact tmp;
        tmp.date = contact.date;
        tmp.contact_1 = contact.phone_1;
        tmp.contact_2 = contact.phone_2;
        contacts.push_back(tmp);
        total++;
        return *this;
    }

    bool cached(int contact, vector<int> searched) const
    {
        int size = searched.size();
        for (int i = 0; i < size; i++)
        {
            if (contact == searched[i])
                return false;
        }
        return true;
    }

    bool check_dates(unsigned long long int date, unsigned long long int date_min, unsigned long long int date_max) const
    {
        return (date >= date_min && date <= date_max);
    }

    vector<int> get_results(int phone, bool date, unsigned long long int stamp_1 = 0, unsigned long long int stamp_2 = 0) const
    {
        vector<int> results;
        vector<int> searched;
        for (int i = 0; i < total; i++)
        {
            int search = (phone == contacts[i].contact_1) ? contacts[i].contact_2 : 0;
            search = (search == 0 && phone == contacts[i].contact_2) ? contacts[i].contact_1 : search;
            bool check_date = (date) ? check_dates(contacts[i].date, stamp_1, stamp_2) : true;

            if (search != 0 && cached(search, searched) && search != phone && check_date)
            {
                results.push_back(search);
                searched.push_back(search);
            }
        }
        return results;
    }

    vector<int> listContacts(int phone) const
    {
        return get_results(phone, false);
    }

    vector<int> listContacts(int phone, CTimeStamp stamp_1, CTimeStamp stamp_2) const
    {
        return get_results(phone, true, stamp_1.date, stamp_2.date);
    }
};

#ifndef __PROGTEST__
int main()
{
    CEFaceMask test;
    test.addContact(CContact(CTimeStamp(2021, 1, 10, 12, 40, 10), 123456789, 999888777));
    test.addContact(CContact(CTimeStamp(2021, 1, 12, 12, 40, 10), 123456789, 111222333))
        .addContact(CContact(CTimeStamp(2021, 2, 5, 15, 30, 28), 999888777, 555000222));
    test.addContact(CContact(CTimeStamp(2021, 2, 21, 18, 0, 0), 123456789, 999888777));
    test.addContact(CContact(CTimeStamp(2021, 1, 5, 18, 0, 0), 123456789, 456456456));
    test.addContact(CContact(CTimeStamp(2021, 2, 1, 0, 0, 0), 123456789, 123456789));
    assert(test.listContacts(123456789) == (vector<int>{999888777, 111222333, 456456456}));
    assert(test.listContacts(999888777) == (vector<int>{123456789, 555000222}));
    assert(test.listContacts(191919191) == (vector<int>{}));
    assert(test.listContacts(123456789, CTimeStamp(2021, 1, 5, 18, 0, 0), CTimeStamp(2021, 2, 21, 18, 0, 0)) == (vector<int>{999888777, 111222333, 456456456}));
    assert(test.listContacts(123456789, CTimeStamp(2021, 1, 5, 18, 0, 1), CTimeStamp(2021, 2, 21, 17, 59, 59)) == (vector<int>{999888777, 111222333}));
    assert(test.listContacts(123456789, CTimeStamp(2021, 1, 10, 12, 41, 9), CTimeStamp(2021, 2, 21, 17, 59, 59)) == (vector<int>{111222333}));
    return 0;
}
#endif /* __PROGTEST__ */
