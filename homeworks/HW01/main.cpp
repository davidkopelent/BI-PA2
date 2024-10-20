#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <memory>
#endif /* __PROGTEST__ */

struct CLand
{
    CLand(const std::string &_city,
          const std::string &_addr,
          const std::string &_region,
          unsigned int _id,
          const std::string &_owner) : city(_city),
                                       addr(_addr),
                                       region(_region),
                                       id(_id),
                                       owner(_owner)
    {
    }

    std::string city;
    std::string addr;
    std::string region;
    unsigned int id;
    std::string owner;
};

class CIterator
{
public:
    CIterator(const std::vector<CLand *> &_lands)
    {
        for (const auto &ptr : _lands)
            mData.push_back(*ptr);
        mCurrent = mData.begin();
    }

    bool atEnd() const
    {
        return mCurrent == mData.end();
    }

    void next()
    {
        ++mCurrent;
    }

    std::string city() const
    {
        if (atEnd())
            return "";
        return mCurrent->city;
    }

    std::string addr() const
    {
        if (atEnd())
            return "";
        return mCurrent->addr;
    }

    std::string region() const
    {
        if (atEnd())
            return "";
        return mCurrent->region;
    }

    unsigned id() const
    {
        if (atEnd())
            return 0;
        return mCurrent->id;
    }

    std::string owner() const
    {
        if (atEnd())
            return "";
        return mCurrent->owner;
    }

private:
    std::vector<CLand> mData;
    std::vector<CLand>::const_iterator mCurrent;
};

class CLandRegister
{
public:
    CLandRegister()
    {
        insertNewOwner("");
    }

    bool add(const std::string &city, const std::string &addr, const std::string &region, unsigned int id)
    {
        CLand *land = new CLand(city, addr, region, id, "");
        if (std::binary_search(mLandsByCity.begin(), mLandsByCity.end(), land, compareLandsByCity) ||
            std::binary_search(mLandsByRegion.begin(), mLandsByRegion.end(), land, compareLandsByRegion))
        {
            delete land;
            return false;
        }

        mLandsByCity.insert(std::lower_bound(mLandsByCity.begin(), mLandsByCity.end(), land, compareLandsByCity), land);
        mLandsByRegion.insert(std::lower_bound(mLandsByRegion.begin(), mLandsByRegion.end(), land, compareLandsByRegion), land);
        mLands.push_back(land);
        updateOwnerCount("");
        return true;
    }

    bool del(const std::string &city, const std::string &addr)
    {
        CLand *land = new CLand(city, addr, "", 0, "");
        auto itCity = std::lower_bound(mLandsByCity.begin(), mLandsByCity.end(), land, compareLandsByCity);

        if (itCity >= mLandsByCity.end() || (*itCity)->addr != addr || (*itCity)->city != city)
        {
            delete land;
            return false;
        }

        CLand *toDelete = mLandsByCity.at(itCity - mLandsByCity.begin());
        land->region = (*itCity)->region;
        land->id = (*itCity)->id;
        auto itRegion = std::lower_bound(mLandsByRegion.begin(), mLandsByRegion.end(), land, compareLandsByRegion);

        eraseLand(itCity, itRegion);
        delete toDelete;
        delete land;
        return true;
    }

    bool del(const std::string &region, unsigned int id)
    {
        CLand *land = new CLand("", "", region, id, "");
        auto itRegion = std::lower_bound(mLandsByRegion.begin(), mLandsByRegion.end(), land, compareLandsByRegion);

        if (itRegion >= mLandsByRegion.end() || (*itRegion)->region != region || (*itRegion)->id != id)
        {
            delete land;
            return false;
        }

        CLand *toDelete = mLandsByRegion.at(itRegion - mLandsByRegion.begin());
        land->city = (*itRegion)->city;
        land->addr = (*itRegion)->addr;
        auto itCity = std::lower_bound(mLandsByCity.begin(), mLandsByCity.end(), land, compareLandsByCity);

        eraseLand(itCity, itRegion);
        delete toDelete;
        delete land;
        return true;
    }

