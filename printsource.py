#! /usr/bin/python
import os

# Define the directories to search
#directories = ['include', 'src']
directories = ['examples']

# Define the file extensions to include
extensions = {'.c', '.cpp', '.h', '.hpp'}

def iterate_and_print_files(directories, extensions):
    for directory in directories:
        if not os.path.isdir(directory):
            print(f"Directory '{directory}' does not exist.")
            continue

        for root, _, files in os.walk(directory):
            for file in files:
                file_ext = os.path.splitext(file)[1].lower()
                if file_ext in extensions:
                    file_path = os.path.join(root, file)
                    print(f"\n=== File: {file_path} ===\n")
                    try:
                        with open(file_path, 'r', encoding='utf-8') as f:
                            content = f.read()
                            print(content)
                    except Exception as e:
                        print(f"Error reading {file_path}: {e}")

if __name__ == "__main__":
    iterate_and_print_files(directories, extensions)
