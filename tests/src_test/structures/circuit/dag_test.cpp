#include "src/common/csat_types.hpp"
#include "src/structures/circuit/dag.hpp"
#include "gtest/gtest.h"

namespace {

TEST(DAGTest, SimpleConstruction)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::AND, {0, 1}}
        },
        {2});
    ASSERT_TRUE(dag.getNumberOfGates() == 3);
    ASSERT_TRUE(dag.getGateUsers(0) == csat::GateIdContainer({2}));
    ASSERT_TRUE(dag.getGateUsers(1) == csat::GateIdContainer({2}));
}

TEST(DAGTest, Calculation)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::OR, {0, 1}}
        },
        {2});
    ASSERT_TRUE(dag.getGateUsers(0) == csat::GateIdContainer({2}));
    ASSERT_TRUE(dag.getGateUsers(1) == csat::GateIdContainer({2}));
    
    auto asmt = csat::VectorAssignment<>{};
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::UNDEFINED);
    asmt.assign(0, csat::GateState::TRUE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::TRUE);
    asmt.assign(1, csat::GateState::TRUE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::TRUE);
    asmt.assign(0, csat::GateState::FALSE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::TRUE);
    asmt.assign(1, csat::GateState::FALSE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::FALSE);
}

TEST(DAGTest, CalculationMultipleOutputs)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::OR, {0, 1}},
            {csat::GateType::AND, {0, 1}},
            {csat::GateType::XOR, {0, 1}}
        },
        {2, 3, 4});
    ASSERT_TRUE(dag.getGateUsers(0) == csat::GateIdContainer({2, 3, 4}));
    ASSERT_TRUE(dag.getGateUsers(1) == csat::GateIdContainer({2, 3, 4}));
    ASSERT_TRUE(dag.getGateUsers(2) == csat::GateIdContainer({}));
    ASSERT_TRUE(dag.getGateUsers(3) == csat::GateIdContainer({}));
    ASSERT_TRUE(dag.getGateUsers(4) == csat::GateIdContainer({}));
    
    auto asmt = csat::VectorAssignment<>{};
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::UNDEFINED);
    asmt.assign(0, csat::GateState::TRUE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::TRUE);
    asmt.assign(1, csat::GateState::TRUE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::TRUE);
    asmt.assign(0, csat::GateState::TRUE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(2) == csat::GateState::TRUE);
    asmt.assign(1, csat::GateState::FALSE);
    auto result_assignment = dag.evaluateCircuit(asmt);
    ASSERT_TRUE(result_assignment->getGateState(2) == csat::GateState::TRUE);
    ASSERT_TRUE(result_assignment->getGateState(3) == csat::GateState::FALSE);
    ASSERT_TRUE(result_assignment->getGateState(4) == csat::GateState::TRUE);
}

TEST(DAGTest, CalculationComplex)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::NOT, {1}},
            {csat::GateType::OR, {0, 3}},
            {csat::GateType::AND, {3, 2}},
            {csat::GateType::AND, {4, 5}}
        },
        {6});
    ASSERT_TRUE(dag.getGateUsers(0) == csat::GateIdContainer({4}));
    ASSERT_TRUE(dag.getGateUsers(1) == csat::GateIdContainer({3}));
    ASSERT_TRUE(dag.getGateUsers(2) == csat::GateIdContainer({5}));
    ASSERT_TRUE(dag.getGateUsers(3) == csat::GateIdContainer({4, 5}));
    ASSERT_TRUE(dag.getGateUsers(4) == csat::GateIdContainer({6}));
    ASSERT_TRUE(dag.getGateUsers(5) == csat::GateIdContainer({6}));
    ASSERT_TRUE(dag.getGateUsers(6) == csat::GateIdContainer({}));
    
    auto asmt = csat::VectorAssignment<>{};
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(6) == csat::GateState::UNDEFINED);
    asmt.assign(0, csat::GateState::TRUE);
    asmt.assign(1, csat::GateState::TRUE);
    asmt.assign(2, csat::GateState::TRUE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(6) == csat::GateState::FALSE);
    asmt.assign(1, csat::GateState::FALSE);
    ASSERT_TRUE(dag.evaluateCircuit(asmt)->getGateState(6) == csat::GateState::TRUE);
}

TEST(DAGTest, CalculationWithConstGates)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::CONST_FALSE, {}},
            {csat::GateType::CONST_TRUE, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::NOT, {1}},
            {csat::GateType::OR, {0, 3}},
            {csat::GateType::OR, {3, 2}},
            {csat::GateType::OR, {4, 5}}
        },
        {6});
    ASSERT_TRUE(dag.getGateUsers(0) == csat::GateIdContainer({4}));
    ASSERT_TRUE(dag.getGateUsers(1) == csat::GateIdContainer({3}));
    ASSERT_TRUE(dag.getGateUsers(2) == csat::GateIdContainer({5}));
    ASSERT_TRUE(dag.getGateUsers(3) == csat::GateIdContainer({4, 5}));
    ASSERT_TRUE(dag.getGateUsers(4) == csat::GateIdContainer({6}));
    ASSERT_TRUE(dag.getGateUsers(5) == csat::GateIdContainer({6}));
    ASSERT_TRUE(dag.getGateUsers(6) == csat::GateIdContainer({}));
    
    auto asmt = csat::VectorAssignment<>{};
    
    auto asmt_1 = dag.evaluateCircuit(asmt);
    ASSERT_TRUE(asmt_1->getGateState(0) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_1->getGateState(1) == csat::GateState::TRUE);
    ASSERT_TRUE(asmt_1->getGateState(2) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(asmt_1->getGateState(3) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_1->getGateState(4) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_1->getGateState(5) == csat::GateState::UNDEFINED);
    ASSERT_TRUE(asmt_1->getGateState(6) == csat::GateState::UNDEFINED);
    
    asmt.assign(2, csat::GateState::TRUE);
    auto asmt_2 = dag.evaluateCircuit(asmt);
    ASSERT_TRUE(asmt_2->getGateState(0) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_2->getGateState(1) == csat::GateState::TRUE);
    ASSERT_TRUE(asmt_2->getGateState(2) == csat::GateState::TRUE);
    ASSERT_TRUE(asmt_2->getGateState(3) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_2->getGateState(4) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_2->getGateState(5) == csat::GateState::TRUE);
    ASSERT_TRUE(asmt_2->getGateState(6) == csat::GateState::TRUE);
    
    asmt.assign(2, csat::GateState::FALSE);
    auto asmt_3 = dag.evaluateCircuit(asmt);
    ASSERT_TRUE(asmt_3->getGateState(0) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_3->getGateState(1) == csat::GateState::TRUE);
    ASSERT_TRUE(asmt_3->getGateState(2) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_3->getGateState(3) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_3->getGateState(4) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_3->getGateState(5) == csat::GateState::FALSE);
    ASSERT_TRUE(asmt_3->getGateState(6) == csat::GateState::FALSE);
}

TEST(DAGTest, CalculationManyOperands)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::AND, {0, 1, 2}},
            {csat::GateType::OR, {0, 1, 2}}
        },
        {});
    ASSERT_TRUE(dag.getGateUsers(0) == csat::GateIdContainer({3, 4}));
    ASSERT_TRUE(dag.getGateUsers(1) == csat::GateIdContainer({3, 4}));
    ASSERT_TRUE(dag.getGateUsers(2) == csat::GateIdContainer({3, 4}));
    ASSERT_TRUE(dag.getGateUsers(3) == csat::GateIdContainer({}));
    ASSERT_TRUE(dag.getGateUsers(4) == csat::GateIdContainer({}));
}

} // namespace
