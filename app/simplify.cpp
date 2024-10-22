#include <cerrno>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

#include "src/parser/bench_to_circuit.hpp"
#include "src/simplification/composition.hpp"
#include "src/simplification/strategy.hpp"
#include "src/simplification/three_inputs_optimization.hpp"
#include "src/simplification/three_inputs_optimization_bench.hpp"
#include "src/structures/circuit/dag.hpp"
#include "src/utility/encoder.hpp"
#include "src/utility/logger.hpp"
#include "src/utility/write_utils.hpp"
#include "third_party/argparse/include/argparse/argparse.hpp"

/**
 * prints circuit (encoded name => name from file)
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
    }
}

void parseArguments(int argn, char** argv, argparse::ArgumentParser& program)
{
    try
    {
        program.parse_args(argn, argv);
    }
    catch (std::runtime_error const& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::abort();
    }
}

std::ifstream openInputFile(std::string const& file_path, csat::Logger& logger)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Can't open file, path is incorrect." << std::endl;
        std::abort();
    }
    logger.error("File opened.");
    return file;
}

std::unique_ptr<csat::DAG> parseCircuitFile(
    std::ifstream& file,
    std::string const& file_path,
    csat::Logger& logger,
    csat::utils::GateEncoder<std::string>& encoder)
{
    logger.debug("Reading file ", file_path, ".");
    auto parser = csat::parser::BenchToCircuit<csat::DAG>();
    parser.parseStream(file);
    encoder = parser.getEncoder();
    return parser.instantiate();
}

std::tuple<std::unique_ptr<csat::DAG>, std::unique_ptr<csat::utils::GateEncoder<std::string> > > applyPreprocessing(
    std::string const& basis,
    std::unique_ptr<csat::DAG>& csat_instance,
    csat::utils::GateEncoder<std::string>& encoder)
{
    if (basis == "AIG")
    {
        return csat::simplification::Composition<
                   csat::DAG,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimization<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimization<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimization<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimization<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimization<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG> >()
            .apply(*csat_instance, encoder);
    }
    else if (basis == "BENCH")
    {
        return csat::simplification::Composition<
                   csat::DAG,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimizationBench<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimizationBench<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimizationBench<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimizationBench<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                   csat::simplification::ThreeInputsSubcircuitMinimizationBench<csat::DAG>,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG> >()
            .apply(*csat_instance, encoder);
    }
    else
    {
        std::cerr << "Incorrect basis! Choose one of [AIG, BENCH]" << std::endl;
        std::abort();
    }
}

void writeOutputFiles(
    argparse::ArgumentParser const& program,
    std::unique_ptr<csat::DAG>& csat_instance,
    csat::utils::GateEncoder<std::string>& encoder,
    std::string const& file_path)
{
    if (auto output_dir = program.present("-o"))
    {
        std::ofstream file_out(*output_dir / std::filesystem::path(file_path).filename());
        WriteBenchFile(*csat_instance, encoder, file_out);
    }
    else
    {
        printCircuit(*csat_instance, encoder);
    }
}

std::optional<std::ofstream> openFileStat(argparse::ArgumentParser const& program)
{
    if (auto output_file = program.present("-s"))
    {
        std::ofstream file_stat(*output_file);
        file_stat << std::setprecision(3) << std::fixed;
        file_stat << "File path,Gates before,Gates after,Preprocessing time";
        for (int i = 0; i < 5; ++i)
        {
            file_stat << ",subcircuits_number_" << i;
        }
        for (int i = 0; i < 5; ++i)
        {
            file_stat << ",skipped_subcircuits_" << i;
        }
        for (int i = 0; i < 5; ++i)
        {
            file_stat << ",max_subcircuits_size_" << i;
        }
        for (int i = 0; i < 5; ++i)
        {
            file_stat << ",circuit_size_" << i;
        }
        file_stat << ",iter_number,total_gates_in_subcircuits\n";
        return file_stat;
    }
    return std::nullopt;
}

void writeStatistics(
    std::optional<std::ofstream>& file_stat,
    std::string const& file_path,
    int64_t gatesBefore,
    int64_t gatesAfter,
    long double simplifyTime,
    std::vector<int32_t> const& subcircuits_number_by_iter,
    std::vector<int32_t> const& skipped_subcircuits_by_iter,
    std::vector<int32_t> const& max_subcircuit_size_by_iter,
    std::vector<int32_t> const& circuit_size_by_iter,
    int64_t iterationsNumber,
    int64_t total_gates_in_subcircuits)
{
    if (file_stat)
    {
        *file_stat << std::setprecision(3) << std::fixed;
        *file_stat << file_path << "," << gatesBefore << "," << gatesAfter << "," << simplifyTime;
        for (auto t : subcircuits_number_by_iter)
        {
            *file_stat << "," << t;
        }
        for (auto t : skipped_subcircuits_by_iter)
        {
            *file_stat << "," << t;
        }
        for (auto t : max_subcircuit_size_by_iter)
        {
            *file_stat << "," << t;
        }
        for (auto t : circuit_size_by_iter)
        {
            *file_stat << "," << t;
        }
        *file_stat << "," << iterationsNumber << "," << total_gates_in_subcircuits << "\n";
    }
}

void runBenchmark(
    std::string const& file_path,
    argparse::ArgumentParser const& program,
    csat::Logger& logger,
    std::optional<std::ofstream>& file_stat)
{
    auto file = openInputFile(file_path, logger);

    csat::utils::GateEncoder<std::string> encoder;
    auto csat_instance = parseCircuitFile(file, file_path, logger, encoder);

    int64_t gatesBefore = csat_instance->getNumberOfGates();
    auto timeStart      = std::chrono::steady_clock::now();

    logger.debug(file_path, ": simplification start.");
    csat::simplification::CircuitStatsSingleton::getInstance().iter_number                 = 0;
    csat::simplification::CircuitStatsSingleton::getInstance().subcircuits_number_by_iter  = {0, 0, 0, 0, 0};
    csat::simplification::CircuitStatsSingleton::getInstance().skipped_subcircuits_by_iter = {0, 0, 0, 0, 0};
    csat::simplification::CircuitStatsSingleton::getInstance().max_subcircuit_size_by_iter = {0, 0, 0, 0, 0};
    csat::simplification::CircuitStatsSingleton::getInstance().circuit_size_by_iter        = {0, 0, 0, 0, 0};
    csat::simplification::CircuitStatsSingleton::getInstance().total_gates_in_subcircuits  = 0;
    std::string basis                            = program.get<std::string>("--basis");
    auto [processed_instance, processed_encoder] = applyPreprocessing(basis, csat_instance, encoder);
    encoder                                      = *processed_encoder;
    logger.debug(file_path, ": simplification end.");

    auto timeEnd        = std::chrono::steady_clock::now();
    int64_t gatesAfter  = processed_instance->getNumberOfGates();
    double simplifyTime = std::chrono::duration<double>(timeEnd - timeStart).count();

    writeOutputFiles(program, processed_instance, encoder, file_path);
    writeStatistics(
        file_stat,
        file_path,
        gatesBefore,
        gatesAfter,
        simplifyTime,
        csat::simplification::CircuitStatsSingleton::getInstance().subcircuits_number_by_iter,
        csat::simplification::CircuitStatsSingleton::getInstance().skipped_subcircuits_by_iter,
        csat::simplification::CircuitStatsSingleton::getInstance().max_subcircuit_size_by_iter,
        csat::simplification::CircuitStatsSingleton::getInstance().circuit_size_by_iter,
        csat::simplification::CircuitStatsSingleton::getInstance().iter_number,
        csat::simplification::CircuitStatsSingleton::getInstance().total_gates_in_subcircuits);
}

void readDatabases(argparse::ArgumentParser const& program, csat::Logger const& logger)
{
    std::string basis = program.get<std::string>("--basis");
    if (basis == "BENCH")
    {
        auto timeStart = std::chrono::steady_clock::now();
        csat::simplification::DBSingleton::getInstance().bench_db =
            std::make_shared<csat::simplification::CircuitDB>("database_bench.txt", csat::Basis::BENCH);
        auto timeEnd         = std::chrono::steady_clock::now();
        long double duration = std::chrono::duration<double>(timeEnd - timeStart).count();
        logger.info("Reading databases from database_bench.txt: ", duration, "sec\n");
    }
    if (basis == "AIG")
    {
        auto timeStart = std::chrono::steady_clock::now();
        csat::simplification::DBSingleton::getInstance().aig_db =
            std::make_shared<csat::simplification::CircuitDB>("database_aig.txt", csat::Basis::AIG);
        auto timeEnd         = std::chrono::steady_clock::now();
        long double duration = std::chrono::duration<double>(timeEnd - timeStart).count();
        logger.info("Reading databases from database_aig.txt: ", duration, "sec\n");
    }
}

std::string const DEFAULT_BASIS = "BENCH";

/**
 * Clearing the transferred schema with the ability to write a new schema to a file
 */
int main(int argn, char** argv)
{
    csat::Logger logger("Simplify");
    argparse::ArgumentParser program("simplify", "0.1");

    program.add_argument("input-path").help("directory with input .BENCH files");
    program.add_argument("-o", "--output").help("path to resulting directory");
    program.add_argument("-s", "--statistics").metavar("FILE").help("path to file for statistics writing");
    program.add_argument("-b", "--basis").default_value(std::string(DEFAULT_BASIS)).help("Choose basis [AIG|BENCH]");

    parseArguments(argn, argv, program);

    auto file_stat = openFileStat(program);

    std::string input_dir = program.get<std::string>("input-path");
    readDatabases(program, logger);

    for (auto& file_path : std::filesystem::directory_iterator(input_dir))
    {
        if (!file_path.is_regular_file())
        {
            continue;
        }
        try
        {
            runBenchmark(file_path.path().string(), program, logger, file_stat);
        }
        catch (std::exception const& err)
        {
            std::cerr << err.what() << std::endl;
        }
    }

    return 0;
}
