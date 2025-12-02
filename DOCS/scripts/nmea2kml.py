def nmea_to_degrees(nmea_value):
    parts = nmea_value.split('.')
    degrees = int(parts[0][:-2])
    minutes = float(parts[0][-2:] + '.' + parts[1])
    return degrees + minutes / 60

def convert_nmea_to_kml(nmea_file_path, kml_file_path):
    kml_content = (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<kml xmlns="http://www.opengis.net/kml/2.2">\n'
        '<Document>\n'
    )

    with open(nmea_file_path, 'r') as nmea_file:
        for line in nmea_file:
            if line.startswith('$GNGGA'):
                parts = line.split(',')
                lat = nmea_to_degrees(parts[2])
                lon = nmea_to_degrees(parts[4])
                
                placemark = (
                    f'<Placemark>\n'
                    f'  <name></name>\n'
                    f'  <description>{line}</description>\n'
                    f'  <Point>\n'
                    f'    <coordinates>{lon},{lat}</coordinates>\n'
                    f'  </Point>\n'
                    f'</Placemark>\n'
                )
                
                kml_content += placemark
    
    kml_content += '</Document>\n</kml>'

    with open(kml_file_path, 'w') as kml_file:
        kml_file.write(kml_content)

# Replace these with your file paths
nmea_file_path = 'input.nmea'
kml_file_path = 'output.kml'

convert_nmea_to_kml(nmea_file_path, kml_file_path)
