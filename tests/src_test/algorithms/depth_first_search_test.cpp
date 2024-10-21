#include "src/structures/circuit/dag.hpp"
#include "src/algo.hpp"

#include "gtest/gtest.h"


namespace
{

using namespace csat;

class DepthFirstSearchTestFixture : public ::testing::Test
{
  protected:
    //  0 -.
    //      \
    //       4 -- 7
    //      /
    //  1 -:
    //      \
    //       5
    //      /
    //  2 -:
    //      \
    //       6
    //      /
    //  3 -'
    DAG simple_graph_01 = DAG(
        {
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::INPUT, {}},
            {GateType::AND, {0, 1}},
            {GateType::AND, {1, 2}},
            {GateType::AND, {2, 3}},
            {GateType::NOT, {4}},
        },
        {});
    
    // Graph with cycle (0, 1, 2, 3)
    //   .- 0 -.
    //  /       \
    // 3         1 -- 4
    //  \       /
    //   '- 2 -'
    DAG simple_graph_02 = DAG(
        {
            {GateType::NOT, {3}},
            {GateType::AND, {0, 4}},
            {GateType::NOT, {1}},
            {GateType::NOT, {2}},
            {GateType::INPUT, {}},
        },
        {});
};

TEST_F(DepthFirstSearchTestFixture, SimpleVisitingCheck)
{
    ASSERT_EQ(
        algo::performDepthFirstSearch(simple_graph_01, {3}),
        std::vector<algo::DFSState>({
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::VISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
        })
    );
    ASSERT_EQ(
        algo::performDepthFirstSearch(simple_graph_01, {7}),
        std::vector<algo::DFSState>({
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::VISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::VISITED,
        })
    );
    ASSERT_EQ(
        algo::performDepthFirstSearch(simple_graph_01, {7, 5}),
        std::vector<algo::DFSState>({
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::VISITED,
        })
    );
    ASSERT_EQ(
        algo::performDepthFirstSearch(simple_graph_01, {7, 6}),
        std::vector<algo::DFSState>({
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
        })
    );
}

TEST_F(DepthFirstSearchTestFixture, CycleVisitingCheck)
{
    ASSERT_EQ(
        algo::performDepthFirstSearch(simple_graph_02, {4}),
        std::vector<algo::DFSState>({
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::UNVISITED,
            algo::DFSState::VISITED,
        })
    );
    ASSERT_EQ(
        algo::performDepthFirstSearch(simple_graph_02, {0}),
        std::vector<algo::DFSState>({
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
            algo::DFSState::VISITED,
        })
    );
}

TEST_F(DepthFirstSearchTestFixture, TestBaseHooks)
{
    std::vector<GateId> visit_stack{};
    std::vector<GateId> unvisited_list{};
    uint8_t dfs_over_calls_counter = 0;
    
    algo::performDepthFirstSearch(
        simple_graph_01,
        {7},
        [&visit_stack](GateId gateId, algo::DFSStateVector const&)
        {
            visit_stack.push_back(gateId);
        },
        [&visit_stack](GateId gateId, algo::DFSStateVector const&)
        {
            visit_stack.push_back(gateId);
        },
        [&dfs_over_calls_counter]()
        {
            ++dfs_over_calls_counter;
        },
        [&unvisited_list](GateId gateId, algo::DFSStateVector const&)
        {
            unvisited_list.push_back(gateId);
        }
    );
    
    ASSERT_EQ(visit_stack, std::vector<GateId>({7, 4, 0, 0, 1, 1, 4, 7}));
    ASSERT_EQ(dfs_over_calls_counter, 1);
    ASSERT_EQ(unvisited_list, std::vector<GateId>({2, 3, 5, 6}));
}

} // namespace


