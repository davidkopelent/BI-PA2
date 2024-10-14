#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <climits>
#include <cfloat>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <optional>
#include <compare>
#include <charconv>
#include <span>
#include <utility>
#include "expression.h"
using namespace std::literals;
using CValue = std::variant<std::monostate, double, std::string>;

constexpr unsigned SPREADSHEET_CYCLIC_DEPS = 0x01;
constexpr unsigned SPREADSHEET_FUNCTIONS = 0x02;
constexpr unsigned SPREADSHEET_FILE_IO = 0x04;
constexpr unsigned SPREADSHEET_SPEED = 0x08;
constexpr unsigned SPREADSHEET_PARSER = 0x10;
#endif /* __PROGTEST__ */

class CCell;

class CPos {
public:
    CPos() = default;
    CPos(std::string_view str);
    CPos(const CPos &pos);
    std::string getId() const;
    std::string getColumn() const;
    size_t getRow() const;
    size_t getColumnNumber() const;

    /**
     * Converts numeric representation of column to coresponding string id (e.g. 1 -> A, 27 -> AA)
     * @param number Column position
     * @return Column id
    */
    static std::string numberToColumn(size_t number);

    /**
     * Converts string representation of column to coresponding numeric position (e.g. A -> 1, AA -> 27)
     * @param column Column id
     * @return Column position
    */
    size_t columnToNumber(const std::string& column);

private:
    bool isValidRowNumber(const std::string &rowNumber) const;
    std::string m_Id;
    std::string m_Column;
    size_t m_Row;
    size_t m_ColumnNumber;
};

/****************************************************************************/

class CNode {
public:
    /**
     * Method to recursively evaluate AST and get cell value
     * @param table Table data
     * @return Evaluated value
    */
    virtual CValue evaluate(std::map<std::string, CCell> &table) = 0;

    /**
     * Method to recursively clone AST nodes when copying cells
     * @param dst Target cell position
     * @return Pointer to copied node
    */
    virtual std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) = 0;
    virtual ~CNode() = default;
};

/****************************************************************************/

class CCell {
public:
    CCell();
    CCell(CPos id, const std::string &expr, std::unique_ptr<CNode> AST);
    CPos getPos() const;
    void setValue(CValue val);
    CValue getValue() const;
    std::string getExpression() const;

    /**
     * Evaluates cell and returns its value
     * @param table Table data
     * @return Evaluated cell value
    */
    CValue evaluate(std::map<std::string, CCell> &table);
    bool isEmpty() const;

    /**
     * Creates copy of this cell with recalculated references
     * @param dst Target cell position
     * @return Copied cell
    */
    CCell copyCell(CPos dst, std::vector<std::string> &dependencies);

private:
    CPos m_Pos;
    std::string m_Expression;
    CValue m_Value;
    // Root of AST
    std::shared_ptr<CNode> m_Root;
    bool m_IsEmpty;
};

/****************************************************************************/

class CNumberNode : public CNode {
public:
    CNumberNode(double num);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;

private:
    double m_Value;
};

class CStringNode : public CNode {
public:
    CStringNode(const std::string &str);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;

private:
    std::string m_Value;
};

/****************************************************************************/

class CBinaryOperatorNode : public CNode {
public:
    CBinaryOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    
protected:
    // Left operand
    std::unique_ptr<CNode> m_Left;

    // Right operand
    std::unique_ptr<CNode> m_Right;
};

