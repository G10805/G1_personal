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
    PRINT(("setprop [property][value]"));
}

int argParse(int argc, char **argv)
{
    if(argc !=3){
        PRINT(("Invalid format!"));
        return false;
    }
    g_key = string(argv[1]);
    g_value = string(argv[2]);
    if (  (g_key == "")
       || (g_value == "")
       || (g_key.size()>MAX_PROPERTY_KEY_SIZE)
       || (g_value.size()>MAX_PROPERTY_VALUE_SIZE)){
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
    bool not_set = true;


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
    while ( not_set && (pItem != NULL)){
        string strfileName = string(pItem->d_name);
        string strFullFileName = "";
        string strTempFullFileName = "";

        pItem = readdir(pDir);

        if((strfileName == string("."))|| (strfileName == string(".."))){
            continue;
        }
        if(strfileName.find(".conf") == string::npos){
            continue;
        }

        strFullFileName = string(PROPERTY_FILE_PATH) + string("/") + strfileName;
        strTempFullFileName = string(PROPERTY_FILE_PATH) + string("/.tmp_") + strfileName;


        ifstream prop_file(strFullFileName);
        ofstream prop_file_tmp(strTempFullFileName);

        if(!prop_file){
            continue;
        }

        if(prop_file_tmp){
            string line;
            while (getline(prop_file, line)){
                string key, value, orig_line;
                orig_line = line;

                PROP_DEBUG(("%s: line:%s", __func__, line.c_str()));
                if(parse_key_value(line, key, value) && not_set){
                    if(g_key == key){
                        PROP_DEBUG(("%s: key:%s, value:%s", __func__, key.c_str(), value.c_str()));
                        prop_file_tmp<< g_key + string(" = ") + g_value<<std::endl;
                        not_set = false;
                    }else{
                        prop_file_tmp<<orig_line<<std::endl;
                    }
                }else{
                    prop_file_tmp<<orig_line<<std::endl;
                }
            }
            if(not_set){
                prop_file_tmp<< g_key + string(" = ") + g_value<<std::endl;
                not_set = false;
            }
            prop_file.close();
            prop_file_tmp.close();
            if (remove(strFullFileName.c_str())!=-1){
                rename(strTempFullFileName.c_str(), strFullFileName.c_str());
            } else {
                PRINT(("Can't remove old property file."));
                return -1;
            }
            break;
        }else{
            continue;
        }
    }
    if (not_set){
        return -1;
    }
    return 0;
}

