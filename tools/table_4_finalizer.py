import pathlib

import click
import pandas as pd

from cli_group import tools_cli

__all__ = [
    'table_4_finalizer',
]


@tools_cli.command()
@click.option(
    '-e',
    '--experiment-directory',
    required=True,
    type=str,
    help='Path to a directory where an experiment was conducted.',
)
def table_4_finalizer(
    experiment_directory: str,
):
    """
    Finalizes statistics collected during "Table 4. Simplification statistics for
    various classes of circuits in the BENCH basis." described in the README.

    Note that some paths are hardcoded by intention because this script purpose is
    solely to finalize a specific experiment (as result, it expects specific files to be
    located in the provided experiment directory).

    """
    resulting_df = pd.DataFrame(columns=["class", "improvement (%)"])

    experiment_dir_path = pathlib.Path(experiment_directory)
    for child in experiment_dir_path.iterdir():
        if not str(child).endswith("_result.csv"):
            continue

        class_name = str(child).replace("_result.csv", "" ).rsplit('/')[-1]

        df = pd.read_csv(child, delimiter=',')
        improvement = (df['Gates before'] - df['Gates after']) / df['Gates before']

        resulting_df.loc[len(df)] = {
            "class": class_name,
            "improvement (%)": round(improvement.mean() * 100, 2),
        }

    resulting_df.to_csv(experiment_dir_path / "final_results.csv", index=False)

    click.echo(resulting_df.to_string(index=False))
