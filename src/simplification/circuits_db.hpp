#pragma once

#include <filesystem>

namespace csat::simplification {

    enum class Basis {
        BENCH,
        AIG
    };

    struct CircuitDB {
        std::map<std::vector<int32_t>, int32_t> subcircuit_pattern_to_index;
        std::vector<GateIdContainer> subcircuit_outputs;
        std::vector<std::vector<GateIdContainer>> gates_operands;
        std::vector<int32_t> OPER_number;
        std::vector<std::vector<GateType>> gates_operations;

        inline CircuitDB(const std::filesystem::path &db_path, Basis basis) {
            if (basis == Basis::BENCH) {
                read_bench(db_path);
            } else if (basis == Basis::AIG) {
                read_aig(db_path);
            }
        }

        inline void read_bench(const std::filesystem::path &db_path) {
            std::ifstream database(db_path);
            int32_t subcircuit_index = 0;
            size_t inputs_number;
            while (database >> inputs_number) {
                size_t outputs_number;
                database >> outputs_number;
                std::vector<int32_t> outputs_patterns(outputs_number);
                for (size_t i = 0; i < outputs_number; ++i) {
                    database >> outputs_patterns[i];
                }
                subcircuit_pattern_to_index[outputs_patterns] = subcircuit_index;
                std::vector<GateId> cur_outputs(outputs_number);
                GateId max_index = 0;
                for (size_t i = 0; i < outputs_number; ++i) {
                    database >> cur_outputs[i];
                    max_index = std::max(max_index, cur_outputs[i]);
                }
                subcircuit_outputs.push_back(cur_outputs);

                gates_operands.push_back({});
                gates_operations.push_back({});
                OPER_number.push_back(0);

                for (size_t i = 3; i <= max_index; ++i) {
                    std::string operation;
                    GateIdContainer operands;
                    GateId operand_1, operand_2;

                    database >> operation;
                    if (operation == "AND") {
                        database >> operand_1 >> operand_2;
                        gates_operations[subcircuit_index].push_back(GateType::AND);
                    } else if (operation == "NOT") {
                        database >> operand_1;
                        gates_operations[subcircuit_index].push_back(GateType::NOT);
                    } else if (operation == "OR") {
                        database >> operand_1 >> operand_2;
                        gates_operations[subcircuit_index].push_back(GateType::OR);
                    } else if (operation == "XOR") {
                        database >> operand_1 >> operand_2;
                        gates_operations[subcircuit_index].push_back(GateType::XOR);
                    } else if (operation == "NAND") {
                        database >> operand_1 >> operand_2;
                        gates_operations[subcircuit_index].push_back(GateType::NAND);
                    } else if (operation == "NOR") {
                        database >> operand_1 >> operand_2;
                        gates_operations[subcircuit_index].push_back(GateType::NOR);
                    } else if (operation == "NXOR") {
                        database >> operand_1 >> operand_2;
                        gates_operations[subcircuit_index].push_back(GateType::NXOR);
                    }
                    operands.push_back(operand_1);
                    max_index = std::max(max_index, operand_1);
                    if (operation != "NOT") {
                        operands.push_back(operand_2);
                        max_index = std::max(max_index, operand_2);
                        ++OPER_number[subcircuit_index];
                    }
                    gates_operands[subcircuit_index].push_back(operands);
                }
                ++subcircuit_index;
            }
        }

        inline void read_aig(const std::filesystem::path &db_path) {
            std::ifstream database(db_path);
            int32_t subcircuit_index = 0;
            size_t inputs_number;
            while (database >> inputs_number) {
                size_t outputs_number;
                database >> outputs_number;
                std::vector<int32_t> outputs_patterns(outputs_number);
                for (size_t i = 0; i < outputs_number; ++i) {
                    database >> outputs_patterns[i];
                }
                subcircuit_pattern_to_index[outputs_patterns] = subcircuit_index;
                std::vector<GateId> cur_outputs(outputs_number);
                GateId max_index = 0;
                for (size_t i = 0; i < outputs_number; ++i) {
                    database >> cur_outputs[i];
                    max_index = std::max(max_index, cur_outputs[i]);
                }
                subcircuit_outputs.push_back(cur_outputs);

                gates_operands.push_back({});
                gates_operations.push_back({});
                OPER_number.push_back(0);

                for (size_t i = 3; i <= max_index; ++i) {
                    std::string operation;
                    GateIdContainer operands;
                    GateId operand_1, operand_2;

                    database >> operation;
                    if (operation == "AND") {
                        database >> operand_1 >> operand_2;
                        gates_operations[subcircuit_index].push_back(GateType::AND);
                    } else if (operation == "NOT") {
                        database >> operand_1;
                        gates_operations[subcircuit_index].push_back(GateType::NOT);
                    }
                    operands.push_back(operand_1);
                    max_index = std::max(max_index, operand_1);
                    if (operation == "AND") {
                        operands.push_back(operand_2);
                        max_index = std::max(max_index, operand_2);
                        ++OPER_number[subcircuit_index];
                    }
                    gates_operands[subcircuit_index].push_back(operands);
                }
                ++subcircuit_index;
            }
        }
    };

}