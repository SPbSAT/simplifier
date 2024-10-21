import os
import subprocess
import time

import pandas as pd
from tqdm import tqdm  # Import tqdm for progress bar

BENCHMARKS_DIRECTORY = "all_sets_under_50000"
SIMP_DIRECTORY = "all_sets_under_50000_simp"
PATH_TO_ABC = "../abc/abc"
TL = 120


# Function to compute circuit size based on AND gates
def circuit_size(filename):
    size = 0
    try:
        with open(filename) as file:
            for line in file:
                if "AND" in line:
                    size += 1
    except FileNotFoundError:
        return None  # In case the file is not found
    return size


def construct_command(command_list):
    """Constructs one command line from several commands, separating them with ;"""
    return "; ".join(command_list)


def read_command(filename):
    return f"read {filename}"


def strash_command():
    return "strash"


def resyn_command():
    return "resyn2"


def write_command(filename):
    return f"write {filename}"


def run_abc_command(command):
    try:
        # Start the process
        start_time = time.time()
        process = subprocess.Popen(
            [PATH_TO_ABC],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        out, err = process.communicate(command, timeout=TL)

        # print("Output:", out)
        if err:
            print("Error:", err)

        exec_time = round(time.time() - start_time, 2)
        return exec_time

    except subprocess.TimeoutExpired:
        process.kill()
        print("Process timed out.")
        return None


def convert_to_aig(input_path, output_path):
    command = construct_command(
        [read_command(input_path), strash_command(), write_command(output_path)]
    )
    return run_abc_command(command)


def resyn(input_path, output_path):
    command = construct_command(
        [
            read_command(input_path),
            strash_command(),
            resyn_command(),
            write_command(output_path),
        ]
    )
    return run_abc_command(command)


def resyn2(input_path, output_path):
    command = construct_command(
        [
            read_command(input_path),
            strash_command(),
            resyn_command(),
            resyn_command(),
            write_command(output_path),
        ]
    )
    return run_abc_command(command)


def resyn3(input_path, output_path):
    command = construct_command(
        [
            read_command(input_path),
            strash_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            write_command(output_path),
        ]
    )
    return run_abc_command(command)


def resyn4(input_path, output_path):
    command = construct_command(
        [
            read_command(input_path),
            strash_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            write_command(output_path),
        ]
    )
    return run_abc_command(command)


def resyn5(input_path, output_path):
    command = construct_command(
        [
            read_command(input_path),
            strash_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            write_command(output_path),
        ]
    )
    return run_abc_command(command)


def resyn6(input_path, output_path):
    command = construct_command(
        [
            read_command(input_path),
            strash_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            write_command(output_path),
        ]
    )
    return run_abc_command(command)


def resyn7(input_path, output_path):
    command = construct_command(
        [
            read_command(input_path),
            strash_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            resyn_command(),
            write_command(output_path),
        ]
    )
    return run_abc_command(command)


def main():
    # List to store results
    results = []

    if not os.path.exists(SIMP_DIRECTORY):
        os.makedirs(SIMP_DIRECTORY)

    cnt = 0

    # Get total number of files for tqdm progress bar
    total_files = sum([len(files) for _, _, files in os.walk(BENCHMARKS_DIRECTORY)])

    for root, dirs, files in os.walk(BENCHMARKS_DIRECTORY):
        for file in tqdm(files, total=total_files, desc="Processing Benchmarks"):

            cnt += 1

            if os.path.splitext(file)[1] != ".bench":
                print(f"Skipping {file}")
                continue

            origin_path = os.path.join(root, file)

            # Original size
            original_size = circuit_size(origin_path)

            # Placeholder for each step
            resyn_times = {}
            resyn_sizes = {}
            # resyn_functions = [resyn, resyn2, resyn3, resyn4, resyn5, resyn6]
            resyn_functions = [resyn7]

            for i, resyn_function in enumerate(resyn_functions, start=1):
                i = 7
                resyn_dir = SIMP_DIRECTORY + os.sep + f'resyn{i}'
                if not os.path.exists(resyn_dir):
                    os.makedirs(resyn_dir)

                resyn_path = os.path.join(resyn_dir, file)

                if not os.path.isfile(resyn_path):
                    time_taken = resyn_function(origin_path, resyn_path)
                    size_after_resyn = circuit_size(resyn_path)
                else:
                    time_taken = None
                    size_after_resyn = circuit_size(resyn_path)

                resyn_times[f'Resyn{i} Time'] = time_taken
                resyn_sizes[f'Resyn{i} Size'] = size_after_resyn
                # origin_path = resyn_path  # Update for next resyn step

            # Adding results to list
            results.append(
                {
                    'Benchmark': file,
                    'Original Size': original_size,
                    **resyn_times,
                    **resyn_sizes,
                }
            )

    # Convert results list to DataFrame
    results_df = pd.DataFrame(results)

    # Save the DataFrame to a CSV file
    results_df.to_csv("resyn7_results.csv", index=False)
    print("Results saved to resyn_results.csv")


if __name__ == "__main__":
    main()
