# Circuit SAT Preprocessor

`preprocessor` is a program, that accepts an arbitrary circuit instance in `BENCH`
format and returns (stdout or file) equivalent circuit, transformed in a way to
reduce number of gates in it.

## Environment

Preprocessor is written using C++20, and is ready to be compiled on some common Linux distributive,
e.g. Ubuntu.

Preprocessor has one external dependency: [argparse](https://github.com/p-ranav/argparse/tree/v2.9). 
It is used to ease CLI arguments parsing.

Preprocessor also includes one development dependency: [GoogleTest](https://github.com/google/googletest).
Gtest is used to run unit tests which cover main logic of preprocessor.

To make this code package whole, `argparse` and `gtest` repository snapshots are included in this
archive and are presented in the `vendor/` directory. 

## Building

Preprocessor is build using `cmake` tool.

To build `preprocessor` binary execute following steps

```
$ mkdir build
$ cd build
$ cmake [-DCMAKE_BUILD_TYPE=[Release|Debug]] ..
$ cmake --build .
```

As a result `preprocessor` binary will be built.

To get info on how preprocessor should be used execute resulting binary with following command:

```
$ ./preprocessor --help
```

## Launching

Preprocessor supports simplification in 2 basis: BENCH and AIG

To run simplification in the following basis you need to have a database with small circuits in the same directory with running script named as ```database_aig.txt``` or ```database_bench.txt``` respectively

Both databases arre already presented and you can use them for circuit simplification and are stored in ```/core/databases``` directory