#include "Utils.h"
#include <log/log.h>
#include <fstream>
#include <sstream>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

std::vector<std::unordered_map<std::string, std::string>> retrieveEventIdForEachDisplay(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        ALOGE("Utils: Failed to open file: %s", path.c_str());
        return {};
    }

    std::string line;
    std::vector<std::unordered_map<std::string, std::string>> final;
    std::unordered_map<std::string, std::string> data;

    // Skip the first line
    std::getline(file, line);

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Start of a new device block
        if (line.rfind("I:", 0) == 0 && !data.empty()) {
            final.push_back(data);
            data.clear();
        }

        // Remove prefix (e.g., "I:", "N:", etc.)
        size_t prefixEnd = line.find(':');
        if (prefixEnd != std::string::npos) {
            line = line.substr(prefixEnd + 1);
        }

        // Parse key=value pairs
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            size_t equalPos = token.find('=');
            if (equalPos != std::string::npos) {
                std::string key = token.substr(0, equalPos);
                std::string value = token.substr(equalPos + 1);
                if (!value.empty() && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.size() - 2);
                }
                data[key] = value;
            }
        }
    }

    if (!data.empty()) {
        final.push_back(data);
    }

    file.close();
    return final;
}

}