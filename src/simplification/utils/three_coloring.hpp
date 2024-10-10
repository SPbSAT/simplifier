#pragma once

#include "src/simplification/transformer_base.hpp"
#include "src/algo.hpp"
#include "src/utility/converters.hpp"
#include "src/common/csat_types.hpp"
#include "src/simplification/utils/two_coloring.hpp"

#include <cassert>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <memory>


namespace csat::utils
{

using ColorId = size_t;

struct ThreeColor
{
  public:
    // Keep parent ids ordered by ascending
    GateId first_parent;
    GateId second_parent;
    GateId third_parent;

  protected:
    GateIdContainer gates_;

  public:
    ThreeColor(GateId parent_one, GateId parent_two, GateId parent_three)
    {
        GateIdContainer parents = ThreeColor::sortedParents(parent_one, parent_two, parent_three);
        first_parent = parents[0];
        second_parent = parents[1];
        third_parent = parents[2];
    }

    void addGate(GateId gateId)
    {
        gates_.push_back(gateId);
    }

    GateIdContainer const& getGates() const
    {
        return gates_;
    }

    GateIdContainer const getParents() const
    {
        return {first_parent, second_parent, third_parent};
    }

    bool hasParent(GateId gateId) const  
    {  
        return first_parent == gateId || second_parent == gateId || third_parent == gateId;
    }

    static GateIdContainer sortedParents(GateId parent_one, GateId parent_two, GateId parent_three)
    {
        GateIdContainer parents = {parent_one, parent_two, parent_three};
        std::sort(parents.begin(), parents.end());
        return parents;
    }
};


class ThreeColoring
{
  public:
    std::vector<ThreeColor> colors; // list of all 3-parents colors
    std::vector<std::vector<ColorId>> gateColors; // contains up to 2 colors for each gate, otherwise: {}
    // TODO: use better key type for this map.  
    std::map<std::vector<GateId>, ColorId> parentsToColor; // takes parent ids in ascdending order
    // TMP added negationUsers, TODO: remove
    GateIdContainer negationUsers;

    size_t getColorsNumber() const
    {
        return next_color_id_;
    }

  protected:
    ColorId next_color_id_ = 0;

    ColorId addColor(GateId first_parent, GateId second_parent, GateId third_parent) // returns new color ID
    {
        colors.push_back(ThreeColor(first_parent, second_parent, third_parent));
        GateIdContainer sortedParents = ThreeColor::sortedParents(first_parent, second_parent, third_parent);
        parentsToColor[sortedParents] = next_color_id_;
        return next_color_id_++;
    }

    void paintGate(GateId gateId, ColorId colorId)
    {
        colors.at(colorId).addGate(gateId);
        gateColors.at(gateId).push_back(colorId);
    }

  public:
    ThreeColoring(ICircuit const& circuit)
    {
        csat::GateIdContainer gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(circuit));
        size_t circuit_size = circuit.getNumberOfGates();
        TwoColoring twoColoring = TwoColoring(circuit);
        gateColors.resize(circuit_size, {});

        // TODO: maybe remove from here and evaluate in block with strategy
        // User constructed as negation of chosen gate ('SIZE_MAX' if there is no such users)
        negationUsers.resize(circuit_size, SIZE_MAX);

