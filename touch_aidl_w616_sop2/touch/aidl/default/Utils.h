#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

std::vector<std::unordered_map<std::string, std::string>> retrieveEventIdForEachDisplay(const std::string& path);

}