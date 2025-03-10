# Simplifier Benchmarking

This README contains information about how one can conduct tool benchmarking of the tool.
It it divided into the following sections:

1. **Easy review** section provides a guideline on performing a **light review** of a tool.
2. **Environment configuration** section contains instructions on installation of requirements.
3. **Usage instruction** section contains instructions on how Simplifier tool can be used.
4. **Main results reproduction** provides guidelines on how experiment results presented in the
related paper can be reproduced.
5. **Technical info** provides some specific technical details, irrelevant for the paper, but
   still giving additional information on the tool itself.

Each guideline section provides two paths: "all-in-one" command to complete section
and "step-by-step" guide to allow one to evaluate process in details. Note that instructions
in this README are given for the default Ubuntu shell and may not work in other environment.

## Easy review

Partial results suitable for easy review can be produced by the following steps:

1. Configure environment using "all-in-one" command (note that C++ sources may take quite a long time to compile):

    ```sh
    # ABC tool
    (cd third_party/abc/ && make ABC_USE_NO_READLINE=1)
    # The simplifier tool
    cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE
    cmake --build build/ --config RELEASE
    # Install python dependencies globally
    pip3 install tools/dependencies/*
    ```

2. The Table 3 is quite easily reproducible, so if there is a need to check
   that tool works at all, one can execute following command (which is a copy
   of command in relevant section below):

    ```sh
    mkdir experiment_table_3
    tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/
    python3.10 tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a  third_party/abc/abc -n 1 -s experiment_table_3/r_results.csv
    ./build/simplifier -i experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/
    python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
    python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
    python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
    python3.10 tools/cli table-3-finalizer -e experiment_table_3/
    ```

3. The Table 4 allows one to select an arbitrary subset of classes to reproduce
   results, for example following commands should give results in appropriate time:

    ```sh
    mkdir light_review_table_4
    tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./light_review_table_4/
    ./build/simplifier -i light_review_table_4/benchmarks/php/ -o light_review_table_4/benchmarks_s/php/ -s light_review_table_4/php_result.csv --basis BENCH --databases databases/
    ./build/simplifier -i light_review_table_4/benchmarks/mult_miter/ -o light_review_table_4/benchmarks_s/mult_miter/ -s light_review_table_4/mult_miter_result.csv --basis BENCH --databases databases/
    python3.10 tools/cli table-4-finalizer -e light_review_table_4/
    ```

## Environment configuration

Before conducting any executions of the tool, it is necessary
to perform a mandatory environment configuration. The tool
itself requires only to be compiled, whilst experiments
framework requires access to the ABC tool executable and
python environment for the utility scripts.

As a result of configuration steps of this section one should have:

- Installed additional Ubuntu dependencies;
- Installed `python` packages.
- Compiled `simplifier` executable located at `build/simplifier`;
- Compiled `ABC` executable located at `third_party/abc/abc`;

In the other sections of this README it will be assumed, that all shell commands are executed
in the environment with python dependencies, with `abc` and `simplifier` executables available at
the paths specified above.

### All-in-one command

```sh
# ABC tool
(cd third_party/abc/ && make ABC_USE_NO_READLINE=1)
# The simplifier tool
cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE
cmake --build build/ --config RELEASE
# Install python dependencies globally
pip3 install tools/dependencies/*
```

### Step-by-step guide

#### Simplifier compilation (C++)

Simplifier tool can be compiled using `cmake` tool and a standard `C++` compiler
(`gcc` should do). To build `simplifier` binary execute following steps:

1. Compile `cmake` project using command:
    ```sh
    cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE
    ```
2. Build `simplifier` tool executable using command:
    ```sh
    cmake --build build/ --config RELEASE
    ```

(Note that in `Release` build wast amount of logs is disabled.
If one need more logs, `DEBUG` compilation type may be useful.)

As a result ``simplifier`` binary will be built. To get info on how ``simplifier`` should be used
execute resulting binary with following command:

