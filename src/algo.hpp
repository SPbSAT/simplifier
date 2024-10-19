#pragma once

#include "src/common/csat_types.hpp"
#include "src/structures/circuit/dag.hpp"

#include <deque>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>
#include <stack>
#include <list>

/**
 * Namespace contains some algorithms for data structures,
 * and data processing. It includes a DFS and different
 * TopSort realisations.
 */
namespace csat::algo
{

enum class DFSState : uint8_t
{
    UNVISITED,
    ENTERED,
    VISITED
};

using DFSStateVector = std::vector<DFSState>;

using OperationDFSGate = std::function<void(GateId, DFSStateVector const&)>;
using OperationDFSVoid = std::function<void()>;

[[maybe_unused]] static void voidOperationGate_(GateId /*unused*/, DFSStateVector const& /*unused*/) { }
[[maybe_unused]] static void voidOperationVoid_() { }


/**
 * Performs Depth First Search on Circuit gates, where arcs are thought to
 * point from gates to their operands. DFS is iteratively run on `startGates`
 * node in begin to end order. Body of DFS is customizable with method parameters.
 *
 * @tparam fromOperatorsToOperands -- bool flag, if True, then arcs in
 *         circuit will be thought to point from gate to its operands.
 *
 * @param circuit -- a circuit to perform DFS on.
 * @param startGates -- gates that will be start points of DFS.
 * @param previsitOperation -- operation, that is performed on
 *        gate right before first visiting it in DFS.
 * @param postvisitOperation -- operation, that is performed on
 *        gate right after first visiting it in DFS.
 * @param dfsOverOperation -- method that is called right after
 *        dfs over, and takes no arguments.
 * @param unvisitedVertexOperation -- operation, that is performed
 *        on all unvisited gates, without any ordering guarantee.
 *
 * @return mask of visited gates.
 */
template<bool fromOperatorsToOperands = true>
inline DFSStateVector performDepthFirstSearch(
    ICircuit const& circuit,
    GateIdContainer const& startGates,
    OperationDFSGate const& previsitOperation = voidOperationGate_,
    OperationDFSGate const& postvisitOperation = voidOperationGate_,
    OperationDFSVoid const& dfsOverOperation = voidOperationVoid_,
    OperationDFSGate const& unvisitedVertexOperation = voidOperationGate_)
{
    // Believe that all gates are named from 0 through N.
    std::vector<DFSState> dfs_state(circuit.getNumberOfGates(), DFSState::UNVISITED);
    
    // Getter of next node depends on template parameter (arcs orientation).
    std::function<GateIdContainer(GateId)> next_getter;
    if constexpr(fromOperatorsToOperands)
    {
        next_getter = [&circuit](GateId gateId) -> GateIdContainer const&
        {
            return circuit.getGateOperands(gateId);
        };
    }
    else
    {
        next_getter = [&circuit](GateId gateId) -> GateIdContainer const&
        {
            return circuit.getGateUsers(gateId);
        };
    }
    
    std::stack<GateId> queue_{};
    
    auto enqueue_next = [&dfs_state, &queue_](GateId nextGateId) -> void
    {
        if (dfs_state[nextGateId] == DFSState::UNVISITED)
        {
            queue_.push(nextGateId);
        }
    };
    
    for (auto start : startGates)
    {
        enqueue_next(start);
        while (!queue_.empty())
        {
            GateId const gateId = queue_.top();
            switch (dfs_state[gateId])
            {
                case DFSState::UNVISITED:
                {
                    previsitOperation(gateId, dfs_state); // custom
                    dfs_state[gateId] = DFSState::ENTERED;
        
                    GateIdContainer const &nextContainer = next_getter(gateId);
                    std::for_each(
                        nextContainer.rbegin(),
                        nextContainer.rend(),
                        enqueue_next);
                    break;
                }
                case DFSState::ENTERED:
                {
                    dfs_state[gateId] = DFSState::VISITED;
                    postvisitOperation(gateId, dfs_state); // custom
                    queue_.pop();
                    break;
                }
                case DFSState::VISITED:
                {
                    // gate was already entered and left,
                    // so we just silently pop it from queue.
                    queue_.pop();
                    break;
                }
                default:
                {
                    std::cerr << "During DFS execution wrong DFSState occurred." << std::endl;
                    std::abort();
                }
            }
        }
    }

    dfsOverOperation(); // custom

    for (size_t idx = 0; idx < dfs_state.size(); ++idx)
    {
        if (dfs_state[idx] == DFSState::UNVISITED)
        {
            unvisitedVertexOperation(idx, dfs_state); // custom
        }
    }

    return dfs_state;
}


// Auxiliary struct to be used as template parameters.
struct DFSTopSort;

/**
 * Base template class for topological sorting algorithms.
 * @tparam Algorithm -- which algorithm to use.
 */
template<class Algorithm>
struct TopSortAlgorithm {};


template<>
struct TopSortAlgorithm<DFSTopSort>
{
  public:
    /**
     * @param circuit -- topology of circuit, which gates must be sorted
     * in topological order.
     * @return a vector of gates 1..N, sorted in topological order according
     * to depth first search topological sorting algorithm.
     */
    static GateIdContainer sorting(ICircuit const& circuit)
    {
        // Gather all sources in a circuit to start DFS from them.
        GateIdContainer sources{};
        for (GateId gateId = 0; gateId < circuit.getNumberOfGates(); ++gateId)
        {
            if (circuit.getGateUsers(gateId).empty())
            {
                sources.push_back(gateId);
            }
        }
        
        GateIdContainer gateSorting{};
        gateSorting.reserve(circuit.getNumberOfGates());
  
        performDepthFirstSearch(
            circuit,
            sources,
            voidOperationGate_,
            // Add gate to sorting on leaving.
            [&gateSorting](GateId gate, DFSStateVector const&)
            {
                gateSorting.push_back(gate);
            },
            // Reverse sorting after DFS is over.
            [&gateSorting]()
            {
                std::reverse(gateSorting.begin(), gateSorting.end());
            },
            // Add rest (not connected) gates
            // to fulfill return contract.
            [&gateSorting](GateId gate, DFSStateVector const&)
            {
                gateSorting.push_back(gate);
            }
        );
  
        return gateSorting;
    }
};


} // namespace csat::algo
