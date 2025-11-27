# Read from unique_lines.txt and write to input.nmea
with open('unique_lines.txt', 'r') as infile, open('input.nmea', 'w') as outfile:
    for line in infile:
        outfile.write(line.strip() + '\n')

print("input.nmea has been created successfully.")