    bool getOwner(const std::string &city, const std::string &addr, std::string &owner) const
    {
        CLand *land = new CLand(city, addr, "", 0, "");
        auto it = std::lower_bound(mLandsByCity.begin(), mLandsByCity.end(), land, compareLandsByCity);

        if (it >= mLandsByCity.end() || (*it)->addr != addr || (*it)->city != city)
        {
            delete land;
            return false;
        }

        owner = (*it)->owner;
        delete land;
        return true;
    }

    bool getOwner(const std::string &region, unsigned int id, std::string &owner) const
    {
        CLand *land = new CLand("", "", region, id, "");
        auto it = std::lower_bound(mLandsByRegion.begin(), mLandsByRegion.end(), land, compareLandsByRegion);

        if (it >= mLandsByRegion.end() || (*it)->region != region || (*it)->id != id)
        {
            delete land;
            return false;
        }

        owner = (*it)->owner;
        delete land;
        return true;
    }

    void recalculateOwnedLands(const std::string &a, const std::string &b)
    {
        bool found = false;

        for (size_t i = 0; i < mOwners.size(); i++)
        {
            if (mOwners[i].first == a && a == b)
            {
                mOwners[i].second++;
                found = true;
                break;
            }
            else if (mOwners[i].first == a && a != b)
            {
                mOwners[i].second--;
            }
            else if (mOwners[i].first == b && a != b)
            {
                mOwners[i].second++;
                found = true;
                break;
            }
        }

        if (!found)
            insertNewOwner(b, 1);
    }

    bool newOwner(const std::string &city, const std::string &addr, const std::string &owner)
    {
        CLand *land = new CLand(city, addr, "", 0, "");
        auto it = std::lower_bound(mLandsByCity.begin(), mLandsByCity.end(), land, compareLandsByCity);

        if (it >= mLandsByCity.end() || (*it)->addr != addr || (*it)->city != city || toLowercase((*it)->owner) == toLowercase(owner))
        {
            delete land;
            return false;
        }

        recalculateOwnedLands(toLowercase((*it)->owner), toLowercase(owner));
        for (size_t i = 0; i < mLands.size(); i++)
        {
            if (city == mLands[i]->city && addr == mLands[i]->addr)
            {
                mLands.erase(i + mLands.begin());
                break;
            }
        }

        mLands.push_back(*it);
        (*it)->owner = owner;
        delete land;
        return true;
    }

    bool newOwner(const std::string &region, unsigned int id, const std::string &owner)
    {
        CLand *land = new CLand("", "", region, id, "");
        auto it = std::lower_bound(mLandsByRegion.begin(), mLandsByRegion.end(), land, compareLandsByRegion);

        if (it >= mLandsByRegion.end() || (*it)->region != region || (*it)->id != id || toLowercase((*it)->owner) == toLowercase(owner))
        {
            delete land;
            return false;
        }

        recalculateOwnedLands(toLowercase((*it)->owner), toLowercase(owner));
        for (size_t i = 0; i < mLands.size(); i++)
        {
            if (region == mLands[i]->region && id == mLands[i]->id)
            {
                mLands.erase(i + mLands.begin());
                break;
            }
        }

        mLands.push_back(*it);
        (*it)->owner = owner;
        delete land;
        return true;
    }

    size_t count(const std::string &owner) const
    {
        std::string ownerToFind = toLowercase(owner);
        std::pair<std::string, size_t> _owner{ownerToFind, 1};
        auto it = std::lower_bound(mOwners.begin(), mOwners.end(), _owner, compareOwners);
        if (it >= mOwners.end() || it->first != ownerToFind)
            return 0;
        return it->second;
    }

    CIterator listByAddr() const
    {
        return CIterator(mLandsByCity);
    }

    CIterator listByOwner(const std::string &owner) const
    {
        std::vector<CLand *> ownedLands;
        for (size_t i = 0; i < mLands.size(); i++)
        {
            if (toLowercase(owner) == toLowercase(mLands[i]->owner))
                ownedLands.push_back(mLands[i]);
        }
        return CIterator(ownedLands);
    }

