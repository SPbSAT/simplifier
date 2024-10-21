import os

import click
import pandas as pd

from cli_group import tools_cli
from utils import number_of_ands


__all__ = [
    'collect_sizes_aig',
]


@tools_cli.command()
@click.option(
    '-i',
    '--input-directory',
    required=True,
    type=str,
    help='Path to a directory where input circuits are located.',
)
@click.option(
    '-s',
    '--stats-path',
    required=True,
    type=str,
    help='Path where resulting statistics should be stored.',
)
def collect_sizes_aig(
    input_directory: str,
    stats_path: str,
):
    """
    Calculate AIG circuit sizes and save to a CSV file.
    Size of an AIG circuit is a number of AND gates in it.

    :param input_directory: path to a directory where input circuits are located.
    :param stats_path: path where resulting statistics should be stored.

    """
    # Get the DataFrame
    df = _iterate_over_circuits(input_directory)

    # Save the DataFrame to the specified output file
    df.to_csv(stats_path, index=False)
    click.echo(f"Statistics was saved to '{stats_path}'.")


def _iterate_over_circuits(folder_path):
    data = []

    # Loop through each file in the folder
    for filename in os.listdir(folder_path):
        file_path = os.path.join(folder_path, filename)

        # Check if it's a file (not a directory)
        if os.path.isfile(file_path):
            size = number_of_ands(file_path)
            data.append({"circuit_name": filename, "size": size})

    # Create a DataFrame from the data
    df = pd.DataFrame(data)
    return df
