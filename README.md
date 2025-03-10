# Simplifier: a New Tool for Boolean Circuit Simplification

The Boolean circuit simplification problem involves finding a smaller circuit that computes the
same function as a given Boolean circuit. This problem is closely related to several key areas
with both theoretical and practical applications, such as logic synthesis, satisfiability, and
verification.

This repository contains source files for ``simplifier``, a tool for boolean circuit simplification.

## Building

Simplifier tool can be compiled using `cmake` tool and a standard `C++` compiler
supporing C++20 standard (`gcc` should do). To build ``simplifier`` binary execute
following steps:

```sh
cmake -B build/ -DCMAKE_BUILD_TYPE=RELEASE
cmake --build build/ --config RELEASE
```

(Note that in `Release` build wast amount of logs is disabled.
If one need more logs, `DEBUG` compilation type may be useful.)

As a result ``simplifier`` binary will be built. To get info on how
``simplifier`` should be used execute resulting binary with following
command:

```sh
build/simplifier --help
```

## Usage instruction

The Simplifier tool performs simplification of boolean circuits provided in
one of two bases: `AIG` or `BENCH`. To run simplification one should provide
an `--input-path` and `--output` parameters: first is a path to the directory
with boolean circuits, and second is a path where simplified circuits are to
be stored. Both input and output paths should be directories. The tool will
take an attempt to read all files in input directory as `.bench` circuits.
Each circuit will be processed by the tool distinctly.

Required basis of input circuits should be specified manually using a `--basis`
parameter. It will serve as a hint for the tool, which will help it to choose
suitable simplification algorithms.

One also should provide a path to the directory with databases containing
(nearly) optimal circuits with three inputs and three outputs by providing
a `--databases` parameter. Note that databases are available in `databases/`
directory located at the repository root, which is a default value for `--databases`.

To store statistics of the simplification process one may additionally specify
a `--statistics` parameter, which is a path to location where a `*.csv` file
with gathered statistics should be dumped. Note that resulting csv file will use
a `,` character as a delimiter, whilst `;` character may be a valid item value.

Example usage command:

```sh
./build/simplifier -i input_circuit/ -o result_circuits/ -s statistics.csv
```

### Statistics format descriptions

Statistics is written in `.csv` files with `,` delimiter. Note that in some
values character `;` is used as internal value separator.

Circuit sizes for `AIG` based tables are given as number of `AND` gates, while for the
`BENCH` based tables size is represented by total number of gates excluding `INPUT`gates.

## Benchmarking

A detailed step-by-step guide on the tool benchmarking is located in the
[BENCHMARKING.md](./BENCHMARKING.md) file. This guide contains steps to reproduce
results, presented in the related paper.

## Technical info

### Code structure

Main `simplifier` directory contains following directories:

- `app/` directory contains compilable `simplifier.cpp` file, which contains an entry-point (`main`).
- `benchmarks/` directory contains `tar` archives with boolean circuit benchmarks used for experiments.
- `databases/` directory contains databases of the (nearly) optimal small circuits for BENCH and AIG bases.
- `src/` directory implements main tool functionalities, and organized as a header-only library.
- `tests/` directory implements unit tests for the main functionalities of the tool.
- `third_party/` directory contains third-party libraries, used either for the tool itself (`argparse`),
  code quality checks (`GTest`), or for the experiments conduction (`ABC`). Those libraries are vendored
  within this repository for the completeness and reproducibility.
- `tools/` directory contains CLI utilities useful for experiments, which are to be run within
  right `python 3.10` environment.

### Platform

Simplifier is written using `C++20`, was developed and tested on the `Ubuntu OS`
equipped with `gcc x86-64` compiler, but should not cause any problems on other
common Linux distributions.

### Dependencies

Simplifier tool utilizes external library [argparse](https://github.com/p-ranav/argparse/tree/v2.9)
to ease CLI arguments parsing and usage. Also test framework [GoogleTest](https://github.com/google/googletest)
is used to run unit tests which cover main functionalities of the tool.

To make this repository whole, `argparse` and `gtest` repository are vendored as submodules
and are presented in the `third_party/` directory.

Experiment environment of a tool includes additional dependencies such as
`ABC tool` and `python 3.10` environment.

### Code quality

#### Testing

Core parts of a code are covered with tests, located at `tests/` directory.
Test are written using `GTest` framework.

In addition to unit tests, main capabilities of the tool were tested one the wide range of
Boolean circuits, both industrial and hand-crafter. Results of simplification were tested
for equivalence to the original circuits.

#### Static code analysis

`clang-format` and `clang-tidy` are used for maintaining code quality.
Note that both of them are used as a part of `CI` flow, and their execution
may be checked there.
