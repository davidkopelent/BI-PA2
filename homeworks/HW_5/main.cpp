#ifndef __PROGTEST__
#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <memory>
#include <functional>
#endif /* __PROGTEST__ */

class CComponent {
public:
    CComponent() 
    {
    }

    virtual CComponent* duplicate() const = 0;
    virtual std::ostream& toString(std::ostream &out, std::pair<std::string, std::string> indents) const = 0;
    virtual ~CComponent() = default;
};

class CComputer {
public:
    CComputer(const std::string &name) 
        : m_Name(name)
    {
    }

    void copyData(const CComputer &computer) {
        m_Name = computer.m_Name;
        m_IPs.clear();
        m_Components.clear();

        for (const auto &ip : computer.m_IPs)
            m_IPs.push_back(ip);

        for (const auto &component : computer.m_Components)
            m_Components.push_back(component->duplicate());
    }

    CComputer(const CComputer &computer) 
    {
        copyData(computer);
    }

    CComputer& operator = (const CComputer &computer) {
        if (&computer == this) return *this;
        copyData(computer);
        return *this;
    }

    CComputer& addComponent(const CComponent &component) {
        m_Components.push_back(component.duplicate());
        return *this;
    }

    CComputer& addAddress(const std::string &addr) {
        m_IPs.push_back(addr);
        return *this;
    }

    std::string getName() const {
        return m_Name;
    }

    std::ostream& toString(std::ostream &out, std::pair<std::string, std::string> indents) const {
        out << indents.second << "Host: " << m_Name << "\n";

        for (const auto &ip: m_IPs) {
            out << indents.first << "+-" << ip << "\n";
        }

        for (const auto &component: m_Components) {
            if (&component == &m_Components.back()) {
                indents.second = "\\-";
            } else {
                indents.second = "+-";
            }

            component->toString(out, indents);
        }

        return out;
    }

    friend std::ostream& operator << (std::ostream &out, const CComputer &computer) {
        return computer.toString(out, {"", ""});
    }

    ~CComputer() {
        for (auto & it : m_Components)
            delete it;
        m_Components.clear();
    }

private:
    std::string m_Name;
    std::list<std::string> m_IPs;
    std::list<CComponent*> m_Components;
};

class CNetwork {
public:
    CNetwork(const std::string &network) 
        : m_Size(0)
        , m_Name(network)
    {
    }

    void copyData(const CNetwork &network) {
        m_Name = network.m_Name;
        m_Size = network.m_Size;
        m_Computers.clear();

        for (const auto &computer: network.m_Computers)
            m_Computers.push_back(computer);
    }

    CNetwork(const CNetwork &network) 
    {
        copyData(network);
    }

    CNetwork& operator=(const CNetwork &network){
        if (this == &network) return *this;
        copyData(network);
        return *this;
    }

    CNetwork& addComputer(const CComputer& computer) {
        m_Computers.push_back(computer);
        m_Size++;
        return *this;
    }

    CComputer* findComputer(const std::string& computer) {
        auto it = std::find_if(m_Computers.begin(), m_Computers.end(), [computer](const CComputer& c) {
            return c.getName() == computer;
        });
        
        if (it != m_Computers.end()) {
            return &(*it); 
        } else {
            return nullptr; 
        }
    }

    std::ostream& toString(std::ostream &out) const {
        out << "Network: " << m_Name << "\n";

        std::pair<std::string, std::string> indents;
        if (m_Size > 1) {
            indents = {"| ", "+-"};
        }

        size_t count = 0;
        for (auto &computer: m_Computers) {
            if (m_Size > 1 && count == m_Size - 1) {
                indents.first = "  ";
                indents.second = "\\-";
            }
            computer.toString(out, indents);
            count++;
        }

        return out;
    }

    friend std::ostream& operator << (std::ostream &out, const CNetwork &network) {
        return network.toString(out);
    }

private:
    size_t m_Size;
    std::string m_Name;
    std::list<CComputer> m_Computers;
};

class CCPU : public CComponent {
public:
    CCPU(int cores, int frequency) 
        : m_Cores(cores)
        , m_Frequency(frequency)
    {
    }

    CComponent* duplicate() const override {
        return new CCPU(*this);
    }

    std::ostream& toString(std::ostream &out, std::pair<std::string, std::string> indents) const override {
        out << indents.first << indents.second << "CPU, " << m_Cores << " cores @ " << m_Frequency << "MHz\n";
        return out;
    }

private:
    int m_Cores;
    int m_Frequency;
};

class CMemory : public CComponent {
public:
    CMemory(int size) 
        : m_Size(size)
    {
    }

    CComponent* duplicate() const override {
        return new CMemory(*this);
    }