```sh
build/simplifier --help
```

#### ABC compilation

ABC synthesis tool (and patricullary its `resyn2` command) is a state-of-art framework for circuit
synthesis. It is used in the paper as a baseline solution for a circuit simplification, which allows
one to access contribution and novelty of the Simplifier tool.

For convenience and reproducibility, specific revision of the ABC synthesis tool is vendored
in the `third_party/` directory. To compile it, one need to execute following command:
```sh
(cd third_party/abc/ && make ABC_USE_NO_READLINE=1)
```

(see. original repository for details https://github.com/berkeley-abc/abc).

#### Python dependencies

Python 3.10 is available in the provided virtual machine and is used
for the scripts located in the `tools/` directory.

To install additional python dependencies globally execute:

```sh
pip3 install tools/dependencies/*
```

Now you should be able to run CLI of tools. Try to use `python3.10 tools/cli --help`
to get more info on the available CLI commands.


## Usage instruction

The Simplifier tool provides simplification of boolean circuits provided in
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
./build/simplifier -i input_circuit/ -o result_circuits/ -s statistics.csv
```


## Main results reproduction

This section contains guidelines on experiments reproduction, provided in the related paper.
This section provide following set of information:

1. Name of a subsection represents a name of the corresponding table in the paper;
2. Both "all-in-one" command to run, and a "step-by-step" guide on how results are computed;
3. Description of resulting artifacts and their mappings to results in the paper.

All instructions presented below carry results of main intermediate steps. To validate
correctness any stage of simplification, one may use utility CLI command `check-equiv`:

```sh
python3.10 tools/cli check-equiv --help
```

#### Statistics descriptions

Statistics is written in `.csv` files with `,` delimiter. Note that in some data `;` is used in a value part.

Circuit sizes for `AIG` based tables are given as number of `AND` gates, while for the
`BENCH` based tables size is represented by total number of gates excluding `INPUT`gates.

Final steps of main results reproduction guidelines print main results to the `stdout`
and save results a file which name pattern is `final_results*.csv`


### Table 1. Distribution of classes and functions by circuit size.

Table 1 humbly represents a distribution of circuit sizes, contained in the database of (nearly)
optimal circuits. This database contains all possible circuits on three inputs and three outputs,
and their particular counts can be reproduced by executing a specialized tool and are not part of
the main statistical result of the paper.

### Table 2. Comparison of several runs of resyn2 against several runs of resyn2 followed by a run of the Simplifier.

Note: we are aware that first to columns of sub-table 2 of the Table 2 in the paper are interchanged,
and we apologize for that. We have observed it only after paper deadline passed, and we are intended
to fix it when paper is accepted for publication.

#### All-in-one command

```sh
mkdir experiment_table_2
tar -xvf ./benchmark/all_sets_under_50000_for_stat.tar.xz -C ./experiment_table_2/
python3.10 tools/cli abc-resyn2 -i experiment_table_2/benchmarks/ -o experiment_table_2/benchmarks_r/ -a  third_party/abc/abc -n 2 -n 3 -n 6 -s experiment_table_2/r_results.csv
./build/simplifier -i experiment_table_2/benchmarks_r/resyn2_2/ -o experiment_table_2/benchmarks_r2_s/ -s experiment_table_2/r2_s_result.csv --basis AIG --databases databases/
./build/simplifier -i experiment_table_2/benchmarks_r/resyn2_6/ -o experiment_table_2/benchmarks_r6_s/ -s experiment_table_2/r6_s_result.csv --basis AIG --databases databases/
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks -s experiment_table_2/benchmark_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_3 -s experiment_table_2/benchmark_r3_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_6 -s experiment_table_2/benchmark_r6_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r2_s -s experiment_table_2/benchmark_r2_s_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r6_s -s experiment_table_2/benchmark_r6_s_sizes.csv
python3.10 tools/cli table-2-finalizer -e experiment_table_2/
```

#### Step-by-step guide:

1. Make clean experiment directory:

```sh
mkdir experiment_table_2
```

2. Extract `benchmark/all_sets_under_50000_for_stat.tar.xz` to the created directory:

```sh
tar -xvf ./benchmark/all_sets_under_50000_for_stat.tar.xz -C ./experiment_table_2/
```

3. Run `ABC` `resyn2` on the extracted benchmarks using provided CLI command:

```sh
python3.10 tools/cli abc-resyn2 -i experiment_table_2/benchmarks/ -o experiment_table_2/benchmarks_r/ -a  third_party/abc/abc -n 2 -n 3 -n 6 -s experiment_table_2/r_results.csv
```

4. Run `simplifier` tool on several configurations of resulting circuits:

```sh
./build/simplifier -i experiment_table_2/benchmarks_r/resyn2_2/ -o experiment_table_2/benchmarks_r2_s/ -s experiment_table_2/r2_s_result.csv --basis AIG --databases databases/
./build/simplifier -i experiment_table_2/benchmarks_r/resyn2_6/ -o experiment_table_2/benchmarks_r6_s/ -s experiment_table_2/r6_s_result.csv --basis AIG --databases databases/
```

5. Collect statistics

```sh
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks -s experiment_table_2/benchmark_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_3 -s experiment_table_2/benchmark_r3_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_6 -s experiment_table_2/benchmark_r6_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r2_s -s experiment_table_2/benchmark_r2_s_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r6_s -s experiment_table_2/benchmark_r6_s_sizes.csv
```

6. Build final paper-ready statistics

```sh
python3.10 tools/cli table-2-finalizer -e experiment_table_2/
```

After all commands completed, results are located as follows:

1. `final_results_1.csv` and `final_results_2.csv` which contains final results as it is presented in the paper,
   in first part and a second part of a table respectfully.
2. `r_results.csv` contains statistics on `resyn2` running.
3. `rX_s_result.csv` contains results of `simplifier` evaluation on the relevant scenario `X` (note that sizes in it are calculated as for BENCH basis).
4. `benchmark_*_sizes.csv` contains sizes (measured as number of AND gates) of circuits on different simplification stages.

### Table 3. Comparison of circuit sizes and running times between resyn2 and resyn2+simplifier

#### All-in-one command

```sh
mkdir experiment_table_3
tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/
python3.10 tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a  third_party/abc/abc -n 1 -s experiment_table_3/r_results.csv
./build/simplifier -i experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/
python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
python3.10 tools/cli table-3-finalizer -e experiment_table_3/
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

3. Run `ABC` `resyn2` on the extracted benchmarks using provided CLI command:

```sh
python3.10 tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a  third_party/abc/abc -n 1 -s experiment_table_3/r_results.csv
```

4. Run `simplifier` tool on resulting circuits:

```sh
./build/simplifier -i experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/
```

5. Collect benchmark sizes

```sh
python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
python3.10 tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
```

6. Build final paper-ready statistics

```sh
python3.10 tools/cli table-3-finalizer -e experiment_table_3/
```

After all commands completed, results are located as follows:

1. `final_results.csv` contains final result as it is presented in the paper.
2. `r_results.csv` contains statistics on `resyn2` execution.
3. `rs_result.csv` contains results of `simplifier` evaluation (note that sizes in it are calculated as for BENCH basis).
4. `benchmark_sizes.csv` contains information about original circuit sizes.
5. `benchmark_r_sizes.csv` contains information about circuit after `resyn2`.
6. `benchmark_rs_sizes.csv` contains information about circuit after `simplifier`.

### Table 4. Simplification statistics for various classes of circuits in the BENCH basis.

#### All-in-one command

Warning: this command may take extremely long time (several hours on modern computer) to be
executed sequentially, so `simplifier` executions are parallelized by default. If hardware is
unsuitable for such bends of fate, one can either endure and run the sequential test, or run
benchmarking on only a part of classes, picked randomly and unbiased. Yet, some classes may
take up to nearly two hours to be simplified, so proceed with awareness of it.

```sh
mkdir experiment_table_4
tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./experiment_table_4/
./build/simplifier -i experiment_table_4/benchmarks/factorization/ -o experiment_table_4/benchmarks_s/factorization/ -s experiment_table_4/factorization_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/ecp/ -o experiment_table_4/benchmarks_s/ecp/ -s experiment_table_4/ecp_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/clique/ -o experiment_table_4/benchmarks_s/clique/ -s experiment_table_4/clique_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/php/ -o experiment_table_4/benchmarks_s/php/ -s experiment_table_4/php_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/I99T/ -o experiment_table_4/benchmarks_s/I99T/ -s experiment_table_4/I99T_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/BristolFashion/ -o experiment_table_4/benchmarks_s/BristolFashion/ -s experiment_table_4/BristolFashion_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/miter_sum/ -o experiment_table_4/benchmarks_s/miter_sum/ -s experiment_table_4/miter_sum_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/miter_thr2/ -o experiment_table_4/benchmarks_s/miter_thr2/ -s experiment_table_4/miter_thr2_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/miter_transalg_sort/ -o experiment_table_4/benchmarks_s/miter_transalg_sort/ -s experiment_table_4/miter_transalg_sort_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/mult_miter/ -o experiment_table_4/benchmarks_s/mult_miter/ -s experiment_table_4/mult_miter_result.csv --basis BENCH --databases databases/ &
wait
python3.10 tools/cli table-4-finalizer -e experiment_table_4/
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

3. Run `simplifier` tool on resulting circuits:

Warning: this command may take extremely long time (several hours on modern computer) to be
executed sequentially, so `simplifier` executions are parallelized by default. If hardware is
unsuitable for such bends of fate, one can either endure and run the sequential test, or run
benchmarking on only a part of classes, picked randomly and unbiased. Yet, some classes may
take up to nearly two hours to be simplified, so proceed with awareness of it.

```sh
./build/simplifier -i experiment_table_4/benchmarks/factorization/ -o experiment_table_4/benchmarks_s/factorization/ -s experiment_table_4/factorization_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/ecp/ -o experiment_table_4/benchmarks_s/ecp/ -s experiment_table_4/ecp_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/clique/ -o experiment_table_4/benchmarks_s/clique/ -s experiment_table_4/clique_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/php/ -o experiment_table_4/benchmarks_s/php/ -s experiment_table_4/php_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/I99T/ -o experiment_table_4/benchmarks_s/I99T/ -s experiment_table_4/I99T_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/BristolFashion/ -o experiment_table_4/benchmarks_s/BristolFashion/ -s experiment_table_4/BristolFashion_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/miter_sum/ -o experiment_table_4/benchmarks_s/miter_sum/ -s experiment_table_4/miter_sum_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/miter_thr2/ -o experiment_table_4/benchmarks_s/miter_thr2/ -s experiment_table_4/miter_thr2_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/miter_transalg_sort/ -o experiment_table_4/benchmarks_s/miter_transalg_sort/ -s experiment_table_4/miter_transalg_sort_result.csv --basis BENCH --databases databases/ &
./build/simplifier -i experiment_table_4/benchmarks/mult_miter/ -o experiment_table_4/benchmarks_s/mult_miter/ -s experiment_table_4/mult_miter_result.csv --basis BENCH --databases databases/ &
wait
```

6. Build final paper-ready statistics

```sh
python3.10 tools/cli table-4-finalizer -e experiment_table_4/
```

After all commands completed, results are located as follows:

1. `final_results.csv` contains final result as it is presented in the paper.
2. `<class>_result.csv` contains results of `simplifier` evaluation to the circuits of `class`
(note that sizes in it are calculated as for BENCH basis).