class CAddOperatorNode : public CBinaryOperatorNode {
public:
    CAddOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CSubOperatorNode : public CBinaryOperatorNode {
public:
    CSubOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CDivOperatorNode : public CBinaryOperatorNode {
public:
    CDivOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CMulOperatorNode : public CBinaryOperatorNode {
public:
    CMulOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CPowOperatorNode : public CBinaryOperatorNode {
public:
    CPowOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

/****************************************************************************/

class CRelationalOperatorNode : public CNode {
public:
    CRelationalOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    
protected:
    std::unique_ptr<CNode> m_Left;
    std::unique_ptr<CNode> m_Right;
};

class CEqOperatorNode : public CRelationalOperatorNode {
public:
    CEqOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CNeOperatorNode : public CRelationalOperatorNode {
public:
    CNeOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CLtOperatorNode : public CRelationalOperatorNode {
public:
    CLtOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CLeOperatorNode : public CRelationalOperatorNode {
public:
    CLeOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CGtOperatorNode : public CRelationalOperatorNode {
public:
    CGtOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CGeOperatorNode : public CRelationalOperatorNode {
public:
    CGeOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

/****************************************************************************/

class CReferenceNode : public CNode {
public:
    CReferenceNode(CPos cellId, CPos refId, const std::string &ref);

    /**
     * Returns referenced value
     * @param table Table data
     * @return Reference value
    */
    CValue getValue(std::map<std::string, CCell> &table);

protected:
    // The position of the cell in which the reference is located
    CPos m_CellId;

    // Position of referenced cell
    CPos m_RefId;

    // String representation of reference
    std::string m_Reference;
};

class CRelativeReferenceNode : public CReferenceNode {
public:
    CRelativeReferenceNode(CPos cellId, CPos refId, const std::string &ref);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    
    /**
     * Clones Relative reference and recalculates reference
     * @param CPos Copy destination
     * @return Pointer to recalculated reference
    */
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CAbsoluteReferenceNode : public CReferenceNode {
public:
    CAbsoluteReferenceNode(CPos cellId, CPos refId, const std::string &ref);
    CValue evaluate(std::map<std::string, CCell> &table) override;
    
    /**
     * Clones Absolute reference
     * @param CPos Copy destination
     * @return Pointer to reference
    */
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CAbsRelReferenceNode : public CReferenceNode {
public:
    CAbsRelReferenceNode(CPos cellId, CPos refId, const std::string &ref);
    CValue evaluate(std::map<std::string, CCell> &table) override;

    /**
     * Clones Abs/Rel reference and recalculates reference
     * @param CPos Copy destination
     * @return Pointer to recalculated reference
    */
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CRelAbsReferenceNode : public CReferenceNode {
public:
    CRelAbsReferenceNode(CPos cellId, CPos refId, const std::string &ref);
    CValue evaluate(std::map<std::string, CCell> &table) override;

    /**
     * Clones Rel/Abs reference and recalculates reference
     * @param CPos Copy destination
     * @return Pointer to recalculated reference
    */
    std::unique_ptr<CNode> clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) override;
};

class CBuilder : public CExprBuilder {
public:
    CBuilder(CPos pos);
    void opAdd() override;
    void opSub() override;
    void opMul() override;
    void opDiv() override;
    void opPow() override;
    void opNeg() override;
    void opEq() override;
    void opNe() override;
    void opLt() override;
    void opLe() override;
    void opGt() override;
    void opGe() override;
    void valNumber(double num) override;
    void valString(std::string str) override;
    void valReference(std::string str) override;
    void valRange(std::string str) override;
    void funcCall(std::string fnName, int paramCnt) override;
    std::unique_ptr<CNode> getTopNode();
    std::vector<std::string> getDependencies() const;
    std::unique_ptr<CNode> buildAST();

private:
    std::stack<std::unique_ptr<CNode>> m_Nodes;
    std::vector<std::string> m_Dependencies;
    CPos m_Pos;
};

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
/******************************************************
 * Filename: cell.cpp
 * Author: David Kopelent
 * Date: 24.04.2024
 * Description: This code implements methods for cell representation and is responsible for building AST nodes
 * 
 ******************************************************/


/***********************************************
*        Cell Position Section
***********************************************/

CPos::CPos(std::string_view str) {
    if (str.empty())
        throw std::invalid_argument("Invalid ID!");

    size_t pos = str.find_first_of("0123456789");
    if (pos == std::string::npos)
        throw std::invalid_argument("Invalid ID!");

    for (size_t i = 0; i < pos; i++) {
        if (!std::isalpha(str[i]))
            throw std::invalid_argument("Invalid ID!");
        m_Column.push_back(std::toupper(str[i]));
    }

    std::string row(str.substr(pos));
    if (!isValidRowNumber(row))
        throw std::invalid_argument("Invalid ID!");

    m_Row = std::stoi(row);
    m_Id = str;
    m_ColumnNumber = columnToNumber(m_Column);
}

CPos::CPos(const CPos &pos) 
    : m_Id(pos.m_Id)
    , m_Column(pos.m_Column)
    , m_Row(pos.m_Row)
    , m_ColumnNumber(pos.m_ColumnNumber) {}

bool CPos::isValidRowNumber(const std::string &rowNumber) const {
    for (size_t i = 0; i < rowNumber.length(); i++) {
        if (!std::isdigit(rowNumber[i]))
            return false;
    }
    return !(rowNumber.size() > 1 && rowNumber[0] == '0');
}

size_t CPos::columnToNumber(const std::string& column) {
    size_t result = 0;
    size_t size = column.size();
    for (size_t i = 0; i < size; ++i) {
        result += (column[size - i - 1] - 'A' + 1) * std::pow(26, i);
    }
    return result;
}

std::string CPos::numberToColumn(size_t number) {
    std::string result = "";
    while (number > 0) {
        int remainder = (number - 1) % 26; // Adjusted to 0-based index
        result = char('A' + remainder) + result;
        number = (number - 1) / 26;
    }
    return result;
}

std::string CPos::getId() const {
    return m_Id;
}

std::string CPos::getColumn() const {
    return m_Column;
}

size_t CPos::getRow() const {
    return m_Row;
}

size_t CPos::getColumnNumber() const {
    return m_ColumnNumber;
}

/***********************************************
*        Cell Section
***********************************************/

CCell::CCell() 
    : m_IsEmpty(true) {}   

CCell::CCell(CPos id, const std::string &expr, std::unique_ptr<CNode> AST)
    : m_Pos(id)
    , m_Expression(expr)
    , m_Root(std::move(AST))
    , m_IsEmpty(false) {}

CPos CCell::getPos() const {
    return m_Pos;
}

void CCell::setValue(CValue val) {
    m_Value = val;
}

CValue CCell::getValue() const {
    return m_Value;
}

std::string CCell::getExpression() const {
    return m_Expression;
}

CValue CCell::evaluate(std::map<std::string, CCell> &table) {
    if (m_Root == nullptr) {
        m_Value = CValue();
        return m_Value;
    }

    m_Value = m_Root->evaluate(table);
    return m_Value;
}

bool CCell::isEmpty() const {
    return m_IsEmpty;
}

CCell CCell::copyCell(CPos dst, std::vector<std::string> &dependencies) {
    std::string expression = m_Expression;
    std::unique_ptr<CNode> root = m_Root->clone(dst, expression, dependencies);
    return CCell(dst, expression, std::move(root));
}   

/***********************************************
*        AST Node Types Section
***********************************************/

CNumberNode::CNumberNode(double num) 
    : m_Value(num) {}

CValue CNumberNode::evaluate(std::map<std::string, CCell> &table) {
    if (std::isinf(m_Value)) return CValue();
    return CValue(m_Value);
}

std::unique_ptr<CNode> CNumberNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CNumberNode>(m_Value);
}

CStringNode::CStringNode(const std::string &str) 
    : m_Value(str) {}

CValue CStringNode::evaluate(std::map<std::string, CCell> &table) {
    return CValue(m_Value);
}

std::unique_ptr<CNode> CStringNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CStringNode>(m_Value);
}

/***********************************************
*        Arithmetics Operators Section
***********************************************/

CBinaryOperatorNode::CBinaryOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : m_Left(std::move(left))
    , m_Right(std::move(right)) {}

CAddOperatorNode::CAddOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CBinaryOperatorNode(std::move(left), std::move(right)) {}

CValue CAddOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<double>(rightValue)) {
        return CValue(std::get<std::string>(leftValue) + std::to_string(std::get<double>(rightValue)));

    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue(std::to_string(std::get<double>(leftValue)) + std::get<std::string>(rightValue));

    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        double result = std::get<double>(leftValue) + std::get<double>(rightValue);
        return CValue(result);

    } else if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue(std::get<std::string>(leftValue) + std::get<std::string>(rightValue));
    }

    return CValue();
}

std::unique_ptr<CNode> CAddOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CAddOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CSubOperatorNode::CSubOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CBinaryOperatorNode(std::move(left), std::move(right)) {}

CValue CSubOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        double result = std::get<double>(leftValue) - std::get<double>(rightValue);
        return CValue(result);
    }

    return CValue();
}

std::unique_ptr<CNode> CSubOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CSubOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CDivOperatorNode::CDivOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CBinaryOperatorNode(std::move(left), std::move(right)) {}

CValue CDivOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        if (std::get<double>(rightValue) == 0) return CValue();
        double result = std::get<double>(leftValue) / std::get<double>(rightValue);
        return CValue(result);
    }

    return CValue();
}

std::unique_ptr<CNode> CDivOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CDivOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CMulOperatorNode::CMulOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CBinaryOperatorNode(std::move(left), std::move(right)) {}

CValue CMulOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        double result = std::get<double>(leftValue) * std::get<double>(rightValue);
        return CValue(result);
    }

    return CValue();
}

std::unique_ptr<CNode> CMulOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CMulOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CPowOperatorNode::CPowOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CBinaryOperatorNode(std::move(left), std::move(right)) {}

CValue CPowOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        double result = std::pow(std::get<double>(leftValue), std::get<double>(rightValue));
        return CValue(result);
    }