    std::ostream& toString(std::ostream &out, std::pair<std::string, std::string> indents) const override {
        out << indents.first << indents.second << "Memory, " << m_Size << " MiB\n";
        return out;
    }

private:
    int m_Size;
};

class CDisk  : public CComponent {
public:
    enum DiskType { MAGNETIC, SSD };

    CDisk(DiskType type, int size)
        : m_Type(type)
        , m_Size(size)
    {
    }

    CDisk& addPartition(int size, const std::string &id) {
        m_Partitions.push_back({size, id});
        return *this;
    }

    CComponent* duplicate() const override {
        return new CDisk(*this);
    }

    std::ostream& toString(std::ostream &out, std::pair<std::string, std::string> indents) const override {
        out << indents.first << indents.second << (m_Type == DiskType::MAGNETIC ? "HDD" : "SSD");
        out << ", " << m_Size << " GiB\n";

        size_t id = 0;
        std::pair<std::string, std::string> indents2{"| ", "+-"};
        
        if (indents.second == "\\-") {
            indents2.first = "  ";
            indents2.second = "+-";
        }

        for (const auto &partition: m_Partitions) {
            out << indents.first;

            if (&partition == &m_Partitions.back()) {
                indents2.second = "\\-";
            }

            out << indents2.first << indents2.second;
            out << "[" << id << "]: ";
            out << partition.first << " GiB, " << partition.second << "\n";
            id++;
        }

        return out;
    }

private:
    DiskType m_Type;
    int m_Size;
    std::list<std::pair<int, std::string>> m_Partitions;
};

#ifndef __PROGTEST__
template<typename T_>
std::string toString ( const T_ & x )
{
  std::ostringstream oss;
  oss << x;
  return oss . str ();
}

