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