    return CValue();
}

std::unique_ptr<CNode> CPowOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CPowOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

/***********************************************
*        Relational Operators Section
***********************************************/

CRelationalOperatorNode::CRelationalOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : m_Left(std::move(left))
    , m_Right(std::move(right)) {}

CEqOperatorNode::CEqOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CRelationalOperatorNode(std::move(left), std::move(right)) {}

CValue CEqOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue((std::get<std::string>(leftValue) == std::get<std::string>(rightValue)) ? 1.0 : 0.0);
    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        return CValue((std::get<double>(leftValue) == std::get<double>(rightValue)) ? 1.0 : 0.0);
    }

    return CValue();
}

std::unique_ptr<CNode> CEqOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CEqOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CNeOperatorNode::CNeOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CRelationalOperatorNode(std::move(left), std::move(right)) {}

CValue CNeOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue((std::get<std::string>(leftValue) != std::get<std::string>(rightValue)) ? 1.0 : 0.0);
    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        return CValue((std::get<double>(leftValue) != std::get<double>(rightValue)) ? 1.0 : 0.0);
    }

    return CValue();
}

std::unique_ptr<CNode> CNeOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CNeOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CLtOperatorNode::CLtOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CRelationalOperatorNode(std::move(left), std::move(right)) {}

