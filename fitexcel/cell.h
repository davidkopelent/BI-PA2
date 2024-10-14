#ifndef __PROGTEST__
#include "expression.h"
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