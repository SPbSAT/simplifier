import os
import subprocess
import tempfile
import time

import click


def run_command(command, abc_path, tl):
    process = subprocess.Popen(
        abc_path,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    start_time = time.time()
    out, _ = process.communicate(command, timeout=tl)
    exec_time = round(time.time() - start_time, 5)
    process.kill()
    return exec_time, out


def cec(file_path_1, file_path_2, abc_path, tl):
    return run_command(f"cec -n {file_path_1} {file_path_2}", abc_path, tl)


def change_inputs_order(file_path_1, file_path_2):
    inputs = []
    with open(file_path_1, 'r') as file1:
        for line in file1.readlines():
            if line.startswith("INPUT"):
                inputs.append(line)
    with open(file_path_2, 'r') as file2, tempfile.NamedTemporaryFile(
        mode='w+', delete=False, suffix=".bench"
    ) as temp_file:
        for inp in inputs:
            temp_file.write(inp)
        for line in file2.readlines():
            if not line.startswith("INPUT"):
                temp_file.write(line)
    return temp_file.name


def shorten(string, width):
    if len(string) <= width:
        return string
    return string[:width] + "..."


@click.command()
@click.argument('directory_1', type=click.Path(exists=True))
@click.argument('directory_2', type=click.Path(exists=True))
@click.option(
    '--abc', type=str, help="Path to the executable ABC file.", default="./abc/abc"
)
@click.option(
    '--tl',
    type=int,
    help="Time limit for a single execution [default 60 sec].",
    default=60,
)
def check_equiv(directory_1, directory_2, abc, tl):
    """
    Verify the equivalence of sets of benchmarks, checking only those with identical
    names.

    DIRECTORY_1 is the path to the first directory with benchmarks.

    DIRECTORY_1 is the path to the second directory with benchmarks.

    """
    n_of_ok = 0
    n_of_tl = 0
    n_of_bad = 0
    n_of_not_equiv = 0
    total_time = 0
    for path, subdirs, files in os.walk(directory_1):
        for file in files:
            path_to_file_1 = os.path.join(path, file)
            path_to_file_2 = path_to_file_1.replace(directory_1, directory_2)
            if not os.path.exists(path_to_file_2):
                continue
            new_path_to_file_2 = change_inputs_order(path_to_file_1, path_to_file_2)
            try:
                t, out = cec(path_to_file_1, new_path_to_file_2, abc, tl)
                total_time += t
                if not "Networks are equivalent" in out:
                    if "Networks are NOT EQUIVALENT" in out:
                        click.echo(
                            f"Circuits with name {shorten(file, 20)} are not equivalent"
                        )
                        n_of_not_equiv += 1
                    else:
                        click.echo(
                            f"Circuits with name {shorten(file, 20)} cannot be verified"
                        )
                        n_of_bad += 1
                else:
                    n_of_ok += 1
            except subprocess.TimeoutExpired:
                n_of_tl += 1
            os.remove(new_path_to_file_2)
    click.echo(
        f"{n_of_ok} circuits are equivalent.\n{n_of_not_equiv} circuits are not equivalent."
    )
    if n_of_bad > 0:
        click.echo(f"{n_of_bad} circuits cannot be verified.")
    if n_of_tl > 0:
        click.echo(
            f"To check the remaining {n_of_tl} circuits, increase the time limit."
        )
    click.echo(f"Total time: {total_time}")


def main():
    check_equiv()


if __name__ == "__main__":
    check_equiv()