CValue CLtOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue((std::get<std::string>(leftValue) < std::get<std::string>(rightValue)) ? 1.0 : 0.0);
    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        return CValue((std::get<double>(leftValue) < std::get<double>(rightValue)) ? 1.0 : 0.0);
    }

    return CValue();
}

std::unique_ptr<CNode> CLtOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CLtOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CLeOperatorNode::CLeOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CRelationalOperatorNode(std::move(left), std::move(right)) {}

CValue CLeOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue((std::get<std::string>(leftValue) <= std::get<std::string>(rightValue)) ? 1.0 : 0.0);
    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        return CValue((std::get<double>(leftValue) <= std::get<double>(rightValue)) ? 1.0 : 0.0);
    }

    return CValue();
}

std::unique_ptr<CNode> CLeOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CLeOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CGtOperatorNode::CGtOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CRelationalOperatorNode(std::move(left), std::move(right)) {}

CValue CGtOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue((std::get<std::string>(leftValue) > std::get<std::string>(rightValue)) ? 1.0 : 0.0);
    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        return CValue((std::get<double>(leftValue) > std::get<double>(rightValue)) ? 1.0 : 0.0);
    }

    return CValue();
}

std::unique_ptr<CNode> CGtOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CGtOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

CGeOperatorNode::CGeOperatorNode(std::unique_ptr<CNode> left, std::unique_ptr<CNode> right) 
    : CRelationalOperatorNode(std::move(left), std::move(right)) {}

CValue CGeOperatorNode::evaluate(std::map<std::string, CCell> &table) {
    auto leftValue = m_Left->evaluate(table);
    auto rightValue = m_Right->evaluate(table);

    if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
        return CValue((std::get<std::string>(leftValue) >= std::get<std::string>(rightValue)) ? 1.0 : 0.0);
    } else if (std::holds_alternative<double>(leftValue) && std::holds_alternative<double>(rightValue)) {
        return CValue((std::get<double>(leftValue) >= std::get<double>(rightValue)) ? 1.0 : 0.0);
    }

    return CValue();
}

std::unique_ptr<CNode> CGeOperatorNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CGeOperatorNode>(m_Left->clone(dst, expr, dependencies), m_Right->clone(dst, expr, dependencies));
}

/***********************************************
*        References Section
***********************************************/

CReferenceNode::CReferenceNode(CPos cellId, CPos refId, const std::string &ref) 
    : m_CellId(cellId)
    , m_RefId(refId)
    , m_Reference(ref) {}

CValue CReferenceNode::getValue(std::map<std::string, CCell> &table) {
    auto cell = table.find(m_Reference);
    if (cell != table.end()) {
        return cell->second.evaluate(table);
    }
    return CValue();
}

CRelativeReferenceNode::CRelativeReferenceNode(CPos cellId, CPos refId, const std::string &ref) 
    : CReferenceNode(cellId, refId, ref) {}

CValue CRelativeReferenceNode::evaluate(std::map<std::string, CCell> &table) {
    return this->getValue(table);
}

std::unique_ptr<CNode> CRelativeReferenceNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    size_t rowOffset = dst.getRow() - m_CellId.getRow();
    size_t colOffset = dst.getColumnNumber() - m_CellId.getColumnNumber();

    std::string newReference = m_Reference;
    size_t refRow = m_RefId.getRow();
    size_t refCol = m_RefId.getColumnNumber();

    // Adjust row and column based on offsets
    refRow += rowOffset;
    refCol += colOffset;
    newReference = CPos::numberToColumn(refCol) + std::to_string(refRow);

    size_t start_pos = 0;
    std::string from = m_Reference;
    std::string to =  newReference;
    while ((start_pos = expr.find(from, start_pos)) != std::string::npos) {
        expr.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return std::make_unique<CRelativeReferenceNode>(dst, CPos(newReference), newReference);
}

CAbsoluteReferenceNode::CAbsoluteReferenceNode(CPos cellId, CPos refId, const std::string &ref) 
    : CReferenceNode(cellId, refId, ref) {}

CValue CAbsoluteReferenceNode::evaluate(std::map<std::string, CCell> &table) {
    return this->getValue(table);
}

std::unique_ptr<CNode> CAbsoluteReferenceNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    return std::make_unique<CAbsoluteReferenceNode>(dst, m_RefId, m_Reference);
}

CAbsRelReferenceNode::CAbsRelReferenceNode(CPos cellId, CPos refId, const std::string &ref) 
    : CReferenceNode(cellId, refId, ref) {}

CValue CAbsRelReferenceNode::evaluate(std::map<std::string, CCell> &table) {
    return this->getValue(table);
}

