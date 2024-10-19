#pragma once

#include "src/structures/assignment/iassignment.hpp"

#include "src/common/csat_types.hpp"

#include <vector>


namespace csat
{

using StateVector = std::vector<GateState>;

/**
 * Trivial assignment realisation using a vector.
 *
 * @param DynamicResize -- if true, enables automatic resizing.
 */
template<bool DynamicResize = true>
struct VectorAssignment : public IAssignment
{
  protected:
    /* carries current assignment in a vector */
    StateVector gate_state_;
   
  public:
    VectorAssignment() = default;
    ~VectorAssignment() override = default;
    
    void assign(
        GateId gateId,
        GateState state) final
    {
        if constexpr (DynamicResize)
        {
            ensureCapacity(gateId);
        }
        gate_state_.at(gateId) = state;
    };
    
    [[nodiscard]]
    GateState getGateState(GateId gateId) const final
    {
        if (containsValueOf_(gateId))
        {
            return gate_state_.at(gateId);
        }
        else
        {
            return GateState::UNDEFINED;
        }
    };
    
    [[nodiscard]]
    bool isUndefined(GateId gateId) const final
    {
        return getGateState(gateId) == GateState::UNDEFINED;
    };
    
    void clear() noexcept override
    {
        gate_state_ = StateVector();
    }
    
    void ensureCapacity(GateId sz) final
    {
        gate_state_.resize(
            std::max(sz + 1, gate_state_.size()),
            GateState::UNDEFINED);
    }
   
  protected:
    [[nodiscard]]
    bool containsValueOf_(GateId gateId) const noexcept
    {
        return gate_state_.size() > gateId;
    };
};

} // csat namespace
