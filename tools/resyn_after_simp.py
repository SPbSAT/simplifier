import pandas as pd
import subprocess
import shutil
import time
import os
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
    """
    Constructs one command line from several commands, separating them with ;
    """
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
            text=True
        )

        out, err = process.communicate(command, timeout=TL)

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


def resyn_iter(input_path, output_path, iterations):
    command_list = [read_command(input_path), strash_command()] + [resyn_command()] * iterations + [write_command(output_path)]
    command = construct_command(command_list)
    return run_abc_command(command)


def main():
    # List to store results
    results = []

    # Check if SIMP_DIRECTORY exists
    if not os.path.exists(SIMP_DIRECTORY):
        os.makedirs(SIMP_DIRECTORY)

    # Get all subdirectories with '_simp' suffix
    simp_folders = [folder for folder in os.listdir(SIMP_DIRECTORY) if folder.endswith("_simp")]

    for simp_folder in simp_folders:
        simp_folder_path = os.path.join(SIMP_DIRECTORY, simp_folder)

        # Get total number of files for tqdm progress bar
        total_files = sum([len(files) for _, _, files in os.walk(simp_folder_path)])

        for root, _, files in os.walk(simp_folder_path):
            for file in tqdm(files, total=total_files, desc=f"Processing {simp_folder}"):
                if os.path.splitext(file)[1] != ".bench":
                    print(f"Skipping {file}")
                    continue

                origin_path = os.path.join(root, file)
                original_size = circuit_size(origin_path)

                # Initialize a dictionary to store results for this benchmark
                benchmark_result = {
                    'Benchmark': file,
                    'Original Folder': simp_folder,
                    'Original Size': original_size,
                }

                for iteration in [1, 2]:
                    # Define new output directory
                    new_folder_name = f"{simp_folder}_resyn{iteration}"
                    new_folder_path = os.path.join(SIMP_DIRECTORY, new_folder_name)

                    # Create the new directory if it doesn't exist
                    if not os.path.exists(new_folder_path):
                        os.makedirs(new_folder_path)

                    output_path = os.path.join(new_folder_path, file)

                    time_taken = resyn_iter(origin_path, output_path, iteration)
                    size_after_resyn = circuit_size(output_path)

                    # Add results for the current iteration to the benchmark_result dictionary
                    benchmark_result[f'Resyn{iteration} Time'] = time_taken
                    benchmark_result[f'Resyn{iteration} Size'] = size_after_resyn

                # Append the completed benchmark result to the results list
                results.append(benchmark_result)

    # Convert results list to DataFrame
    results_df = pd.DataFrame(results)

    # Save the DataFrame to a CSV file
    results_df.to_csv("resyn_simp_results.csv", index=False)
    print("Results saved to resyn_simp_results.csv")


if __name__ == "__main__":
    main()
