#include "GnssSimulate.h"
#include <fstream>
#include <regex>
#include <sstream>
#include <thread>
#include <chrono>
#include <cmath>
#include <cstring>
#include <utils/SystemClock.h>
#include <log/log.h>

// Helper: Parse KML and extract coordinates (decimal degrees)
std::vector<std::pair<double, double>> GnssSimulate::parseKmlCoordinates(const std::string& kmlPath) {
    std::vector<std::pair<double, double>> coords;
    std::ifstream file(kmlPath);
    if (!file.is_open()) return coords;
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::smatch match;
    std::regex coordTag("<coordinates>([^<]+)</coordinates>");
    if (std::regex_search(content, match, coordTag)) {
        std::string coordStr = match[1];
        std::istringstream iss(coordStr);
        std::string token;
        while (std::getline(iss, token, ' ')) {
            if (token.empty()) continue;
            std::istringstream pairStream(token);
            std::string lon, lat;
            if (std::getline(pairStream, lon, ',') && std::getline(pairStream, lat, ',')) {
                try {
                    coords.emplace_back(std::stod(lat), std::stod(lon));
                } catch (...) {}
            }
        }
    }
    return coords;
}

// Converts decimal degrees to NMEA degrees-minutes (ddmm.mmmmm)
// 12.971598; deg = 12; min = 58.2958
static void decimalToNmeaDM(double decimal, int& deg, double& min) {
    deg = static_cast<int>(std::abs(decimal));
    min = (std::abs(decimal) - deg) * 60.0;
}

static unsigned char calculateChecksum(const std::string& sentence) {
    unsigned char checksum = 0;
    for (size_t i = 1; i < sentence.length() && sentence[i] != '*'; ++i) {
        checksum ^= sentence[i];
    }
    return checksum;
}

// Generate valid RMC NMEA frame using NMEA degrees-minutes
std::string GnssSimulate::generateRMC(double lat, double lon) {
    int latDeg, lonDeg;
    double latMin, lonMin;
    decimalToNmeaDM(lat, latDeg, latMin);
    decimalToNmeaDM(lon, lonDeg, lonMin);
    char latHem = (lat >= 0) ? 'N' : 'S';
    char lonHem = (lon >= 0) ? 'E' : 'W';

    char buffer[128];
    snprintf(buffer, sizeof(buffer),
        "$GNRMC,120000.00,A,%02d%09.6f,%c,%03d%09.6f,%c,2.0,195.0,290725,,,A",
        latDeg, latMin, latHem, lonDeg, lonMin, lonHem);

    std::string sentence(buffer);
    unsigned char checksum = calculateChecksum(sentence);

    char fullSentence[512];
    snprintf(fullSentence, sizeof(fullSentence), "%s*%02X\r\n", sentence.c_str(), checksum);
    return std::string(fullSentence);
}

// Generate valid GGA NMEA frame using NMEA degrees-minutes
std::string GnssSimulate::generateGGA(double lat, double lon) {
    int latDeg, lonDeg;
    double latMin, lonMin;
    decimalToNmeaDM(lat, latDeg, latMin);
    decimalToNmeaDM(lon, lonDeg, lonMin);
    char latHem = (lat >= 0) ? 'N' : 'S';
    char lonHem = (lon >= 0) ? 'E' : 'W';

    char buffer[128];
    snprintf(buffer, sizeof(buffer),
        "$GNGGA,120000.00,%02d%09.6f,%c,%03d%09.6f,%c,1,08,1.0,10.0,M,0.0,M,,",
        latDeg, latMin, latHem, lonDeg, lonMin, lonHem);

    std::string sentence(buffer);
    unsigned char checksum = calculateChecksum(sentence);
    
    char fullSentence[512];
    snprintf(fullSentence, sizeof(fullSentence), "%s*%02X\r\n", sentence.c_str(), checksum);
    return std::string(fullSentence);
}

//Generate NFATT frame
std::string GnssSimulate::generateNFATT() {
    std::string utc="123519.000";
    float roll=0.5f;
    float pitch=1.2f;
    float yaw=44.9f;

    char buffer[128];
    snprintf(buffer, sizeof(buffer),
            "$NFATT,%s,%.1f,%.1f,%.1f",
            utc.c_str(),roll,pitch,yaw);
 
    std::string sentence(buffer);
    unsigned char checksum = calculateChecksum(sentence);

    char fullSentence[512];
    snprintf(fullSentence, sizeof(fullSentence), "%s*%02X\r\n", sentence.c_str(), checksum);
    return std::string(fullSentence);
}

