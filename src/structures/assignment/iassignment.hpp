#pragma once

#include "src/common/csat_types.hpp"

namespace csat
{

/**
 * Interface that defines behaviour for structures, that carry
 * current assignment (TRUE, FALSE, UNDEFINED) of Circuit gates.
 *
 * Assignment is a (full or partial) assignment of gate outputs to
 * GateState value (e.g. False, True, Undefined). Circuit could be
 * evaluated on specific assignment, if it is sufficient to imply
 * values of output gates.
 */
class IAssignment
{
  public:
    IAssignment()          = default;
    virtual ~IAssignment() = default;

    /* Sets state of gate `gateId` to `state` */
    virtual void assign(GateId, GateState) = 0;
    /* Clears all assignments and history. */
    virtual void clear() = 0;

    /* Ensures capacity of underlying array is enough to assign gate value. If it is not, resizes array. */
    virtual void ensureCapacity(GateId) = 0;

    /* Returns state of gate `gateId`. Must return UNDEFINED when GateId is not assigned yet. */
    [[nodiscard]]
    virtual GateState getGateState(GateId) const = 0;
    /* Returns True iff gate state is not set or is explicitly set UNDEFINED. */
    [[nodiscard]]
    virtual bool isUndefined(GateId) const = 0;
};

}  // namespace csat
