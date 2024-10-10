#include "src/structures/circuit/dag.hpp"
#include "src/algo.hpp"

#include "gtest/gtest.h"


namespace
{

using namespace csat;


TEST(TopSortTest, SimpleCircuit)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::AND, {0, 1}}
        },
        {2}
    );
    
    csat::GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    ASSERT_EQ(gateSorting.at(0), 2);
    ASSERT_EQ(gateSorting.at(1), 1);
    ASSERT_EQ(gateSorting.at(2), 0);
}

TEST(TopSortTest, MediumCircuit)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::AND, {0, 1}},
            {csat::GateType::AND, {1, 2}},
            {csat::GateType::AND, {0, 1}},
            {csat::GateType::OR, {3, 4, 5}}
        },
        {6}
    );

    csat::GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    ASSERT_EQ(gateSorting.at(0), 6);
    ASSERT_EQ(gateSorting.at(1), 5);
    ASSERT_EQ(gateSorting.at(2), 4);
    ASSERT_EQ(gateSorting.at(3), 2);
    ASSERT_EQ(gateSorting.at(4), 3);
    ASSERT_EQ(gateSorting.at(5), 1);
    ASSERT_EQ(gateSorting.at(6), 0);
}

TEST(TopSortTest, MultiOutputCircuit)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::AND, {0, 1}},
            {csat::GateType::AND, {1, 2}},
            {csat::GateType::AND, {0, 1}},
            {csat::GateType::OR, {3, 5}},
            {csat::GateType::AND, {4, 5}}
        },
        {6, 7}
    );
    
    csat::GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    ASSERT_EQ(gateSorting.at(0), 7);
    ASSERT_EQ(gateSorting.at(1), 4);
    ASSERT_EQ(gateSorting.at(2), 2);
    ASSERT_EQ(gateSorting.at(3), 6);
    ASSERT_EQ(gateSorting.at(4), 5);
    ASSERT_EQ(gateSorting.at(5), 3);
    ASSERT_EQ(gateSorting.at(6), 1);
    ASSERT_EQ(gateSorting.at(7), 0);

    auto dag2 = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::AND, {0, 1}},
            {csat::GateType::AND, {2, 1}},
        },
        {2, 3}
    );
    gateSorting= algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag2);
    ASSERT_EQ(gateSorting.at(0), 3);
    ASSERT_EQ(gateSorting.at(1), 2);
    ASSERT_EQ(gateSorting.at(2), 0);
    ASSERT_EQ(gateSorting.at(3), 1);
}


TEST(TopSortTest, DisconnectedGraphCircuit)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::AND, {0, 1}},
            {csat::GateType::INPUT, {}},
            {csat::GateType::OR, {4}}
        },
        {3}
    );

    csat::GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(dag));
    ASSERT_EQ(gateSorting.at(0), 5);
    ASSERT_EQ(gateSorting.at(1), 4);
    ASSERT_EQ(gateSorting.at(2), 3);
    ASSERT_EQ(gateSorting.at(3), 1);
    ASSERT_EQ(gateSorting.at(4), 0);
    ASSERT_EQ(gateSorting.at(5), 2);
}

TEST(TopSortTest, GatesWithoutUsers)
{
    auto dag = csat::DAG(
        {
            {csat::GateType::AND, {1, 3}},
            {csat::GateType::NOT, {3}},
            {csat::GateType::NOT, {1}},
            {csat::GateType::INPUT, {}}
        },
        {3}
    );
}

} // anonymous namespace
