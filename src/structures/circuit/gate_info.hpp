#pragma once

#include <algorithm>
#include <vector>
#include "src/utility/converters.hpp"


namespace csat
{

/**
 * Auxiliary structure to carry information about some gate.
 *
 * Invariants:
 *   - operands are always sorted in ascending order.
 *
 */
struct GateInfo
{
  protected:
    GateType type_ = GateType::UNDEFINED;
    GateIdContainer operands_;
  
  public:
    GateInfo() = default;
    ~GateInfo() = default;
    
    GateInfo(GateType type, GateIdContainer const& operands)
        : type_(type)
    {
        operands_ = GateIdContainer(operands.size());
        if (csat::utils::symmetricOperatorQ(type))
        {
            std::partial_sort_copy(
                operands.begin(),
                operands.end(),
                operands_.begin(),
                operands_.end());
        }
        else
        {
            std::copy(
                operands.begin(),
                operands.end(),
                operands_.begin());
        }
    }
    
    GateInfo(GateType type, GateIdContainer&& operands)
        : type_(type)
        , operands_(std::move(operands))
    {
        if (csat::utils::symmetricOperatorQ(type))
        {
           std::sort(operands_.begin(), operands_.end());
        }
    }
    
    [[nodiscard]]
    GateIdContainer const& getOperands() const
    {
        return operands_;
    }
    
    [[nodiscard]]
    GateType getType() const
    {
        return type_;
    }
    
    [[nodiscard]]
    GateIdContainer&& moveOperands()
    {
        // This function leaves `this` in
        // invalid state. Use it carefully.
        return std::move(operands_);
    }
};


/** At i'th position carries info about gate with Id = i. **/
using GateInfoContainer = std::vector<GateInfo>;


} // namespace csat
