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


TEST(ConstantGateReducer, SimpleTest)
{
    auto csat_instance = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::CONST_TRUE, {}},
            {csat::GateType::AND, {0, 3}},
            {csat::GateType::OR, {1, 2, 4}}
        },
        {5}
    );
    
    GateEncoder<std::string> encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("1");
    encoder.encodeGate("2");
    encoder.encodeGate("5");
    encoder.encodeGate("3");
    encoder.encodeGate("4");
    
    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  4);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(3),  GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(3),  GateIdContainer({0, 1, 2}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({3}));
}


TEST(ConstantGateReducer, ChangeOutput)
{
    auto csat_instance = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::CONST_TRUE, {}},
            {csat::GateType::AND, {0, 1}}
        },
        {2}
    );
    
    GateEncoder<std::string> encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("2");
    encoder.encodeGate("1");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    ASSERT_EQ(circuit->getNumberOfGates(),  1);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({0}));
}


TEST(ConstantGateReducer, KnownAnswer)
{
    auto csat_instance = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::CONST_TRUE, {}},
            {csat::GateType::OR, {0, 1}}
        },
        {2}
    );
    
    GateEncoder<std::string> encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("2");
    encoder.encodeGate("1");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(2),  GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(ConstantGateReducer, NoChanges)
{
    auto csat_instance = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::OR, {0, 1}}
        },
        {2}
    );
    
    GateEncoder<std::string> encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("1");
    encoder.encodeGate("2");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    ASSERT_EQ(circuit->getNumberOfGates(),  3);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(2),  GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2}));
}


TEST(ConstantGateReducer, UnusedGates)
{
    auto csat_instance = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::CONST_TRUE, {}},
            {csat::GateType::OR, {0, 1}},
            {csat::GateType::CONST_TRUE, {}},
            {csat::GateType::AND, {0, 3}}
        },
        {4}
    );
    
    GateEncoder<std::string> encoder{};
    encoder.encodeGate("0");
    encoder.encodeGate("1");
    encoder.encodeGate("2");

    auto [circuit, _] = Composition<
        DAG,
        ConstantGateReducer<DAG>
    >().apply(csat_instance, encoder);

    ASSERT_EQ(circuit->getNumberOfGates(),  1);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({0}));
}


TEST(ConstantGateReducer, SeveralOutputs)
{
    std::string const dag = "INPUT(0)\n"
                            "OUTPUT(2)\n"
                            "OUTPUT(4)\n"
                            "1 = CONST(1)\n"
                            "2 = MUX(1, 0, 1)\n"
                            "3 = NOT(1)\n"
                            "4 = AND(1, 3)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> csat_instance = parser.instantiate();
    csat::utils::GateEncoder<std::string> encoder = parser.getEncoder();
    
    auto [circuit, _] = Composition<
        DAG,
        csat::simplification::ConstantGateReducer<csat::DAG>
    >().apply(*csat_instance, encoder);
    
    ASSERT_EQ(circuit->getNumberOfGates(),  4);
    ASSERT_EQ(circuit->getGateType(0),  GateType::INPUT);
    ASSERT_EQ(circuit->getGateType(1),  GateType::NOT);
    ASSERT_EQ(circuit->getGateOperands(1),  GateIdContainer({0}));
    ASSERT_EQ(circuit->getGateType(2),  GateType::OR);
    ASSERT_EQ(circuit->getGateOperands(2),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getGateType(3),  GateType::AND);
    ASSERT_EQ(circuit->getGateOperands(3),  GateIdContainer({0, 1}));
    ASSERT_EQ(circuit->getOutputGates(), GateIdContainer({2, 3}));
}

TEST(ConstantGateReducer, SaveCONST)
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
