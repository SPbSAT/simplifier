#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

#include "src/parser/bench_to_circuit.hpp"
#include "src/simplification/composition.hpp"
#include "src/simplification/nest.hpp"
#include "src/simplification/strategy.hpp"
#include "src/simplification/three_inputs_optimization.hpp"
#include "src/simplification/three_inputs_optimization_bench.hpp"
#include "src/utility/encoder.hpp"
#include "src/utility/write_utils.hpp"
#include "third_party/argparse/include/argparse/argparse.hpp"

// Controls the number of subcircuit minimization iterations.
constexpr size_t NUMBER_OF_ITERATIONS = 5;

std::string const AIG_BASIS              = "AIG";
std::string const BENCH_BASIS            = "BENCH";
std::string const DEFAULT_BASIS          = BENCH_BASIS;
std::string const DEFAULT_DATABASES_PATH = "databases/";

/**
 * Helper for file stream opening.
 */
std::ifstream openFileStream(std::string const& file_path, csat::Logger& logger)
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

/**
 * Helper to run specific simplification strategies on the circuit in the provided basis.
 */
std::tuple<std::unique_ptr<csat::DAG>, std::unique_ptr<csat::utils::GateEncoder<std::string> > > applySimplification(
    std::string const& basis,
    std::unique_ptr<csat::DAG>& csat_instance,
    csat::utils::GateEncoder<std::string>& encoder)
{
    if (basis == AIG_BASIS)
    {
        return csat::simplification::Composition<
                   csat::DAG,
                   csat::simplification::Nest<
                       csat::DAG,
                       NUMBER_OF_ITERATIONS,
                       csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                       csat::simplification::ThreeInputsSubcircuitMinimization<csat::DAG> >,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG> >()
            .apply(*csat_instance, encoder);
    }
    else if (basis == BENCH_BASIS)
    {
        return csat::simplification::Composition<
                   csat::DAG,
                   csat::simplification::Nest<
                       csat::DAG,
                       NUMBER_OF_ITERATIONS,
                       csat::simplification::DuplicateOperandsCleaner<csat::DAG>,
                       csat::simplification::ThreeInputsSubcircuitMinimizationBench<csat::DAG> >,
                   csat::simplification::DuplicateOperandsCleaner<csat::DAG> >()
            .apply(*csat_instance, encoder);
    }
    else
    {
        std::cerr << "Incorrect basis! Choose one of [AIG, BENCH]" << std::endl;
        std::abort();
    }
}

/**
 * Writes resulting circuit either to an output file, or to the stdout if first is not given.
 */
void writeResult(
    argparse::ArgumentParser const& program,
    csat::DAG const& simplified_circuit,
    csat::utils::GateEncoder<std::string> const& encoder,
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
        writeBenchFile(simplified_circuit, encoder, file_out);
    }
    else
    {
        csat::printCircuit(simplified_circuit, encoder);
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
    std::string basis = program.get<std::string>("--basis");
    
    if (auto output_file = program.present("-s"))
    {
        std::ofstream statistics_stream(*output_file);
        statistics_stream << std::setprecision(3) << std::fixed;
        statistics_stream << "File path,Gates before,Gates after,Simplify time";
        
        // The following statistics is currently supported only for AIG basis.
        if (basis == AIG_BASIS)
        {
            statistics_stream <<",Reduced subcircuits by iter";
            for (std::size_t i = 0; i < NUMBER_OF_ITERATIONS; ++i)
            {
                statistics_stream << ",subcircuits_number_" << i;
            }
            for (std::size_t i = 0; i < NUMBER_OF_ITERATIONS; ++i)
            {
                statistics_stream << ",skipped_subcircuits_" << i;
            }
            for (std::size_t i = 0; i < NUMBER_OF_ITERATIONS; ++i)
            {
                statistics_stream << ",max_subcircuits_size_" << i;
            }
            for (std::size_t i = 0; i < NUMBER_OF_ITERATIONS; ++i)
            {
                statistics_stream << ",circuit_size_" << i;
            }
            statistics_stream << ",iter_number,total_gates_in_subcircuits";
        }
        statistics_stream << "\n";
        
        return statistics_stream;
    }
    return std::nullopt;
}

