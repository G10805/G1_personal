#include "nmea_regenerator.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

vector<string> split(string str, char delimiter)
{
    // Using str in a string stream
    stringstream ss(str);
    vector<string> res;
    string token;
    while (getline(ss, token, delimiter)) {
        res.push_back(token);
    }
    return res;
}

void nmeaCheckSum(std::string& nmeaString)
{
    uint32_t j = 0;
    int32_t checksum = 0;
    std::stringstream nmea_stream;

    for (j = 0; j < nmeaString.length(); j++) {
        if (nmeaString.at(j) == '*') {
            break;
        }
        if (nmeaString.at(j) != '$') {
            checksum = checksum ^ nmeaString.at(j);
        }
    }

    //making string without characters followed by *, to append calculated checksum at end
    nmeaString = nmeaString.substr(0, j+1);
    nmea_stream << std::uppercase << std::setfill('0') <<  std::setw(2) << std::right << std::hex << checksum;
    nmeaString += nmea_stream.str();
}

void nmea_regenerator_gga(char nmea_cstring[])
{
    string str_nmea(nmea_cstring);
    vector<string> res = split(str_nmea, ',');

    //res[1] - UTC timestamp
    //res[2] - Latitude
    //res[4] - Longitude
    //res[7] - number of satellites used

    if (!res[1].empty())
    {
        res[1] = res[1].substr(0, res[1].size() - 1);

        if (!res[2].empty() && !res[4].empty()) {
            res[2].push_back('0');
            res[4].push_back('0');
        }

        if (res[7].length() == 1) {
            res[7].insert(0, "0");
        }

        std::stringstream nmea_ss;
        for (size_t i = 0; i < res.size(); i++) {
            if (i != 0) {
                nmea_ss << ",";
            }
            nmea_ss << res[i];
        }
        string nmea = nmea_ss.str();
        //calcuate checksum
        nmeaCheckSum(nmea); 
        strcpy(nmea_cstring, nmea.c_str());
    } else {
        //Nothing to do
    }
}


void nmea_regenerator_rmc(char nmea_cstring[])
{
    string str_nmea(nmea_cstring);
    vector<string> res = split(str_nmea, ',');

    //res[1] - UTC timestamp
    //res[3] - Latitude
    //res[5] - Longitude

    if (!res[1].empty()) {
        res[1] = res[1].substr(0, res[1].size() - 1);

        if (!res[3].empty() && !res[5].empty()) {
            res[3].push_back('0');
            res[5].push_back('0');
        }

        std::stringstream nmea_ss;
        for (size_t i = 0; i < res.size(); i++) {
            if (i != 0) {
                nmea_ss << ",";
            }
            nmea_ss << res[i];
        }
        string nmea = nmea_ss.str();
        //calculate checksum
        nmeaCheckSum(nmea);
        strcpy(nmea_cstring, nmea.c_str());
    } else {
        //Nothing to do
    }
}

