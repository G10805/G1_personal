/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <string>
#include <vector>

using std::string;
using std::vector;


typedef struct
{
    int32_t status;
    string info;
} HalStatusParam;

bool VendorSerializeHalStatus(int32_t status, const string& info, vector<uint8_t>& payload);

bool VendorParseHalStatus(const uint8_t* data, size_t length, HalStatusParam& param);