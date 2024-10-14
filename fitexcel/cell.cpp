/******************************************************
 * Filename: cell.cpp
 * Author: David Kopelent
 * Date: 24.04.2024
 * Description: This code implements methods for cell representation and is responsible for building AST nodes
 * 
 ******************************************************/

#include "cell.h"

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