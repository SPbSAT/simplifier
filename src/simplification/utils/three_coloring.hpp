#pragma once

#include <cassert>
#include <memory>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "src/algo.hpp"
#include "src/common/csat_types.hpp"
#include "src/simplification/transformer_base.hpp"
#include "src/simplification/utils/two_coloring.hpp"
#include "src/utility/converters.hpp"

namespace csat::utils
{

using ColorId = size_t;

/**
 * Class for selecting a subcircuit of three inputs and the gates that use them. To distinguish
 * this subcircuit from the other gates of the circuit, we will put color marks. If the gates
 * have the same color, then the gates are part of one subcircuit, which can be simplified
 * if a smaller circuit is found
 */
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
        first_parent            = parents[0];
        second_parent           = parents[1];
        third_parent            = parents[2];
    }

    void addGate(GateId gateId)
    {
        gates_.push_back(gateId);
    }

    [[nodiscard]]
    GateIdContainer const& getGates() const
    {
        return gates_;
    }

    [[nodiscard]]
    GateIdContainer getParents() const
    {
        return {first_parent, second_parent, third_parent};
    }

    [[nodiscard]]
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

/**
 * Сlass for coloring the whole circuit.
 */
class ThreeColoring
{
  public:
    std::vector<ThreeColor> colors;                         // list of all colors
    std::vector<std::vector<ColorId>> gateColors;           // contains up to 2 colors for each gate
    std::map<std::vector<GateId>, ColorId> parentsToColor;  // takes parent ids in ascdending order
    GateIdContainer negationUsers;

    /**
     * Shows the number of colors (number of subschemes found)
     */
    [[nodiscard]]
    size_t getColorsNumber() const
    {
        return next_color_id_;
    }

  protected:
    ColorId next_color_id_ = 0;

    /**
     * Create a new object of ThreeColor. This object represents a three-input subcircuit
     * that is planned to be simplified if possible.
     * @param first_parent -- first input
     * @param second_parent -- second input
     * @param third_parent -- third input
     * @return new color ID
     */
    ColorId addColor(GateId first_parent, GateId second_parent, GateId third_parent)
    {
        colors.emplace_back(first_parent, second_parent, third_parent);
        GateIdContainer const sortedParents = ThreeColor::sortedParents(first_parent, second_parent, third_parent);
        parentsToColor[sortedParents]       = next_color_id_;
        return next_color_id_++;
    }

    /**
     * Add new gate to existing color (object of ThreeColor; three-input subcircuit).
     * @param gateId -- gate ID what needs to be added
     * @param colorId -- color ID where need to add
     */
    void paintGate(GateId gateId, ColorId colorId)
    {
        colors.at(colorId).addGate(gateId);
        gateColors.at(gateId).push_back(colorId);
    }

  public:
    /**
     * Painting the whole circuit.
     */
    explicit ThreeColoring(ICircuit const& circuit)
    {
        // Top sort and some preparations
        csat::GateIdContainer gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(circuit));
        size_t const circuit_size = circuit.getNumberOfGates();
        gateColors.resize(circuit_size, {});
        negationUsers.resize(circuit_size, SIZE_MAX);

        // Color circuit in two colors
        TwoColoring twoColoring = TwoColoring(circuit);

        // Painting process in three color start from input to output
        for (uint64_t const gateId : std::ranges::reverse_view(gate_sorting))
        {
            GateIdContainer const& operands = circuit.getGateOperands(gateId);

            // Gate is input or constant
            if (operands.empty())
            {
                continue;
            }
            // Unary operation
            if (operands.size() == 1)
            {
                for (ColorId const color : gateColors.at(operands[0]))
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
                std::cerr << "ThreeColoring got circuit which gate has more than two operands. Gate id: " << gateId
                          << std::endl;
                std::abort();
            }

            ColorId const two_color = twoColoring.gateColor.at(gateId);
            // If gate doesn't have TwoColor, it won't have ThreeColor
            if (two_color == SIZE_MAX)
            {
                continue;
            }

            // Remember the color parents (subcircuit inputs)
            GateId const child_1 = twoColoring.colors.at(two_color).first_parent;
            GateId const child_2 = twoColoring.colors.at(two_color).second_parent;

            // If both gate's TwoColor parents don't have TwoColor parent, then gate won't have ThreeColor
            if (twoColoring.gateColor.at(child_1) == SIZE_MAX && twoColoring.gateColor.at(child_2) == SIZE_MAX)
            {
                continue;
            }

            std::vector<ColorId> common_colors = {};
            // Search for such children patterns
            ColorId color_type_13 = SIZE_MAX;
            ColorId color_type_31 = SIZE_MAX;

            // Remember colors from parents
            for (ColorId const first_child_color : gateColors.at(child_1))
            {
                for (ColorId const second_child_color : gateColors.at(child_2))
                {
                    if (first_child_color == second_child_color)
                    {
                        // Remember color if parents have the same color.
                        common_colors.push_back(first_child_color);
                    }
                    else if (colors.at(second_child_color).hasParent(child_1))
                    {
                        // Remember the color if the first parent is the parent of the second parent's color
                        color_type_13 = second_child_color;
                    }
                }
                if (colors.at(first_child_color).hasParent(child_2))
                {
                    // Remember the color if the second parent is the parent of the first parent's color
                    color_type_31 = first_child_color;
                }
            }

            // Paint our gates twice if both parents' colors match.
            if (common_colors.size() == 2)
            {
                paintGate(gateId, common_colors[0]);
                paintGate(gateId, common_colors[1]);
                continue;
            }

            // Paint our gates if one parents' color match and, if possible, we paint a second time
            // if one of the parents is the parent of the second parent's color.
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

            // If there are no identical colors in the parents, but the first parent is the parent
            // of the second parent's color.
            if (color_type_13 != SIZE_MAX)
            {
                paintGate(gateId, color_type_13);
                // Checks if the first parent color of a two-color coloring exists
                ColorId const first_child_two_color = twoColoring.gateColor.at(child_1);
                if (first_child_two_color != SIZE_MAX)
                {
                    // Find the parents of this color.
                    GateId const parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                    GateId const parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                    ColorId color_type_23 = SIZE_MAX;

                    // We go through the colors of the second parent and look for a color that has both of these
                    // parents.
                    for (ColorId const second_child_color : gateColors.at(child_2))
                    {
                        if (colors.at(second_child_color).hasParent(parent_1) &&
                            colors.at(second_child_color).hasParent(parent_2))
                        {
                            color_type_23 = second_child_color;
                            break;
                        }
                    }

                    // If we find them, we paint them, if not, we form a set of parents and either
                    // paint them in their existing color or create a new one
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

            // Similar to the previous part, but concerns the second parent.
            if (color_type_31 != SIZE_MAX)
            {
                paintGate(gateId, color_type_31);
                ColorId const second_child_two_color = twoColoring.gateColor.at(child_2);
                if (second_child_two_color != SIZE_MAX)
                {
                    GateId const parent_1 = twoColoring.colors.at(second_child_two_color).first_parent;
                    GateId const parent_2 = twoColoring.colors.at(second_child_two_color).second_parent;
                    ColorId color_type_32 = SIZE_MAX;

                    for (ColorId const first_child_color : gateColors.at(child_1))
                    {
                        if (colors.at(first_child_color).hasParent(parent_1) &&
                            colors.at(first_child_color).hasParent(parent_2))
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
            // We get the colors of the first and second parent from the two-color coloring array.
            ColorId const first_child_two_color  = twoColoring.gateColor.at(child_1);
            ColorId const second_child_two_color = twoColoring.gateColor.at(child_2);

            // If the second parent has a color, extract the parents for the second parent's color to
            // check their connections. Go through the colors of the first parent and check
            // if it has a color that has both of the found parents. If a color is found, color
            // the current gate with that color.
            if (second_child_two_color != SIZE_MAX)
            {
                GateId const parent_1 = twoColoring.colors.at(second_child_two_color).first_parent;
                GateId const parent_2 = twoColoring.colors.at(second_child_two_color).second_parent;
                ColorId color_type_32 = SIZE_MAX;
                for (ColorId const first_child_color : gateColors.at(child_1))
                {
                    if (colors.at(first_child_color).hasParent(parent_1) &&
                        colors.at(first_child_color).hasParent(parent_2))
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

            // Similar to the previous part, but concerns the first parent.
            if (first_child_two_color != SIZE_MAX)
            {
                GateId const parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                GateId const parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                ColorId color_type_23 = SIZE_MAX;
                for (ColorId const second_child_color : gateColors.at(child_2))
                {
                    if (colors.at(second_child_color).hasParent(parent_1) &&
                        colors.at(second_child_color).hasParent(parent_2))
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
            // If both parents have certain colors
            if (first_child_two_color != SIZE_MAX && second_child_two_color != SIZE_MAX)
            {
                // Extract four parents of two colors.
                GateId const parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                GateId const parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                GateId const parent_3 = twoColoring.colors.at(second_child_two_color).first_parent;
                GateId const parent_4 = twoColoring.colors.at(second_child_two_color).second_parent;

                // Check if the parents of the first color have a connection to the second color.
                // If so, create a container with the selected parents and color our gate either
                // in the existing color of these parents or in the newly created color.
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
                    // Else create containers for two colors with parents that includes the first and second parent
                    // of the first color and one of the parents of our gate color, and color the current gate.
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

            // Create new color or paint gate in the color of three parents
            GateIdContainer color_parents = {};
            if (first_child_two_color != SIZE_MAX)
            {
                GateId const parent_1 = twoColoring.colors.at(first_child_two_color).first_parent;
                GateId const parent_2 = twoColoring.colors.at(first_child_two_color).second_parent;
                color_parents         = ThreeColor::sortedParents(parent_1, parent_2, child_2);
            }
            else
            {
                GateId const parent_1 = twoColoring.colors.at(second_child_two_color).first_parent;
                GateId const parent_2 = twoColoring.colors.at(second_child_two_color).second_parent;
                color_parents         = ThreeColor::sortedParents(parent_1, parent_2, child_1);
            }

            if (parentsToColor.find(color_parents) == parentsToColor.end())
            {
                addColor(color_parents[0], color_parents[1], color_parents[2]);
            }
            paintGate(gateId, parentsToColor[color_parents]);
        }
    }
};

}  // namespace csat::utils