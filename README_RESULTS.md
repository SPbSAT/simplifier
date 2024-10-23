# Simplify: a tool for a boolean circuit simplification.

[//]: # (TODO: make a nice readme)

...

## 

ABC dependency

Tools dependencies: python3.9, pipx, poetry, clang-tidy, clang-format


## Code structure

sources
third_party

todo

[//]: # (## Notes)

[//]: # ()
[//]: # (Tar archive with circuit benchmarks are located in `benchmarks`.)

[//]: # ()
[//]: # (ABC can be compiled using following command: `make ABC_USE_NO_READLINE=1`.)

[//]: # ()
[//]: # (One can run `resyn2` circuit simplification using tool &#40;while in python environment&#41;)

[//]: # ()
[//]: # (`python cli abc-resyn2 -i ../.tmp/all_sets_under_50000_for_stat -o ../.tmp/all_sets_under_50000_for_stat_simp_003/ -a ../.tmp/abc-master/abc -s ../.tmp/resyn_results/ -n 7`)

[//]: # ()
[//]: # (To check circuits for equivalence after simplification one may use)

[//]: # ()
[//]: # (`python tools/cli check-equiv .tmp/all_sets_under_50000_for_stat_simp/resyn7/ .tmp/all_sets_under_50000_for_stat_simp_003/resyn2_7/ -a .tmp/abc-master/abc`)

## Preliminaries

0. todo: ABC path
1. todo: simplify compilation
2. todo: python environment
3. todo: database paths

todo: debug compilation for more logs.

In the rest of the README it will be assumed, that shell commands are executed within python
environment, with `abc` and `simplify` executables available at paths specified above.

## Light review

Table 3 is quite easily reproducible, so if there is needs to check
that anything works, one can jump straight to section below devoted
to table 3.

todo: opportunity to calculate something on the one class only.


## Main results reproduction

Note that instructions are given for the default Ubuntu shell and may not work in other environment.

Sections below are structured as follows:

1. Name of a subsection is a table name in the paper.
2. Subsection with all-in-one command to run.
3. Subsection with step-by-step guide on how results are computed.
4. Description of resulting artifacts and their mappings to results in the paper.

### Table 2. Comparison of several runs of resyn2 against several runs of resyn2 followed by a run of simplify.

#### All-in-one command

...
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

### Table 3. Comparison of circuit sizes and running times between resyn2 and resyn2+simplify

#### All-in-one command

...

#### Step-by-step guide

1. Make clean experiment directory:

`mkdir experiment_table_3`

2. Extract `benchmark/representative_benchmarks.tar.xz` to the created directory:

`tar -xvf ./benchmark/representative_benchmarks.tar.xz -C ./experiment_table_3/`

In result `all_sets_for_stat` directory must appear.

3. Run `ABC` `resyn2` on the extracted benchmarks using provided CLI command:

`python tools/cli abc-resyn2 -i experiment_table_3/benchmarks/ -o experiment_table_3/benchmarks_r/ -a .tmp/abc-master/abc -n 1 -s experiment_table_3/r_results.csv`

4. Run `simplify` tool on resulting circuits:

`./build/simplify experiment_table_3/benchmarks_r/resyn2_1/ -o experiment_table_3/benchmarks_rs/ -s experiment_table_3/rs_result.csv --basis AIG --databases databases/`

5. To validate ...

`python tools/cli check-equiv experiment_table_3/benchmarks experiment_table_3/benchmarks_rs -a .tmp/abc-master/abc`

6. Collect statistics

```sh
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks -s experiment_table_3/benchmark_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_r/resyn2_1 -s experiment_table_3/benchmark_r_sizes.csv
python tools/cli collect-sizes-aig -i experiment_table_3/benchmarks_rs -s experiment_table_3/benchmark_rs_sizes.csv
```

After all commands completed, results are located as follows.

TODO: not true, simplify should write better statistics
1. `r_result` contains information about circuit sizes before and after `resyn2` execution.
2. `rs_result` contains information about circuit sizes before and after `simplify` execution, including information about per-iteration sizes (see `circuit_size_*` columns).


### Table 4. Simplification statistics for various classes of circuits in the BENCH basis.

#### All-in-one command

...

#### Step-by-step guide

1. Make clean experiment directory:

`mkdir experiment_table_4`

2. Extract `benchmark/bench_for_stat.tar.xz` to the created directory:

`tar -xvf ./benchmark/bench_for_stat.tar.xz -C ./experiment_table_4/`

In result `all_sets_for_stat` directory must appear.

3. Run `simplify` tool on resulting circuits:

```sh
./build/simplify experiment_table_4/benchmarks/BristolFashion/ -o experiment_table_4/benchmarks_s/BristolFashion/ -s experiment_table_4/BristolFashion_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/clique/ -o experiment_table_4/benchmarks_s/clique/ -s experiment_table_4/clique_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/ecp/ -o experiment_table_4/benchmarks_s/ecp/ -s experiment_table_4/ecp_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/factorization/ -o experiment_table_4/benchmarks_s/factorization/ -s experiment_table_4/factorization_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/I99T/ -o experiment_table_4/benchmarks_s/I99T/ -s experiment_table_4/I99T_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/iscas85/ -o experiment_table_4/benchmarks_s/iscas85/ -s experiment_table_4/iscas85_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/miter_sum/ -o experiment_table_4/benchmarks_s/miter_sum/ -s experiment_table_4/miter_sum_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/miter_thr2/ -o experiment_table_4/benchmarks_s/miter_thr2/ -s experiment_table_4/miter_thr2_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/miter_transalg_sort/ -o experiment_table_4/benchmarks_s/miter_transalg_sort/ -s experiment_table_4/miter_transalg_sort_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/mult_miter/ -o experiment_table_4/benchmarks_s/mult_miter/ -s experiment_table_4/mult_miter_result.csv --basis BENCH --databases databases/
./build/simplify experiment_table_4/benchmarks/php/ -o experiment_table_4/benchmarks_s/php/ -s experiment_table_4/php_result.csv --basis BENCH --databases databases/
```

4. Count average improvement% on each circuit $(before - after) / before$.
