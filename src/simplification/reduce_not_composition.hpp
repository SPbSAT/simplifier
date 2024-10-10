#pragma once

#include "src/simplification/transformer_base.hpp"
#include "src/algo.hpp"
#include "src/utility/converters.hpp"
#include "src/common/csat_types.hpp"

#include <vector>
#include <type_traits>
#include <memory>


namespace csat::simplification
{


/**
 * Transformer, that cleans the circuit from unnecessary gates NOT.
 * For example: NOT(NOT(x)) => x
 *
 * Note that this algorithm requires RedundantGatesCleaner to be applied right after.
 *
 * @tparam CircuitT
 */
template<
    class CircuitT,
    typename = std::enable_if_t<
        std::is_base_of_v<ICircuit, CircuitT>
    >
>
class ReduceNotComposition_ : public ITransformer<CircuitT>
{
  private:
    csat::Logger logger{"ReduceNotComposition"};
    std::set<GateType> validParams;
  
  public:
    CircuitAndEncoder<CircuitT, std::string> transform(
        std::unique_ptr<CircuitT> circuit,
        std::unique_ptr<GateEncoder<std::string>> encoder)
    {
        logger.debug("=========================================================================================");
        logger.debug("START ReduceNotComposition");
        
        logger.debug("Top sort");
        csat::GateIdContainer gate_sorting(algo::TopSortAlgorithm<algo::DFSTopSort>::sorting(*circuit));
        
        logger.debug("Rebuild schema");
        GateInfoContainer gate_info(circuit->getNumberOfGates());
        
        for (GateId gateId : gate_sorting)
        {
            GateIdContainer new_operands_{};
            for (GateId operands : circuit->getGateOperands(gateId))
            {
                if (circuit->getGateType(operands) == GateType::NOT)
                {
                    new_operands_.push_back(get_operand(*circuit, operands));
                }
                else
                {
                    new_operands_.push_back(operands);
                }
            }
            gate_info.at(gateId) = {circuit->getGateType(gateId), new_operands_};
        }
        
        logger.debug("END ReduceNotComposition");
        logger.debug("=========================================================================================");
        
        return {
            std::make_unique<CircuitT>(gate_info, circuit->getOutputGates()),
            std::make_unique<GateEncoder<std::string>>(*encoder)
        };
    };

  private:
    inline GateId get_operand(CircuitT const& circuit, GateId gateId)
    {
        bool flag = false;
        GateId check_gate = circuit.getGateOperands(gateId).at(0);
        while (circuit.getGateType(check_gate) == GateType::NOT)
        {
            flag = not flag;
            gateId = check_gate;
            check_gate = circuit.getGateOperands(gateId).at(0);
        }
    
        if (flag)
        {
            return check_gate;
        }
        else
        {
            return gateId;
        }
    }
};

} // csat namespace