    ~CLandRegister()
    {
        for_each(mLandsByCity.begin(), mLandsByCity.end(), [](CLand *&land)
                 { delete land; });
        mLandsByCity.clear();
        mLandsByRegion.clear();
    }

private:
    void eraseLand(std::vector<CLand *>::const_iterator itCity, std::vector<CLand *>::const_iterator itRegion)
    {
        std::string city = (*itCity)->city;
        std::string addr = (*itCity)->addr;
        updateOwnerCount(toLowercase((*itCity)->owner), -1);
        mLandsByCity.erase(itCity);
        mLandsByRegion.erase(itRegion);
        mLands.erase(mLands.begin() + getLandPosition(city, addr));
    }

    int getLandPosition(const std::string &city, const std::string &addr)
    {
        int position = 0;

        for (size_t i = 0; i < mLands.size(); i++)
        {
            if (mLands[i]->addr == addr && mLands[i]->city == city)
            {
                position = i;
                break;
            }
        }

        return position;
    }

    void insertNewOwner(const std::string &owner, size_t ownedLands = 0)
    {
        std::pair<std::string, size_t> ownerToInsert{owner, ownedLands};
        auto it = std::lower_bound(mOwners.begin(), mOwners.end(), ownerToInsert, compareOwners);
        mOwners.insert(it, ownerToInsert);
    }

    void updateOwnerCount(const std::string &owner, int updateBy = 1)
    {
        std::pair<std::string, size_t> ownerToUpdate{owner, 0};
        auto it = std::lower_bound(mOwners.begin(), mOwners.end(), ownerToUpdate, compareOwners);
        it->second = it->second + updateBy;
    }

    static std::string toLowercase(const std::string &str)
    {
        std::string result;
        for (char c : str)
            result += std::tolower(c);
        return result;
    }

    static bool compareOwners(const std::pair<std::string, size_t> &a, const std::pair<std::string, size_t> &b)
    {
        return toLowercase(a.first) < toLowercase(b.first);
    };

    static bool compareLandsByCity(const CLand *a, const CLand *b)
    {
        int compareResult = a->city.compare(b->city);
        if (compareResult == 0)
            compareResult = a->addr.compare(b->addr);
        return compareResult < 0;
    };

    static bool compareLandsByRegion(const CLand *a, const CLand *b)
    {
        int compareResult = a->region.compare(b->region);
        if (compareResult == 0)
            return a->id < b->id;
        return compareResult < 0;
    };

    std::vector<CLand *> mLandsByCity;
    std::vector<CLand *> mLandsByRegion;
    std::vector<CLand *> mLands;
    std::vector<std::pair<std::string, size_t>> mOwners;
};