int main ()
{
  CNetwork n ( "FIT network" );
  n . addComputer (
        CComputer ( "progtest.fit.cvut.cz" ) .
          addAddress ( "147.32.232.142" ) .
          addComponent ( CCPU ( 8, 2400 ) ) .
          addComponent ( CCPU ( 8, 1200 ) ) .
          addComponent ( CDisk ( CDisk::MAGNETIC, 1500 ) .
            addPartition ( 50, "/" ) .
            addPartition ( 5, "/boot" ).
            addPartition ( 1000, "/var" ) ) .
          addComponent ( CDisk ( CDisk::SSD, 60 ) .
            addPartition ( 60, "/data" )  ) .
          addComponent ( CMemory ( 2000 ) ).
          addComponent ( CMemory ( 2000 ) ) ) .
      addComputer (
        CComputer ( "courses.fit.cvut.cz" ) .
          addAddress ( "147.32.232.213" ) .
          addComponent ( CCPU ( 4, 1600 ) ) .
          addComponent ( CMemory ( 4000 ) ).
          addComponent ( CDisk ( CDisk::MAGNETIC, 2000 ) .
            addPartition ( 100, "/" )   .
            addPartition ( 1900, "/data" ) ) ) .
      addComputer (
        CComputer ( "imap.fit.cvut.cz" ) .
          addAddress ( "147.32.232.238" ) .
          addComponent ( CCPU ( 4, 2500 ) ) .
          addAddress ( "2001:718:2:2901::238" ) .
          addComponent ( CMemory ( 8000 ) ) );

  assert ( toString ( n ) ==
    "Network: FIT network\n"
    "+-Host: progtest.fit.cvut.cz\n"
    "| +-147.32.232.142\n"
    "| +-CPU, 8 cores @ 2400MHz\n"
    "| +-CPU, 8 cores @ 1200MHz\n"
    "| +-HDD, 1500 GiB\n"
    "| | +-[0]: 50 GiB, /\n"
    "| | +-[1]: 5 GiB, /boot\n"
    "| | \\-[2]: 1000 GiB, /var\n"
    "| +-SSD, 60 GiB\n"
    "| | \\-[0]: 60 GiB, /data\n"
    "| +-Memory, 2000 MiB\n"
    "| \\-Memory, 2000 MiB\n"
    "+-Host: courses.fit.cvut.cz\n"
    "| +-147.32.232.213\n"
    "| +-CPU, 4 cores @ 1600MHz\n"
    "| +-Memory, 4000 MiB\n"
    "| \\-HDD, 2000 GiB\n"
    "|   +-[0]: 100 GiB, /\n"
    "|   \\-[1]: 1900 GiB, /data\n"
    "\\-Host: imap.fit.cvut.cz\n"
    "  +-147.32.232.238\n"
    "  +-2001:718:2:2901::238\n"
    "  +-CPU, 4 cores @ 2500MHz\n"
    "  \\-Memory, 8000 MiB\n" );
  CNetwork x = n;
  auto c = x . findComputer ( "imap.fit.cvut.cz" );
           
  assert ( toString ( *c ) ==
    "Host: imap.fit.cvut.cz\n"
    "+-147.32.232.238\n"
    "+-2001:718:2:2901::238\n"
    "+-CPU, 4 cores @ 2500MHz\n"
    "\\-Memory, 8000 MiB\n" );
  c -> addComponent ( CDisk ( CDisk::MAGNETIC, 1000 ) .
         addPartition ( 100, "system" ) .
         addPartition ( 200, "WWW" ) .
         addPartition ( 700, "mail" ) );

  assert ( toString ( x ) ==
    "Network: FIT network\n"
    "+-Host: progtest.fit.cvut.cz\n"
    "| +-147.32.232.142\n"
    "| +-CPU, 8 cores @ 2400MHz\n"
    "| +-CPU, 8 cores @ 1200MHz\n"
    "| +-HDD, 1500 GiB\n"
    "| | +-[0]: 50 GiB, /\n"
    "| | +-[1]: 5 GiB, /boot\n"
    "| | \\-[2]: 1000 GiB, /var\n"
    "| +-SSD, 60 GiB\n"
    "| | \\-[0]: 60 GiB, /data\n"
    "| +-Memory, 2000 MiB\n"
    "| \\-Memory, 2000 MiB\n"
    "+-Host: courses.fit.cvut.cz\n"
    "| +-147.32.232.213\n"
    "| +-CPU, 4 cores @ 1600MHz\n"
    "| +-Memory, 4000 MiB\n"
    "| \\-HDD, 2000 GiB\n"
    "|   +-[0]: 100 GiB, /\n"
    "|   \\-[1]: 1900 GiB, /data\n"
    "\\-Host: imap.fit.cvut.cz\n"
    "  +-147.32.232.238\n"
    "  +-2001:718:2:2901::238\n"
    "  +-CPU, 4 cores @ 2500MHz\n"
    "  +-Memory, 8000 MiB\n"
    "  \\-HDD, 1000 GiB\n"
    "    +-[0]: 100 GiB, system\n"
    "    +-[1]: 200 GiB, WWW\n"
    "    \\-[2]: 700 GiB, mail\n" );
  assert ( toString ( n ) ==
    "Network: FIT network\n"
    "+-Host: progtest.fit.cvut.cz\n"
    "| +-147.32.232.142\n"
    "| +-CPU, 8 cores @ 2400MHz\n"
    "| +-CPU, 8 cores @ 1200MHz\n"
    "| +-HDD, 1500 GiB\n"
    "| | +-[0]: 50 GiB, /\n"
    "| | +-[1]: 5 GiB, /boot\n"
    "| | \\-[2]: 1000 GiB, /var\n"
    "| +-SSD, 60 GiB\n"
    "| | \\-[0]: 60 GiB, /data\n"
    "| +-Memory, 2000 MiB\n"
    "| \\-Memory, 2000 MiB\n"
    "+-Host: courses.fit.cvut.cz\n"
    "| +-147.32.232.213\n"
    "| +-CPU, 4 cores @ 1600MHz\n"
    "| +-Memory, 4000 MiB\n"
    "| \\-HDD, 2000 GiB\n"
    "|   +-[0]: 100 GiB, /\n"
    "|   \\-[1]: 1900 GiB, /data\n"
    "\\-Host: imap.fit.cvut.cz\n"
    "  +-147.32.232.238\n"
    "  +-2001:718:2:2901::238\n"
    "  +-CPU, 4 cores @ 2500MHz\n"
    "  \\-Memory, 8000 MiB\n" );

CComputer ccc ("00581251.kicbvk.cz");  
  ccc.addAddress("4.249.14.115"). addAddress("4.250.214.93").
  addComponent(CMemory(9837)). addComponent(CMemory(5372)).
  addComponent(CMemory(14849)). addComponent(CMemory(13294)).
  addComponent(CDisk(CDisk::SSD, 2716).addPartition(619,"aqjpfjmjurqisjfugbwmmolxav").
  addPartition(565, "bsjoiowqfehajbylnlwcnvg"));

    assert(toString(ccc)==
  "Host: 00581251.kicbvk.cz\n"
  "+-4.249.14.115\n"
  "+-4.250.214.93\n"
  "+-Memory, 9837 MiB\n"
  "+-Memory, 5372 MiB\n"
  "+-Memory, 14849 MiB\n"
  "+-Memory, 13294 MiB\n"
  "\\-SSD, 2716 GiB\n"
  "  +-[0]: 619 GiB, aqjpfjmjurqisjfugbwmmolxav\n"
  "  \\-[1]: 565 GiB, bsjoiowqfehajbylnlwcnvg\n");
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */