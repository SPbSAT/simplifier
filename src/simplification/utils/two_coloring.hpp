#pragma once

#include "src/simplification/transformer_base.hpp"
#include "src/algo.hpp"
#include "src/utility/converters.hpp"
#include "src/common/csat_types.hpp"

#include <cassert>
#include <ranges>
#include <vector>
#include <unordered_map>
#include <memory>


namespace csat::utils
{

using ColorId = size_t;

struct TwoColor
{
  public:
    // Keep parent ids ordered by ascending
    GateId first_parent;
    GateId second_parent;

  protected:
    GateIdContainer gates_;

  public:
    TwoColor(GateId first_parent, GateId second_parent):
        first_parent(std::min(first_parent, second_parent)),
        second_parent(std::max(first_parent, second_parent)) {};

    void addGate(GateId gateId)
    {
        gates_.push_back(gateId);
    }

    [[nodiscard]] GateIdContainer const& getGates() const
    {
        return gates_;
    }

    [[nodiscard]] GateIdContainer getParents() const
    {
        return {first_parent, second_parent};
    }

    [[nodiscard]] bool hasParent(GateId gateId) const  
    {  
        return first_parent == gateId || second_parent == gateId;  
    }

    static GateIdContainer sortedParents(GateId first_parent, GateId second_parent)
    {
        return {std::min(first_parent, second_parent), std::max(first_parent, second_parent)};
    }
};


class TwoColoring
{
  public:
    std::vector<TwoColor> colors;
    std::vector<ColorId> gateColor; // if vertex is not colored, then value is 'SIZE_MAX'
    // TODO: use better key type for this map.
    std::map<GateIdContainer, ColorId> parentsToColor;

    [[nodiscard]] size_t getColorsNumber() const
    {
        return next_color_id_;
    }

    /**
    * Returns 'True' if 'gateId' is a parent for following 'color', otherwise returns 'False'
    */
    [[nodiscard]] bool isParentOfColor(GateId gateId, ColorId colorId) const
    {
        return colors[colorId].hasParent(gateId); 
    }

  protected:
    ColorId next_color_id_ = 0;

    ColorId addColor(GateId first_parent, GateId second_parent)
    {
        colors.emplace_back(first_parent, second_parent);
        parentsToColor[TwoColor::sortedParents(first_parent, second_parent)] = next_color_id_;
        return next_color_id_++;
    }

    void paintGate(GateId gateId, ColorId colorId)
    {
        colors.at(colorId).addGate(gateId);
        gateColor.at(gateId) = colorId;
    }

  public:
    explicit TwoColoring(ICircuit const& circuit)
    {
        csat::GateIdContainer gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(circuit));
        size_t const circuit_size = circuit.getNumberOfGates();
        gateColor.resize(circuit_size, SIZE_MAX);

        // TODO: maybe remove from here and evaluate in block with strategy
        // User constructed as negation of chosen gate ('SIZE_MAX' if there is no such users)
        GateIdContainer negationUsers(circuit_size, SIZE_MAX);

        // Painting process
        for (unsigned long const gateId : std::ranges::reverse_view(gate_sorting))
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
                ColorId const newColor = gateColor.at(operands.at(0));
                if (newColor != SIZE_MAX)
                {
                    paintGate(gateId, newColor);
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
                std::cerr << "TwoColoring got circuit which gate has more than two operands. Gate id: " << gateId << std::endl;
                std::exit(-1);  
            }

            GateId child_1 = operands[0];
            GateId child_2 = operands[1];
            while (circuit.getGateOperands(child_1).size() == 1)
            {
                child_1 = circuit.getGateOperands(child_1)[0];
            }
            while (circuit.getGateOperands(child_2).size() == 1)
            {
                child_2 = circuit.getGateOperands(child_2)[0];
            }

            ColorId const color_1 = gateColor.at(child_1);
            ColorId const color_2 = gateColor.at(child_2);

            if (child_1 == child_2)
            {
                if (color_1 != SIZE_MAX)
                {
                    paintGate(gateId, color_1);
                }
                continue;
            }

            GateIdContainer const children = TwoColor::sortedParents(child_1, child_2);
            if (color_1 != SIZE_MAX && color_1 == color_2)
            {
                paintGate(gateId, color_1);
            }
            else if (color_1 != SIZE_MAX && isParentOfColor(child_2, color_1))
            {
                paintGate(gateId, color_1);
            }
            else if (color_2 != SIZE_MAX && isParentOfColor(child_1, color_2))
            {
                paintGate(gateId, color_2);
            }
            else if (parentsToColor.find(children) == parentsToColor.end())
            {
                ColorId const newColor = addColor(child_1, child_2);
                paintGate(gateId, newColor);
            }
            else
            {
                paintGate(gateId, parentsToColor.at(children));
            }
        }
    }
};

}  // namespace csat::utils