/**
 * Helper to dump a vector to ofstream.
 */
template<class T>
void dumpVector(std::ofstream& stream, std::vector<T> const& vec)
{
    stream << "," << "[";
    if (!vec.empty())
    {
        stream << vec.at(0);
        for (std::size_t idx = 1; idx < vec.size(); ++idx)
        {
            stream << ";" << vec.at(idx);
        }
    }
    stream << "]";
}

/**
 * Dumps current subcircuit simplification statistics to the stats file.
 */
void dumpStatistics(
    std::ofstream& statistics_stream,
    std::string const& basis,
    std::string const& file_path,
    std::size_t gatesBefore,
    std::size_t gatesAfter,
    long double simplifyTime)
{
    statistics_stream << std::setprecision(3) << std::fixed;
    statistics_stream << file_path << "," << gatesBefore << "," << gatesAfter << "," << simplifyTime;
    
    // The following statistics is currently supported only for AIG basis.
    if (basis == AIG_BASIS)
    {
        dumpVector(
            statistics_stream, csat::simplification::CircuitStatsSingleton::getInstance().reduced_subcircuit_by_iter);
    
        for (auto t: csat::simplification::CircuitStatsSingleton::getInstance().subcircuits_number_by_iter)
        {
            statistics_stream << "," << t;
        }
        for (auto t: csat::simplification::CircuitStatsSingleton::getInstance().skipped_subcircuits_by_iter)
        {
            statistics_stream << "," << t;
        }
        for (auto t: csat::simplification::CircuitStatsSingleton::getInstance().max_subcircuit_size_by_iter)
        {
            statistics_stream << "," << t;
        }
        for (auto t: csat::simplification::CircuitStatsSingleton::getInstance().circuit_size_by_iter)
        {
            statistics_stream << "," << t;
        }
        statistics_stream << "," << csat::simplification::CircuitStatsSingleton::getInstance().iter_number << ","
                          << csat::simplification::CircuitStatsSingleton::getInstance().total_gates_in_subcircuits;
    }
    statistics_stream<< "\n";
}

/**
 * Performs simplification of a circuit located at the `instance_path`.
 *
 * @param instance_path path to the input circuit.
 * @param program argparse program.
 * @param logger Logger instance.
 * @param statistics_stream stream for statistics dumping (if provided).
 */
void simplify(
    std::string const& instance_path,
    argparse::ArgumentParser const& program,
    csat::Logger& logger,
    std::optional<std::ofstream>& statistics_stream)
{
    // Filestream for circuit reading.
    auto circuit_fs = openFileStream(instance_path, logger);

    // Parse a circuit from a file stream.
    logger.debug("Parsing a circuit file ", instance_path, ".");
    csat::parser::BenchToCircuit<csat::DAG> parser{};
    parser.parseStream(circuit_fs);

    auto encoder       = parser.getEncoder();
    auto csat_instance = parser.instantiate();

    // Start simplification step.
    std::size_t gatesBefore = csat_instance->getNumberOfGatesWithoutInputs();
    auto timeStart          = std::chrono::steady_clock::now();

    logger.debug(instance_path, ": simplification start.");
    csat::simplification::CircuitStatsSingleton::getInstance().cleanState();

    std::string basis = program.get<std::string>("--basis");

    auto [simplified_instance, simplified_encoder] = applySimplification(basis, csat_instance, encoder);
    logger.debug(instance_path, ": simplification end.");

    auto timeEnd           = std::chrono::steady_clock::now();
    double simplifyTime    = std::chrono::duration<double>(timeEnd - timeStart).count();
    std::size_t gatesAfter = simplified_instance->getNumberOfGatesWithoutInputs();

    writeResult(program, *simplified_instance, *simplified_encoder, instance_path);

    // Dump simplification statistics if statistics path was specified.
    if (statistics_stream.has_value())
    {
        dumpStatistics(statistics_stream.value(), basis, instance_path, gatesBefore, gatesAfter, simplifyTime);
    }
}

/**
 * Loads (nearly) optimal circuits database to memory and saves it into a singleton object.
 */
