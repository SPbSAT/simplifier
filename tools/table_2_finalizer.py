import pathlib

import click
import pandas as pd

from cli_group import tools_cli

__all__ = [
    'table_2_finalizer',
]


@tools_cli.command()
@click.option(
    '-e',
    '--experiment-directory',
    required=True,
    type=str,
    help='Path to a directory where an experiment was conducted.',
)
def table_2_finalizer(
    experiment_directory: str,
):
    """
    Finalizes statistics collected during "Table 2. Comparison of several runs of resyn2
    against several runs of resyn2 followed by a run of simplify." described in the README.

    Note that some paths are hardcoded by intention because this script purpose is solely
    to finalize a specific experiment (as result, it expects specific files to be located
    in the provided experiment directory).

    """
    experiment_dir_path = pathlib.Path(experiment_directory)

    benchmark_sizes = pd.read_csv(experiment_dir_path / "benchmark_sizes.csv", delimiter=',')
    benchmark_sizes = benchmark_sizes.rename(columns={"size": "size"})

    benchmark_r3_sizes = pd.read_csv(experiment_dir_path / "benchmark_r3_sizes.csv", delimiter=',')
    benchmark_r3_sizes = benchmark_r3_sizes.rename(columns={"size": "size_r3"})

    benchmark_r6_sizes = pd.read_csv(experiment_dir_path / "benchmark_r6_sizes.csv", delimiter=',')
    benchmark_r6_sizes = benchmark_r6_sizes.rename(columns={"size": "size_r6"})

    benchmark_r2_s_sizes = pd.read_csv(experiment_dir_path / "benchmark_r2_s_sizes.csv", delimiter=',')
    benchmark_r2_s_sizes = benchmark_r2_s_sizes.rename(columns={"size": "size_r2_s"})

    benchmark_r6_s_sizes = pd.read_csv(experiment_dir_path / "benchmark_r6_s_sizes.csv", delimiter=',')
    benchmark_r6_s_sizes = benchmark_r6_s_sizes.rename(columns={"size": "size_r6_s"})

    r_results = pd.read_csv(experiment_dir_path / "r_results.csv", delimiter=',')

    r6_s_result = pd.read_csv(experiment_dir_path / "r6_s_result.csv", delimiter=',')
    r6_s_result = r6_s_result.rename(columns={'File path': 'circuit_name', 'Simplify time': 'time_r6_s'})
    r6_s_result = r6_s_result[['circuit_name', 'time_r6_s']]
    r6_s_result['circuit_name'] = r6_s_result['circuit_name'].apply(lambda x: x.rsplit('/')[-1])

    r2_s_result = pd.read_csv(experiment_dir_path / "r2_s_result.csv", delimiter=',')
    r2_s_result = r2_s_result.rename(columns={'File path': 'circuit_name', 'Simplify time': 'time_r2_s'})
    r2_s_result = r2_s_result[['circuit_name', 'time_r2_s']]
    r2_s_result['circuit_name'] = r2_s_result['circuit_name'].apply(lambda x: x.rsplit('/')[-1])

    # Stage I. First table.
    r_results_1_df = r_results[['Benchmark', 'resyn2_6 Time']]
    r_results_1_df = r_results_1_df.rename(columns={'Benchmark': 'circuit_name', 'resyn2_6 Time': 'time_r6'})

    table_1_df = pd.merge(benchmark_sizes, benchmark_r6_sizes, on='circuit_name')
    table_1_df = pd.merge(table_1_df, benchmark_r6_s_sizes, on='circuit_name')
    table_1_df = pd.merge(table_1_df, r_results_1_df, on='circuit_name')
    table_1_df = pd.merge(table_1_df, r6_s_result, on='circuit_name')
    table_1_df['class'] = table_1_df['circuit_name'].map(lambda x: x.split('__')[0].rstrip('_sat').rstrip('_unsat'))
    table_1_df['absolute'] = (table_1_df['size_r6'] - table_1_df['size_r6_s']) / table_1_df['size_r6']
    table_1_df['relative'] = (table_1_df['size_r6'] - table_1_df['size_r6_s']) / (table_1_df['size'] - table_1_df['size_r6'])
    table_1_df['time'] = ((table_1_df['time_r6_s'] + table_1_df['time_r6']) - table_1_df['time_r6']) / table_1_df['time_r6']
    table_1_df = table_1_df[['class', 'relative', 'absolute', 'time']]

    mean_1_df = round(table_1_df.groupby('class').mean() * 100, 2)
    median_1_df = round(table_1_df.groupby('class').median() * 100, 2)
    result_1_df = pd.merge(mean_1_df, median_1_df, on='class', suffixes=(' (avg)', ' (median)'))

    click.echo(result_1_df)

    result_1_df.to_csv(experiment_dir_path / "final_results_1.csv", index=False)

    # Stage II. Second table.
    r_results_2_df = r_results[['Benchmark', 'resyn2_2 Time', 'resyn2_3 Time']]
    r_results_2_df = r_results_2_df.rename(columns={'Benchmark': 'circuit_name', 'resyn2_2 Time': 'time_r2', 'resyn2_3 Time': 'time_r3'})

    table_2_df = pd.merge(benchmark_sizes, benchmark_r3_sizes, on='circuit_name')
    table_2_df = pd.merge(table_2_df, benchmark_r2_s_sizes, on='circuit_name')
    table_2_df = pd.merge(table_2_df, r_results_2_df, on='circuit_name')
    table_2_df = pd.merge(table_2_df, r2_s_result, on='circuit_name')
    table_2_df['class'] = table_2_df['circuit_name'].map(lambda x: x.split('__')[0].rstrip('_sat').rstrip('_unsat'))
    table_2_df['absolute'] = (table_2_df['size_r3'] - table_2_df['size_r2_s']) / table_2_df['size_r3']
    table_2_df['relative'] = (table_2_df['size_r3'] - table_2_df['size_r2_s']) / (table_2_df['size'] - table_2_df['size_r3'])
    table_2_df['time'] = ((table_2_df['time_r2_s'] + table_2_df['time_r3']) - table_2_df['time_r3']) / table_2_df['time_r3']
    table_2_df = table_2_df[['class', 'relative', 'absolute', 'time']]

    mean_2_df = round(table_2_df.groupby('class').mean() * 100, 2)
    median_2_df = round(table_2_df.groupby('class').median() * 100, 2)
    result_2_df = pd.merge(mean_2_df, median_2_df, on='class', suffixes=(' (avg)', ' (median)'))

    click.echo(result_2_df)

    result_2_df.to_csv(experiment_dir_path / "final_results_2.csv", index=False)
