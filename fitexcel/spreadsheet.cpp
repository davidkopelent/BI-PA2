/******************************************************
 * Filename: spreadsheet.cpp
 * Author: David Kopelent
 * Date: 24.04.2024
 * Description: This file implements methods for representing and manipulating spreadsheets.
 *              It contains the definitions of the member functions of the CSpreadsheet and CDependencyChecker class.
 ******************************************************/

#include "spreadsheet.h"

CSpreadsheet::CSpreadsheet(const CSpreadsheet &sheet) 
    : m_Table(sheet.m_Table)
    , m_Dependencies(sheet.m_Dependencies) {}

CSpreadsheet& CSpreadsheet::operator=(const CSpreadsheet &sheet) {
    if (&sheet == this) return *this;
    m_Table = sheet.m_Table;
    m_Dependencies = sheet.m_Dependencies;
    return *this;
}

// Load spreadsheet data from an input stream
bool CSpreadsheet::load(std::istream &is) {
    if (is.fail()) {
        return false;
    }

    // Store input stream inside string
    std::stringstream buffer;
    buffer << is.rdbuf();
    std::string content = buffer.str();

    size_t headStart = content.find("<HEAD>");
    size_t headEnd = content.find("</HEAD>");

    if (headStart == std::string::npos || headEnd == std::string::npos)
        return false;

    // Check headers for file corruption
    std::string head = content.substr(headStart + 6, headEnd - headStart - 6);
    if (head != std::to_string(hashTableContent(content.substr(headEnd + 7))))
        return false;

    // Clear existing data
    m_Table.clear();
    m_Dependencies.clear();

    if (content.substr(headEnd + 7) == "<EMPTY>")
        return true;

    size_t idStart = content.find("<ID>");
    size_t valStart = content.find("<VAL>");
    
    while (idStart != std::string::npos && valStart != std::string::npos) {
        size_t idEnd = content.find("</ID>", idStart);

        if (idEnd == std::string::npos) {
            return false;
        }

        std::string cell = content.substr(idStart + 4, idEnd - idStart - 4);
        idStart = content.find("<ID>", idEnd);

        size_t valEnd = content.find("</VAL>", valStart);

        if (valEnd == std::string::npos) {
            return false;
        }

        std::string value = content.substr(valStart + 5, valEnd - valStart - 5);
        valStart = content.find("<VAL>", valEnd);

        try {
            setCell(CPos(cell), value);
        } catch(std::invalid_argument &e) {
            m_Table.clear();
            m_Dependencies.clear();
            return false;
        }
    }

    return true;
}

size_t CSpreadsheet::hashTableContent(const std::string& str) const {
    std::hash<std::string> hasher;
    return hasher(str);
}

// Save spreadsheet data to an output stream
bool CSpreadsheet::save(std::ostream &os) const {
    if (os.fail())
        return false;

    os.clear();
    std::string head;
    std::string content = "";

    if (m_Table.empty()) {
        content = "<EMPTY>";
        head = "<HEAD>" + std::to_string(hashTableContent(content)) + "</HEAD>";
        os << head << content;
        return !os.fail();
    }

    for (const auto &d: m_Table) {
        content += "<ID>" + d.second.getPos().getId() + "</ID><VAL>" + d.second.getExpression() + "</VAL>";
    }

    head = "<HEAD>" + std::to_string(hashTableContent(content)) + "</HEAD>";
    os << head << content;
    return !os.fail();
}