std::unique_ptr<CNode> CAbsRelReferenceNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    size_t rowOffset = dst.getRow() - m_CellId.getRow();
    size_t refRow = m_RefId.getRow();

    // Construct the adjusted reference
    std::string newReference = m_RefId.getColumn() + std::to_string(refRow + rowOffset);

    size_t start_pos = 0;
    std::string from = "$" + m_RefId.getColumn() + std::to_string(m_RefId.getRow());
    std::string to =  "$" + m_RefId.getColumn() + std::to_string(refRow + rowOffset);
    while ((start_pos = expr.find(from, start_pos)) != std::string::npos) {
        expr.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return std::make_unique<CAbsRelReferenceNode>(dst, CPos(newReference), newReference);
}

CRelAbsReferenceNode::CRelAbsReferenceNode(CPos cellId, CPos refId, const std::string &ref) 
    : CReferenceNode(cellId, refId, ref) {}

CValue CRelAbsReferenceNode::evaluate(std::map<std::string, CCell> &table) {
    return this->getValue(table);
}

std::unique_ptr<CNode> CRelAbsReferenceNode::clone(CPos dst, std::string &expr, std::vector<std::string> &dependencies) {
    size_t colOffset = dst.getColumnNumber() - m_CellId.getColumnNumber();
    size_t refRow = m_RefId.getRow();
    size_t refCol = m_RefId.getColumnNumber();
    refCol += colOffset;

    // Construct the adjusted reference
    std::string newReference = CPos::numberToColumn(refCol) + std::to_string(refRow);

    size_t start_pos = 0;
    std::string from = m_RefId.getColumn() + "$" + std::to_string(m_RefId.getRow());
    std::string to = CPos::numberToColumn(refCol) + "$" + std::to_string(m_RefId.getRow());
    while ((start_pos = expr.find(from, start_pos)) != std::string::npos) {
        expr.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return std::make_unique<CRelAbsReferenceNode>(dst, CPos(newReference), newReference);
}
/******************************************************
 * Filename: builder.cpp
 * Author: David Kopelent
 * Date: 24.04.2024
 * Description: This code implements methods for building AST of parsed expression
 * 
 ******************************************************/


CBuilder::CBuilder(CPos pos)
    : m_Pos(pos) {}

void CBuilder::opAdd() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CAddOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opSub() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CSubOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opMul() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CMulOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opDiv() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CDivOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opPow() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CPowOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opNeg() {
    if (m_Nodes.empty()) return;
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CMulOperatorNode>(std::make_unique<CNumberNode>(-1.0), std::move(right)));
}

void CBuilder::opEq() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CEqOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opNe() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CNeOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opLt() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CLtOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opLe() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CLeOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opGt() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CGtOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::opGe() {
    if (m_Nodes.empty() || m_Nodes.size() < 2) return;
    auto left = getTopNode();
    auto right = getTopNode();
    m_Nodes.push(std::make_unique<CGeOperatorNode>(std::move(right), std::move(left)));
}

void CBuilder::valNumber(double num) {
    m_Nodes.push(std::make_unique<CNumberNode>(num));
}

void CBuilder::valString(std::string str) {
    m_Nodes.push(std::make_unique<CStringNode>(str));
}

void CBuilder::valReference(std::string str) {
    bool absoluteColumn = (str[0] == '$'); // Check if the first character is '$'
    bool absoluteRow = false; // Check if the last character is '$'

    // Check if any character between the first and last is '$'
    for (size_t i = 1; i < str.length() - 1; ++i) {
        if (str[i] == '$') {
            absoluteRow = true;
            break;
        }
    }

    bool relative = !absoluteColumn && !absoluteRow;
    if (!relative)
        str.erase(std::remove_if(str.begin(), str.end(), [](char c) { return c == '$'; }), str.end());

    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });

    if (relative) {
        // Relative
        m_Nodes.push(std::make_unique<CRelativeReferenceNode>(m_Pos, CPos(str), str));
    } else if (absoluteColumn && absoluteRow) {
        // Absolute
        m_Nodes.push(std::make_unique<CAbsoluteReferenceNode>(m_Pos, CPos(str), str));
    } else if (absoluteColumn) {
        // Absolute column Relative row
        m_Nodes.push(std::make_unique<CAbsRelReferenceNode>(m_Pos, CPos(str), str));
    } else {
        // Relative column Absolute row
        m_Nodes.push(std::make_unique<CRelAbsReferenceNode>(m_Pos, CPos(str), str));
    }

    m_Dependencies.push_back(str);
}

void CBuilder::valRange(std::string str) {
    // todo
}

void CBuilder::funcCall(std::string fnName, int paramCnt) {
    // todo
}

std::unique_ptr<CNode> CBuilder::getTopNode() {
    auto node = std::move(m_Nodes.top());
    m_Nodes.pop();
    return node;
}

std::vector<std::string> CBuilder::getDependencies() const {
    return m_Dependencies;
}

std::unique_ptr<CNode> CBuilder::buildAST() {
    if (m_Nodes.empty() || m_Nodes.size() != 1) return nullptr;
    return getTopNode();
}
/******************************************************
 * Filename: spreadsheet.cpp
 * Author: David Kopelent
 * Date: 24.04.2024
 * Description: This file implements methods for representing and manipulating spreadsheets.
 *              It contains the definitions of the member functions of the CSpreadsheet and CDependencyChecker class.
 ******************************************************/


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
#ifndef __PROGTEST__

