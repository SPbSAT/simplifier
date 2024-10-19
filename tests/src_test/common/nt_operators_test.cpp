#include "src/common/csat_types.hpp"
#include "src/common/operators.hpp"

#include <gtest/gtest.h>

namespace
{

TEST(OperatorsNTTest, GetOperator)
{
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::NOT) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::NOT<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::AND) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::AND<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::NAND) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::NAND<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::OR) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::OR<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::NOR) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::NOR<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::XOR) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::XOR<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::NXOR) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::NXOR<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::IFF) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::IFF<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::MUX) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::MUX<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::CONST_FALSE) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::CONST_FALSE<csat::GateId>);
    ASSERT_TRUE(csat::op::getOperatorNT<csat::GateId>(csat::GateType::CONST_TRUE) ==
                (csat::op::OperatorNT<csat::GateId>) &csat::op::CONST_TRUE<csat::GateId>);
}

} // namespace