// Set the contents of a cell
bool CSpreadsheet::setCell(CPos pos, std::string contents) {
    if (contents.empty())
        return false;
        
    CBuilder builder(pos);
    std::string expression = contents;

    try {
        parseExpression(contents, builder); 
    } catch(std::invalid_argument &e) {
        auto cell = m_Table.find(pos.getId());
        if (cell != m_Table.end()) {
            m_Table.erase(cell);
        }

        auto dependencyIterator = m_Dependencies.find(pos.getId());
        if (dependencyIterator != m_Dependencies.end()) {
            m_Dependencies.erase(dependencyIterator->first);
        }
        return false;
    }

    // Create a new cell object
    CCell newCell(pos, expression, builder.buildAST());

    // Check if the cell already exists in the table
    auto cell = m_Table.find(pos.getId());
    if (cell != m_Table.end()) {
        cell->second = newCell;
    } else {
        // Insert the new cell into the table
        m_Table.insert({pos.getId(), newCell});
    }

    auto dependencyIterator = m_Dependencies.find(pos.getId());
    if (dependencyIterator != m_Dependencies.end()) {
        dependencyIterator->second = builder.getDependencies();
    } else {
        m_Dependencies.insert({pos.getId(), builder.getDependencies()});
    }

    return true;
}

// Get the value of a cell
CValue CSpreadsheet::getValue(CPos pos) {
    auto cell = m_Table.find(pos.getId());
    if (cell != m_Table.end()) {
        CDependencyChecker checker(m_Dependencies);
        if (checker.containsCycle(pos.getId())) {
            return CValue();
        }
        return cell->second.evaluate(m_Table);
    }
    return CValue();
}

// Copy a rectangular range of cells within the spreadsheet
void CSpreadsheet::copyRect(CPos dst, CPos src, int w, int h) {
    // Determine start and destination positions
    size_t startCol = src.getColumnNumber();
    size_t startRow = src.getRow();
    size_t dstCol = dst.getColumnNumber();
    size_t dstRow = dst.getRow();
    std::vector<std::pair<CPos, CCell>> from;
    std::vector<CPos> to;

    // Get list of cells for copying
    for (size_t i = startCol; i < startCol + w; i++) {
        std::string col = CPos::numberToColumn(i);
    
        for (size_t j = startRow; j < startRow + h; j++) {
            auto cell = m_Table.find(col + std::to_string(j));
            if (cell != m_Table.end()) {
                from.push_back({CPos(col + std::to_string(j)), cell->second});
            } else {
                from.push_back({CPos(col + std::to_string(j)), CCell()});
            }
        }
    }

    // Get list of target cell positions
    for (size_t i = dstCol; i < dstCol + w; i++) {
        std::string col = CPos::numberToColumn(i);

        for (size_t j = dstRow; j < dstRow + h; j++) {
            to.push_back(CPos(col + std::to_string(j)));
        }
    }
    
    // Copy cells
    for (size_t i = 0; i < from.size(); i++) {
        auto dependencyIterator = m_Dependencies.find(to[i].getId());

        if (from[i].second.isEmpty()) {
            m_Table.erase(to[i].getId());
            
            // Remove cell dependencies
            if (dependencyIterator != m_Dependencies.end())
                m_Dependencies.erase(to[i].getId());
            continue;
        } 

        std::vector<std::string> dependencies;
        CCell newCell = from[i].second.copyCell(to[i], dependencies);
        auto cell = m_Table.find(to[i].getId());

        if (cell != m_Table.end()) {
            m_Table.at(cell->first) = newCell;
        } else {
            m_Table.insert({to[i].getId(), newCell});
        }

        if (dependencyIterator != m_Dependencies.end()) {
            m_Dependencies.at(dependencyIterator->first) = dependencies;
        }
    }
}

CDependencyChecker::CDependencyChecker(const std::map<std::string, std::vector<std::string>>& dependencies) 
    : m_Dependencies(dependencies) {}

bool CDependencyChecker::isCyclicUtil(const std::string& vertex) {
    if (!visited.count(vertex)) {
        visited.insert(vertex);
        recursionStack.insert(vertex);

        for (const auto& dependent : m_Dependencies[vertex]) {
            if (!visited.count(dependent) && isCyclicUtil(dependent))
                return true;
            else if (recursionStack.count(dependent))
                return true;
        }
    }
    recursionStack.erase(vertex);
    return false;
}

bool CDependencyChecker::containsCycle(const std::string &vertex) {
    return isCyclicUtil(vertex);
}