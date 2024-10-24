#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <type_traits>

#include "src/common/csat_types.hpp"
#include "src/structures/circuit/dag.hpp"
#include "src/structures/circuit/icircuit.hpp"
#include "src/utility/converters.hpp"
#include "src/utility/encoder.hpp"
#include "src/utility/logger.hpp"

namespace csat
{

/**
 * Write the circuit to a bench file
 *
 * @tparam CircuitT
 */
template<class CircuitT, typename = std::enable_if_t<std::is_base_of_v<ICircuit, CircuitT> > >
void writeBenchFile(
    CircuitT const& circuit,
    csat::utils::GateEncoder<std::string> const& encoder,
    std::ofstream& file_out)
{
    csat::Logger const logger("writeBenchFile");
    logger.debug("writeBenchFile start.");

    logger.debug("recording INPUTs.");
    for (GateId input : circuit.getInputGates())
    {
        file_out << "INPUT(" << encoder.decodeGate(input) << ")\n";
    }
    file_out << "\n";

    logger.debug("recording OUTPUTs.");
    for (GateId output : circuit.getOutputGates())
    {
        file_out << "OUTPUT(" << encoder.decodeGate(output) << ")\n";
    }
    file_out << "\n";

    logger.debug("recording Gates.");
    for (size_t gateId = 0; gateId < circuit.getNumberOfGates(); ++gateId)
    {
        if (circuit.getGateType(gateId) != GateType::INPUT)
        {
            file_out << encoder.decodeGate(gateId) << " = "
                     << csat::utils::gateTypeToString(circuit.getGateType(gateId)) << "(";

            auto operands       = circuit.getGateOperands(gateId);
            size_t num_operands = operands.size();

            if (num_operands == 0)
            {
                file_out << ")\n";
                continue;
            }

            for (size_t operand = 0; operand < (num_operands - 1); ++operand)
            {
                file_out << encoder.decodeGate(operands.at(operand)) << ", ";
            }
            file_out << encoder.decodeGate(operands.at(num_operands - 1)) << ")\n";
        }
    }
    logger.debug("writeBenchFile end.");
}

/**
 * Prints circuit to stdout where each gate is written in the following notation
 * "<encoded name> => <name from original file>"
 */
void printCircuit(csat::DAG const& circuit, csat::utils::GateEncoder<std::string> const& encoder)
{
    for (auto input : circuit.getInputGates())
    {
        std::cout << "INPUT(" << input << " => " << encoder.decodeGate(input) << ")\n";
    }

    for (auto output : circuit.getOutputGates())
    {
        std::cout << "OUTPUT(" << output << " => " << encoder.decodeGate(output) << ")\n";
    }

    for (size_t gateId = 0; gateId < circuit.getNumberOfGates(); ++gateId)
    {
        if (circuit.getGateType(gateId) != csat::GateType::INPUT)
        {
            std::cout << gateId << " => " << encoder.decodeGate(gateId) << " = "
                      << csat::utils::gateTypeToString(circuit.getGateType(gateId)) << "(";

            auto operands       = circuit.getGateOperands(gateId);
            size_t num_operands = operands.size();

            if (num_operands == 0)
            {
                std::cout << ")\n";
                continue;
            }

            for (size_t operand = 0; operand < (num_operands - 1); ++operand)
            {
                std::cout << operands.at(operand) << " => " << encoder.decodeGate(operands.at(operand)) << ", ";
            }
            std::cout << operands.at(num_operands - 1) << " => " << encoder.decodeGate(operands.at(num_operands - 1))
                      << ")\n";
        }

        std::cout << std::flush;
    }
}

}  // namespace csat
