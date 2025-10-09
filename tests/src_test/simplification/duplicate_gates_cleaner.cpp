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


TEST(DuplicateGatesCleaner, SimpleTest)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(4)\n"
                            "2 = AND(0, 1)\n"
                            "3 = AND(0, 1)\n"
                            "4 = AND(2, 3)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        DuplicateGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 4);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateType(3), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({2, 2}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({3}));
}


TEST(DuplicateGatesCleaner, NoSorted)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(4)\n"
                            "2 = AND(0, 1)\n"
                            "3 = AND(1, 0)\n"
                            "4 = AND(2, 3)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        DuplicateGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 4);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateType(3), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({2, 2}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({3}));
}


TEST(DuplicateGatesCleaner, SeveralLevels)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "\n"
                            "OUTPUT(11)\n"
                            "\n"
                            "3 = OR(0, 1)\n"
                            "4 = AND(0, 1)\n"
                            "5 = AND(1, 0)\n"
                            "6 = NOT(2)\n"
                            "7 = OR(3, 4)\n"
                            "8 = OR(3, 5)\n"
                            "9 = NOT(7)\n"
                            "10 = OR(6, 8)\n"
                            "11 = AND(9, 10)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        DuplicateGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 10);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateType(3), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateType(4), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({2, 3}));
    ASSERT_EQ(circuit->getGateType(5), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(5), GateIdContainer({4}));
    ASSERT_EQ(circuit->getGateType(6), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(7), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(7), GateIdContainer({6}));
    ASSERT_EQ(circuit->getGateType(8), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(8), GateIdContainer({4, 7}));
    ASSERT_EQ(circuit->getGateType(9), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(9), GateIdContainer({5, 8}));
}


TEST(DuplicateGatesCleaner, SeveralOutput)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "OUTPUT(2)\n"
                            "OUTPUT(3)\n"
                            "2 = AND(0, 1)\n"
                            "3 = AND(1, 0)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        DuplicateGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 3);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2, 2}));
}

TEST(DuplicateGatesCleaner, SeveralLevelsMUX)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "\n"
                            "OUTPUT(11)\n"
                            "\n"
                            "3 = MUX(0, 1, 12)\n"
                            "4 = MUX(0, 12, 1)\n"
                            "5 = MUX(0, 12, 1)\n"
                            "6 = NOT(2)\n"
                            "7 = OR(3, 4)\n"
                            "8 = OR(5, 3)\n"
                            "9 = NOT(7)\n"
                            "10 = OR(6, 8)\n"
                            "11 = AND(9, 10)\n"
                            "12 = CONST(0)";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        DuplicateGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(), 11);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::CONST_FALSE);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({}));
    ASSERT_EQ(circuit->getGateType(3), GateType::MUX);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 1, 2}));
    ASSERT_EQ(circuit->getGateType(4), GateType::MUX);
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({0, 2, 1}));
    ASSERT_EQ(circuit->getGateType(5), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(5), GateIdContainer({3, 4}));
    ASSERT_EQ(circuit->getGateType(6), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(6), GateIdContainer({5}));
    ASSERT_EQ(circuit->getGateType(7), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(8), GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(8), GateIdContainer({7}));
    ASSERT_EQ(circuit->getGateType(9), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(9), GateIdContainer({5, 8}));
    ASSERT_EQ(circuit->getGateType(10), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(10), GateIdContainer({6, 9}));
}


TEST(DuplicateGatesCleaner, ReducibleDuplicateOfOperands)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "\n"
                            "2 = AND(1, 0)\n"
                            "3 = AND(0, 1)\n"
                            "4 = AND(0, 2, 3)\n"
                            "5 = AND(0, 2)\n"
                            "6 = OR(4, 5)\n"
                            "\n"
                            "OUTPUT(6)\n";

    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        DuplicateGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);

    ASSERT_EQ(circuit->getNumberOfGates(), 5);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateType(3), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 2, 2}));
    ASSERT_EQ(circuit->getGateType(4), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({3, 3}));
}

TEST(DuplicateGatesCleaner, XorNotReducible)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "\n"
                            "2 = AND(1, 0)\n"
                            "3 = AND(0, 1)\n"
                            "4 = XOR(0, 2, 3)\n"
                            "5 = XOR(0, 2)\n"
                            "6 = OR(4, 5)\n"
                            "\n"
                            "OUTPUT(6)\n";

    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);

    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();

    auto [circuit, _] = Composition<
        DAG,
        DuplicateGatesCleaner<DAG>
    >().apply(*csat_instance, encoder);

    ASSERT_EQ(circuit->getNumberOfGates(), 6);
    ASSERT_EQ(circuit->getGateType(0), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1), GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2), GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2), GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateType(3), GateType::XOR);
    ASSERT_EQ(circuit->getGateOperands(3), GateIdContainer({0, 2, 2}));
    ASSERT_EQ(circuit->getGateType(4), GateType::XOR);
    ASSERT_EQ(circuit->getGateOperands(4), GateIdContainer({0, 2}));
    ASSERT_EQ(circuit->getGateType(5), GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(5), GateIdContainer({3, 4}));
}

} // namespace
