# Simplify: a tool for a boolean circuit simplification.

Simplify is a program, that accepts an arbitrary circuit instance provided in `BENCH` or `AIG`
format and returns (stdout or file) equivalent circuit, transformed in such a way as to reduce
the number of gates in it.

## Building and execution

### Building

Simplify can be built using `cmake` tool.

To build `simplify` binary execute following steps

```
$ mkdir build
$ cd build
$ cmake [-DCMAKE_BUILD_TYPE=[Release|Debug]] ..
$ cmake --build .
```

As a result ``simplify`` binary will be built.

To get info on how ``simplify`` should be used execute resulting binary with following command:

```sh
$ simplify --help
```

### Execution

Simplify supports simplification of circuit provided in one of two bases: `AIG` or `BENCH`.

To run simplification in the following basis you need to have a database with small circuits in the
same directory with running script named as `database_aig.txt` or `database_bench.txt` respectively.

Both databases are already presented in the `./databases` directory and are ready to be used for a
circuit simplification.


## Source code structure

...


## Technical info

### Platform

Simplify is written using C++20. Simplify was developed and tested on Ubuntu 24.04 with gcc x86-64
compiler, but should not cause any problems on other common Linux distributions.

### Dependencies

Simplify utilizes external library [argparse](https://github.com/p-ranav/argparse/tree/v2.9) to ease
CLI arguments parsing and usage. Also test framework [GoogleTest](https://github.com/google/googletest).
is used to run unit tests which cover main functionalities of the tool.

To make this code snapshot whole, `argparse` and `gtest` repository snapshots are vendored
alongside it and are presented in the `third_party/` directory.

### Code quality

`clang-format` and `clang-tidy` are used for maintaining code quality.

Both of them need to be separately installed before can be used. For example,
for Ubuntu they can be installed using following command:

```sudo apt install clang-tidy clang-format```

#### Linter

`clang-tidy` is used for linting purposes.

To manually run `clang-tidy`, first one needs to compile cmake project to be able
to use a build artifact `build/compile_commands.json`. Then one will be able to
run linter by a simple command:

`clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy`

To make `cland-tidy` fix warnings for you, simply run same command with flags
`--fix` and `--fix-errors`:

`clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy --fix --fix-errors --fix-notes`

Though it may break code in some cases, so be careful and check all made fixes.

#### Formatter

`clang-format-18` is used to maintain code uniformity.

`find ./src/ ./app/ -name '*.cpp' -o -name '*.hpp' | xargs clang-format --style="file:.clang-format" -i`

### Tools

Python >=3.8 is used for scripts in the `tools/` directory.

1. Install following packages using your package manager:
    - dev version of `python3.9` and `python3.9-distutils` (e.g. `sudo apt install python3.9-dev`)
1. Install `poetry` ([instruction](https://python-poetry.org/docs/)).
1. Change directory `cd tools`
1. Setup virtual environment by running `poetry install`
1. Set your env to the oldest supported Python version `poetry env use 3.9`
1. Enable virtual environment using `poetry shell`

Now you should be able to run CLI of tools. Use `python ./cli --help` to get more info.