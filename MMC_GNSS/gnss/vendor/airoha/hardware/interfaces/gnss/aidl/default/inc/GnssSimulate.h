#pragma once

#include <string>
#include <vector>
#include <utility>
#include "airo_gps.h"

#include <android/log.h>

using namespace Airoha::Gnss;

namespace GnssSimulate {

// Parse KML and extract coordinates (lat, lon in decimal degrees)
std::vector<std::pair<double, double>> parseKmlCoordinates(const std::string& kmlPath);

// Generate valid RMC NMEA frame using NMEA degrees-minutes
std::string generateRMC(double lat, double lon);

// Generate valid GGA NMEA frame using NMEA degrees-minutes
std::string generateGGA(double lat, double lon);

// Generate NFATT frame
std::string generateNFATT();

// Fill simulated satellite status
void fillSimulatedSvStatus(PreGnssSvStatus* svStatusBlock);

// Fill simulated location
void fillSimulatedLocation(PreGnssLocation* location, double lat, double lon);

// Main simulation loop
void simulateFromKml(
    const std::string& kmlPath,
    void (*reportNMEA)(const NavNmea*),
    void (*reportSvStatus)(const PreGnssSvStatus*),
    void (*reportLocation)(const PreGnssLocation*));

} // namespace GnssSimulate
