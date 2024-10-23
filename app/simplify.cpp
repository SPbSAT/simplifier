#include <cerrno>
#include <chrono>
#include <filesystem>
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

std::string const AIG_BASIS              = "AIG";
std::string const BENCH_BASIS            = "BENCH";
std::string const DEFAULT_BASIS          = BENCH_BASIS;
std::string const DEFAULT_DATABASES_PATH = "databases/";

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

std::ifstream openInputFile(std::string const& file_path, csat::Logger& logger)
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Can't open file, path is incorrect." << std::endl;
        std::abort();
    }
    logger.debug("File opened.");
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
    if (basis == AIG_BASIS)
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
    else if (basis == BENCH_BASIS)
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
        // Create directory if it doesn't exist yet.
        std::filesystem::path output_path{*output_dir};
        if (!std::filesystem::exists(output_path))
        {
            std::filesystem::create_directories(output_path);
        }

        // Write resulting circuit to an output path by original name.
        std::ofstream file_out(output_path / std::filesystem::path(file_path).filename());
        WriteBenchFile(*csat_instance, encoder, file_out);
    }
    else
    {
        printCircuit(*csat_instance, encoder);
    }
}

/**
 * If path for statistics is specified, opens `ofstream` to write statistics.
 * Writes file column names before any statistics is collected.
 *
 * If path is not given, returns `nullopt`.
 *
 * @param program argparse program.
 * @return either opened `ofstream`, or `nullopt`.
 */
std::optional<std::ofstream> openFileStat(argparse::ArgumentParser const& program)
{
    if (auto output_file = program.present("-s"))
    {
        std::ofstream statistics_stream(*output_file);
        statistics_stream << std::setprecision(3) << std::fixed;
        statistics_stream << "File path,Gates before,Gates after,Preprocessing time";
        for (int i = 0; i < 5; ++i)
        {
            statistics_stream << ",subcircuits_number_" << i;
        }
        for (int i = 0; i < 5; ++i)
        {
            statistics_stream << ",skipped_subcircuits_" << i;
        }
        for (int i = 0; i < 5; ++i)
        {
            statistics_stream << ",max_subcircuits_size_" << i;
        }
        for (int i = 0; i < 5; ++i)
        {
            statistics_stream << ",circuit_size_" << i;
        }
        statistics_stream << ",iter_number,total_gates_in_subcircuits\n";
        return statistics_stream;
    }
    return std::nullopt;
}

/**
 * Writes current subcircuit simplification statistics to the stats file.
 */
void writeStatistics(
    std::ofstream& statistics_stream,
    std::string const& file_path,
    int64_t gatesBefore,
    int64_t gatesAfter,
    long double simplifyTime)
{
    statistics_stream << std::setprecision(3) << std::fixed;
    statistics_stream << file_path << "," << gatesBefore << "," << gatesAfter << "," << simplifyTime;
    for (auto t : csat::simplification::CircuitStatsSingleton::getInstance().subcircuits_number_by_iter)
    {
        statistics_stream << "," << t;
    }
    for (auto t : csat::simplification::CircuitStatsSingleton::getInstance().skipped_subcircuits_by_iter)
    {
        statistics_stream << "," << t;
    }
    for (auto t : csat::simplification::CircuitStatsSingleton::getInstance().max_subcircuit_size_by_iter)
    {
        statistics_stream << "," << t;
    }
    for (auto t : csat::simplification::CircuitStatsSingleton::getInstance().circuit_size_by_iter)
    {
        statistics_stream << "," << t;
    }
    statistics_stream << "," << csat::simplification::CircuitStatsSingleton::getInstance().iter_number << ","
                      << csat::simplification::CircuitStatsSingleton::getInstance().total_gates_in_subcircuits << "\n";
}

void runBenchmark(
    std::string const& file_path,
    argparse::ArgumentParser const& program,
    csat::Logger& logger,
    std::optional<std::ofstream>& statistics_stream)
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

    if (statistics_stream)
    {
        writeStatistics(statistics_stream.value(), file_path, gatesBefore, gatesAfter, simplifyTime);
    }
}

/**
 * Reads needed database and saves it into a singleton object.
 */
void readDatabases(argparse::ArgumentParser const& program, csat::Logger const& logger)
{
    std::string basis          = program.get<std::string>("--basis");
    std::string databases_path = program.get<std::string>("--databases");
    if (basis == BENCH_BASIS)
    {
        auto timeStart                                            = std::chrono::steady_clock::now();
        csat::simplification::DBSingleton::getInstance().bench_db = std::make_shared<csat::simplification::CircuitDB>(
            databases_path / std::filesystem::path("database_bench.txt"), csat::Basis::BENCH);
        auto timeEnd = std::chrono::steady_clock::now();

        long double duration = std::chrono::duration<double>(timeEnd - timeStart).count();
        logger.debug("Reading databases from database_bench.txt: ", duration, "sec.");
    }
    if (basis == AIG_BASIS)
    {
        auto timeStart                                          = std::chrono::steady_clock::now();
        csat::simplification::DBSingleton::getInstance().aig_db = std::make_shared<csat::simplification::CircuitDB>(
            databases_path / std::filesystem::path("database_aig.txt"), csat::Basis::AIG);
        auto timeEnd = std::chrono::steady_clock::now();

        long double duration = std::chrono::duration<double>(timeEnd - timeStart).count();
        logger.debug("Reading databases from database_aig.txt: ", duration, "sec.");
    }
}

/**
 * Performs simplification of circuits provided in the `input-path`.
 * Writes resulting simplified circuits to the `--output`, and dumps
 * statistics at the `--statistics`.
 *
 * Allows to manually hint a circuit basis, which will result in
 * executing a right version of a subcircuit minimization algorithm.
 *
 */
int main(int argn, char** argv)
{
    csat::Logger logger("Simplify");

    // Set up argument parser.
    argparse::ArgumentParser program("simplify", "0.1");
    program.add_argument("input-path").help("directory with input .BENCH files");
    program.add_argument("-o", "--output").help("path to resulting directory");
    program.add_argument("-s", "--statistics").metavar("FILE").help("path to file for statistics writing");
    program.add_argument("-b", "--basis").default_value(std::string(DEFAULT_BASIS)).help("Choose basis [AIG|BENCH]");
    program.add_argument("-d", "--databases")
        .default_value(std::string(DEFAULT_DATABASES_PATH))
        .help("Path to a directory with databases.");

    // Parse provided program arguments.
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

    // Open file where statistics will be dumped.
    auto statistics_stream = openFileStat(program);

    // Read small circuit databases apriori to allow simplification use them.
    readDatabases(program, logger);

    // Iterate over input directory of circuits.
    // Program will perform simplification of each found circuit.
    std::string input_dir = program.get<std::string>("input-path");
    for (auto& file_path : std::filesystem::directory_iterator(input_dir))
    {
        // Skip directories and other specific files.
        if (!file_path.is_regular_file())
        {
            continue;
        }

        std::string path = file_path.path().string();
        logger.info("Processing benchmark ", path, ".");
        runBenchmark(path, program, logger, statistics_stream);
    }

    return 0;
}
