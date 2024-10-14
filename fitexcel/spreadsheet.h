#include "builder.h"

class CSpreadsheet {
public:
    static unsigned capabilities() {
        return SPREADSHEET_SPEED | SPREADSHEET_CYCLIC_DEPS | SPREADSHEET_FILE_IO;
    }
    
    CSpreadsheet() = default;
    CSpreadsheet(const CSpreadsheet &sheet);
    CSpreadsheet& operator=(const CSpreadsheet &sheet);
    bool load(std::istream &is);
    bool save(std::ostream &os) const;
    bool setCell(CPos pos, std::string contents);
    CValue getValue(CPos pos);
    void copyRect(CPos dst, CPos src, int w = 1, int h = 1);

private:
    std::map<std::string, CCell> m_Table;
    std::map<std::string, std::vector<std::string>> m_Dependencies;
    size_t hashTableContent(const std::string& str) const;
};

class CDependencyChecker {
public:
    CDependencyChecker(const std::map<std::string, std::vector<std::string>>& dependencies);
    bool containsCycle(const std::string& vertex);

private:
    std::map<std::string, std::vector<std::string>> m_Dependencies;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursionStack;
    bool isCyclicUtil(const std::string& vertex);
};
