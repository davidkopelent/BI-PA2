/******************************************************
 * Filename: builder.cpp
 * Author: David Kopelent
 * Date: 24.04.2024
 * Description: This code implements methods for building AST of parsed expression
 * 
 ******************************************************/

#include "builder.h"

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