#pragma once

#include <algorithm>
#include <memory>
#include <stack>
#include <utility>

#include "src/common/csat_types.hpp"
#include "src/common/operators.hpp"
#include "src/structures/assignment/vector_assignment.hpp"
#include "src/structures/circuit/gate_info.hpp"

namespace csat
{

/**
 * Interface that defines structures,
 * that carry circuits topology.
 *
 * Circuit consist of gates and relations between them. Gates could be
 * variables (INPUT) and operators (NOT, AND, NAND, OR, NOR, XOR, NXOR, etc.).
 *
 * Invariants:
 *
 * 1. Implementation must guarantee, that its instance contains all nodes from `0` to `getNumberOfGates()` value.
 * 2. Id of any operand of any gate must lie between `0` and `getNumberOfGates()` value.
 *
 */
class ICircuit
{
  public:
    ICircuit()                = default;
    ICircuit(ICircuit const&) = default;
    virtual ~ICircuit()       = default;
    ICircuit(GateInfoContainer const& /*unused*/, GateIdContainer const& /*unused*/){};

    // ========== Circuit Info ========== //
    /* Returns type of gate. */
    [[nodiscard]]
    virtual GateType getGateType(GateId gateId) const = 0;
    /* Returns operands of gate. */
    [[nodiscard]]
    virtual GateIdContainer const& getGateOperands(GateId gateId) const = 0;
    /* Returns users of gate -- gates, that use current as operand. */
    [[nodiscard]]
    virtual GateIdContainer const& getGateUsers(GateId gateId) const = 0;
    /* Returns number of gates in structures. */
    [[nodiscard]]
    virtual GateId getNumberOfGates() const = 0;
    /* Returns constant reference to GateIdContainer with all output gates. */
    [[nodiscard]]
    virtual GateIdContainer const& getOutputGates() const = 0;
    /* Returns constant reference to GateIdContainer with all input gates. */
    [[nodiscard]]
    virtual GateIdContainer const& getInputGates() const = 0;
    /* Returns true iff gate is output. */
    [[nodiscard]]
    virtual bool isOutputGate(GateId gateId) const = 0;

    // ========== Circuit Evaluation Methods ========== //
    /**
     * @param input_asmt -- some (partial) assignment.
     * @return additional to input_asmt (with no intersection) assignment,
     * of gates, which values could be implied by input_asmt. Evaluates
     * only gates reachable from outputs.
     *
     * @tparam AssignmentT -- structure to carry resulting assignment.
     */
    template<
        class AssignmentT = VectorAssignment<false>,
        typename          = std::enable_if_t<std::is_base_of_v<IAssignment, AssignmentT> > >
    [[nodiscard]]
    std::unique_ptr<AssignmentT> evaluateCircuit(IAssignment const& input_asmt) const
    {
        auto internal_asmt = std::make_unique<AssignmentT>();
        internal_asmt->ensureCapacity(getNumberOfGates());
        evaluateCircuit_(input_asmt, getOutputGates(), *internal_asmt);

        return internal_asmt;
    }

  protected:
    void evaluateCircuit_(IAssignment const& input_asmt, GateIdContainer const& sinks, IAssignment& internal_asmt) const
    {
        for (auto sink : sinks)
        {
            evaluateGate_(sink, input_asmt, internal_asmt);
        }
    }

    /* Internal purpose gate evaluation. */
    GateState evaluateGate_(GateId gateId, IAssignment const& input_asmt, IAssignment& internal_asmt) const
    {
        std::stack<GateId> queue_{};
        queue_.push(gateId);
        BoolVector evaluated_(getNumberOfGates(), 0);

        auto get_gate_state_ = [&input_asmt, &internal_asmt](GateId operand)
        {
            return (
                internal_asmt.isUndefined(operand) ? input_asmt.getGateState(operand)
                                                   : internal_asmt.getGateState(operand));
        };

        while (!queue_.empty())
        {
            GateId const currentGateId = queue_.top();

            // Gate state is set or gate is Input. If gate is Input, its
            // state must be either set in input_asmt, or be Unknown.
            if (getGateType(currentGateId) == GateType::INPUT || !input_asmt.isUndefined(currentGateId))
            {
                internal_asmt.assign(currentGateId, input_asmt.getGateState(currentGateId));
                evaluated_.at(currentGateId) = 1;
                queue_.pop();
                continue;
            }

            bool operandsEvaluated = true;
            for (auto operandId : getGateOperands(currentGateId))
            {
                if (evaluated_.at(operandId) == 0)
                {
                    operandsEvaluated = false;
                    queue_.push(operandId);
                }
            }

            if (operandsEvaluated)
            {
                op::OperatorNT<GateId> oper = op::getOperatorNT<GateId>(getGateType(currentGateId));

                internal_asmt.assign(currentGateId, oper(getGateOperands(currentGateId), get_gate_state_));
                evaluated_.at(currentGateId) = 1;
                queue_.pop();
                continue;
            }
        }

        return internal_asmt.getGateState(gateId);
    }
};

}  // namespace csat
