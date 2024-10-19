#include "src/common/csat_types.hpp"
#include "src/structures/assignment/vector_assignment.hpp"

#include "gtest/gtest.h"

namespace {

TEST(VectorAssignmentTest, Set)
{
    csat::VectorAssignment<> assignment{};
    assignment.assign(1, csat::GateState::TRUE);
    assignment.assign(2, csat::GateState::FALSE);
    assignment.assign(3, csat::GateState::UNDEFINED);
    assignment.assign(10, csat::GateState::FALSE);
    ASSERT_TRUE(assignment.getGateState(1) == csat::GateState::TRUE);
    ASSERT_TRUE(assignment.getGateState(2) == csat::GateState::FALSE);
    ASSERT_TRUE(assignment.getGateState(3) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(assignment.getGateState(10) == csat::GateState::FALSE);
}

TEST(VectorAssignmentTest, Basic)
{
    csat::VectorAssignment<> asmt{};
    asmt.assign(2, csat::GateState::TRUE);
    ASSERT_TRUE(asmt.getGateState(2) == csat::GateState::TRUE);
    asmt.assign(1, csat::GateState::FALSE);
    ASSERT_TRUE(asmt.getGateState(1) == csat::GateState::FALSE);
    asmt.assign(2, csat::GateState::FALSE);
    ASSERT_TRUE(asmt.getGateState(2) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt.getGateState(3) == csat::GateState::UNDEFINED);
}

TEST(VectorAssignmentTest, Clear)
{
    csat::VectorAssignment<> assignment{};
    assignment.assign(1, csat::GateState::TRUE);
    assignment.assign(2, csat::GateState::FALSE);
    assignment.assign(3, csat::GateState::UNDEFINED);
    ASSERT_TRUE(assignment.getGateState(1) == csat::GateState::TRUE);
    ASSERT_TRUE(assignment.getGateState(2) == csat::GateState::FALSE);
    ASSERT_TRUE(assignment.getGateState(3) == csat::GateState::UNDEFINED);
    assignment.clear();
    ASSERT_TRUE(assignment.getGateState(1) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(assignment.getGateState(2) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(assignment.getGateState(3) == csat::GateState::UNDEFINED);
}

} // namespace
