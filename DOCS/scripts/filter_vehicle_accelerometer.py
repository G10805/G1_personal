import re

input_file = "values_file.txt"
output_file = "extracted_with_timestamps.txt"

# Regex patterns
timestamp_pattern = re.compile(r"\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}")
vehicle_speed_pattern = re.compile(r"vehicle_speed\s*[:=]\s*(-?\d+\.\d+)")
accel_x_pattern = re.compile(r"accelerometer\s*[-_]?\s*x\s*[:=]\s*(-?\d+\.\d+)")
accel_y_pattern = re.compile(r"accelerometer\s*[-_]?\s*y\s*[:=]\s*(-?\d+\.\d+)")
accel_z_pattern = re.compile(r"accelerometer\s*[-_]?\s*z\s*[:=]\s*(-?\d+\.\d+)")

with open(input_file, "r", encoding="utf-8", errors="ignore") as infile, \
     open(output_file, "w", encoding="utf-8") as outfile:

    for line in infile:
        timestamp = timestamp_pattern.search(line)
        vehicle_speed = vehicle_speed_pattern.search(line.lower())
        accel_x = accel_x_pattern.search(line.lower())
        accel_y = accel_y_pattern.search(line.lower())
        accel_z = accel_z_pattern.search(line.lower())

        if timestamp or vehicle_speed or accel_x or accel_y or accel_z:
            values = [
                f"timestamp: {timestamp.group()}" if timestamp else "",
                f"vehicle_speed: {vehicle_speed.group(1)}" if vehicle_speed else "",
                f"accel_x: {accel_x.group(1)}" if accel_x else "",
                f"accel_y: {accel_y.group(1)}" if accel_y else "",
                f"accel_z: {accel_z.group(1)}" if accel_z else ""
            ]
            outfile.write(", ".join([v for v in values if v]) + "\n")

print(f"✅ Data with timestamps saved to '{output_file}'")
