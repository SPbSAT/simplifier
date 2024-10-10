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


TEST(DuplicateOperandsCleaner, KnownAnswer1)
{
    std::string const dag = "INPUT(0)\n"
                            "1 = NOT(0)\n"
                            "2 = AND(1, 0)\n"
                            "3 = AND(2, 1)\n"
                            "4 = AND(3, 2)\n"
                            "OUTPUT(4)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(2),  GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(DuplicateOperandsCleaner, Bamboo)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "3 = OR(0, 0)\n"
                            "4 = OR(3, 3)\n"
                            "5 = AND(4, 0)\n"
                            "6 = AND(5, 1)\n"
                            "OUTPUT(6)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2),  GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(DuplicateOperandsCleaner, CreateNOT)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "2 = NAND(0, 0)\n"
                            "3 = AND(2, 2)\n"
                            "4 = AND(3, 1)\n"
                            "OUTPUT(4)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  4);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({1}));
    ASSERT_EQ(circuit->getGateType(3),  GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(3),  GateIdContainer({0, 2}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({3}));
}


TEST(DuplicateOperandsCleaner, MaxReductions)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "INPUT(3)\n"
                            "4 = NAND(0, 0)\n"
                            "5 = AND(4, 4)\n"
                            "6 = AND(5, 1)\n"
                            "7 = NAND(6, 2)\n"
                            "8 = NOR(7, 7)\n"
                            "9 = AND(8, 3)\n"
                            "OUTPUT(9)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  9);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(3),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(4),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(4),  GateIdContainer({3}));
    ASSERT_EQ(circuit->getGateType(5),  GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(5),  GateIdContainer({2, 4}));
    ASSERT_EQ(circuit->getGateType(6),  GateType::NAND);
    ASSERT_EQ(circuit->getGateOperands(6),  GateIdContainer({1, 5}));
    ASSERT_EQ(circuit->getGateType(7),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(7),  GateIdContainer({6}));
    ASSERT_EQ(circuit->getGateType(8),  GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(8),  GateIdContainer({0, 7}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({8}));
}


TEST(DuplicateOperandsCleaner, ChangeOutput1)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "2 = NOT(0)\n"
                            "3 = AND(2, 0)\n"
                            "4 = NOT(3)\n"
                            "5 = XOR(4, 1)\n"
                            "OUTPUT(5)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, encoder_] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  2);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(encoder_->decodeGate(0),  "1");
    ASSERT_EQ(circuit->getGateType(1),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({0}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({1}));
}


TEST(DuplicateOperandsCleaner, ChangeOutputsType)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "3 = NOT(0)\n"
                            "4 = AND(3, 0)\n"
                            "5 = NOT(4)\n"
                            "6 = XOR(5, 1, 2)\n"
                            "OUTPUT(6)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, encoder_] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(encoder_->decodeGate(0),  "1");
    ASSERT_EQ(circuit->getGateType(1),  GateType::INPUT);
    ASSERT_EQ(encoder_->decodeGate(1),  "2");
    ASSERT_EQ(circuit->getGateType(2),  GateType::NXOR);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(DuplicateOperandsCleaner, KnownAnswer2)
{
    std::string const dag = "INPUT(0)\n"
                            "1 = XOR(0, 0)\n"
                            "OUTPUT(1)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(2),  GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(DuplicateOperandsCleaner, ChangeOutput2)
{
    std::string const dag = "INPUT(0)\n"
                            "1 = NAND(0, 0)\n"
                            "2 = NOT(1)\n"
                            "3 = XOR(0, 1, 2)\n"
                            "OUTPUT(3)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  2);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({0}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({1}));
}


TEST(DuplicateOperandsCleaner, InputIsOutput)
{
    std::string const dag = "INPUT(0)\n"
                            "1 = NAND(0, 0)\n"
                            "2 = NOT(1)\n"
                            "3 = NAND(0, 0)\n"
                            "4 = NOT(3)\n"
                            "5 = XOR(0, 1, 2, 3, 4)\n"
                            "OUTPUT(5)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  1);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({0}));
}


TEST(DuplicateOperandsCleaner, SeveralOutputs)
{
    std::string const dag = "INPUT(0)\n"
                            "1 = NAND(0, 0)\n"
                            "2 = NOT(0)\n"
                            "3 = NAND(2, 2)\n"
                            "4 = XOR(0, 1, 2, 3)\n"
                            "5 = AND(0, 0)\n"
                            "OUTPUT(4)\n"
                            "OUTPUT(5)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::DuplicateOperandsCleaner<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(2),  GateType::NXOR);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2, 0}));
}

TEST(DuplicateOperandsCleaner, SaveCONST)
{
    std::string const dag = "INPUT(0)\n"
                            "OUTPUT(2)\n"
                            "OUTPUT(3)\n"
                            "1 = CONST(1)\n"
                            "2 = MUX(1, 4, 0)\n"
                            "3 = MUX(0, 1, 2)\n"
                            "4 = CONST(1)";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::ConstantGateReducer<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::CONST_TRUE);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({}));
    ASSERT_EQ(circuit->getGateType(2),  GateType::MUX);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1, 0}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({0, 2}));
}

} // anonymous namespace