bool valueMatch(const CValue &r, const CValue &s) {
    if (r.index() != s.index())
        return false;
    if (r.index() == 0)
        return true;
    if (r.index() == 2)
        return std::get<std::string>(r) == std::get<std::string>(s);
    if (std::isnan(std::get<double>(r)) && std::isnan(std::get<double>(s)))
        return true;
    if (std::isinf(std::get<double>(r)) && std::isinf(std::get<double>(s)))
        return (std::get<double>(r) < 0 && std::get<double>(s) < 0) || (std::get<double>(r) > 0 && std::get<double>(s) > 0);
    return fabs(std::get<double>(r) - std::get<double>(s)) <= 1e8 * DBL_EPSILON * fabs(std::get<double>(r));
}

int main() {
    CSpreadsheet x0, x1;
    std::ostringstream oss;
    std::istringstream iss;
    std::string data;
    assert(x0.setCell(CPos("A1"), "10"));
    assert(x0.setCell(CPos("A2"), "20.5"));
    assert(x0.setCell(CPos("A3"), "3e1"));
    assert(x0.setCell(CPos("A4"), "=40"));
    assert(x0.setCell(CPos("A5"), "=5e+1"));
    assert(x0.setCell(CPos("A6"), "raw text with any characters, including a quote \" or a newline\n"));
    assert(x0.setCell(CPos("A7"), "=\"quoted string, quotes must be doubled: \"\". Moreover, backslashes are needed for C++.\""));
    assert(valueMatch(x0.getValue(CPos("A1")), CValue(10.0)));
    assert(valueMatch(x0.getValue(CPos("A2")), CValue(20.5)));
    assert(valueMatch(x0.getValue(CPos("A3")), CValue(30.0)));
    assert(valueMatch(x0.getValue(CPos("A4")), CValue(40.0)));
    assert(valueMatch(x0.getValue(CPos("A5")), CValue(50.0)));
    assert(valueMatch(x0.getValue(CPos("A6")), CValue("raw text with any characters, including a quote \" or a newline\n")));
    assert(valueMatch(x0.getValue(CPos("A7")), CValue("quoted string, quotes must be doubled: \". Moreover, backslashes are needed for C++.")));
    assert(valueMatch(x0.getValue(CPos("A8")), CValue()));
    assert(valueMatch(x0.getValue(CPos("AAAA9999")), CValue()));
    assert(x0.setCell(CPos("B1"), "=A1+A2*A3"));
    assert(x0.setCell(CPos("B2"), "= -A1 ^ 2 - A2 /  2   "));
    assert(x0.setCell(CPos("B3"), "= 2 ^ $A$1"));
    assert(x0.setCell(CPos("B4"), "=($A1+A$2)^2"));
    assert(x0.setCell(CPos("B5"), "=B1+B2+B3+B4"));
    assert(x0.setCell(CPos("B6"), "=B1+B2+B3+B4+B5"));
    assert(valueMatch(x0.getValue(CPos("B1")), CValue(625.0)));
    assert(valueMatch(x0.getValue(CPos("B2")), CValue(-110.25)));
    assert(valueMatch(x0.getValue(CPos("B3")), CValue(1024.0)));
    assert(valueMatch(x0.getValue(CPos("B4")), CValue(930.25)));
    assert(valueMatch(x0.getValue(CPos("B5")), CValue(2469.0)));
    assert(valueMatch(x0.getValue(CPos("B6")), CValue(4938.0)));
    assert(x0.setCell(CPos("A1"), "12"));
    assert(valueMatch(x0.getValue(CPos("B1")), CValue(627.0)));
    assert(valueMatch(x0.getValue(CPos("B2")), CValue(-154.25)));
    assert(valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert(valueMatch(x0.getValue(CPos("B4")), CValue(1056.25)));
    assert(valueMatch(x0.getValue(CPos("B5")), CValue(5625.0)));
    assert(valueMatch(x0.getValue(CPos("B6")), CValue(11250.0)));
    x1 = x0;
    assert(x0.setCell(CPos("A2"), "100"));
    assert(x1.setCell(CPos("A2"), "=A3+A5+A4"));
    assert(valueMatch(x0.getValue(CPos("B1")), CValue(3012.0)));
    assert(valueMatch(x0.getValue(CPos("B2")), CValue(-194.0)));
    assert(valueMatch(x0.getValue(CPos("B3")), CValue(4096.0)));
    assert(valueMatch(x0.getValue(CPos("B4")), CValue(12544.0)));
    assert(valueMatch(x0.getValue(CPos("B5")), CValue(19458.0)));
    assert(valueMatch(x0.getValue(CPos("B6")), CValue(38916.0)));
    assert(valueMatch(x1.getValue(CPos("B1")), CValue(3612.0)));
    assert(valueMatch(x1.getValue(CPos("B2")), CValue(-204.0)));
    assert(valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert(valueMatch(x1.getValue(CPos("B4")), CValue(17424.0)));
    assert(valueMatch(x1.getValue(CPos("B5")), CValue(24928.0)));
    assert(valueMatch(x1.getValue(CPos("B6")), CValue(49856.0)));
    oss.clear();
    oss.str("");
    assert(x0.save(oss));
    data = oss.str();
    iss.clear();
    iss.str(data);
    assert(x1.load(iss));
    assert(valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert(valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert(valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert(valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert(valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert(valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    assert(x0.setCell(CPos("A3"), "4e1"));
    assert(valueMatch(x1.getValue(CPos("B1")), CValue(3012.0)));
    assert(valueMatch(x1.getValue(CPos("B2")), CValue(-194.0)));
    assert(valueMatch(x1.getValue(CPos("B3")), CValue(4096.0)));
    assert(valueMatch(x1.getValue(CPos("B4")), CValue(12544.0)));
    assert(valueMatch(x1.getValue(CPos("B5")), CValue(19458.0)));
    assert(valueMatch(x1.getValue(CPos("B6")), CValue(38916.0)));
    oss.clear();
    oss.str("");
    assert(x0.save(oss));
    data = oss.str();
    for (size_t i = 0; i < std::min<size_t>(data.length(), 10); i++)
      data[i] ^= 0x5a;
    iss.clear();
    iss.str(data);
    assert(!x1.load(iss));
    assert(x0.setCell(CPos("D0"), "10"));
    assert(x0.setCell(CPos("D1"), "20"));
    assert(x0.setCell(CPos("D2"), "30"));
    assert(x0.setCell(CPos("D3"), "40"));
    assert(x0.setCell(CPos("D4"), "50"));
    assert(x0.setCell(CPos("E0"), "60"));
    assert(x0.setCell(CPos("E1"), "70"));
    assert(x0.setCell(CPos("E2"), "80"));
    assert(x0.setCell(CPos("E3"), "90"));
    assert(x0.setCell(CPos("E4"), "100"));
    assert(x0.setCell(CPos("F10"), "=D0+5"));
    assert(x0.setCell(CPos("F11"), "=$D0+5"));
    assert(x0.setCell(CPos("F12"), "=D$0+5"));
    assert(x0.setCell(CPos("F13"), "=$D$0+5"));
    x0.copyRect(CPos("G11"), CPos("F10"), 1, 4);
    assert(valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert(valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert(valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert(valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert(valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert(valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    x0.copyRect(CPos("G11"), CPos("F10"), 2, 4);
    assert(valueMatch(x0.getValue(CPos("F10")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F11")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F12")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F13")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("F14")), CValue()));
    assert(valueMatch(x0.getValue(CPos("G10")), CValue()));
    assert(valueMatch(x0.getValue(CPos("G11")), CValue(75.0)));
    assert(valueMatch(x0.getValue(CPos("G12")), CValue(25.0)));
    assert(valueMatch(x0.getValue(CPos("G13")), CValue(65.0)));
    assert(valueMatch(x0.getValue(CPos("G14")), CValue(15.0)));
    assert(valueMatch(x0.getValue(CPos("H10")), CValue()));
    assert(valueMatch(x0.getValue(CPos("H11")), CValue()));
    assert(valueMatch(x0.getValue(CPos("H12")), CValue()));
    assert(valueMatch(x0.getValue(CPos("H13")), CValue(35.0)));
    assert(valueMatch(x0.getValue(CPos("H14")), CValue()));
    assert(x0.setCell(CPos("F0"), "-27"));
    assert(valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));
    x0.copyRect(CPos("H12"), CPos("H13"), 1, 2);
    assert(valueMatch(x0.getValue(CPos("H12")), CValue(25.0)));
    assert(valueMatch(x0.getValue(CPos("H13")), CValue(-22.0)));
    assert(valueMatch(x0.getValue(CPos("H14")), CValue(-22.0)));
    assert(x0.setCell(CPos("F0"), "=1-(---27)"));
    assert(valueMatch(x0.getValue(CPos("F0")), CValue(28.0)));


    CSpreadsheet x2;
    assert(x2.setCell(CPos("A1"), "1"));
    assert(x2.setCell(CPos("A2"), "2"));
    assert(x2.setCell(CPos("A3"), "3"));
    assert(x2.setCell(CPos("A4"), "4"));
    assert(x2.setCell(CPos("B1"), "=A1"));
    assert(x2.setCell(CPos("B2"), "=A4"));
    assert(x2.setCell(CPos("B3"), "a"));
    assert(x2.setCell(CPos("B4"), "b"));
    x2.copyRect(CPos("D1"), CPos("A1"), 2, 4);
    assert(valueMatch(x2.getValue(CPos("A1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("A2")), CValue(2.0)));
    assert(valueMatch(x2.getValue(CPos("A3")), CValue(3.0)));
    assert(valueMatch(x2.getValue(CPos("A4")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("B1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("B2")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("B3")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("B4")), CValue("b")));

    assert(valueMatch(x2.getValue(CPos("D1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("D2")), CValue(2.0)));
    assert(valueMatch(x2.getValue(CPos("D3")), CValue(3.0)));
    assert(valueMatch(x2.getValue(CPos("D4")), CValue(4.0)));

    assert(valueMatch(x2.getValue(CPos("E1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("E2")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("E3")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("E4")), CValue("b")));

    x2.copyRect(CPos("A2"), CPos("D1"), 2, 4);
    assert(valueMatch(x2.getValue(CPos("A1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("A2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("A3")), CValue(2.0)));
    assert(valueMatch(x2.getValue(CPos("A4")), CValue(3.0)));
    assert(valueMatch(x2.getValue(CPos("A5")), CValue(4.0)));

    assert(valueMatch(x2.getValue(CPos("B1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("B2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("B3")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("B4")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("B5")), CValue("b")));

    x2.copyRect(CPos("C1"), CPos("A1"), 2, 5);
    assert(valueMatch(x2.getValue(CPos("A1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("A2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("A3")), CValue(2.0)));
    assert(valueMatch(x2.getValue(CPos("A4")), CValue(3.0)));
    assert(valueMatch(x2.getValue(CPos("A5")), CValue(4.0)));

    assert(valueMatch(x2.getValue(CPos("B1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("B2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("B3")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("B4")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("B5")), CValue("b")));

    assert(valueMatch(x2.getValue(CPos("C1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("C2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("C3")), CValue(2.0)));
    assert(valueMatch(x2.getValue(CPos("C4")), CValue(3.0)));
    assert(valueMatch(x2.getValue(CPos("C5")), CValue(4.0)));

    assert(valueMatch(x2.getValue(CPos("D1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("D2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("D3")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("D4")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("D5")), CValue("b")));

    assert(valueMatch(x2.getValue(CPos("E1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("E2")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("E3")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("E4")), CValue("b")));
    assert(valueMatch(x2.getValue(CPos("E5")), CValue()));

    x2.copyRect(CPos("A1"), CPos("A1"), 5, 5);
    assert(valueMatch(x2.getValue(CPos("A1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("A2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("A3")), CValue(2.0)));
    assert(valueMatch(x2.getValue(CPos("A4")), CValue(3.0)));
    assert(valueMatch(x2.getValue(CPos("A5")), CValue(4.0)));

    assert(valueMatch(x2.getValue(CPos("B1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("B2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("B3")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("B4")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("B5")), CValue("b")));

    assert(valueMatch(x2.getValue(CPos("C1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("C2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("C3")), CValue(2.0)));
    assert(valueMatch(x2.getValue(CPos("C4")), CValue(3.0)));
    assert(valueMatch(x2.getValue(CPos("C5")), CValue(4.0)));

    assert(valueMatch(x2.getValue(CPos("D1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("D2")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("D3")), CValue(4.0)));
    assert(valueMatch(x2.getValue(CPos("D4")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("D5")), CValue("b")));

    assert(valueMatch(x2.getValue(CPos("E1")), CValue(1.0)));
    assert(valueMatch(x2.getValue(CPos("E2")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("E3")), CValue("a")));
    assert(valueMatch(x2.getValue(CPos("E4")), CValue("b")));
    assert(valueMatch(x2.getValue(CPos("E5")), CValue()));

    CSpreadsheet x3;
    assert(x3.setCell(CPos("A0"), "1"));
    assert(x3.setCell(CPos("A1"), "2"));
    assert(x3.setCell(CPos("A2"), "1"));
    assert(x3.setCell(CPos("A3"), "2"));
    assert(x3.setCell(CPos("B0"), "=A0"));
    assert(x3.setCell(CPos("B1"), "=A1"));
    assert(x3.setCell(CPos("B2"), "1"));
    assert(x3.setCell(CPos("B3"), "2"));
    assert(valueMatch(x3.getValue(CPos("A0")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("A1")), CValue(2.0)));
    assert(valueMatch(x3.getValue(CPos("A2")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("A3")), CValue(2.0)));
    assert(valueMatch(x3.getValue(CPos("B0")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("B1")), CValue(2.0)));
    assert(valueMatch(x3.getValue(CPos("B2")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("B3")), CValue(2.0)));

    x3.copyRect(CPos("C0"), CPos("B1"), 1, 1);
    assert(valueMatch(x3.getValue(CPos("A0")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("A1")), CValue(2.0)));
    assert(valueMatch(x3.getValue(CPos("A2")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("A3")), CValue(2.0)));
    assert(valueMatch(x3.getValue(CPos("B0")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("B1")), CValue(2.0)));
    assert(valueMatch(x3.getValue(CPos("B2")), CValue(1.0)));
    assert(valueMatch(x3.getValue(CPos("B3")), CValue(2.0)));
    assert(valueMatch(x3.getValue(CPos("C0")), CValue(1.0)));
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