        for (auto it = gate_sorting.rbegin(); it != gate_sorting.rend(); ++it)
        {
            GateId gateId = *it;
            GateIdContainer const& operands = circuit.getGateOperands(gateId);
            
            // Gate is input or constant
            if (operands.size() == 0)
            {
                continue;
            }
            // Unary operation
            if (operands.size() == 1)
            {
                for (ColorId color: gateColors.at(operands[0]))
                {
                    paintGate(gateId, color);
                }

                // Actually, here we want only 'NOT' operations be possible, but check for safety
                if (circuit.getGateType(gateId) == GateType::NOT)
                {
                    negationUsers.at(operands[0]) = gateId;
                }
                continue;
            }
            // Check of non-binary gates
            if (operands.size() > 2)  
            {
                std::cerr << "ThreeColoring got circuit which gate has more than two operands. Gate id: " << gateId << std::endl;  
                std::exit(-1);  
            }

            ColorId two_color = twoColoring.gateColor.at(gateId);
            // If gate doesn't have TwoColor, it won't have ThreeColor
            if (two_color == SIZE_MAX) 
            {
                continue;
            }

            GateId child_1 = twoColoring.colors.at(two_color).first_parent;
            GateId child_2 = twoColoring.colors.at(two_color).second_parent;

            // If both gate's TwoColor parents don't have TwoColor parent, then gate won't have ThreeColor
            if (twoColoring.gateColor.at(child_1) == SIZE_MAX
                && twoColoring.gateColor.at(child_2) == SIZE_MAX)
            {
                continue;
            }

            std::vector<ColorId> common_colors = {};
            // Search for such children patterns
            ColorId color_type_13 = SIZE_MAX;
            ColorId color_type_31 = SIZE_MAX;

            for (ColorId first_child_color: gateColors.at(child_1))
            {
                for (ColorId second_child_color: gateColors.at(child_2))
                {
                    if (first_child_color == second_child_color)
                    {
                        common_colors.push_back(first_child_color);
                    }
                    else if (colors.at(second_child_color).hasParent(child_1))
                    {
                        color_type_13 = second_child_color;
                    }
                }
                if (colors.at(first_child_color).hasParent(child_2))
                {
                    color_type_31 = first_child_color;
                }
            }

            if (common_colors.size() == 2)
            {
                paintGate(gateId, common_colors[0]);
                paintGate(gateId, common_colors[1]);
                continue;
            }

            if (common_colors.size() == 1)
            {
                paintGate(gateId, common_colors[0]);
                if (color_type_13 != SIZE_MAX)
                {
                    paintGate(gateId, color_type_13);
                }
                else if (color_type_31 != SIZE_MAX)
                {
                    paintGate(gateId, color_type_31);
                }
                continue;
            }

            if (color_type_13 != SIZE_MAX)
            {
                paintGate(gateId, color_type_13);
                ColorId first_child_two_color = twoColoring.gateColor.at(child_1);
                if (first_child_two_color != SIZE_MAX)
                {
                    GateId parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                    GateId parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                    ColorId color_type_23 = SIZE_MAX;

                    for (ColorId second_child_color: gateColors.at(child_2))
                    {
                        if (colors.at(second_child_color).hasParent(parent_1)
                            && colors.at(second_child_color).hasParent(parent_2))
                        {
                            color_type_23 = second_child_color;
                            break;
                        }
                    }

                    if (color_type_23 != SIZE_MAX)
                    {
                        paintGate(gateId, color_type_23);
                    }
                    else
                    {
                        GateIdContainer color_parents = ThreeColor::sortedParents(parent_1, parent_2, child_2);
                        if (parentsToColor.find(color_parents) == parentsToColor.end())
                        {
                            addColor(color_parents[0], color_parents[1], color_parents[2]);
                        }
                        paintGate(gateId, parentsToColor[color_parents]);
                    }
                }
                continue;
            }

            if (color_type_31 != SIZE_MAX)
            {
                paintGate(gateId, color_type_31);
                ColorId second_child_two_color = twoColoring.gateColor.at(child_2);
                if (second_child_two_color != SIZE_MAX)
                {
                    GateId parent_1 = twoColoring.colors.at(second_child_two_color).first_parent;
                    GateId parent_2 = twoColoring.colors.at(second_child_two_color).second_parent;
                    ColorId color_type_32 = SIZE_MAX;

                    for (ColorId first_child_color: gateColors.at(child_1))
                    {
                        if (colors.at(first_child_color).hasParent(parent_1)
                            && colors.at(first_child_color).hasParent(parent_2))
                        {
                            color_type_32 = first_child_color;
                            break;
                        }
                    }

                    if (color_type_32 != SIZE_MAX)
                    {
                        paintGate(gateId, color_type_32);
                    }
                    else
                    {
                        GateIdContainer color_parents = ThreeColor::sortedParents(parent_1, parent_2, child_1);
                        if (parentsToColor.find(color_parents) == parentsToColor.end())
                        {
                            addColor(color_parents[0], color_parents[1], color_parents[2]);
                        }
                        paintGate(gateId, parentsToColor[color_parents]);
                    }
                }
                continue;
            }

            // Check for single 3-2 or 2-3 pattern
            ColorId first_child_two_color = twoColoring.gateColor.at(child_1);
            ColorId second_child_two_color = twoColoring.gateColor.at(child_2);

            if (second_child_two_color != SIZE_MAX)
            {
                GateId parent_1 = twoColoring.colors.at(second_child_two_color).first_parent;
                GateId parent_2 = twoColoring.colors.at(second_child_two_color).second_parent;
                ColorId color_type_32 = SIZE_MAX;
                for (ColorId first_child_color: gateColors.at(child_1))
                {
                    if (colors.at(first_child_color).hasParent(parent_1)
                        && colors.at(first_child_color).hasParent(parent_2))
                    {
                        color_type_32 = first_child_color;
                        break;
                    }
                }
                if (color_type_32 != SIZE_MAX)
                {
                    paintGate(gateId, color_type_32);
                    continue;
                }
            }

            if (first_child_two_color != SIZE_MAX)
            {
                GateId parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                GateId parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                ColorId color_type_23 = SIZE_MAX;
                for (ColorId second_child_color: gateColors.at(child_2))
                {
                    if (colors.at(second_child_color).hasParent(parent_1)
                        && colors.at(second_child_color).hasParent(parent_2))
                    {
                        color_type_23 = second_child_color;
                        break;
                    }
                }
                if (color_type_23 != SIZE_MAX)
                {
                    paintGate(gateId, color_type_23);
                    continue;
                }
            }
            
            // Check for 2-2 pattern
            if (first_child_two_color != SIZE_MAX && second_child_two_color != SIZE_MAX)
            {
                GateId parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                GateId parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                GateId parent_3 = twoColoring.colors.at(second_child_two_color).first_parent;
                GateId parent_4 = twoColoring.colors.at(second_child_two_color).second_parent;
                if (twoColoring.colors.at(second_child_two_color).hasParent(parent_1))
                {
                    GateIdContainer color_parents = ThreeColor::sortedParents(parent_2, parent_3, parent_4);
                    if (parentsToColor.find(color_parents) == parentsToColor.end())
                    {
                        addColor(color_parents[0], color_parents[1], color_parents[2]);
                    }
                    paintGate(gateId, parentsToColor[color_parents]);
                }
                else if (twoColoring.colors.at(second_child_two_color).hasParent(parent_2))
                {
                    GateIdContainer color_parents = ThreeColor::sortedParents(parent_1, parent_3, parent_4);
                    if (parentsToColor.find(color_parents) == parentsToColor.end())
                    {
                        addColor(color_parents[0], color_parents[1], color_parents[2]);
                    }
                    paintGate(gateId, parentsToColor[color_parents]);
                }
                else
                {
                    GateIdContainer color_parents = ThreeColor::sortedParents(parent_1, parent_2, child_2);
                    if (parentsToColor.find(color_parents) == parentsToColor.end())
                    {
                        addColor(color_parents[0], color_parents[1], color_parents[2]);
                    }
                    paintGate(gateId, parentsToColor[color_parents]);
                    
                    color_parents = ThreeColor::sortedParents(parent_3, parent_4, child_1);
                    if (parentsToColor.find(color_parents) == parentsToColor.end())
                    {
                        addColor(color_parents[0], color_parents[1], color_parents[2]);
                    }
                    paintGate(gateId, parentsToColor[color_parents]);
                }
                continue;
            }

            GateIdContainer color_parents = {};
            if (first_child_two_color != SIZE_MAX)
            {
                GateId parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                GateId parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                color_parents = ThreeColor::sortedParents(parent_1, parent_2, child_2);
            }
            else
            {
                GateId parent_1 = twoColoring.colors.at(second_child_two_color).first_parent;
                GateId parent_2 = twoColoring.colors.at(second_child_two_color).second_parent;
                color_parents = ThreeColor::sortedParents(parent_1, parent_2, child_1);
            }

            if (parentsToColor.find(color_parents) == parentsToColor.end())
            {
                addColor(color_parents[0], color_parents[1], color_parents[2]);
            }
            paintGate(gateId, parentsToColor[color_parents]);
        }
    }
};

}