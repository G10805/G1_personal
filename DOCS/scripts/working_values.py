import os

def extract_sensor_data(input_folder, output_file):
    keys = ["vehicle_speed", "Accelerometer_x", "Accelerometer_y", "Accelerometer_z"]

    with open(output_file, "w") as out_f:
        for root, dirs, files in os.walk(input_folder):
            for file in files:
                if file.startswith("main_"):
                    file_path = os.path.join(root, file)
                    try:
                        with open(file_path, "r") as in_f:
                            for line in in_f:
                                for key in keys:
                                    if key in line:
                                        out_f.write(line)
                                        break  # Avoid writing the same line multiple times
                    except Exception as e:
                        print(f"Error reading {file_path}: {e}")

# Example usage
input_folder = "vstgloballogcontrol_2025_10_7_18_11_7"  # Replace with your actual folder path
output_file = "filtered_output.txt"
extract_sensor_data(input_folder, output_file)

print(f"Filtered data has been written to {output_file}.")