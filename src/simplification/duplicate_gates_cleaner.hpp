#pragma once

#include "src/simplification/transformer_base.hpp"
#include "src/algo.hpp"
#include "src/utility/converters.hpp"

#include <vector>
#include <type_traits>
#include <memory>


namespace csat::simplification
{


/**
 * Transformer, that cleans circuit from duplicate gates.
 * Duplicates are gates with the same operands and operator
 *
 * Note that this algorithm requires RedundantGatesCleaner to be applied right before.
 *
 * @tparam CircuitT
 */
template<
    class CircuitT,
    typename = std::enable_if_t<
    std::is_base_of_v<ICircuit, CircuitT>
    >
>
class DuplicateGatesCleaner_ : public ITransformer<CircuitT>
{
    csat::Logger logger{"DuplicateGatesCleaner"};
  
  public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<GateEncoder<std::string>> encoder)
    {
        logger.debug("=========================================================================================");
        logger.debug("START DuplicateGatesCleaner");
        
        logger.debug("Top sort");
        csat::GateIdContainer gateSorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit));
        std::reverse(gateSorting.begin(), gateSorting.end());
        
        logger.debug("Building mask to delete gates and filling map -- old_to_new_gateId");
        BoolVector safe_mask(circuit->getNumberOfGates(), true); // 0 -- if gate is a duplicate, 1 -- otherwise
        GateEncoder<std::string> auxiliary_names_encoder{}; // maps encoded (`operator_operand1_operand2...`) gate to new gate id
        std::map<GateId, GateId> old_to_new_gateId{}; // surjection of old gate ids to new gate ids
        GateEncoder<GateId> new_encoder{}; // bijection of old gate ids to new gate ids
        std::string encoded_name;
        
        for (GateId gateId: gateSorting)
        {
            encoded_name = get_gate_auxiliary_name_(
                gateId,
                circuit->getGateType(gateId),
                circuit->getGateOperands(gateId),
                old_to_new_gateId
            );
            logger.debug("Gate number ", gateId, ". Its encoded_name is ", encoded_name);
            
            if (auxiliary_names_encoder.keyExists(encoded_name)) {
                logger.debug("Gate number ", gateId, " is a Duplicate and will be removed.");
                safe_mask.at(gateId) = false;
            }
            else
            {
                new_encoder.encodeGate(gateId);
            }
            
            old_to_new_gateId[gateId] = auxiliary_names_encoder.encodeGate(encoded_name);
        }
        
        logger.debug("Building new circuit");
        GateInfoContainer gate_info(auxiliary_names_encoder.size());
        for (GateId gateId = 0; gateId < circuit->getNumberOfGates(); ++gateId)
        {
            if (safe_mask.at(gateId))
            {
                logger.debug(
                    "New Gate ", old_to_new_gateId.at(gateId),
                    "; Type: ", utils::gateTypeToString(circuit->getGateType(gateId)),
                    "; Operands: "
                );
                
                GateIdContainer masked_operands_{};
                for (GateId operand: circuit->getGateOperands(gateId))
                {
                    masked_operands_.push_back(old_to_new_gateId.at(operand));
                    logger.debug(old_to_new_gateId.at(operand));
                }
                gate_info.at(old_to_new_gateId.at(gateId)) = {
                    circuit->getGateType(gateId),
                    masked_operands_
                };
            }
        }
        
        GateIdContainer new_output_gates{};
        new_output_gates.reserve(circuit->getOutputGates().size());
        for (GateId output_gate: circuit->getOutputGates())
        {
            new_output_gates.push_back(old_to_new_gateId.at(output_gate));
        }
    
        logger.debug("END DuplicateGatesCleaner");
        logger.debug("=========================================================================================");
        return {
            std::make_unique<CircuitT>(gate_info, new_output_gates),
            utils::mergeGateEncoders(*encoder, new_encoder)
        };
    };
  
  private:
    std::string get_gate_auxiliary_name_(
        GateId idx,
        GateType type,
        GateIdContainer const& operands,
        std::map<GateId, GateId> const& encoder)
    {
        std::string encoded_name;
        encoded_name = std::to_string(static_cast<int>(type));
        
        if (type == GateType::INPUT)
        {
            encoded_name += '_' + std::to_string(idx);
        }
        
        for (GateId operand: operands)
        {
            encoded_name += '_' + std::to_string(encoder.at(operand));
        }
        
        return encoded_name;
    };
};


} // csat namespace
