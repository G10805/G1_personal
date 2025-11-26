/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include "properties.h"
using namespace std;

string g_key ="";
string g_value = "";
extern bool parse_key_value(string& line, string& key, string& value);

void printHelp()
{
    PRINT(("USAGE:"));
    PRINT(("getprop [property]"));
}

int argParse(int argc, char **argv)
{
    if(argc !=2){
        PRINT(("Invalid format!"));
        return false;
    }
    g_key = string(argv[1]);
    if (g_key == ""){
        PRINT(("Invalid parameter!"));
        return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    DIR *pDir = NULL;
    struct dirent *pItem = NULL;
    char new_value[PROPERTY_VALUE_MAX] = {0};

    if (!argParse(argc, argv)) {
        printHelp();
        return -1;
    }
    pDir = opendir(PROPERTY_FILE_PATH);
    if(pDir == NULL){
        PRINT(("Can't open the property folder."));
        return -1;
    }

    pItem = readdir(pDir);
    while (pItem != NULL){
        string strfileName = string(pItem->d_name);
        string strFullFileName = "";

        pItem = readdir(pDir);

        if((strfileName == string("."))|| (strfileName == string(".."))){
            continue;
        }
        if(strfileName.find(".conf") == string::npos){
            continue;
        }

        strFullFileName = string(PROPERTY_FILE_PATH) + string("/") + strfileName;
        ifstream prop_file(strFullFileName);

        if(prop_file){
            string line;
            while (getline(prop_file, line)){
                string key, value;
                if(parse_key_value(line, key, value)){
                    PROP_DEBUG(("%s: key:%s, value:%s", __func__, key.c_str(), value.c_str()));
                    if(g_key == key){
                        PRINT(("%s", value.c_str()));
                        return 0;
                    }else{
                        PROP_DEBUG(("%s: line:%s, key:%s, value:%s", __func__, line.c_str(), key.c_str(), value.c_str()));
                    }
                }else{
                    PROP_DEBUG(("%s: line:%s", __func__, line.c_str()));
                }
            }
            prop_file.close();
        }
        else
        {
            PRINT(("Can't open the property file."));
        }
    }
    PRINT(("Can't find the property."));
    return -1;
}

