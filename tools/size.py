import argparse
import os

import pandas as pd


# Define your function
def circuit_size(filename):
    size = 0
    try:
        with open(filename, 'r') as file:
            for line in file:
                if "AND" in line:
                    size += 1
    except FileNotFoundError:
        return None  # In case the file is not found
    return size


# Define the function to iterate over a folder and collect circuit sizes
def iterate_over_circuits(folder_path):
    data = []

    # Loop through each file in the folder
    for filename in os.listdir(folder_path):
        file_path = os.path.join(folder_path, filename)

        # Check if it's a file (not a directory)
        if os.path.isfile(file_path):
            size = circuit_size(file_path)
            data.append({"circuit_name": filename, "size": size})

    # Create a DataFrame from the data
    df = pd.DataFrame(data)
    return df


# Define the main function that parses command-line arguments
def main():
    # Set up argument parsing
    parser = argparse.ArgumentParser(
        description='Calculate circuit sizes and save to a CSV file.'
    )
    parser.add_argument(
        'folder', type=str, help='Path to the folder containing circuit files.'
    )
    parser.add_argument('output', type=str, help='Name of the output CSV file.')

    # Parse the arguments
    args = parser.parse_args()

    # Get the DataFrame
    df = iterate_over_circuits(args.folder)

    # Save the DataFrame to the specified output file
    df.to_csv(args.output, index=False)
    print(f"Results saved to {args.output}")


# Run the main function when this script is executed
if __name__ == '__main__':
    main()
