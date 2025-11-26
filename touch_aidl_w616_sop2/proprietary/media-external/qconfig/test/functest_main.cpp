/*!
 * @file functest_main.cpp
 *
 * @cr
 * Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services
 */
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "QConfigClient.h"

using namespace std;
using namespace ::aidl::vendor::qti::hardware::qconfig;

static void printHelp() {
    cout << "******QConfig Function test tool******" << endl;
    cout << "Options:" << endl;
    cout << "    -h: print help message" << endl;
    cout << "    -a: run API: 0:setUserConfigs; 1:getUserConfigValue;" << endl;
    cout << "                 2:clearUserConfig; 3:getPresets" << endl;
    cout << "    -m: the module's name" << endl;
    cout << "    -k: the key's name" << endl;
    cout << "    -v: the value's name" << endl;
    cout << "    -p: the preset id's name" <<endl;
    cout << endl;
    cout << "Example:" << endl;
    cout << "0. query the interested package name's uid: adb shell ps -o uid,NAME" << endl;
    cout << "**********************************" << endl;
    cout << "1. setUserConfigs: " << endl;
    cout << "   adb shell qconfigfunctest -a 0 -m QModKey_Static_VppFilter -k 1013 -v aie" << endl;
    cout << "2. getUserConfigValue: " << endl;
    cout << "   adb shell qconfigfunctest -a 1 -m QModKey_Static_VppFilter -k 1013" << endl;
    cout << "3. clearUserConfig: " << endl;
    cout << "   adb shell qconfigfunctest -a 2 -m QModKey_Static_VppFilter -k 1013" << endl;
    cout << "4. getPresets: " << endl;
    cout << "   adb shell qconfigfunctest -a 3 -m QModKey_Static_VppFilter -p aie" << endl;
    cout << "**********************************" << endl;
}

int main(int argc, char **argv)
{
    const char* optstr = "a:m:k:v:p:h";
    int c;
    int api = -1;
    string moduleName;
    string key;
    string value;
    string presetId;

    while((c=getopt(argc, argv, optstr))!=-1)
    {
        switch(c) {
            case 'h':
                printHelp();
                break;
            case 'a':
                api = atoi(optarg);
                break;
            case 'm':
                moduleName = string(optarg);
                break;
            case 'k':
                key = string(optarg);
                break;
            case 'v':
                value = string(optarg);
                break;
            case 'p':
                presetId = string(optarg);
                break;
            default:
                break;
        }
    }
    if (api < 0) {
        cout << "Invalide API" << endl;
        return 0;
    }
    bool ret = false;
    QConfigClient client(true);

    switch(api) {
        case 0: {
            cout << "setUserConfigs: m-" << moduleName << "-k-" << key << "-v-"<< value << endl;
            ret = client.setUserConfigs(moduleName, {{key, value}});
            cout << "result: " << (ret ? "success" : "fail") << endl;
            break;
        }
        case 1: {
            cout << "getUserConfigValue: m-" << moduleName << "-k-" << key << endl;
            string retValue;
            ret = client.getUserConfigValue(moduleName, key, &retValue);
            cout << "result: " << (ret ? "success" : "fail") << " -value: " << retValue << endl;
            break;
        }
        case 2: {
            cout << "clearUserConfig: m-" << moduleName << "-k-" << key << endl;
            ret = client.clearUserConfig(moduleName, key);
            cout << "result: " << (ret ? "success" : "fail") << endl;
            break;
        }
        case 3: {
            unordered_map<string, string> clientPresets;
            cout << "getPresets: m-" << moduleName << "-p-" << presetId << endl;
            ret = client.getPresets(moduleName, presetId, &clientPresets);
            cout << "result: " << (ret ? "success" : "fail") << endl;
            for (const auto& item : clientPresets) {
                cout << "preset name: " << item.first << " value: " << item.second << endl;
            }
            break;
        }
        default:
            break;
    }
    return 0;
}
