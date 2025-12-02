import os
import gzip
import shutil

def extract_gz_files_recursive(folder_path):
    for root, dirs, files in os.walk(folder_path):
        for filename in files:
            if filename.endswith(".gz"):
                gz_path = os.path.join(root, filename)
                output_path = os.path.join(root, filename[:-3])  # Remove .gz extension

                try:
                    with gzip.open(gz_path, 'rb') as f_in:
                        with open(output_path, 'wb') as f_out:
                            shutil.copyfileobj(f_in, f_out)
                    print("Extracted: {} to {}".format(gz_path, output_path))
                except IOError:
                    print("Skipped (not a valid gzip file): {}".format(gz_path))

if __name__ == "__main__":
    folder_name = raw_input("Enter the folder path: ").strip()  # Use input() if Python 3
    if os.path.isdir(folder_name):
        extract_gz_files_recursive(folder_name)
    else:
        print("Invalid folder path. Please check and try again.")
