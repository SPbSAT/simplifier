#include "src/common/csat_types.hpp"
#include "src/common/operators.hpp"

#include <gtest/gtest.h>

namespace
{

TEST(OperatorsTest, NOT)
{
    ASSERT_TRUE(csat::op::NOT(csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NOT(csat::GateState::FALSE)     == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NOT(csat::GateState::TRUE)      == csat::GateState::FALSE);
}

TEST(OperatorsTest, AND)
{
    ASSERT_TRUE(csat::op::AND(csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::AND(csat::GateState::UNDEFINED, csat::GateState::FALSE)     == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::AND(csat::GateState::UNDEFINED, csat::GateState::TRUE)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::AND(csat::GateState::FALSE, csat::GateState::UNDEFINED)     == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::AND(csat::GateState::FALSE, csat::GateState::FALSE)         == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::AND(csat::GateState::FALSE, csat::GateState::TRUE)          == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::AND(csat::GateState::TRUE, csat::GateState::UNDEFINED)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::AND(csat::GateState::TRUE, csat::GateState::FALSE)          == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::AND(csat::GateState::TRUE, csat::GateState::TRUE)           == csat::GateState::TRUE);
}

TEST(OperatorsTest, OR)
{
    ASSERT_TRUE(csat::op::OR(csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::OR(csat::GateState::UNDEFINED, csat::GateState::FALSE)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::OR(csat::GateState::UNDEFINED, csat::GateState::TRUE)      == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::OR(csat::GateState::FALSE, csat::GateState::UNDEFINED)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::OR(csat::GateState::FALSE, csat::GateState::FALSE)         == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::OR(csat::GateState::FALSE, csat::GateState::TRUE)          == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::OR(csat::GateState::TRUE, csat::GateState::UNDEFINED)      == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::OR(csat::GateState::TRUE, csat::GateState::FALSE)          == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::OR(csat::GateState::TRUE, csat::GateState::TRUE)           == csat::GateState::TRUE);
}

TEST(OperatorsTest, XOR)
{
    ASSERT_TRUE(csat::op::XOR(csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::UNDEFINED, csat::GateState::FALSE)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::UNDEFINED, csat::GateState::TRUE)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::FALSE, csat::GateState::UNDEFINED)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::FALSE, csat::GateState::FALSE)         == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::FALSE, csat::GateState::TRUE)          == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::TRUE, csat::GateState::UNDEFINED)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::TRUE, csat::GateState::FALSE)          == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::XOR(csat::GateState::TRUE, csat::GateState::TRUE)           == csat::GateState::FALSE);
}

TEST(OperatorsTest, NAND)
{
    ASSERT_TRUE(csat::op::NAND(csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::UNDEFINED, csat::GateState::FALSE)     == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::UNDEFINED, csat::GateState::TRUE)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::FALSE, csat::GateState::UNDEFINED)     == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::FALSE, csat::GateState::FALSE)         == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::FALSE, csat::GateState::TRUE)          == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::TRUE, csat::GateState::UNDEFINED)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::TRUE, csat::GateState::FALSE)          == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NAND(csat::GateState::TRUE, csat::GateState::TRUE)           == csat::GateState::FALSE);
}

TEST(OperatorsTest, NOR)
{
    ASSERT_TRUE(csat::op::NOR(csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::UNDEFINED, csat::GateState::FALSE)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::UNDEFINED, csat::GateState::TRUE)      == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::FALSE, csat::GateState::UNDEFINED)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::FALSE, csat::GateState::FALSE)         == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::FALSE, csat::GateState::TRUE)          == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::TRUE, csat::GateState::UNDEFINED)      == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::TRUE, csat::GateState::FALSE)          == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::NOR(csat::GateState::TRUE, csat::GateState::TRUE)           == csat::GateState::FALSE);
}

TEST(OperatorsTest, NXOR)
{
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::UNDEFINED, csat::GateState::FALSE)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::UNDEFINED, csat::GateState::TRUE)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::FALSE, csat::GateState::UNDEFINED)     == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::FALSE, csat::GateState::FALSE)         == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::FALSE, csat::GateState::TRUE)          == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::TRUE, csat::GateState::UNDEFINED)      == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::TRUE, csat::GateState::FALSE)          == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::NXOR(csat::GateState::TRUE, csat::GateState::TRUE)           == csat::GateState::TRUE);
}

TEST(OperatorsTest, IFF)
{
    ASSERT_TRUE(csat::op::IFF(csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::IFF(csat::GateState::FALSE) == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::IFF(csat::GateState::TRUE) == csat::GateState::TRUE);
}

TEST(OperatorsTest, MUX_F)
{
    csat::GateState logic = csat::GateState::FALSE;
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::FALSE) == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::FALSE) == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::FALSE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::TRUE) == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::TRUE) == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::TRUE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::UNDEFINED) == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::UNDEFINED) == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
}

TEST(OperatorsTest, MUX_T)
{
    csat::GateState logic = csat::GateState::TRUE;
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::FALSE) == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::FALSE) == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::FALSE) == csat::GateState::FALSE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::TRUE) == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::TRUE) == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::TRUE) == csat::GateState::TRUE);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
}

TEST(OperatorsTest, MUX_U)
{
    csat::GateState logic = csat::GateState::UNDEFINED;
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::FALSE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::FALSE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::FALSE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::TRUE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::TRUE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::TRUE) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::FALSE, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::TRUE, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(csat::op::MUX(logic, csat::GateState::UNDEFINED, csat::GateState::UNDEFINED) == csat::GateState::UNDEFINED);
}

TEST(OperatorsTest, ConstFalse)
{
    ASSERT_TRUE(csat::op::CONST_FALSE() == csat::GateState::FALSE);
}

TEST(OperatorsTest, ConstTrue)
{
    ASSERT_TRUE(csat::op::CONST_TRUE() == csat::GateState::TRUE);
}

TEST(OperatorsTest, GetOperator)
{
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::NOT) == (csat::op::Operator) &csat::op::NOT);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::AND) == (csat::op::Operator) &csat::op::AND);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::NAND) == (csat::op::Operator) &csat::op::NAND);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::OR) == (csat::op::Operator) &csat::op::OR);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::NOR) == (csat::op::Operator) &csat::op::NOR);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::XOR) == (csat::op::Operator) &csat::op::XOR);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::NXOR) == (csat::op::Operator) &csat::op::NXOR);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::IFF) == (csat::op::Operator) &csat::op::IFF);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::MUX) == (csat::op::Operator) &csat::op::MUX);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::CONST_FALSE) == (csat::op::Operator) &csat::op::CONST_FALSE);
    ASSERT_TRUE(csat::op::getOperator(csat::GateType::CONST_TRUE) == (csat::op::Operator) &csat::op::CONST_TRUE);
}

} // anonymous namespace