void loadDatabases(argparse::ArgumentParser const& program, csat::Logger const& logger)
{
    std::string basis          = program.get<std::string>("--basis");
    std::string databases_path = program.get<std::string>("--databases");

    std::filesystem::path database_abs_path;

    auto timeStart = std::chrono::steady_clock::now();
    if (basis == BENCH_BASIS)
    {
        database_abs_path = databases_path / std::filesystem::path("database_bench.txt");
        csat::simplification::DBSingleton::getInstance().bench_db =
            std::make_shared<csat::simplification::CircuitDB>(database_abs_path, csat::Basis::BENCH);
    }
    else if (basis == AIG_BASIS)
    {
        database_abs_path = databases_path / std::filesystem::path("database_aig.txt");
        csat::simplification::DBSingleton::getInstance().aig_db =
            std::make_shared<csat::simplification::CircuitDB>(database_abs_path, csat::Basis::AIG);
    }
    else
    {
        std::cerr << "Incorrect basis! Choose one of [AIG, BENCH]" << std::endl;
        std::abort();
    }
    auto timeEnd = std::chrono::steady_clock::now();

    long double duration = std::chrono::duration<double>(timeEnd - timeStart).count();
    logger.debug("Read database from ", database_abs_path.string(), ": ", duration, "sec.");
}

/**
 * Performs simplification of circuits provided in the `--input-path`.
 * Writes resulting simplified circuits to the `--output`, and dumps
 * statistics at the `--statistics`.
 *
 * Allows to manually hint a circuit basis, which will result in
 * executing a right version of a subcircuit minimization algorithm.
 *
 * For more details see description of argparse program below.
 *
 */
int main(int argn, char** argv)
{
    csat::Logger logger("Simplify");

    // Set up argument parser.
    argparse::ArgumentParser program("simplify", "0.1");
    program.add_argument("-i", "--input-path").help("directory with input .BENCH files");
    program.add_argument("-o", "--output").help("path to resulting directory");
    program.add_argument("-s", "--statistics").metavar("FILE").help("path to file for statistics writing");
    program.add_argument("-b", "--basis").default_value(std::string(DEFAULT_BASIS)).help("Choose basis [AIG|BENCH]");
    program.add_argument("-d", "--databases")
        .default_value(std::string(DEFAULT_DATABASES_PATH))
        .help("Path to a directory with databases.");
    
    program.add_description(
        "Simplify tool provides simplification of boolean circuits provided in one of\n"
        "two bases: `AIG` or `BENCH`.\n"
        "\n"
        "To run simplification one should provide an `--input-path` and `--output`\n"
        "arguments, describing a path to the directory where simplified boolean\n"
        "circuits should be stored. Both input and output paths should be directories.\n"
        "Input directory should contain '*.bench' files, which are processed by tool\n"
        "distinctly.\n"
        "\n"
        "Also required basis should be specified manually using a `--basis` parameter\n"
        "and provide a path to the directory with databases describing small circuits\n"
        "on three inputs an three outputs by providing a `--databases` parameter. Note\n"
        "that databases are available at `databases/` project's root directory.\n"
        "\n"
        "Note that databases are already available in the `databases/` at the project's\n"
        "root and are ready to be used for a circuit simplification.\n"
        "\n"
        "To store statistics on simplification process one may additionally specify\n"
        "a `--statistics` parameter, which is a path to location where a `*.csv` file\n"
        "with statistics will be stored. Note that resulting csv will be written using\n"
        "',' delimiter, whilst ';' character may be a part of a valid value.");

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
    loadDatabases(program, logger);

    // Iterate over input directory of circuits.
    // Program will perform simplification of each found circuit.
    std::string input_dir = program.get<std::string>("--input-path");
    for (auto& instance_path : std::filesystem::directory_iterator(input_dir))
    {
        // Skip directories and other specific files.
        if (!instance_path.is_regular_file())
        {
            continue;
        }

        std::string path = instance_path.path().string();
        logger.info("Processing benchmark ", path, ".");
        simplify(path, program, logger, statistics_stream);
    }

    return 0;
}
