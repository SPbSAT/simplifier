#pragma once

#include "src/structures/circuit/icircuit.hpp"

#include <memory>

namespace csat
{

/**
 * Base class for boolean circuit builders.
 * @tparam CircuitT -- data structure that will
 * be returned by member-function `instantiate`.
 */
template<
    class CircuitT,
    typename = std::enable_if_t<
        std::is_base_of_v<ICircuit, CircuitT>
    >
>
class ICircuitBuilder
{
    static_assert(
        std::is_base_of<ICircuit, CircuitT>::value,
        "CircuitT template parameter must be a class, derived from ICircuit."
     );
    
  public:
    /**
     * Instantiates d CircuitT.
     * @return Circuit instance, built according to current parser info.
     */
    virtual std::unique_ptr<CircuitT> instantiate() = 0;
};

}  // namespace csat