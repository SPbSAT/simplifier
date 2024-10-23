#include "src/common/csat_types.hpp"
#include "src/structures/circuit/dag.hpp"
#include "src/parser/bench_to_circuit.hpp"

#include "src/simplification/composition.hpp"
#include "src/simplification/strategy.hpp"

#include <string>

#include "gtest/gtest.h"

namespace
{

using namespace csat;
using namespace csat::simplification;


TEST(ReduceNotComposition, SimpleTest)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(9)\n"
                            "6 = NOT(0)\n"
                            "7 = NOT(6)\n"
                            "8 = NOT(7)\n"
                            "9 = AND(8, 1)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::ReduceNotComposition<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 4);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({1, 3}));
    ASSERT_EQ(circuit->getGateType(3), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(ReduceNotComposition, UseMiddleNot)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(7)\n"
                            "2 = NOT(0)\n"
                            "3 = NOT(2)\n"
                            "4 = NOT(3)\n"
                            "5 = NOT(4)\n"
                            "6 = MUX(4, 0, 1)\n"
                            "7 = AND(5, 6)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::ReduceNotComposition<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 5);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 4}));
    ASSERT_EQ(circuit->getGateType(3), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(4), GateType::MUX);
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({3, 0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(ReduceNotComposition, NotIsOutput)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(8)\n"
                            "OUTPUT(9)\n"
                            "6 = NOT(0)\n"
                            "7 = NOT(6)\n"
                            "8 = NOT(7)\n"
                            "9 = AND(7, 1)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::ReduceNotComposition<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 4);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(3), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2, 3}));
}


TEST(ReduceNotComposition, NoChanges)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(9)\n"
                            "6 = NOT(0)\n"
                            "7 = NOR(6, 0)\n"
                            "8 = NOT(7)\n"
                            "9 = AND(8, 1)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::ReduceNotComposition<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 6);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({1, 5}));
    ASSERT_EQ(circuit->getGateType(3), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(4), GateType::NOR);
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({0, 3}));
    ASSERT_EQ(circuit->getGateType(5), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(5), GateIdContainer({4}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}

} // namespace
