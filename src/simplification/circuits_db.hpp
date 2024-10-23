#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/common/csat_types.hpp"

namespace csat::simplification
{

struct CircuitDB
{
    std::map<std::vector<int32_t>, int32_t> subcircuit_pattern_to_index;
    std::vector<GateIdContainer> subcircuit_outputs;
    std::vector<std::vector<GateIdContainer>> gates_operands;
    std::vector<int32_t> OPER_number;
    std::vector<std::vector<GateType>> gates_operations;

    CircuitDB(std::filesystem::path const& db_path, Basis basis)
    {
        if (!std::filesystem::exists(db_path))
        {
            std::cerr << "There is no small-circuit database at " << db_path.string() << std::endl;
            std::abort();
        }

        if (basis == Basis::BENCH)
        {
            read_bench(db_path);
        }
        else if (basis == Basis::AIG)
        {
            read_aig(db_path);
        }
    }

    void read_bench(std::filesystem::path const& db_path)
    {
        std::ifstream database(db_path);
        int32_t subcircuit_index = 0;
        size_t inputs_number     = 0;
        while (database >> inputs_number)
        {
            size_t outputs_number = 0;
            database >> outputs_number;
            std::vector<int32_t> outputs_patterns(outputs_number);
            for (size_t i = 0; i < outputs_number; ++i)
            {
                database >> outputs_patterns[i];
            }
            subcircuit_pattern_to_index[outputs_patterns] = subcircuit_index;
            std::vector<GateId> cur_outputs(outputs_number);
            GateId max_index = 0;
            for (size_t i = 0; i < outputs_number; ++i)
            {
                database >> cur_outputs[i];
                max_index = std::max(max_index, cur_outputs[i]);
            }
            subcircuit_outputs.push_back(cur_outputs);

            gates_operands.emplace_back();
            gates_operations.emplace_back();
            OPER_number.push_back(0);

            for (size_t i = 3; i <= max_index; ++i)
            {
                std::string operation;
                GateIdContainer operands;
                GateId operand_1 = 0;
                GateId operand_2 = 0;

                database >> operation;
                if (operation == "AND")
                {
                    database >> operand_1 >> operand_2;
                    gates_operations[subcircuit_index].push_back(GateType::AND);
                }
                else if (operation == "NOT")
                {
                    database >> operand_1;
                    gates_operations[subcircuit_index].push_back(GateType::NOT);
                }
                else if (operation == "OR")
                {
                    database >> operand_1 >> operand_2;
                    gates_operations[subcircuit_index].push_back(GateType::OR);
                }
                else if (operation == "XOR")
                {
                    database >> operand_1 >> operand_2;
                    gates_operations[subcircuit_index].push_back(GateType::XOR);
                }
                else if (operation == "NAND")
                {
                    database >> operand_1 >> operand_2;
                    gates_operations[subcircuit_index].push_back(GateType::NAND);
                }
                else if (operation == "NOR")
                {
                    database >> operand_1 >> operand_2;
                    gates_operations[subcircuit_index].push_back(GateType::NOR);
                }
                else if (operation == "NXOR")
                {
                    database >> operand_1 >> operand_2;
                    gates_operations[subcircuit_index].push_back(GateType::NXOR);
                }
                operands.push_back(operand_1);
                max_index = std::max(max_index, operand_1);
                if (operation != "NOT")
                {
                    operands.push_back(operand_2);
                    max_index = std::max(max_index, operand_2);
                    ++OPER_number[subcircuit_index];
                }
                gates_operands[subcircuit_index].push_back(operands);
            }
            ++subcircuit_index;
        }
    }

    void read_aig(std::filesystem::path const& db_path)
    {
        std::ifstream database(db_path);
        int32_t subcircuit_index = 0;
        size_t inputs_number     = 0;
        while (database >> inputs_number)
        {
            size_t outputs_number = 0;
            database >> outputs_number;
            std::vector<int32_t> outputs_patterns(outputs_number);
            for (size_t i = 0; i < outputs_number; ++i)
            {
                database >> outputs_patterns[i];
            }
            subcircuit_pattern_to_index[outputs_patterns] = subcircuit_index;
            std::vector<GateId> cur_outputs(outputs_number);
            GateId max_index = 0;
            for (size_t i = 0; i < outputs_number; ++i)
            {
                database >> cur_outputs[i];
                max_index = std::max(max_index, cur_outputs[i]);
            }
            subcircuit_outputs.push_back(cur_outputs);

            gates_operands.emplace_back();
            gates_operations.emplace_back();
            OPER_number.push_back(0);

            for (size_t i = 3; i <= max_index; ++i)
            {
                std::string operation;
                GateIdContainer operands;
                GateId operand_1 = 0;
                GateId operand_2 = 0;

                database >> operation;
                if (operation == "AND")
                {
                    database >> operand_1 >> operand_2;
                    gates_operations[subcircuit_index].push_back(GateType::AND);
                }
                else if (operation == "NOT")
                {
                    database >> operand_1;
                    gates_operations[subcircuit_index].push_back(GateType::NOT);
                }
                operands.push_back(operand_1);
                max_index = std::max(max_index, operand_1);
                if (operation == "AND")
                {
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

struct DBSingleton
{
  public:
    std::shared_ptr<CircuitDB> bench_db = nullptr;
    std::shared_ptr<CircuitDB> aig_db   = nullptr;

    DBSingleton(DBSingleton const&)            = delete;
    DBSingleton& operator=(DBSingleton const&) = delete;

    static DBSingleton& getInstance()
    {
        static DBSingleton s;
        return s;
    }

    static std::shared_ptr<CircuitDB> getAigDB()
    {
        if (DBSingleton::getInstance().aig_db == nullptr)
        {
            std::cerr << "Aig database is not available, aborting." << std::endl;
            std::abort();
        }
        return DBSingleton::getInstance().aig_db;
    }

    static std::shared_ptr<CircuitDB> getBenchDB()
    {
        if (DBSingleton::getInstance().bench_db == nullptr)
        {
            std::cerr << "Bench database is not available, aborting." << std::endl;
            std::abort();
        }
        return DBSingleton::getInstance().bench_db;
    }

  private:
    DBSingleton()  = default;
    ~DBSingleton() = default;
};

}  // namespace csat::simplification
