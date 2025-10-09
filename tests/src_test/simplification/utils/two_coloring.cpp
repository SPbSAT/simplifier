#include "src/common/csat_types.hpp"
#include "src/structures/circuit/dag.hpp"
#include "src/parser/bench_to_circuit.hpp"

#include "src/simplification/utils/two_coloring.hpp"

#include <string>
#include <utility>

#include "gtest/gtest.h"

namespace
{

using namespace csat;
using namespace csat::simplification;
using namespace csat::utils;

std::pair<std::vector<std::vector<std::string>>, std::vector<std::vector<std::string>>> parse_colors
(
    TwoColoring& twoColoring,
    csat::utils::GateEncoder& encoder
) {
    std::vector<std::vector<std::string>> color_parents;
    std::vector<std::vector<std::string>> gates_by_color;
    for (size_t i = 0; i < twoColoring.getColorsNumber(); ++i) {
        color_parents.emplace_back();
        color_parents.back().push_back(encoder.decodeGate(twoColoring.colors[i].first_parent));
        color_parents.back().push_back(encoder.decodeGate(twoColoring.colors[i].second_parent));

        gates_by_color.emplace_back();
        for (GateId v: twoColoring.colors[i].getGates()) {
            gates_by_color.back().push_back(encoder.decodeGate(v));
        }
        std::sort(color_parents.back().begin(), color_parents.back().end());
        std::sort(gates_by_color.back().begin(), gates_by_color.back().end());
    }
    return {color_parents, gates_by_color};
}


TEST(TwoColoring, SmallTest)
{
        std::string const dag = "INPUT(0)\n"
                                "INPUT(1)\n"
                                "INPUT(2)\n"
                                "OUTPUT(7)\n"
                                "3 = NOT(0)\n"
                                "4 = AND(0, 1)\n"
                                "5 = XOR(3, 1)\n"
                                "6 = AND(4, 2)\n"
                                "7 = NAND(5, 6)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> circuit = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    TwoColoring twoColoring = TwoColoring(*circuit);
    auto [color_parents, gates_by_color] = parse_colors(twoColoring, encoder);

    ASSERT_EQ(twoColoring.getColorsNumber(), 3);
    ASSERT_EQ(color_parents[0], std::vector<std::string>({"0", "1"}));
    ASSERT_EQ(gates_by_color[0], std::vector<std::string>({"4", "5"}));
    ASSERT_EQ(color_parents[1], std::vector<std::string>({"2", "4"}));
    ASSERT_EQ(gates_by_color[1], std::vector<std::string>({"6"}));
    ASSERT_EQ(color_parents[2], std::vector<std::string>({"5", "6"}));
    ASSERT_EQ(gates_by_color[2], std::vector<std::string>({"7"}));
}


TEST(TwoColoring, OneBigColor)
{
        std::string const dag = "INPUT(0)\n"
                                "INPUT(1)\n"
                                "OUTPUT(8)\n"
                                "2 = NOT(0)\n"
                                "3 = NOT(1)\n"
                                "4 = AND(0, 3)\n"
                                "5 = XOR(0, 1)\n"
                                "6 = OR(1, 2)\n"
                                "7 = NOR(4, 5)\n"
                                "8 = XOR(7, 6)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> circuit = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    TwoColoring twoColoring = TwoColoring(*circuit);
    auto [color_parents, gates_by_color] = parse_colors(twoColoring, encoder);

    ASSERT_EQ(twoColoring.getColorsNumber(), 1);
    ASSERT_EQ(color_parents[0], std::vector<std::string>({"0", "1"}));
    ASSERT_EQ(gates_by_color[0], std::vector<std::string>({"4", "5", "6", "7", "8"}));
}


TEST(TwoColoring, LongNegationChains)
{
        std::string const dag = "INPUT(0)\n"
                                "INPUT(1)\n"
                                "OUTPUT(8)\n"
                                "2 = NOT(0)\n"
                                "3 = NOT(1)\n"
                                "4 = NOT(2)\n"
                                "5 = NOT(3)\n"
                                "6 = NOT(4)\n"
                                "7 = AND(1, 4)\n"
                                "8 = XOR(3, 6)\n"
                                "9 = NOR(7, 8)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> circuit = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    TwoColoring twoColoring = TwoColoring(*circuit);
    auto [color_parents, gates_by_color] = parse_colors(twoColoring, encoder);

    ASSERT_EQ(twoColoring.getColorsNumber(), 1);
    ASSERT_EQ(color_parents[0], std::vector<std::string>({"0", "1"}));
    ASSERT_EQ(gates_by_color[0], std::vector<std::string>({"7", "8", "9"}));
}


TEST(TwoColoring, BiggerTest)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "INPUT(3)\n"
                            "OUTPUT(12)\n"
                            "4 = NOT(0)\n"
                            "5 = NOT(1)\n"
                            "6 = AND(0, 5)\n"
                            "7 = XOR(4, 5)\n"
                            "8 = OR(6, 7)\n"
                            "9 = NAND(5, 2)\n"
                            "10 = NOR(3, 1)\n"
                            "11 = XOR(9, 10)\n"
                            "12 = OR(8, 11)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> circuit = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    TwoColoring twoColoring = TwoColoring(*circuit);
    auto [color_parents, gates_by_color] = parse_colors(twoColoring, encoder);

    ASSERT_EQ(twoColoring.getColorsNumber(), 5);
    ASSERT_EQ(color_parents[0], std::vector<std::string>({"0", "1"}));
    ASSERT_EQ(gates_by_color[0], std::vector<std::string>({"6", "7", "8"}));
    ASSERT_EQ(color_parents[1], std::vector<std::string>({"1", "2"}));
    ASSERT_EQ(gates_by_color[1], std::vector<std::string>({"9"}));
    ASSERT_EQ(color_parents[2], std::vector<std::string>({"1", "3"}));
    ASSERT_EQ(gates_by_color[2], std::vector<std::string>({"10"}));
    ASSERT_EQ(color_parents[3], std::vector<std::string>({"10", "9"}));
    ASSERT_EQ(gates_by_color[3], std::vector<std::string>({"11"}));
    ASSERT_EQ(color_parents[4], std::vector<std::string>({"11", "8"}));
    ASSERT_EQ(gates_by_color[4], std::vector<std::string>({"12"}));
}

}