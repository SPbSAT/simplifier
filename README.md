# Simplify: a New Tool for Boolean Circuit Simplification

## Abstract

The Boolean circuit simplification problem involves finding a smaller circuit that computes the
same function as a given Boolean circuit. This problem is closely related to several key areas
with both theoretical and practical applications, such as logic synthesis, satisfiability, and
verification.

In the paper, we introduce a new tool for simplifying Boolean circuits. The tool optimizes
subcircuits with three inputs and at most three outputs, seeking to improve each one. It is
designed as a low-effort method that runs in just a few seconds for circuits of reasonable
size. This efficiency is achieved by combining two key strategies. First, the tool utilizes
a precomputed database of optimized circuits, generated with SAT solvers after carefully
clustering Boolean functions with three inputs and up to three outputs. Second, we demonstrate
that it is sufficient to check a linear number of subcircuits, relative to the size of the
original circuit. This allows a single iteration of the tool to be executed in linear time.

This artifact contains the paper and the tool together with detailed instructions on how to
run the tool and reproduce the results from the paper.

The tool was evaluated on a wide range of Boolean circuits, including both industrial and
hand-crafted examples, in two popular bases: AIG and BENCH. For AIG circuits, after
applying the state-of-the-art ABC framework, the tool achieved an additional 4% average
reduction in size. For BENCH circuits, the tool reduced their size by an average of 30%.

We pretend to achieve Functional, Reusable, and Available badges.

