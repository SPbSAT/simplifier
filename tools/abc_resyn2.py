import os
import pathlib
import subprocess
import time
import typing as tp

import click
import pandas as pd

from cli_group import tools_cli
from tqdm import tqdm
from utils import number_of_ands


__all__ = [
    'abc_resyn2',
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
    '-o',
    '--output-directory',
    required=True,
    type=str,
    help='Path to a directory where resulting simplified circuits will be stored.',
)
@click.option(
    '-s',
    '--stats-path',
    required=True,
    type=str,
    help='Path where resulting statistics should be stored.',
)
@click.option(
    '-a',
    '--abc-path',
    required=True,
    type=str,
    help='Path to an ABC executable, which will be used to simplify circuits.',
)
@click.option(
    '-t',
    '--timelimit',
    required=False,
    default=120,
    type=int,
    help='Timelimit for each resyn2 execution in integer seconds.',
)
@click.option(
    '-n',
    '--number-of-iterations',
    required=False,
    default=[7],
    type=int,
    help='Determines number of consequent resyn2 applications. May be specified several times.',
    multiple=True,
)
def abc_resyn2(
    input_directory: str,
    output_directory: str,
    stats_path: str,
    abc_path: str,
    timelimit: int,
    number_of_iterations: tp.Iterable[int],
):
    """
    Runs several iterations of ABC `resyn2` command on each circuit in the
    `input_directory`, and saves resulting simplified circuit to the `output_directory`.

    :param input_directory: path to a directory where input circuits are located.
    :param output_directory: path to a directory where resulting simplified circuits
        will be stored.
    :param stats_path: path where resulting statistics should be stored.
    :param abc_path: path to an ABC executable, which will be used to simplify circuits.
    :param timelimit: timelimit for each resyn2 execution in integer seconds.
    :param number_of_iterations: each value determines stratagy: a number of consequent
        resyn2 applications.

    """
    # List to store results
    results = []

    # Prepare output directory
    if not os.path.exists(output_directory):
        os.makedirs(output_directory)

    # Get total number of files for tqdm progress bar
    total_files = sum([len(files) for _, _, files in os.walk(input_directory)])

    for root, dirs, files in os.walk(input_directory):
        for file in tqdm(files, total=total_files, desc="Processing Benchmarks"):

            original_path = os.path.join(root, file)

            if os.path.splitext(file)[1] != ".bench":
                click.echo(f"Skipping file '{original_path}'.")
                continue

            # Original size
            original_size = number_of_ands(original_path)

            # Placeholder for each step
            resyn_times = {}
            resyn_sizes = {}

            for noi in number_of_iterations:
                resyn_dir = output_directory + os.sep + f'resyn2_{noi}'
                if not os.path.exists(resyn_dir):
                    os.makedirs(resyn_dir)

                resyn_path = os.path.join(resyn_dir, file)

                if not os.path.isfile(resyn_path):
                    time_taken = execute_resyn(
                        input_path=original_path,
                        output_path=resyn_path,
                        abc_path=abc_path,
                        timelimit=timelimit,
                        number_of_iterations=noi,
                    )
                else:
                    time_taken = None

                size_after_resyn = number_of_ands(resyn_path)

                resyn_times[f'resyn2_{noi} Time'] = time_taken
                resyn_sizes[f'resyn2_{noi} Size'] = size_after_resyn

            # Adding results to list
            results.append(
                {
                    'Benchmark': file,
                    'Original Size': original_size,
                    **resyn_times,
                    **resyn_sizes,
                }
            )

    # Resolve stats csv path.
    stats_path_object = pathlib.Path(stats_path)
    if stats_path_object.is_dir():
        if not os.path.exists(stats_path):
            os.makedirs(stats_path)
        stats_filepath = os.path.join(stats_path, f"abc_resyn2_experiment_results.csv")
    else:
        stats_filepath = stats_path

    # Save the DataFrame to a CSV file.
    results_df = pd.DataFrame(results)
    results_df.to_csv(
        stats_filepath,
        index=False,
    )
    click.echo(f"Results saved to '{stats_filepath}'.")


def execute_resyn(
    *,
    input_path: str,
    output_path: str,
    abc_path: str,
    timelimit: int,
    number_of_iterations: int,
) -> tp.Optional[float]:
    """
    Executes ABC resyn2 command to simplify circuit at `input_path` and stores result at
    `output_path`.

    :param input_path: path to original circuit.
    :param output_path: path where resulting circuit will be stored.
    :param abc_path: path to an ABC executable.
    :param timelimit: timelimit for a resyn2 execution.
    :param number_of_iterations: number of consequent resyn2 applications. :return
        elapsed time of resyn2 execution or None, if execution failed.

    """
    resyn_command_block = [resyn2_command()] * number_of_iterations
    command = join_commands(
        [
            read_command(input_path),
            strash_command(),
        ]
        + resyn_command_block
        + [
            write_command(output_path),
        ]
    )
    return execute_abc_command(
        abc_path=abc_path,
        command=command,
        timelimit=timelimit,
    )


def execute_abc_command(
    *,
    abc_path: str,
    command: str,
    timelimit: int,
) -> tp.Optional[float]:
    """
    Executes provided ABC `command` in a shell subprocess.

    :param abc_path: path to an ABC executable.
    :param command: a command to execute.
    :param timelimit: timelimit for a command execution. :return elapsed time of
        execution or None, if execution failed.

    """
    process: tp.Optional[subprocess.Popen] = None
    try:
        # Start an ABC process.
        start_time = time.monotonic()
        process = subprocess.Popen(
            [abc_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        out, err = process.communicate(command, timeout=timelimit)

        if err:
            click.echo(err, err=True)

        exec_time = round(time.monotonic() - start_time, 2)

        return exec_time

    except subprocess.TimeoutExpired:
        if process is not None:
            process.kill()
            click.echo("Process timed out.", err=True)
        return None


def join_commands(command_list):
    """Constructs one command line from several commands, separating them with `;`"""
    return "; ".join(command_list)


def read_command(filename):
    return f"read {filename}"


def write_command(filename):
    return f"write {filename}"


def strash_command():
    return "strash"


def resyn2_command():
    # resyn2 is actually an alias for this sequence of commands
    # (see `abc.rc` in ABC repository for details)
    return "balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance"
