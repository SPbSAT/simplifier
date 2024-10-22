#pragma once

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include "src/algo.hpp"
#include "src/common/csat_types.hpp"
#include "src/simplification/circuits_db.hpp"
#include "src/simplification/transformer_base.hpp"
#include "src/simplification/utils/three_coloring.hpp"
#include "src/simplification/utils/two_coloring.hpp"
#include "src/structures/circuit/gate_info.hpp"
#include "src/structures/circuit/icircuit.hpp"
#include "src/utility/logger.hpp"

namespace csat::simplification
{

/**
 * @tparam CircuitT
 * Algorithm works with unary/binary operations, supports AIG/BENCH basis
 * Main idea is to iterate and try to simplify all subcircuits with 3 inputs using
 * database with small subcircuits.
 */

struct CircuitStatsSingleton
{
  public:
    int32_t iter_number                              = 0;
    std::vector<int32_t> subcircuits_number_by_iter  = {0, 0, 0, 0, 0};
    std::vector<int32_t> skipped_subcircuits_by_iter = {0, 0, 0, 0, 0};
    std::vector<int32_t> max_subcircuit_size_by_iter = {0, 0, 0, 0, 0};
    std::vector<int32_t> circuit_size_by_iter        = {0, 0, 0, 0, 0};
    std::size_t total_gates_in_subcircuits           = 0;
    std::size_t last_iter_gates_simplification       = 0;

    static CircuitStatsSingleton& getInstance()
    {
        static CircuitStatsSingleton s;
        return s;
    }

    CircuitStatsSingleton(CircuitStatsSingleton const&)            = delete;
    CircuitStatsSingleton& operator=(CircuitStatsSingleton const&) = delete;

  private:
    CircuitStatsSingleton()  = default;
    ~CircuitStatsSingleton() = default;
};

template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT>>>
class ThreeInputsSubcircuitMinimization : public ITransformer<CircuitT>
{
    csat::Logger logger{"ThreeInputsSubcircuitMinimization"};

    /**
     * Class for storaging info about observed subcircuits:
     * 1) not_in_db - subcircuit pattern was not found in database
     * 2) smaller_size - subcircuit in our database was better
     * 3) same_size - size was same in database
     * 3) bigger_size - subcircuit in initial circuit was better than in our database
     * (in this cases we want to 'remember' found subcircuit)
     * 4) many_outputs - subcircuit has >3 outputs (even with heuristics for reducing outputs number)
     */
    class SubcircuitStats
    {
        csat::Logger logger{"SubcircuitStats"};

      public:
        int32_t not_in_db{0};
        int32_t smaller_size{0};
        int32_t same_size{0};
        int32_t bigger_size{0};
        int32_t many_outputs{0};
        int32_t subcircuits_count{0};

        SubcircuitStats() = default;

        void print()
        {
            logger.debug(
                "Many outputs: ",
                many_outputs,
                " | Smaller size: ",
                smaller_size,
                " | Same size: ",
                same_size,
                " | Bigger size: ",
                bigger_size,
                " | Subcircuits count: ",
                subcircuits_count);
        }
    };

  public:
    std::size_t colors_number = 0;
    std::vector<csat::utils::ThreeColor> colors;  // list of all 3-parent colors
    std::vector<std::vector<size_t>> gateColors;  // contains up to 2 colors for each gate, otherwise: 'SIZE_MAX'
    std::map<std::vector<GateId>, size_t> parentsToColor;  // parent ids must be in a sorted order

    std::shared_ptr<CircuitDB> read_db()
    {
        assert(DBSingleton::getInstance().aig_db);
        return DBSingleton::getInstance().aig_db;
    }

