#include "src/common/csat_types.hpp"
#include "src/structures/circuit/dag.hpp"
#include "src/parser/bench_to_circuit.hpp"

#include "src/simplification/utils/three_coloring.hpp"

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
    ThreeColoring& threeColoring,
    csat::utils::GateEncoder& encoder
) {
    std::vector<std::vector<std::string>> color_parents;
    std::vector<std::vector<std::string>> gates_by_color;
    for (size_t i = 0; i < threeColoring.getColorsNumber(); ++i) {
        color_parents.emplace_back();
        color_parents.back().push_back(encoder.decodeGate(threeColoring.colors[i].first_parent));
        color_parents.back().push_back(encoder.decodeGate(threeColoring.colors[i].second_parent));
        color_parents.back().push_back(encoder.decodeGate(threeColoring.colors[i].third_parent));

        gates_by_color.emplace_back();
        for (GateId v: threeColoring.colors[i].getGates()) {
            gates_by_color.back().push_back(encoder.decodeGate(v));
        }
        std::sort(color_parents.back().begin(), color_parents.back().end());
        std::sort(gates_by_color.back().begin(), gates_by_color.back().end());
    }
    return {color_parents, gates_by_color};
}


TEST(ThreeColoring, SmallTest)
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
    
    ThreeColoring threeColoring = ThreeColoring(*circuit);
    auto [color_parents, gates_by_color] = parse_colors(threeColoring, encoder);

    ASSERT_EQ(threeColoring.getColorsNumber(), 1);
    ASSERT_EQ(color_parents[0], std::vector<std::string>({"0", "1", "2"}));
    ASSERT_EQ(gates_by_color[0], std::vector<std::string>({"6", "7"}));
}


TEST(ThreeColoring, BiggerTest)
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
    
    ThreeColoring threeColoring = ThreeColoring(*circuit);
    auto [color_parents, gates_by_color] = parse_colors(threeColoring, encoder);

    ASSERT_EQ(threeColoring.getColorsNumber(), 3);
    ASSERT_EQ(color_parents[0], std::vector<std::string>({"1", "2", "3"}));
    ASSERT_EQ(gates_by_color[0], std::vector<std::string>({"11"}));
    ASSERT_EQ(color_parents[1], std::vector<std::string>({"0", "1", "11"}));
    ASSERT_EQ(gates_by_color[1], std::vector<std::string>({"12"}));
    ASSERT_EQ(color_parents[2], std::vector<std::string>({"10", "8", "9"}));
    ASSERT_EQ(gates_by_color[2], std::vector<std::string>({"12"}));
}


TEST(ThreeColoring, HardTest)
{
    std::string const dag = "INPUT(0)\n"
                            "INPUT(1)\n"
                            "INPUT(2)\n"
                            "INPUT(3)\n"
                            "INPUT(4)\n"
                            "OUTPUT(12)\n"
                            "5 = NOT(0)\n"
                            "6 = NOT(5)\n"
                            "7 = OR(1, 5)\n"
                            "8 = XOR(6, 7)\n"
                            "9 = NAND(6, 2)\n"
                            "10 = XOR(8, 9)\n"
                            "11 = XOR(1, 3)\n"
                            "12 = NOT(3)\n"
                            "13 = OR(12, 7)\n"
                            "14 = AND(8, 11)\n"
                            "15 = XOR(13, 14)\n"
                            "16 = OR(15, 4)\n"
                            "17 = XOR(16, 10)\n";
    
    std::istringstream stream(dag);
    csat::parser::BenchToCircuit<csat::DAG> parser;
    parser.parseStream(stream);
    
    std::unique_ptr<csat::DAG> circuit = parser.instantiate();
    csat::utils::GateEncoder encoder = parser.getEncoder();
    
    ThreeColoring threeColoring = ThreeColoring(*circuit);
    auto [color_parents, gates_by_color] = parse_colors(threeColoring, encoder);

    ASSERT_EQ(threeColoring.getColorsNumber(), 5);
    ASSERT_EQ(color_parents[0], std::vector<std::string>({"0", "1", "2"}));
    ASSERT_EQ(gates_by_color[0], std::vector<std::string>({"10"}));
    ASSERT_EQ(color_parents[1], std::vector<std::string>({"0", "1", "3"}));
    ASSERT_EQ(gates_by_color[1], std::vector<std::string>({"13", "14", "15"}));
    ASSERT_EQ(color_parents[2], std::vector<std::string>({"13", "14", "4"}));
    ASSERT_EQ(gates_by_color[2], std::vector<std::string>({"16"}));
    ASSERT_EQ(color_parents[3], std::vector<std::string>({"16", "8", "9"}));
    ASSERT_EQ(gates_by_color[3], std::vector<std::string>({"17"}));
    ASSERT_EQ(color_parents[4], std::vector<std::string>({"10", "15", "4"}));
    ASSERT_EQ(gates_by_color[4], std::vector<std::string>({"17"}));
}

}