Simplify tool is available both at GitHub repository https://github.com/SPbSAT/circuitsat_tool,
and at Zenodo: [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.13987644.svg)](https://doi.org/10.5281/zenodo.13987644)

The Simplify tool is ready to be used on the **TACAS 2023 Artifact Evaluation Virtual Machine**
https://zenodo.org/records/7113223 (for more details see sections **Environment configuration** and **Technical info**).
It requires no additional proprietary software, or specific hardware (any moderately modern computer should do).

The rest of this README contains following sections:

1. **Early light review** - a guideline on performing a **light review** of a tool.
2. **Environment configuration** section includes instructions on installation of requirements.
3. **Usage instruction** section includes instructions on how Simplify tool can be used.
4. **Main results reproduction** provides guidelines on how paper results can be reproduced.
5. **Technical info** provides some specific technical details, irrelevant for the paper, but
   still giving additional information on the tool itself.

Each guideline section provides two paths: "all-in-one" command to complete section
and "step-by-step" guide to allow one to evaluate process in details. Note that instructions
in this README are given for the default Ubuntu shell and may not work in other environment.

## Early light review

Partial results suitable for light review can be produced in following steps:

1. Configure environment using "all-in-one" command (see next section for details
   on the environment configuration):

    ```sh
    # .deb packages
    (cd packages && sudo dpkg -i *.deb)
    # ABC tool
    (cd third_party/abc/ && make ABC_USE_NO_READLINE=1)
    # The simplify tool
    cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE
    cmake --build build/ --config RELEASE
    # Python environment
    python3.9 -m virtualenv tools/.env
    . tools/.env/bin/activate
    pip3 install tools/dependencies/*
    ```

2. The Table 3 is quite easily reproducible, so if there is a need to check
   that tool works at all, one can execute following command (which is a copy
   of command in relevant section below):

    ```sh
    mkdir experiment_table_3
    tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/
    python tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a  third_party/abc/abc -n 1 -s experiment_table_3/r_results.csv
    ./build/simplify -i experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/
    python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
    python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
    python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
    python tools/cli table-3-finalizer -e experiment_table_3/
    ```

3. The Table 4 allows one to select an arbitrary subset of classes to reproduce
   results, for example following commands should give results in appropriate time:

    ```sh
    mkdir light_review_table_4
    tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./light_review_table_4/
    ./build/simplify -i light_review_table_4/benchmarks/php/ -o light_review_table_4/benchmarks_s/php/ -s light_review_table_4/php_result.csv --basis BENCH --databases databases/
    ./build/simplify -i light_review_table_4/benchmarks/mult_miter/ -o light_review_table_4/benchmarks_s/mult_miter/ -s light_review_table_4/mult_miter_result.csv --basis BENCH --databases databases/
    python tools/cli table-4-finalizer -e light_review_table_4/
    ```

## Environment configuration

Before conducting any executions of the tool, it is necessary
to perform a mandatory environment configuration. The tool
itself requires only to be compiled, whilst experiments
framework requires accessibility of ABC tool executable and
python environment for the utility scripts.

As a result of configuration (for correctness of guidelines after) one should have:

- Installed additional Ubuntu dependencies.
- Compiled `simplify` executable located at `build/simplify`
- Compiled `ABC` executable located at `third_party/abc/abc`
- Activated python environment located at `tools/.env`

In the other sections of this README it will be assumed, that all shell commands are executed
within python environment, with `abc` and `simplify` executables available at paths specified
above.

### All-in-one command

```sh
# .deb packages
(cd packages && sudo dpkg -i *.deb)
# ABC tool
(cd third_party/abc/ && make ABC_USE_NO_READLINE=1)
# The simplify tool
cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE
cmake --build build/ --config RELEASE
# Python environment
python3.9 -m virtualenv tools/.env
. tools/.env/bin/activate
pip3 install tools/dependencies/*
```

### Step-by-step guide

#### Ubuntu packages

Several additional `.deb` pacakges are provided in the `packages/` directory,
and can be installed using the following command:

```sh
(cd packages && sudo dpkg -i *.deb)
```

#### Simplify compilation (C++)

Simplify tool can be compiled using `cmake` tool and a standard `C++` compiler
(`gcc` should do). To build `simplify` binary execute following steps:

1. Compile `cmake` project using command: `cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE`
2. Build `simplify` tool executable using command: `cmake --build build/ --config RELEASE`

(Note that in `Release` build wast amount of logs is disabled.
If one need more logs, `DEBUG` compilation type may be useful.)

As a result ``simplify`` binary will be built. To get info on how ``simplify`` should be used
execute resulting binary with following command:

```sh
build/simplify --help
```

#### ABC compilation

ABC synthesis tool (and patricullary its `resyn2` command) is a state-of-art framework for circuit
sythesis. It is used in the paper as a baseline solution for a circuit simplification, which allows
one to access contribution and novelty of the Simplify tool.

For convenience, specific revision of the ABC synthesis tool is brought alongside Simplify
in the `third_party/` directory. To compile it, one need to execute following commands

```sh
(cd third_party/abc/ && make ABC_USE_NO_READLINE=1)
```

(see. original repository for details https://github.com/berkeley-abc/abc).

#### Python environment

Python 3.9 is used for scripts located in the `tools/` directory.

1. Create virtual environment by running `python3.9 -m virtualenv tools/.env`
2. Activate virtual environment `. tools/.env/bin/activate`
3. Install dependencies `pip3 install tools/dependencies/*`

Note that step (2) should be executed in every new shell process to activate
python virtual environment.

Now you should be able to run CLI of tools. Try to use `python tools/cli --help`
to get more info on the available CLI commands.


## Usage instruction

The Simplify tool provides simplification of boolean circuits provided in
one of two bases: `AIG` or `BENCH`. To run simplification one should provide
an `--input-path` and `--output` parameters: first is a path to the directory
with boolean circuits, and second is a path where simplified circuits are to
be stored. Both input and output paths should be directories. Program will
take attempt to read all files in input directory as `.bench` circuits. Each
circuit then will be processed by the tool distinctly.

Required basis of input circuits should be specified manually using a `--basis`
parameter. It will serve as a hint for the tool, which will help it to choose
suitable algorithm.

One also should provide a path to the directory with databases containing
(nearly) optimal circuits with three inputs and three outputs by providing
a `--databases` parameter. Note that databases are available at `databases/`
project's root directory, which is a default value for `--databases`.

To store statistics of the simplification process one may additionally specify
a `--statistics` parameter, which is a path to location where a `*.csv` file
with gathered statistics is to be stored. Note that resulting csv file will use
a `,` character as a delimiter, whilst `;` character may be a valid item value.

Example usage command:

```sh
./build/simplify -i input_circuit/ -o result_circuits/ -s statistics.csv
```

## Main results reproduction

Table reproduction guidelines below provides following information:

1. Name of a subsection is a table name in the paper.
2. Each section provides both "all-in-one" command to run, and a "step-by-step" guide on how
   results are computed.
4. Description of resulting artifacts and their mappings to results in the paper.

All instructions presented below save results of main intermediate steps. To validate correctness
any stage of simplification, one may use utility CLI command `check-equiv`:

```sh
python tools/cli check-equiv --help
```

### Statistics descriptions

Statistics is written in `.csv` files with `,` delimiter. Note that in some statistics `;` is used in a value part.

Note that circuit sizes for `AIG`-related tables is given as a number of `AND` gates, while for
`BENCH`-related tables size is represented as total number of gates excluding `INPUT` gates.

Final steps of main results reproduction sequences print main results to `stdout`, as well as save
a file which name pattern is `final_results*.csv`


### Table 1. Distribution of classes and functions by circuit size.

Table 1 humbly represents a distribution of circuit sizes, contained in the database of (nearly)
optimal circuits. This database contains all possible circuits on three inputs and three outputs,
and their particular counts can be reproduced by executing a specialized tool and are not part of
the main statistical result of the paper.

### Table 2. Comparison of several runs of resyn2 against several runs of resyn2 followed by a run of simplify.

Note: we are aware that first to columns of sub-table 2 of the Table 2 in the paper are interchanged,
and we apologize for that. We have observed it only after paper deadline passed, and we are intended
to fix it when paper is accepted for publication.

#### All-in-one command

```sh
mkdir experiment_table_2
tar -xvf ./benchmark/all_sets_under_50000_for_stat.tar.xz -C ./experiment_table_2/
python tools/cli abc-resyn2 -i experiment_table_2/benchmarks/ -o experiment_table_2/benchmarks_r/ -a  third_party/abc/abc -n 2 -n 3 -n 6 -s experiment_table_2/r_results.csv
./build/simplify -i experiment_table_2/benchmarks_r/resyn2_2/ -o experiment_table_2/benchmarks_r2_s/ -s experiment_table_2/r2_s_result.csv --basis AIG --databases databases/
./build/simplify -i experiment_table_2/benchmarks_r/resyn2_6/ -o experiment_table_2/benchmarks_r6_s/ -s experiment_table_2/r6_s_result.csv --basis AIG --databases databases/
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks -s experiment_table_2/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_3 -s experiment_table_2/benchmark_r3_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_6 -s experiment_table_2/benchmark_r6_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r2_s -s experiment_table_2/benchmark_r2_s_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r6_s -s experiment_table_2/benchmark_r6_s_sizes.csv
python tools/cli table-2-finalizer -e experiment_table_2/
```

#### Step-by-step guide:

1. Make clean experiment directory:

```sh
mkdir experiment_table_2
```

2. Extract `benchmark/representative_benchmarks.tar.xz` to the created directory:

```sh
tar -xvf ./benchmark/all_sets_under_50000_for_stat.tar.xz -C ./experiment_table_2/
```

3. Run `ABC` `resyn2` on the extracted benchmarks using provided CLI command:

```sh
python tools/cli abc-resyn2 -i experiment_table_2/benchmarks/ -o experiment_table_2/benchmarks_r/ -a  third_party/abc/abc -n 2 -n 3 -n 6 -s experiment_table_2/r_results.csv
```

4. Run `simplify` tool on several configurations of resulting circuits:

```sh
./build/simplify -i experiment_table_2/benchmarks_r/resyn2_2/ -o experiment_table_2/benchmarks_r2_s/ -s experiment_table_2/r2_s_result.csv --basis AIG --databases databases/
./build/simplify -i experiment_table_2/benchmarks_r/resyn2_6/ -o experiment_table_2/benchmarks_r6_s/ -s experiment_table_2/r6_s_result.csv --basis AIG --databases databases/
```

5. Collect statistics

```sh
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks -s experiment_table_2/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_3 -s experiment_table_2/benchmark_r3_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_6 -s experiment_table_2/benchmark_r6_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r2_s -s experiment_table_2/benchmark_r2_s_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r6_s -s experiment_table_2/benchmark_r6_s_sizes.csv
```

6. Build final paper-ready statistics

```sh
python tools/cli table-2-finalizer -e experiment_table_2/
```

After all commands completed, results are located as follows:

1. `final_results_1.csv` and `final_results_2.csv` which contains final results as it is presented in the paper,
   in first part and a second part of a table respectfully.
2. `r_results.csv` contains statistics on `resyn2` running.
3. `rX_s_result.csv` contains results of `simplify` evaluation on the relevant scenario `X` (note that sizes in it are calculated as for BENCH basis).
4. `benchmark_*_sizes.csv` contains sizes (measured as number of AND gates) of circuits on different simplification stages.

### Table 3. Comparison of circuit sizes and running times between resyn2 and resyn2+simplify

#### All-in-one command

```sh
mkdir experiment_table_3
tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/
python tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a  third_party/abc/abc -n 1 -s experiment_table_3/r_results.csv
./build/simplify -i experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
python tools/cli table-3-finalizer -e experiment_table_3/
```

#### Step-by-step guide

1. Make clean experiment directory:

```sh
mkdir experiment_table_3
```

2. Extract `benchmark/representative_benchmarks.tar.xz` to the created directory:

```sh
tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/
```

In result `benchmarks` directory must appear.

3. Run `ABC` `resyn2` on the extracted benchmarks using provided CLI command:

```sh
python tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a  third_party/abc/abc -n 1 -s experiment_table_3/r_results.csv
```

4. Run `simplify` tool on resulting circuits:

```sh
./build/simplify -i experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/
```

5. Collect benchmark sizes

```sh
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
```

6. Build final paper-ready statistics

```sh
python tools/cli table-3-finalizer -e experiment_table_3/
```

After all commands completed, results are located as follows:

1. `final_results.csv` contains final result as it is presented in the paper.
2. `r_results.csv` contains statistics on `resyn2` execution.
3. `rs_result.csv` contains results of `simplify` evaluation (note that sizes in it are calculated as for BENCH basis).
4. `benchmark_sizes.csv` contains information about original circuit sizes.
5. `benchmark_r_sizes.csv` contains information about circuit after `resyn2`.
6. `benchmark_rs_sizes.csv` contains information about circuit after `simplify`.

### Table 4. Simplification statistics for various classes of circuits in the BENCH basis.

#### All-in-one command

Warning: this command may take extremely long time (several hours on modern computer) to be
executed sequentially, so main `simplify` calls are parallelized by default. If hardware is
unsuitable for such bends of fate, one can either endure and run the sequential test, or run
benchmarking on only a part of classes, picked randomly and unbiased.

```sh
mkdir experiment_table_4
tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./experiment_table_4/
./build/simplify -i experiment_table_4/benchmarks/factorization/ -o experiment_table_4/benchmarks_s/factorization/ -s experiment_table_4/factorization_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/ecp/ -o experiment_table_4/benchmarks_s/ecp/ -s experiment_table_4/ecp_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/clique/ -o experiment_table_4/benchmarks_s/clique/ -s experiment_table_4/clique_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/php/ -o experiment_table_4/benchmarks_s/php/ -s experiment_table_4/php_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/I99T/ -o experiment_table_4/benchmarks_s/I99T/ -s experiment_table_4/I99T_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/BristolFashion/ -o experiment_table_4/benchmarks_s/BristolFashion/ -s experiment_table_4/BristolFashion_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/miter_sum/ -o experiment_table_4/benchmarks_s/miter_sum/ -s experiment_table_4/miter_sum_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/miter_thr2/ -o experiment_table_4/benchmarks_s/miter_thr2/ -s experiment_table_4/miter_thr2_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/miter_transalg_sort/ -o experiment_table_4/benchmarks_s/miter_transalg_sort/ -s experiment_table_4/miter_transalg_sort_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/mult_miter/ -o experiment_table_4/benchmarks_s/mult_miter/ -s experiment_table_4/mult_miter_result.csv --basis BENCH --databases databases/ &
wait
python tools/cli table-4-finalizer -e experiment_table_4/
```

#### Step-by-step guide

1. Make clean experiment directory:

```sh
mkdir experiment_table_4
```

2. Extract `benchmark/bench_for_stat.tar.xz` to the created directory:

```sh
tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./experiment_table_4/
```

In result `benchmarks` directory must appear.

3. Run `simplify` tool on resulting circuits:

Warning: this command may take extremely long time (several hours on modern computer) to be
executed sequentially, so main `simplify` calls are parallelized by default. If hardware is
unsuitable for such bends of fate, one can either endure and run the sequential test, or run
benchmarking on only a part of classes, picked randomly and unbiased.

```sh
./build/simplify -i experiment_table_4/benchmarks/factorization/ -o experiment_table_4/benchmarks_s/factorization/ -s experiment_table_4/factorization_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/ecp/ -o experiment_table_4/benchmarks_s/ecp/ -s experiment_table_4/ecp_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/clique/ -o experiment_table_4/benchmarks_s/clique/ -s experiment_table_4/clique_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/php/ -o experiment_table_4/benchmarks_s/php/ -s experiment_table_4/php_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/I99T/ -o experiment_table_4/benchmarks_s/I99T/ -s experiment_table_4/I99T_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/BristolFashion/ -o experiment_table_4/benchmarks_s/BristolFashion/ -s experiment_table_4/BristolFashion_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/iscas85/ -o experiment_table_4/benchmarks_s/iscas85/ -s experiment_table_4/iscas85_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/miter_sum/ -o experiment_table_4/benchmarks_s/miter_sum/ -s experiment_table_4/miter_sum_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/miter_thr2/ -o experiment_table_4/benchmarks_s/miter_thr2/ -s experiment_table_4/miter_thr2_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/miter_transalg_sort/ -o experiment_table_4/benchmarks_s/miter_transalg_sort/ -s experiment_table_4/miter_transalg_sort_result.csv --basis BENCH --databases databases/ &
./build/simplify -i experiment_table_4/benchmarks/mult_miter/ -o experiment_table_4/benchmarks_s/mult_miter/ -s experiment_table_4/mult_miter_result.csv --basis BENCH --databases databases/ &
wait
```

6. Build final paper-ready statistics

```sh
python tools/cli table-4-finalizer -e experiment_table_4/
```

After all commands completed, results are located as follows:

1. `final_results.csv` contains final result as it is presented in the paper.
2. `<class>_result.csv` contains results of `simplify` evaluation to the circuits of `class`
(note that sizes in it are calculated as for BENCH basis).

## Technical info

### Code structure

todo: source files
todo: third_party

Main `simplify` directory contains following directories:

- `app/` directory contains compilable `simplify.cpp` file, which contains an entry-point (`main`).
- `benchmarks/` directory contains tar archives with boolean circuit benchmarks used for experiments.
- `databases/` directory contains databases for (nearly) optimal small circuits for BENCH and AIG bases.
- `packages/` directory contains several `.deb` packages required to work with the tool and run experiments.
- `src/` directory implements main tool functionalities, organized as a header-only library.
- `tests/` directory implements unit tests for the functionalities of the tool.
- `third_party` directory contains third-party libraries, used either for the tool itself (`argparse`),
  code quality checks (`GTest`), or for the experiment conduction (`ABC`). Those libraries are vendored
  along with the tool for the completeness.
- `tools/` directory contains CLI utilities useful for experiments and are to be run within
  right `python 3.9` environment.

### Dependencies

Simplify utilizes external library [argparse](https://github.com/p-ranav/argparse/tree/v2.9) to ease
CLI arguments parsing and usage. Also test framework [GoogleTest](https://github.com/google/googletest).
is used to run unit tests which cover main functionalities of the tool.

To make this code snapshot whole, `argparse` and `gtest` repository snapshots are vendored
alongside it and are presented in the `third_party/` directory.

## Technical info

### Platform

Simplify is written using `C++20`. Simplify was developed and tested on `Ubuntu OS`
with `gcc x86-64` compiler, but should not cause any problems on other common Linux distributions.

Experiment environment of a tool includes additional dependencies such as `ABC tool` and `python 3.9`
virtual environment.

### Dependencies

Simplify tool utilizes external library [argparse](https://github.com/p-ranav/argparse/tree/v2.9)
to ease CLI arguments parsing and usage. Also test framework [GoogleTest](https://github.com/google/googletest)
is used to run unit tests which cover main functionalities of the tool.

To make this code snapshot whole, `argparse` and `gtest` repository snapshots are vendored
alongside it and are presented in the `third_party/` directory.

### Code quality

#### Testing

Core parts of a code are covered with tests, located at `tests/` directory.
Test are written using `GTest` framework.

Main capabilities of the tool were tested one the wide range of Boolean circuits,
both industrial and hand-crafter. Results of simplification were tested for equivalence
to the original boolean circuits.

#### Static code analysis

`clang-format` and `clang-tidy` are used for maintaining code quality.
Note that both of them are used as a part of `CI` flow in the linked
GitHub repository, and their execution may be checked there.

Both of them need to be separately installed before used. For example,
in Ubuntu they can be installed using following command:

```sh
sudo apt install clang-tidy clang-format-18
```

For the experiments related code environment, `black`, `docformatter` and `usort` are used
both to check if code is properly formatted and to format code locally.

##### Linter

To manually run `clang-tidy`, it is required to build a cmake project to be able
to use a build artifact `build/compile_commands.json`. Then one will be able to
run linter by a simple command:

```sh
clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy
```

To make `cland-tidy` fix warnings for you, simply run same command with flags
`--fix` and `--fix-errors`:

```sh
clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy --fix --fix-errors
```

Though it may break code in some cases, so be careful and check all patches made by `clang-tidy`.

##### Formater

`clang-format-18` is used to maintain code uniformity. To check if there are problems in code
one can use the following command:

```sh
find ./src/ ./app/ -name '*.cpp' -o -name '*.hpp' | xargs clang-format --style="file:.clang-format"
```

To make `clang-format` apply automatical fixes one can use the following command:

```sh
find ./src/ ./app/ -name '*.cpp' -o -name '*.hpp' | xargs clang-format --style="file:.clang-format" -i
```
