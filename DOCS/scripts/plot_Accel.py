import pandas as pd
import re
import matplotlib.pyplot as plt

data = []
pattern = re.compile(r'data:\s*\[(?:[^,]*,){3}([-\d.]+),([-\d.]+),([-\d.]+),')

with open('sensor_accelero_raw_data.txt', 'r') as f:
    for line in f:
        match = pattern.search(line)
        if match:
            x, y, z = map(float, match.groups())
            data.append({'x': x, 'y': y, 'z': z})

# Create DataFrame
accel_df = pd.DataFrame(data)
accel_df.reset_index(inplace=True)
accel_df.rename(columns={'index': 'sample'}, inplace=True)

# Save to CSV
accel_df.to_csv('accel_xyz.csv', index=False)

# Plot all three axes
plt.figure(figsize=(12, 6))
plt.plot(accel_df['sample'], accel_df['x'], label='X')
plt.plot(accel_df['sample'], accel_df['y'], label='Y')
plt.plot(accel_df['sample'], accel_df['z'], label='Z')
plt.title('Accelerometer Data (Extracted Values)')
plt.xlabel('Sample Index')
plt.ylabel('Accelerometer Value (m/s²)')
plt.legend()
plt.tight_layout()
plt.savefig('accel_xyz_plot.png')
#plt.show()
plt.savefig("plot.png")