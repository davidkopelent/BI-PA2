#include "cell.h"

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