    bool
    update_primitive_gate(GateId primitive_gate, int32_t pattern, GateInfoContainer& gate_info, GateIdContainer parents)
    {
        if (pattern == 0)
        {
            gate_info.at(primitive_gate) = {
                GateType::XOR, {parents[0], parents[0]}
            };
        }
        else if (pattern == 255)
        {
            gate_info.at(primitive_gate) = {
                GateType::NXOR, {parents[0], parents[0]}
            };
        }
        else if (pattern == 240)
        {
            gate_info.at(primitive_gate) = {
                GateType::AND, {parents[0], parents[0]}
            };
        }
        else if (pattern == 204)
        {
            gate_info.at(primitive_gate) = {
                GateType::AND, {parents[1], parents[1]}
            };
        }
        else if (pattern == 170)
        {
            gate_info.at(primitive_gate) = {
                GateType::AND, {parents[2], parents[2]}
            };
        }
        else if (pattern == 15)
        {
            gate_info.at(primitive_gate) = {GateType::NOT, {parents[0]}};
        }
        else if (pattern == 51)
        {
            gate_info.at(primitive_gate) = {GateType::NOT, {parents[1]}};
        }
        else if (pattern == 85)
        {
            gate_info.at(primitive_gate) = {GateType::NOT, {parents[2]}};
        }
        else
        {
            return false;
        }
        return true;
    }

    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<GateEncoder> encoder)
    {
        logger.debug("=========================================================================================");
        logger.debug("START ThreeInputsSubcircuitMinimization");

        logger.debug("Top sort");
        csat::GateIdContainer gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit));

        logger.debug("Paint gates");
        GateInfoContainer gate_info(circuit->getNumberOfGates());

        csat::utils::TwoColoring twoVertexColoring = csat::utils::TwoColoring(*circuit);
        csat::utils::ThreeColoring threeColoring   = csat::utils::ThreeColoring(*circuit);

        int circuit_size = circuit->getNumberOfGates();
        gateColors.resize(circuit_size, {});

        colors_number  = threeColoring.getColorsNumber();
        colors         = threeColoring.colors;
        gateColors     = threeColoring.gateColors;
        parentsToColor = threeColoring.parentsToColor;

        // Filling GateInfoContainer
        for (uint64_t gateId : std::ranges::reverse_view(gate_sorting))
        {
            GateIdContainer const& operands = circuit->getGateOperands(gateId);
            gate_info.at(gateId)            = {circuit->getGateType(gateId), operands};
        }

        if (CircuitStatsSingleton::getInstance().iter_number != 0 &&
            CircuitStatsSingleton::getInstance().last_iter_gates_simplification == 0)
        {
            return {
                std::make_unique<CircuitT>(gate_info, circuit->getOutputGates()),
                std::make_unique<GateEncoder>(*encoder)};
        }
        CircuitStatsSingleton::getInstance().iter_number += 1;
        CircuitStatsSingleton::getInstance().last_iter_gates_simplification = 0;
        CircuitStatsSingleton::getInstance()
            .circuit_size_by_iter[CircuitStatsSingleton::getInstance().iter_number - 1] = circuit_size;

        // Store database
        auto db                           = read_db();
        auto& subcircuit_pattern_to_index = db->subcircuit_pattern_to_index;
        auto& subcircuit_outputs          = db->subcircuit_outputs;
        auto& subcircuit_gates_operands   = db->gates_operands;
        auto& subcircuit_AND_number       = db->OPER_number;
        auto& subcircuit_gates_operations = db->gates_operations;

        // Parameters for statistics monitoring
        SubcircuitStats stats = SubcircuitStats();

        std::vector<size_t> used_gates(circuit_size, SIZE_MAX);  // contains last color ID when gate occured
        BoolVector is_removed(circuit_size, false);
        BoolVector is_modified(circuit_size, false);

        // Iterating over subcircuits defined by colors and trying to improve them
        for (size_t color_id = 0; color_id < colors.size(); ++color_id)
        {
            csat::utils::ThreeColor color = colors.at(color_id);

            // Check whether subcircuit's inputs were removed (in this case we do not observe it)
            if (is_removed.at(color.first_parent) || is_removed.at(color.second_parent) ||
                is_removed.at(color.third_parent))
            {
                CircuitStatsSingleton::getInstance()
                    .skipped_subcircuits_by_iter[CircuitStatsSingleton::getInstance().iter_number - 1] += 1;
                continue;
            }

            // Will store gates defined by parents of the following color (in topological order), except parents
            GateIdContainer gatesByColor;
            // Outputs in observed subcircuit (Some 'real' outputs will be removed according to our heuristic)
            GateIdContainer outputs;
            // All 'real' outputs
            GateIdContainer all_outputs;

            // Getting gates depending from 1 of parents
            used_gates.at(color.first_parent)  = color_id;
            used_gates.at(color.second_parent) = color_id;
            used_gates.at(color.third_parent)  = color_id;

            for (GateId const parent : color.getParents())
            {
                GateId const negation_user = threeColoring.negationUsers.at(parent);
                if (negation_user != SIZE_MAX)
                {
                    gatesByColor.push_back(negation_user);
                    used_gates.at(negation_user) = color_id;
                }
            }

            // Getting gates depending from 2 of parents
            std::vector<std::vector<GateId>> const parents_pairs = {
                {color.first_parent,  color.second_parent},
                {color.first_parent,  color.third_parent },
                {color.second_parent, color.third_parent }
            };
            for (auto const& pair : parents_pairs)
            {
                if (twoVertexColoring.parentsToColor.find(pair) != twoVertexColoring.parentsToColor.end())
                {
                    for (GateId const gateId :
                         twoVertexColoring.colors.at(twoVertexColoring.parentsToColor.at(pair)).getGates())
                    {
                        gatesByColor.push_back(gateId);
                        used_gates.at(gateId) = color_id;
                    }
                }
            }

            // Getting gates depending from all parents
            for (GateId const gateId : color.getGates())
            {
                gatesByColor.push_back(gateId);
                used_gates.at(gateId) = color_id;
            }

            CircuitStatsSingleton::getInstance()
                .max_subcircuit_size_by_iter[CircuitStatsSingleton::getInstance().iter_number - 1] = std::max(
                CircuitStatsSingleton::getInstance()
                    .max_subcircuit_size_by_iter[CircuitStatsSingleton::getInstance().iter_number - 1],
                static_cast<int32_t>(gatesByColor.size()) + 3);
            CircuitStatsSingleton::getInstance().total_gates_in_subcircuits += gatesByColor.size() + 3;

            // Check whether subcircuit has modified gates (in this case we do not observe it)
            bool has_modified_gates = false;
            for (GateId const gateId : gatesByColor)
            {
                if (is_removed.at(gateId) || is_modified.at(gateId))
                {
                    has_modified_gates = true;
                    break;
                }
            }
            if (has_modified_gates)
            {
                continue;
            }

            /*
             * Gate's pattern describes it in terms of truth table:
             * For all 8 combinations of inputs assignments we look at the resulting
             * value in the following gate.
             * This process is done for all inputs permutations (3! = 6)
             * Constants: 240, 204, 170 - describe initial inputs patterns
             */
            std::vector<std::vector<int32_t>> all_patterns(6, std::vector<int32_t>(circuit_size, INT32_MAX));

            all_patterns[0][color.first_parent]  = 240;
            all_patterns[0][color.second_parent] = 204;
            all_patterns[0][color.third_parent]  = 170;

            all_patterns[1][color.first_parent]  = 240;
            all_patterns[1][color.second_parent] = 170;
            all_patterns[1][color.third_parent]  = 204;

            all_patterns[2][color.first_parent]  = 204;
            all_patterns[2][color.second_parent] = 240;
            all_patterns[2][color.third_parent]  = 170;

            all_patterns[3][color.first_parent]  = 204;
            all_patterns[3][color.second_parent] = 170;
            all_patterns[3][color.third_parent]  = 240;

            all_patterns[4][color.first_parent]  = 170;
            all_patterns[4][color.second_parent] = 240;
            all_patterns[4][color.third_parent]  = 204;

            all_patterns[5][color.first_parent]  = 170;
            all_patterns[5][color.second_parent] = 204;
            all_patterns[5][color.third_parent]  = 240;

            std::vector<std::vector<int32_t>> output_patterns(6);
            GateIdContainer primitive_gates;  // constant gates + gates equal to parents or their negations

            // Getting outputs of the following subcircuit (and check that all gates exist)
            for (GateId gateId : gatesByColor)
            {
                GateIdContainer const& operands = circuit->getGateOperands(gateId);
                GateIdContainer const& users    = circuit->getGateUsers(gateId);
                GateType oper                   = circuit->getGateType(gateId);

                if (oper == GateType::AND)
                {
                    for (size_t i = 0; i < 6; ++i)
                    {
                        all_patterns[i][gateId] = all_patterns[i][operands[0]] & all_patterns[i][operands[1]];
                    }
                }
                else if (oper == GateType::NOT)
                {
                    for (size_t i = 0; i < 6; ++i)
                    {
                        all_patterns[i][gateId] = 255 - all_patterns[i][operands[0]];
                    }
                }
                else
                {
                    std::cout << "Error! Incorrect operation!\n";
                    std::exit(EINVAL);
                }

                if (all_patterns[0][gateId] == 0 || all_patterns[0][gateId] == 255 || all_patterns[0][gateId] == 240 ||
                    all_patterns[0][gateId] == 204 || all_patterns[0][gateId] == 170)
                {
                    primitive_gates.push_back(gateId);
                }

                if (all_patterns[0][gateId] == 15 && (oper != GateType::NOT || operands[0] != color.first_parent))
                {
                    primitive_gates.push_back(gateId);
                }
                if (all_patterns[0][gateId] == 51 && (oper != GateType::NOT || operands[0] != color.second_parent))
                {
                    primitive_gates.push_back(gateId);
                }
                if (all_patterns[0][gateId] == 85 && (oper != GateType::NOT || operands[0] != color.third_parent))
                {
                    primitive_gates.push_back(gateId);
                }

                /**
                 * New Heuristic (!!!)
                 * We do not need all the outputs to be real outputs
                 * As we minimize ANDs number, outputs that are the negations of another outputs
                 * or negations of inputs gate are useless to count them
                 * we can just manually add them as NOT(x) where x is the following gate or copy from parents
                 */

                if (circuit->isOutputGate(gateId))
                {
                    all_outputs.push_back(gateId);
                    int32_t pattern           = all_patterns[0][gateId];
                    GateId gate_first_operand = operands[0];
                    if (update_primitive_gate(gateId, pattern, gate_info, color.getParents()))
                    {
                        if (all_patterns[0][gateId] == 15)
                        {
                            if (oper != GateType::NOT || gate_first_operand != color.first_parent)
                            {
                                is_modified.at(gateId) = true;
                                CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                            }
                        }
                        else if (all_patterns[0][gateId] == 51)
                        {
                            if (oper != GateType::NOT || gate_first_operand != color.second_parent)
                            {
                                is_modified.at(gateId) = true;
                                CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                            }
                        }
                        else if (all_patterns[0][gateId] == 85)
                        {
                            if (oper != GateType::NOT || gate_first_operand != color.third_parent)
                            {
                                is_modified.at(gateId) = true;
                                CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                            }
                        }
                        else
                        {
                            is_modified.at(gateId) = true;
                            CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                        }
                    }
                    else
                    {
                        bool fl = false;
                        for (size_t i = 0; i < output_patterns[0].size(); ++i)
                        {
                            int const output_pattern = output_patterns[0][i];
                            if (all_patterns[0][gateId] == output_pattern)
                            {
                                is_modified.at(gateId) = true;
                                CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                gate_info.at(gateId) = {
                                    GateType::AND, {outputs[i], outputs[i]}
                                };
                                fl = true;
                                break;
                            }
                            else if (all_patterns[0][gateId] == 255 - output_pattern)
                            {
                                /**
                                 * Check whether we have changed the operation for the gate
                                 * We do this in order not to add modified status for unchanged gates
                                 */
                                if (oper != GateType::NOT || operands[0] != outputs[i])
                                {
                                    is_modified.at(gateId) = true;
                                    CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                    gate_info.at(gateId) = {GateType::NOT, {outputs[i]}};
                                }
                                fl = true;
                                break;
                            }
                        }
                        if (fl)
                        {
                            continue;
                        }

                        outputs.push_back(gateId);
                        for (size_t i = 0; i < 6; ++i)
                        {
                            output_patterns[i].push_back(all_patterns[i][gateId]);
                        }
                    }
                }
                else
                {
                    for (GateId const user : users)
                    {
                        if (used_gates[user] != color_id)
                        {
                            all_outputs.push_back(gateId);
                            int32_t pattern           = all_patterns[0][gateId];
                            GateId gate_first_operand = operands[0];
                            if (update_primitive_gate(gateId, pattern, gate_info, color.getParents()))
                            {
                                if (all_patterns[0][gateId] == 15)
                                {
                                    if (oper != GateType::NOT || gate_first_operand != color.first_parent)
                                    {
                                        is_modified.at(gateId) = true;
                                        CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                    }
                                }
                                else if (all_patterns[0][gateId] == 51)
                                {
                                    if (oper != GateType::NOT || gate_first_operand != color.second_parent)
                                    {
                                        is_modified.at(gateId) = true;
                                        CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                    }
                                }
                                else if (all_patterns[0][gateId] == 85)
                                {
                                    if (oper != GateType::NOT || gate_first_operand != color.third_parent)
                                    {
                                        is_modified.at(gateId) = true;
                                        CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                    }
                                }
                                else
                                {
                                    is_modified.at(gateId) = true;
                                    CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                }
                            }
                            else
                            {
                                bool fl = false;
                                for (size_t i = 0; i < output_patterns[0].size(); ++i)
                                {
                                    int const output_pattern = output_patterns[0][i];
                                    if (all_patterns[0][gateId] == output_pattern)
                                    {
                                        is_modified.at(gateId) = true;
                                        CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                        gate_info.at(gateId) = {
                                            GateType::AND, {outputs[i], outputs[i]}
                                        };
                                        fl = true;
                                        break;
                                    }
                                    else if (all_patterns[0][gateId] == 255 - output_pattern)
                                    {
                                        if (oper != GateType::NOT || operands[0] != outputs[i])
                                        {
                                            is_modified.at(gateId) = true;
                                            CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                                            gate_info.at(gateId) = {GateType::NOT, {outputs[i]}};
                                        }
                                        fl = true;
                                        break;
                                    }
                                }
                                if (fl)
                                {
                                    continue;
                                }

                                outputs.push_back(gateId);
                                for (size_t i = 0; i < 6; ++i)
                                {
                                    output_patterns[i].push_back(all_patterns[i][gateId]);
                                }
                                break;
                            }
                        }
                    }
                }
            }

            if (outputs.size() > 3)
            {
                ++stats.many_outputs;
                // Improving primitive gates
                for (GateId primitive_gate : primitive_gates)
                {
                    int32_t pattern = all_patterns[0][primitive_gate];
                    update_primitive_gate(primitive_gate, pattern, gate_info, color.getParents());
                    is_modified[primitive_gate] = true;
                    CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                }
                continue;
            }

            int true_ind = -1;

            for (size_t i = 0; i < 6; ++i)
            {
                std::sort(output_patterns[i].begin(), output_patterns[i].end());
                if (subcircuit_pattern_to_index.find(output_patterns[i]) != subcircuit_pattern_to_index.end())
                {
                    true_ind = static_cast<int>(i);
                    break;
                }
            }

            if (true_ind == -1)
            {
                ++stats.not_in_db;

                for (GateId primitive_gate : primitive_gates)
                {
                    int32_t pattern = all_patterns[0][primitive_gate];
                    update_primitive_gate(primitive_gate, pattern, gate_info, color.getParents());
                    is_modified[primitive_gate] = true;
                    CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                }
                continue;
            }

            int patternIndex = subcircuit_pattern_to_index[output_patterns[true_ind]];

            int32_t AND_number = 0;
            for (GateId gateId : gatesByColor)
            {
                if (circuit->getGateType(gateId) == GateType::AND)
                {
                    ++AND_number;
                }
            }

            if (subcircuit_AND_number[patternIndex] < AND_number)
            {
                ++stats.smaller_size;
                CircuitStatsSingleton::getInstance().last_iter_gates_simplification += 1;
                for (GateId const gateId : gatesByColor)
                {
                    is_removed[gateId] = true;
                }
                // Changed outputs -> all_outputs
                for (GateId const output : all_outputs)
                {
                    is_modified[output] = true;
                    is_removed[output]  = false;
                }
            }
            else
            {
                if (subcircuit_AND_number[patternIndex] == AND_number)
                {
                    ++stats.same_size;
                }
                else
                {
                    ++stats.bigger_size;
                }
                continue;
            }

            std::vector<GateId> bijection(subcircuit_gates_operands[patternIndex].size() + 3, SIZE_MAX);
            if (true_ind == 0)
            {
                bijection[0] = color.first_parent;
                bijection[1] = color.second_parent;
                bijection[2] = color.third_parent;
            }

            if (true_ind == 1)
            {
                bijection[0] = color.first_parent;
                bijection[1] = color.third_parent;
                bijection[2] = color.second_parent;
            }

            if (true_ind == 2)
            {
                bijection[0] = color.second_parent;
                bijection[1] = color.first_parent;
                bijection[2] = color.third_parent;
            }

            if (true_ind == 3)
            {
                bijection[0] = color.third_parent;
                bijection[1] = color.first_parent;
                bijection[2] = color.second_parent;
            }

            if (true_ind == 4)
            {
                bijection[0] = color.second_parent;
                bijection[1] = color.third_parent;
                bijection[2] = color.first_parent;
            }

            if (true_ind == 5)
            {
                bijection[0] = color.third_parent;
                bijection[1] = color.second_parent;
                bijection[2] = color.first_parent;
            }

            for (size_t i = 0; i < outputs.size(); ++i)
            {
                for (GateId output : outputs)
                {
                    if (all_patterns[true_ind][output] == output_patterns[true_ind][i])
                    {
                        bijection[subcircuit_outputs[patternIndex][i]] = output;
                    }
                }
            }

            for (size_t i = 0; i < subcircuit_gates_operands[patternIndex].size(); ++i)
            {
                if (bijection[i + 3] == SIZE_MAX)
                {
                    GateId new_gateID = encoder->encodeGate(
                        "new_gate_pattern_" + std::to_string(patternIndex) + "_" + std::to_string(color_id) + "_" +
                        std::to_string(colors.size()) + "_" + std::to_string(i) + "_" +
                        std::to_string((*encoder).size()));
                    // Create default gates
                    gate_info.emplace_back(GateType::NOT, GateIdContainer(color.first_parent));
                    bijection[i + 3] = new_gateID;
                }
            }

            for (size_t i = 0; i < subcircuit_gates_operands[patternIndex].size(); ++i)
            {
                std::vector<GateId> new_operands;

                for (GateId gateId : subcircuit_gates_operands[patternIndex][i])
                {
                    new_operands.push_back(bijection[gateId]);
                }

                if (bijection[i + 3] == SIZE_MAX)
                {
                    GateId new_gateID = encoder->encodeGate(
                        "new_gate_pattern_" + std::to_string(patternIndex) + "_" + std::to_string(color_id) + "_" +
                        std::to_string(colors.size()) + "_" + std::to_string(i) + "_" +
                        std::to_string((*encoder).size()));
                    gate_info.emplace_back(subcircuit_gates_operations[patternIndex][i], new_operands);
                    bijection[i + 3] = new_gateID;
                }
                else
                {
                    gate_info.at(bijection[i + 3]) = {subcircuit_gates_operations[patternIndex][i], new_operands};
                }
            }
        }
        stats.subcircuits_count = colors.size();
        CircuitStatsSingleton::getInstance()
            .subcircuits_number_by_iter[CircuitStatsSingleton::getInstance().iter_number - 1] += colors.size();
        stats.print();

        return {
            std::make_unique<CircuitT>(gate_info, circuit->getOutputGates()),
            std::make_unique<GateEncoder>(*encoder)};
    }
};

}  // namespace csat::simplification
