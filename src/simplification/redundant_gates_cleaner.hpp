#pragma once

#include <cassert>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "src/algo.hpp"
#include "src/common/csat_types.hpp"
#include "src/simplification/transformer_base.hpp"
#include "src/structures/circuit/gate_info.hpp"
#include "src/structures/circuit/icircuit.hpp"
#include "src/utility/encoder.hpp"
#include "src/utility/logger.hpp"

namespace csat::simplification
{

/**
 * Transformer, that cleans circuit from all
 * gates that are not reachable from outputs.
 *
 * @tparam CircuitT
 * @tparam preserveInputs -- if True, then Inputs won't be removed from a circuit.
 */
template<
    class CircuitT,
    bool preserveInputs = false,
    typename            = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT>>>
class RedundantGatesCleaner_ : public ITransformer<CircuitT>
{
    csat::Logger logger{"RedundantGatesCleaner"};

  public:
    /**
     * Applies RedundantGatesCleaner_ transformer to `circuit`
     * @param circuit -- circuit to transform.
     * @param encoder -- circuit encoder.
     * @return  circuit and encoder after transformation.
     */
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<GateEncoder> encoder)
    {
        logger.debug("=========================================================================================");
        logger.debug("START RedundantGatesCleaner.");

        GateEncoder new_encoder{};
        // Use dfs to get markers of visited and unvisited gates
        auto mask_use_output = algo::performDepthFirstSearch(*circuit, circuit->getOutputGates());

        // First step: getting valid encoding -- encode only gates, which will
        // be taken to the new circuit (hence discarding all redundant gates).
        for (GateId gateId = 0; gateId < circuit->getNumberOfGates(); ++gateId)
        {
            std::string_view gate_name = encoder->decodeGate(gateId);
            if (mask_use_output.at(gateId) != algo::DFSState::UNVISITED ||
                (preserveInputs && circuit->getGateType(gateId) == GateType::INPUT))
            {
                new_encoder.encodeGate(gate_name);
                continue;
            }

            logger.debug("Gate '", gate_name, "' (#", gateId, ") is redundant and will be removed");
        }

        // Second step: recollect each gate data by encoding all its operands with
        // new encoder build above.
        GateInfoContainer gate_info(new_encoder.size());
        for (GateId gateId = 0; gateId < circuit->getNumberOfGates(); ++gateId)
        {
            std::string_view gate_name = encoder->decodeGate(gateId);
            // If new encoder doesn't contain name of a gate, this gate is redundant.
            if (new_encoder.keyExists(gate_name))
            {
                GateIdContainer encoded_operands_{};
                for (GateId operand : circuit->getGateOperands(gateId))
                {
                    // All operands must be visited, since current gate was visited.
                    assert(
                        mask_use_output.at(gateId) != algo::DFSState::UNVISITED ||
                        (preserveInputs && circuit->getGateType(gateId) == GateType::INPUT));

                    std::string_view operand_name = encoder->decodeGate(operand);
                    encoded_operands_.push_back(new_encoder.encodeGate(operand_name));
                }
                // Build new gate info object for current gate.
                gate_info.at(new_encoder.encodeGate(gate_name)) = {
                    circuit->getGateType(gateId), std::move(encoded_operands_)};
            }
        }

        // Third step: recollect output gates.
        // All outputs must be visited since DFS starts from them.
        GateIdContainer new_output_gates{};
        new_output_gates.reserve(circuit->getOutputGates().size());
        for (GateId output_gate : circuit->getOutputGates())
        {
            assert(
                mask_use_output.at(output_gate) != algo::DFSState::UNVISITED ||
                (preserveInputs && circuit->getGateType(output_gate) == GateType::INPUT));

            std::string_view output_name = encoder->decodeGate(output_gate);
            new_output_gates.push_back(new_encoder.encodeGate(output_name));
        }

        logger.debug("END RedundantGatesCleaner.");
        logger.debug("=========================================================================================");
        return {
            std::make_unique<CircuitT>(std::move(gate_info), std::move(new_output_gates)),
            std::make_unique<GateEncoder>(new_encoder)};
    };
};

}  // namespace csat::simplification