// Fill simulated satellite status
void GnssSimulate::fillSimulatedSvStatus(PreGnssSvStatus* svStatusBlock) {
    svStatusBlock->numSvs = 8;
    for (int i = 0; i < 8; ++i) {
        svStatusBlock->gnssSvList[i].svid = 1 + i;
        svStatusBlock->gnssSvList[i].constellation = 1; // GPS
        if (i % 2 == 0) {
            svStatusBlock->gnssSvList[i].cN0Dbhz = 35.0 + i;
        } else {
            svStatusBlock->gnssSvList[i].cN0Dbhz = 40.0 + i;
        }
        svStatusBlock->gnssSvList[i].elevationDegrees = 45.0;
        svStatusBlock->gnssSvList[i].azimuthDegrees = 90.0;
        svStatusBlock->gnssSvList[i].carrierFrequencyHz = 1;
        svStatusBlock->gnssSvList[i].svFlag = 0x07; // Example: all flags set
        svStatusBlock->gnssSvList[i].basebandCN0DbHz = 30.0 + i;
    }
}

// Fill simulated location
void GnssSimulate::fillSimulatedLocation(PreGnssLocation* location, double lat, double lon) {
    location->flags = 0xFF;
    location->latitudeDegrees = lat;
    location->longitudeDegrees = lon;
    location->altitudeMeters = 10.0;
    location->speedMetersPerSec = 13.0;
    location->bearingDegrees = 195.0;
    location->horizontalAccuracyMeters = 5.0;
    location->verticalAccuracyMeters = 8.0;
    location->speedAccuracyMetersPerSecond = 0.5;
    location->bearingAccuracyDegrees = 1.0;
    location->timestamp = ::android::uptimeMillis(); // Should be set by caller
    location->bestLocation = 1;
    location->elapsedTime.flags = 1;
    location->elapsedTime.timestampNs = location->timestamp * 1000000;
    location->elapsedTime.timeUncertaintyNs = 1000000;
}

// Main simulation loop
void GnssSimulate::simulateFromKml(
    const std::string& kmlPath,
    void (*reportNMEA)(const NavNmea*),
    void (*reportSvStatus)(const PreGnssSvStatus*),
    void (*reportLocation)(const PreGnssLocation*))
{
    auto coords = parseKmlCoordinates(kmlPath);
    for (const auto& [lat, lon] : coords) {
        // Generate and report RMC
        std::string rmc = generateRMC(lat, lon);
        NavNmea nmeaBlockRmc = {};
        nmeaBlockRmc.timestamp = ::android::uptimeMillis();
        strncpy(nmeaBlockRmc.nmea, rmc.c_str(), sizeof(nmeaBlockRmc.nmea) - 1);
        reportNMEA(&nmeaBlockRmc);

        // Generate and report GGA
        std::string gga = generateGGA(lat, lon);
        NavNmea nmeaBlockGga = {};
        nmeaBlockGga.timestamp = ::android::uptimeMillis();
        strncpy(nmeaBlockGga.nmea, gga.c_str(), sizeof(nmeaBlockGga.nmea) - 1);
        reportNMEA(&nmeaBlockGga);

        // Simulate and report satellite status
        PreGnssSvStatus svStatus = {};
        fillSimulatedSvStatus(&svStatus);
        reportSvStatus(&svStatus);

        // Simulate and report location
        PreGnssLocation location = {};
        fillSimulatedLocation(&location, lat, lon);
        //Android uptimemillis
        location.timestamp = ::android::uptimeMillis();
        location.elapsedTime.flags = 1; // Set flags as needed
        location.elapsedTime.timestampNs = location.timestamp * 1000000;
        reportLocation(&location);

	// Generate and report NFATT - in actual, populate only after calib completed
        std::string att = generateNFATT();
        NavNmea nmeaBlockNFATT = {};
        nmeaBlockNFATT.timestamp = ::android::uptimeMillis();
        strncpy(nmeaBlockNFATT.nmea, att.c_str(), sizeof(nmeaBlockNFATT.nmea) - 1);
        reportNMEA(&nmeaBlockNFATT); //callback

        // Sleep 1 second to simulate real-time updates
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
