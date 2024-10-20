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


TEST(RedundantGatesCleaner, SimpleTest)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "INPUT(3)\n"
                            "\n"
                            "OUTPUT(4)\n"
                            "\n"
                            "4 = AND(0, 2)\n"
                            "5 = OR(1, 3)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        RedundantGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 3);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(RedundantGatesCleaner, NotBreaksAnything)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "\n"
                            "OUTPUT(3)\n"
                            "OUTPUT(4)\n"
                            "\n"
                            "3 = AND(0, 1)\n"
                            "4 = OR(0, 2)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        RedundantGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 5);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(3), GateType::AND);
    ASSERT_EQ(circuit->getGateType(4), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({0, 2}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({3, 4}));
}


TEST(RedundantGatesCleaner, NotMarkedOutput)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "\n"
                            "OUTPUT(3)\n"
                            "\n"
                            "3 = AND(0, 1)\n"
                            "4 = OR(0, 2, 3)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        RedundantGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 3);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(RedundantGatesCleaner, ConnectedOutputs)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "\n"
                            "OUTPUT(3)\n"
                            "OUTPUT(5)\n"
                            "\n"
                            "3 = AND(0, 1)\n"
                            "4 = OR(3, 2)\n"
                            "5 = NOT(4)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        RedundantGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 6);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(3), GateType::AND);
    ASSERT_EQ(circuit->getGateType(4), GateType::NOT);
    ASSERT_EQ(circuit->getGateType(5), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({5}));
    ASSERT_EQ(circuit->getGateOperands(5), GateIdContainer({2, 3}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({3, 4}));
}

TEST(RedundantGatesCleaner, NewGatesMUXandCONST)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "\n"
                            "OUTPUT(3)\n"
                            "OUTPUT(4)\n"
                            "\n"
                            "3 = AND(0, 1)\n"
                            "4 = MUX(0, 1, 5)\n"
                            "5 = CONST(0)\n"
                            "6 = MUX(3, 4, 2)\n"
                            "7 = CONST(1)";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        RedundantGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 5);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateType(3), GateType::MUX);
    ASSERT_EQ(circuit->getGateType(4), GateType::CONST_FALSE);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 1, 4}));
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2, 3}));

}

} // namespace