#ifndef __PROGTEST__
static void test0()
{
    CLandRegister x;
    std::string owner;

    assert(x.add("Prague", "Thakurova", "Dejvice", 12345));
    assert(x.add("Prague", "Evropska", "Vokovice", 12345));
    assert(x.add("Prague", "Technicka", "Dejvice", 9873));
    assert(x.add("Plzen", "Evropska", "Plzen mesto", 78901));
    assert(x.add("Liberec", "Evropska", "Librec", 4552));
    CIterator i0 = x.listByAddr();
    assert(!i0.atEnd() && i0.city() == "Liberec" && i0.addr() == "Evropska" && i0.region() == "Librec" && i0.id() == 4552 && i0.owner() == "");
    i0.next();
    assert(!i0.atEnd() && i0.city() == "Plzen" && i0.addr() == "Evropska" && i0.region() == "Plzen mesto" && i0.id() == 78901 && i0.owner() == "");
    i0.next();
    assert(!i0.atEnd() && i0.city() == "Prague" && i0.addr() == "Evropska" && i0.region() == "Vokovice" && i0.id() == 12345 && i0.owner() == "");
    i0.next();
    assert(!i0.atEnd() && i0.city() == "Prague" && i0.addr() == "Technicka" && i0.region() == "Dejvice" && i0.id() == 9873 && i0.owner() == "");
    i0.next();
    assert(!i0.atEnd() && i0.city() == "Prague" && i0.addr() == "Thakurova" && i0.region() == "Dejvice" && i0.id() == 12345 && i0.owner() == "");
    i0.next();
    assert(i0.atEnd());

    assert(x.count("") == 5);
    CIterator i1 = x.listByOwner("");
    assert(!i1.atEnd() && i1.city() == "Prague" && i1.addr() == "Thakurova" && i1.region() == "Dejvice" && i1.id() == 12345 && i1.owner() == "");
    i1.next();
    assert(!i1.atEnd() && i1.city() == "Prague" && i1.addr() == "Evropska" && i1.region() == "Vokovice" && i1.id() == 12345 && i1.owner() == "");
    i1.next();
    assert(!i1.atEnd() && i1.city() == "Prague" && i1.addr() == "Technicka" && i1.region() == "Dejvice" && i1.id() == 9873 && i1.owner() == "");
    i1.next();
    assert(!i1.atEnd() && i1.city() == "Plzen" && i1.addr() == "Evropska" && i1.region() == "Plzen mesto" && i1.id() == 78901 && i1.owner() == "");
    i1.next();
    assert(!i1.atEnd() && i1.city() == "Liberec" && i1.addr() == "Evropska" && i1.region() == "Librec" && i1.id() == 4552 && i1.owner() == "");
    i1.next();
    assert(i1.atEnd());

    assert(x.count("CVUT") == 0);
    CIterator i2 = x.listByOwner("CVUT");
    assert(i2.atEnd());

    assert(x.newOwner("Prague", "Thakurova", "CVUT"));
    assert(x.newOwner("Dejvice", 9873, "CVUT"));
    assert(x.newOwner("Plzen", "Evropska", "Anton Hrabis"));
    assert(x.newOwner("Librec", 4552, "Cvut"));
    assert(x.getOwner("Prague", "Thakurova", owner) && owner == "CVUT");
    assert(x.getOwner("Dejvice", 12345, owner) && owner == "CVUT");
    assert(x.getOwner("Prague", "Evropska", owner) && owner == "");
    assert(x.getOwner("Vokovice", 12345, owner) && owner == "");
    assert(x.getOwner("Prague", "Technicka", owner) && owner == "CVUT");
    assert(x.getOwner("Dejvice", 9873, owner) && owner == "CVUT");
    assert(x.getOwner("Plzen", "Evropska", owner) && owner == "Anton Hrabis");
    assert(x.getOwner("Plzen mesto", 78901, owner) && owner == "Anton Hrabis");
    assert(x.getOwner("Liberec", "Evropska", owner) && owner == "Cvut");
    assert(x.getOwner("Librec", 4552, owner) && owner == "Cvut");
    CIterator i3 = x.listByAddr();
    assert(!i3.atEnd() && i3.city() == "Liberec" && i3.addr() == "Evropska" && i3.region() == "Librec" && i3.id() == 4552 && i3.owner() == "Cvut");
    i3.next();
    assert(!i3.atEnd() && i3.city() == "Plzen" && i3.addr() == "Evropska" && i3.region() == "Plzen mesto" && i3.id() == 78901 && i3.owner() == "Anton Hrabis");
    i3.next();
    assert(!i3.atEnd() && i3.city() == "Prague" && i3.addr() == "Evropska" && i3.region() == "Vokovice" && i3.id() == 12345 && i3.owner() == "");
    i3.next();
    assert(!i3.atEnd() && i3.city() == "Prague" && i3.addr() == "Technicka" && i3.region() == "Dejvice" && i3.id() == 9873 && i3.owner() == "CVUT");
    i3.next();
    assert(!i3.atEnd() && i3.city() == "Prague" && i3.addr() == "Thakurova" && i3.region() == "Dejvice" && i3.id() == 12345 && i3.owner() == "CVUT");
    i3.next();
    assert(i3.atEnd());

    assert(x.count("cvut") == 3);
    CIterator i4 = x.listByOwner("cVuT");
    assert(!i4.atEnd() && i4.city() == "Prague" && i4.addr() == "Thakurova" && i4.region() == "Dejvice" && i4.id() == 12345 && i4.owner() == "CVUT");
    i4.next();
    assert(!i4.atEnd() && i4.city() == "Prague" && i4.addr() == "Technicka" && i4.region() == "Dejvice" && i4.id() == 9873 && i4.owner() == "CVUT");
    i4.next();
    assert(!i4.atEnd() && i4.city() == "Liberec" && i4.addr() == "Evropska" && i4.region() == "Librec" && i4.id() == 4552 && i4.owner() == "Cvut");
    i4.next();
    assert(i4.atEnd());

    assert(x.newOwner("Plzen mesto", 78901, "CVut"));
    assert(x.count("CVUT") == 4);
    CIterator i5 = x.listByOwner("CVUT");
    assert(!i5.atEnd() && i5.city() == "Prague" && i5.addr() == "Thakurova" && i5.region() == "Dejvice" && i5.id() == 12345 && i5.owner() == "CVUT");
    i5.next();
    assert(!i5.atEnd() && i5.city() == "Prague" && i5.addr() == "Technicka" && i5.region() == "Dejvice" && i5.id() == 9873 && i5.owner() == "CVUT");
    i5.next();
    assert(!i5.atEnd() && i5.city() == "Liberec" && i5.addr() == "Evropska" && i5.region() == "Librec" && i5.id() == 4552 && i5.owner() == "Cvut");
    i5.next();
    assert(!i5.atEnd() && i5.city() == "Plzen" && i5.addr() == "Evropska" && i5.region() == "Plzen mesto" && i5.id() == 78901 && i5.owner() == "CVut");
    i5.next();
    assert(i5.atEnd());

    assert(x.del("Liberec", "Evropska"));
    assert(x.del("Plzen mesto", 78901));
    assert(x.count("cvut") == 2);
    CIterator i6 = x.listByOwner("cVuT");
    assert(!i6.atEnd() && i6.city() == "Prague" && i6.addr() == "Thakurova" && i6.region() == "Dejvice" && i6.id() == 12345 && i6.owner() == "CVUT");
    i6.next();
    assert(!i6.atEnd() && i6.city() == "Prague" && i6.addr() == "Technicka" && i6.region() == "Dejvice" && i6.id() == 9873 && i6.owner() == "CVUT");
    i6.next();
    assert(i6.atEnd());

    assert(x.add("Liberec", "Evropska", "Librec", 4552));
}

