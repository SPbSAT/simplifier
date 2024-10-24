# Simplify: a tool for a boolean circuit simplification.

[//]: # (TODO: make a nice readme)

## Abstract

Simplify is a program, that accepts an arbitrary circuit instance provided in BENCH format
and represented either in BENCH or AIG basis, performs attempt to simplify this circuit
and returns (stdout or file) equivalent circuit, transformed in such a way as to reduce the
number of gates in it.

Short about results ...

...

Badges: ...

This README file is structured as follows:

1. Environment configuration ...
2. Light review - a guideline on performing a light review of a tool.
3. Main results reproduction ...
4. Technical info ...

Each guideline section provides two paths: "all-in-one" command to complete section
and "step-by-step" guide to allow one to evaluate process in details.

## Environment configuration

Before conducting any tests of the tool, it is necessary
to perform mandatory environment configuration.

### All-in-one command

...

### Step-by-step guide

todo: ABC dependency

todo: tools dependencies: python3.9, pipx, poetry, ...

[//]: # ()
[//]: # (ABC can be compiled using following command: `make ABC_USE_NO_READLINE=1`.)

0. todo: ABC path
1. todo: simplify compilation
2. todo: python environment

todo: debug compilation for more logs.

In the rest of the README it will be assumed, that shell commands are executed within python
environment, with `abc` and `simplify` executables available at paths specified above.

#### Ubuntu packages

...

```sh
sudo apt install python3-dev python3.9-dev gcc cmake build-essential
```

#### Simplify compilation (C++)

Simplify can be built using `cmake` tool and some standard `C++` compiler (`gcc` and `clang`
should do). To build `simplify` binary execute following steps:

```sh
cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE
cmake --build build/ --config RELEASE
```

(Note that in `Release` build wast amount of logs is disabled.
If one need more logs, `DEBUG` compilation type may be useful.)

As a result ``simplify`` binary will be built. To get info on how ``simplify`` should be used
execute resulting binary with following command:

```sh
build/simplify --help
```

#### ABC compilation

ABC synthesis tool (and its `resyn2` command) is a state-of-art framework for circuit sythesis.
It is used in the paper as a baseline solution for a circuit simplification, which allows one to
access contribution and novelty of the Simplify tool.

For convenience, specific revision of the ABC synthesis tool is brought alongside Simplify
in the `third_party` directory. To compile it, one need to execute following commands

```sh
todo
```

(see. original repository for details https://github.com/berkeley-abc/abc).

#### Python environment

Python >=3.9 is used for scripts located in the `tools/` directory.

1. Install following packages using your package manager:
    - dev version of `python3.9` and `python3.9-distutils` (e.g. `sudo apt install python3.9-dev`)
2. Install `poetry` ([instruction](https://python-poetry.org/docs/)).
3. Change directory `cd tools`
4. Setup virtual environment by running `poetry install`
5. Set your env to the oldest supported Python version `poetry env use 3.9`
6. Enable virtual environment using `poetry shell`
7. Leave directory and return to project's root `cd ..`

Now you should be able to run CLI of tools. Use `python tools/cli --help` to get more info
on the available CLI commands.

## Light review

The Table 3 is quite easily reproducible, so if there is a need to check
that tool works at all, one can jump straight to the section below devoted
to the table 3.

The Table 4 allows to select subset of classes to reproduce results, for
example following commands should give results in appropriate time:

```sh
mkdir light_review_table_4
tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./light_review_table_4/
./build/simplify light_review_table_4/benchmarks/php/ -o light_review_table_4/benchmarks_s/php/ -s light_review_table_4/php_result.csv --basis BENCH --databases databases/
./build/simplify light_review_table_4/benchmarks/mult_miter/ -o light_review_table_4/benchmarks_s/mult_miter/ -s light_review_table_4/mult_miter_result.csv --basis BENCH --databases databases/
python tools/cli table-4-finalizer -e light_review_table_4/
```

All instructions presented below save results of main intermediate steps. To validate correctness
any stage of simplification, one may use utility CLI command `check-equiv`:

`python tools/cli check-equiv --help`

## Main results reproduction

Note that instructions are given for the default Ubuntu shell and may not work in other environment.

Sections below are structured as follows:

1. Name of a subsection is a table name in the paper.
2. Subsection with all-in-one command to run.
3. Subsection with step-by-step guide on how results are computed.
4. Description of resulting artifacts and their mappings to results in the paper.

Statistics is written in `.csv` files with `,` delimiter. Note that in some statistics `;` is used in a value part.

Final steps of main results reproduction sequences print main results to `stdout`, as well as save
a file which name pattern is `final_results*.csv`

Note that circuit sizes for `AIG`-related tables is given as a number of `AND` gates, while for
`BENCH`-related tables size is represented as total number of gates excluding `INPUT` gates.

### Table 1. Distribution of classes and functions by circuit size.

Table 1 humbly represents a distribution of circuit sizes, contained in the database of (nearly)
optimal circuits. This database contains all possible circuits on three inputs and three outputs,
and their particular counts can be reproduced by executing a specialized tool and are not part of
the main statistical result of the paper.

### Table 2. Comparison of several runs of resyn2 against several runs of resyn2 followed by a run of simplify.

#### All-in-one command

```sh
mkdir experiment_table_2
tar -xvf ./benchmark/all_sets_under_50000_for_stat.tar.xz -C ./experiment_table_2/
python tools/cli abc-resyn2 -i experiment_table_2/benchmarks/ -o experiment_table_2/benchmarks_r/ -a .tmp/abc-master/abc -n 2 -n 3 -n 6 -s experiment_table_2/r_results.csv
./build/simplify experiment_table_2/benchmarks_r/resyn2_2/ -o experiment_table_2/benchmarks_r2_s/ -s experiment_table_2/r2_s_result.csv --basis AIG --databases databases/
./build/simplify experiment_table_2/benchmarks_r/resyn2_6/ -o experiment_table_2/benchmarks_r6_s/ -s experiment_table_2/r6_s_result.csv --basis AIG --databases databases/
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks -s experiment_table_2/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_3 -s experiment_table_2/benchmark_r3_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r/resyn2_6 -s experiment_table_2/benchmark_r6_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r2_s -s experiment_table_2/benchmark_r2_s_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_2/benchmarks_r6_s -s experiment_table_2/benchmark_r6_s_sizes.csv
python tools/cli table-2-finalizer -e experiment_table_2/
```

#### Step-by-step guide:

1. Make clean experiment directory:

`mkdir experiment_table_2`

2. Extract `benchmark/representative_benchmarks.tar.xz` to the created directory:

`tar -xvf ./benchmark/all_sets_under_50000_for_stat.tar.xz -C ./experiment_table_2/`

3. Run `ABC` `resyn2` on the extracted benchmarks using provided CLI command:

`python tools/cli abc-resyn2 -i experiment_table_2/benchmarks/ -o experiment_table_2/benchmarks_r/ -a .tmp/abc-master/abc -n 2 -n 3 -n 6 -s experiment_table_2/r_results.csv`

4. Run `simplify` tool on several configurations of resulting circuits:

```
./build/simplify experiment_table_2/benchmarks_r/resyn2_2/ -o experiment_table_2/benchmarks_r2_s/ -s experiment_table_2/r2_s_result.csv --basis AIG --databases databases/
./build/simplify experiment_table_2/benchmarks_r/resyn2_6/ -o experiment_table_2/benchmarks_r6_s/ -s experiment_table_2/r6_s_result.csv --basis AIG --databases databases/
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

`python tools/cli table-2-finalizer -e experiment_table_2/`

After all commands completed, results are located as follows:

1. `final_results_1.csv` and `final_results_2.csv` which contains final results as it is presented in the paper,
in first part and a second part of a table respectfully.


### Table 3. Comparison of circuit sizes and running times between resyn2 and resyn2+simplify

#### All-in-one command

```sh
mkdir experiment_table_3
tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/
python tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a .tmp/abc-master/abc -n 1 -s experiment_table_3/r_results.csv
./build/simplify experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
python tools/cli table-3-finalizer -e experiment_table_3/
```

#### Step-by-step guide

1. Make clean experiment directory:

`mkdir experiment_table_3`

2. Extract `benchmark/representative_benchmarks.tar.xz` to the created directory:

`tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/`

In result `benchmarks` directory must appear.

3. Run `ABC` `resyn2` on the extracted benchmarks using provided CLI command:

`python tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a .tmp/abc-master/abc -n 1 -s experiment_table_3/r_results.csv`

4. Run `simplify` tool on resulting circuits:

`./build/simplify experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/`

5. Collect benchmark sizes

```sh
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
```

6. Build final paper-ready statistics

`python tools/cli table-3-finalizer -e experiment_table_3/`

After all commands completed, results are located as follows:

1. `final_results.csv` contains final result as it is presented in the paper.
2. `benchmark_sizes.csv` contains information about original circuit sizes.
3. `benchmark_r_sizes.csv` contains information about circuit after `resyn2`.
4. `benchmark_rs_sizes.csv` contains information about circuit after `simplify`.


### Table 4. Simplification statistics for various classes of circuits in the BENCH basis.

#### All-in-one command

Warning: this command may take extremely long time (several hours on modern computer) to be
executed sequentially, so main `simplify` calls are parallelized by default. If hardware is
unsuitable for such bends of fate, one can either endure and run the sequential test, or run
benchmarking on only a part of classes, picked randomly and unbiased.

```sh
mkdir experiment_table_4
tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./experiment_table_4/
./build/simplify experiment_table_4/benchmarks/factorization/ -o experiment_table_4/benchmarks_s/factorization/ -s experiment_table_4/factorization_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/ecp/ -o experiment_table_4/benchmarks_s/ecp/ -s experiment_table_4/ecp_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/clique/ -o experiment_table_4/benchmarks_s/clique/ -s experiment_table_4/clique_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/php/ -o experiment_table_4/benchmarks_s/php/ -s experiment_table_4/php_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/I99T/ -o experiment_table_4/benchmarks_s/I99T/ -s experiment_table_4/I99T_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/BristolFashion/ -o experiment_table_4/benchmarks_s/BristolFashion/ -s experiment_table_4/BristolFashion_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/iscas85/ -o experiment_table_4/benchmarks_s/iscas85/ -s experiment_table_4/iscas85_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/miter_sum/ -o experiment_table_4/benchmarks_s/miter_sum/ -s experiment_table_4/miter_sum_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/miter_thr2/ -o experiment_table_4/benchmarks_s/miter_thr2/ -s experiment_table_4/miter_thr2_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/miter_transalg_sort/ -o experiment_table_4/benchmarks_s/miter_transalg_sort/ -s experiment_table_4/miter_transalg_sort_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/mult_miter/ -o experiment_table_4/benchmarks_s/mult_miter/ -s experiment_table_4/mult_miter_result.csv --basis BENCH --databases databases/ &
wait
python tools/cli table-4-finalizer -e experiment_table_4/
```

#### Step-by-step guide

1. Make clean experiment directory:

`mkdir experiment_table_4`

2. Extract `benchmark/bench_for_stat.tar.xz` to the created directory:

`tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./experiment_table_4/`

In result `benchmarks` directory must appear.

3. Run `simplify` tool on resulting circuits:

Warning: this command may take extremely long time (several hours on modern computer) to be
executed sequentially, so main `simplify` calls are parallelized by default. If hardware is
unsuitable for such bends of fate, one can either endure and run the sequential test, or run
benchmarking on only a part of classes, picked randomly and unbiased.

```sh
./build/simplify experiment_table_4/benchmarks/factorization/ -o experiment_table_4/benchmarks_s/factorization/ -s experiment_table_4/factorization_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/ecp/ -o experiment_table_4/benchmarks_s/ecp/ -s experiment_table_4/ecp_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/clique/ -o experiment_table_4/benchmarks_s/clique/ -s experiment_table_4/clique_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/php/ -o experiment_table_4/benchmarks_s/php/ -s experiment_table_4/php_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/I99T/ -o experiment_table_4/benchmarks_s/I99T/ -s experiment_table_4/I99T_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/BristolFashion/ -o experiment_table_4/benchmarks_s/BristolFashion/ -s experiment_table_4/BristolFashion_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/iscas85/ -o experiment_table_4/benchmarks_s/iscas85/ -s experiment_table_4/iscas85_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/miter_sum/ -o experiment_table_4/benchmarks_s/miter_sum/ -s experiment_table_4/miter_sum_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/miter_thr2/ -o experiment_table_4/benchmarks_s/miter_thr2/ -s experiment_table_4/miter_thr2_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/miter_transalg_sort/ -o experiment_table_4/benchmarks_s/miter_transalg_sort/ -s experiment_table_4/miter_transalg_sort_result.csv --basis BENCH --databases databases/ &
./build/simplify experiment_table_4/benchmarks/mult_miter/ -o experiment_table_4/benchmarks_s/mult_miter/ -s experiment_table_4/mult_miter_result.csv --basis BENCH --databases databases/ &
wait
```

6. Build final paper-ready statistics

`python tools/cli table-4-finalizer -e experiment_table_4/`

After all commands completed, results are located as follows:

1. `final_results.csv` contains final result as it is presented in the paper.


## Technical info

### Code structure

todo: source files
todo: third_party


[//]: # (## Notes)

[//]: # ()
[//]: # (Tar archive with circuit benchmarks are located in `benchmarks`.)

## Technical info

### Platform

Simplify is written using C++20. Simplify was developed and tested on Ubuntu 24.04 with gcc x86-64
compiler, but should not cause any problems on other common Linux distributions.

### Dependencies

Simplify tool utilizes external library [argparse](https://github.com/p-ranav/argparse/tree/v2.9)
to ease CLI arguments parsing and usage. Also test framework [GoogleTest](https://github.com/google/googletest)
is used to run unit tests which cover main functionalities of the tool.

To make this code snapshot whole, `argparse` and `gtest` repository snapshots are vendored
alongside it and are presented in the `third_party/` directory.

### Code quality

#### Tests

...

#### Static code analysis

`clang-format` and `clang-tidy` are used for maintaining code quality.

Both of them need to be separately installed before can be used. For example,
for Ubuntu they can be installed using following command:

```sudo apt install clang-tidy clang-format```

##### Linting

To manually run `clang-tidy`, first one needs to compile cmake project to be able
to use a build artifact `build/compile_commands.json`. Then one will be able to
run linter by a simple command:

`clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy`

To make `cland-tidy` fix warnings for you, simply run same command with flags
`--fix` and `--fix-errors`:

`clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy --fix --fix-errors --fix-notes`

Though it may break code in some cases, so be careful and check all made fixes.

##### Formatting

`clang-format-18` is used to maintain code uniformity.

`find ./src/ ./app/ -name '*.cpp' -o -name '*.hpp' | xargs clang-format --style="file:.clang-format" -i`
