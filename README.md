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

### Tools

...

#### Linter

To manually run `clang-tidy`, first one needs to compile cmake project to be able
to use a build artifact `build/compile_commands.json`. Then one will be able to
run linter by a simple command:

```clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy```

To make `cland-tidy` fix warnings for you, simply run same command with flags
`--fix` and `--fix-errors`:

```clang-tidy ./src/**/*.* -p build/ --config-file=.clang-tidy --fix --fix-errors --fix-notes```

Though it may break code in some cases, so be careful and check all made fixes.