static void test1()
{
    CLandRegister x;
    std::string owner;

    assert(x.add("Prague", "Thakurova", "Dejvice", 12345));
    assert(x.add("Prague", "Evropska", "Vokovice", 12345));
    assert(x.add("Prague", "Technicka", "Dejvice", 9873));
    assert(!x.add("Prague", "Technicka", "Hradcany", 7344));
    assert(!x.add("Brno", "Bozetechova", "Dejvice", 9873));
    assert(!x.getOwner("Prague", "THAKUROVA", owner));
    assert(!x.getOwner("Hradcany", 7343, owner));
    CIterator i0 = x.listByAddr();
    assert(!i0.atEnd() && i0.city() == "Prague" && i0.addr() == "Evropska" && i0.region() == "Vokovice" && i0.id() == 12345 && i0.owner() == "");
    i0.next();
    assert(!i0.atEnd() && i0.city() == "Prague" && i0.addr() == "Technicka" && i0.region() == "Dejvice" && i0.id() == 9873 && i0.owner() == "");
    i0.next();
    assert(!i0.atEnd() && i0.city() == "Prague" && i0.addr() == "Thakurova" && i0.region() == "Dejvice" && i0.id() == 12345 && i0.owner() == "");
    i0.next();
    assert(i0.atEnd());

    assert(x.newOwner("Prague", "Thakurova", "CVUT"));
    assert(!x.newOwner("Prague", "technicka", "CVUT"));
    assert(!x.newOwner("prague", "Technicka", "CVUT"));
    assert(!x.newOwner("dejvice", 9873, "CVUT"));
    assert(!x.newOwner("Dejvice", 9973, "CVUT"));
    assert(!x.newOwner("Dejvice", 12345, "CVUT"));
    assert(x.count("CVUT") == 1);
    CIterator i1 = x.listByOwner("CVUT");
    assert(!i1.atEnd() && i1.city() == "Prague" && i1.addr() == "Thakurova" && i1.region() == "Dejvice" && i1.id() == 12345 && i1.owner() == "CVUT");
    i1.next();
    assert(i1.atEnd());

    assert(!x.del("Brno", "Technicka"));
    assert(!x.del("Karlin", 9873));
    assert(x.del("Prague", "Technicka"));
    assert(!x.del("Prague", "Technicka"));
    assert(!x.del("Dejvice", 9873));
}

int main(void)
{
    test0();
    test1();
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */