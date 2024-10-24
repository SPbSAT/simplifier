import pathlib

import click
import pandas as pd

from cli_group import tools_cli

__all__ = [
    'table_3_finalizer',
]


@tools_cli.command()
@click.option(
    '-e',
    '--experiment-directory',
    required=True,
    type=str,
    help='Path to a directory where an experiment was conducted.',
)
def table_3_finalizer(
    experiment_directory: str,
):
    """
    Finalizes statistics collected during "Table 3. Comparison of circuit sizes and running times
    between resyn2 and resyn2+simplify" described in the README.

    Note that some paths are hardcoded by intention because this script purpose is solely
    to finalize a specific experiment (as result, it expects specific files to be located
    in the provided experiment directory).

    """
    experiment_dir_path = pathlib.Path(experiment_directory)

    # Stage I: merge sizes
    sizes = pd.read_csv(experiment_dir_path / "benchmark_sizes.csv", delimiter=',')
    sizes = sizes.rename(columns={"size": "original_size"})

    r_sizes = pd.read_csv(experiment_dir_path / "benchmark_r_sizes.csv", delimiter=',')
    r_sizes = r_sizes.rename(columns={"size": "size_r"})

    rs_sizes = pd.read_csv(experiment_dir_path / "benchmark_rs_sizes.csv", delimiter=',')
    rs_sizes = rs_sizes.rename(columns={"size": "size_r_s"})

    sizes_merged = pd.merge(sizes, r_sizes, on='circuit_name')
    sizes_merged = pd.merge(sizes_merged, rs_sizes, on='circuit_name')

    # Stage II: merge time data and data on simplification iterations
    r_results = pd.read_csv(experiment_dir_path / "r_results.csv", delimiter=',')
    r_results = r_results.rename(columns={"Benchmark": "circuit_name", "resyn2_1 Time": "time_r"})
    r_results = r_results[['circuit_name', 'time_r']]

    rs_result = pd.read_csv(experiment_dir_path / "rs_result.csv", delimiter=',')
    rs_result['circuit_name'] = rs_result['File path'].map(lambda x: x.rsplit('/')[-1])
    rs_result = rs_result.rename(columns={"Simplify time": "time_rs"})

    rs_result = rs_result[['circuit_name', 'time_rs', 'Reduced subcircuits by iter']]
    rs_result['circuit_name'] = rs_result[['circuit_name']].map(lambda x: x.rsplit('/')[-1])

    final_df = pd.merge(sizes_merged, r_results, on='circuit_name')
    final_df = pd.merge(final_df, rs_result, on='circuit_name')
    final_df = final_df.sort_values(by='circuit_name')

    final_df.to_csv(experiment_dir_path / "final_results.csv", index=False)

    click.echo(final_df)
