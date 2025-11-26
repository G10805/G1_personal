/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
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
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <pthread.h>
#include <sys/msg.h>
#include "properties.h"
using namespace std;

#define MSG_TYPE_PROP         (222)
#define PROPERTY_WRITE        (0x01)

static pthread_t msgpid;
static int msgQid;
static int on_app_exit = 0;

struct msg_t{
    long type;
    uint16_t operation;
    uint16_t key_size;
    uint8_t key[MAX_PROPERTY_KEY_SIZE+1];
    uint16_t value_size;
    uint8_t value[MAX_PROPERTY_VALUE_SIZE+1];
};


#define ENABLE_REALTIME_WRITE   (1)

#define COMMENT_CHAR            '#'
#define ASSIGNMENT_OP_CHAR      '='

std::unordered_map<std::string, std::string> properties_map;

static bool property_msg_init(void);
static void property_write(string key, string value);
static void property_msg_send(string key, string value);

char char_to_upper(char c)
{
    return std::toupper(c);
}

static string& trim(string& raw)
{
    raw.erase(0, raw.find_first_not_of(" \t\n"));
    return raw.erase(raw.find_last_not_of(" \t\n") + 1);
}

static string to_upper(string& str)
{
    string new_str = str;
    transform(new_str.begin(), new_str.end(), new_str.begin(), char_to_upper);
    return new_str;
}

static bool is_comment(string& line)
{
    string::size_type pos = string::npos;

    pos = line.find_first_not_of(" \t\n");
    if (pos == string::npos || line[pos] == COMMENT_CHAR){
        return true;
    }
    return false;
}

bool parse_key_value(string& line, string& key, string& value)
{
    string::size_type pos = string::npos;
    string tmp_key, tmp_value;

    if (is_comment(line)){
        return false;
    }

    pos = line.find(ASSIGNMENT_OP_CHAR);
    if (pos == string::npos)
    {
        return false;
    }

    tmp_key = line.substr(0, pos);
    tmp_value = line.substr(pos+1);

    key = trim(tmp_key);
    value = trim(tmp_value);

    return true;
}

void property_init(void)
{
    string filename = string(PROPERTY_FILE_PATH) + string("/") + string(PROPERTY_FILE_NAME);

    ifstream prop_file(filename);
    if(prop_file){
        string line;
        PROP_DEBUG(("%s: line:%s", __func__, line.c_str()));
        while (getline(prop_file, line)){
            string key, value;
            if(parse_key_value(line, key, value))
            {
                PROP_DEBUG( ("%s: key:%s, value:%s", __func__, key.c_str(), value.c_str()));
                std::pair<string, string> prop(key, value);
                properties_map.insert(prop);
            }
        }
        prop_file.close();
    }else{
        cerr<<__func__<< ": property file not exist:"<<filename<<endl;
        return;
    }
    property_msg_init();
    return;
}

bool property_get(const char * key, char *value, const char * default_value)
{
    std::unordered_map<string, string> :: const_iterator find = properties_map.find(key);

    PROP_DEBUG(("%s: key:%s", __func__, key));
    if (find == properties_map.end()){
        strlcpy(value, default_value, PROPERTY_VALUE_MAX);
        PROP_DEBUG( ("%s:property not exist:%s", __func__, key));
        return false;
    }
    PROP_DEBUG(("%s: key:%s, value:%s", __func__, key, find->second.c_str()));
    strlcpy(value, find->second.c_str(), PROPERTY_VALUE_MAX);
    return true;
}

bool property_set(const char * key, const char * value)
{
    std::unordered_map<string, string> :: const_iterator find;
    string _skey = string(key);
    string _svalue = string(value);

    string skey = trim(_skey);
    string svalue = trim(_svalue);

    if(key == NULL || value == NULL){
        return false;
    }
    if(skey.size()> MAX_PROPERTY_KEY_SIZE|| svalue.size()> MAX_PROPERTY_VALUE_SIZE){
        return false;
    }
    find = properties_map.find(key);

    PROP_DEBUG( ("%s: key:%s, value:%s", __func__, key, value));
    if (find == properties_map.end()){
        PROP_DEBUG( ("%s:property not exist:%s", __func__, key));
        std::pair<string, string> prop(skey, svalue);
        properties_map.insert(prop);
        return false;
    }
    properties_map.at(skey) = svalue;
    property_msg_send(skey, svalue);
    return true;
}

int32_t property_get_int32(const char * key, int32_t default_value)
{
    char value[PROPERTY_VALUE_MAX]={0};
    string str_default_value = std::to_string(default_value);
    property_get(key, value, str_default_value.c_str());
    return atoi(value);
}

bool property_get_bool(const char * key, bool default_value)
{
    std::unordered_map<string, string> :: const_iterator find;
    string svalue;
    if(key == NULL){
        return default_value;
    }

    find = properties_map.find(key);
    PROP_DEBUG( ("%s: key:%s", __func__, key));
    if (find == properties_map.end()){
        return default_value;
    }
    svalue = find->second;
    if(to_upper(svalue) == "TRUE"){
        return true;
    }
    return false;
}

static void property_write(string key, string value)
{
    string filename = string(PROPERTY_FILE_PATH) + string("/") + string(PROPERTY_FILE_NAME);
    string filename_tmp = string(PROPERTY_FILE_PATH) + string("/.tmp_") + string(PROPERTY_FILE_NAME);
    bool not_existed = false;

    ifstream prop_file(filename);
    ofstream prop_file_tmp(filename_tmp);
    if(prop_file && prop_file_tmp){
        string line;
        while (getline(prop_file, line)){
            string tmp_key, tmp_value, orig_line;
            orig_line = line;
            if(parse_key_value(line, tmp_key, tmp_value))
            {
                PROP_DEBUG( ("%s: line:%s, key:%s, value:%s", __func__, line.c_str(), key.c_str(), value.c_str()));
                if(key == tmp_key){
                    prop_file_tmp<< key + string(" = ") + value<<std::endl;
                    not_existed = true;
                }else{
                    prop_file_tmp<<orig_line<<std::endl;
                }
            }
            else
            {
                prop_file_tmp<<orig_line<<std::endl;
            }
        }
        if (!not_existed){
            prop_file_tmp<< key + string(" = ") + value<<std::endl;
            not_existed = true;
        }
        prop_file.close();
        prop_file_tmp.close();
    }
    else
    {
        cerr<< "Can't open property file to save."<<endl;
        return;
    }
    if (remove(filename.c_str())!=-1){
        rename(filename_tmp.c_str(), filename.c_str());
    }
    else
    {
        cerr<< "Can't remove old property file."<<endl;
        return;
    }
}

void property_exit(void)
{
    on_app_exit = 1;
}

void *property_work_thread(void *arg)
{
    int msq_id = *(int*)arg;
    while (!on_app_exit){
        msg_t msg;
        int err;

        err = msgrcv(msq_id, &msg, sizeof(msg_t)-sizeof(long), MSG_TYPE_PROP, 0);
        if(err == -1)
        {
            usleep(500000);
            continue;
        }
        switch (msg.operation)
        {
            case PROPERTY_WRITE:
            {
                property_write(string((char*)msg.key), string((char *)msg.value));
            }
            break;
        }
    }
    return NULL;
}

static bool property_msg_init(void)
{
   key_t msg_key = -1;
   int err = 0;
   msg_key = ftok("/vendor/prop/msg/", 'b');
   if(msg_key<0){
       return false;
   }

   msgQid = msgget(msg_key, IPC_CREAT|0666);
   if(msgQid<0){
       return false;
   }
   err = pthread_create(&msgpid, NULL, property_work_thread, &msgQid);
   if (err){
       return false;
   }
   return true;
}

static void property_msg_send(string key, string value)
{
    msg_t msg;
    int err;

    if(!key.size()){
        return;
    }
    msg.type = MSG_TYPE_PROP;
    msg.operation = PROPERTY_WRITE;

    memcpy(msg.key, (uint8_t *)key.c_str(), key.size());
    msg.key_size = key.size();
    msg.key[msg.key_size] = '\0';

    if(value.size()){
        memcpy(msg.value, (uint8_t *)value.c_str(), value.size());
        msg.value_size = value.size();
        msg.value[msg.value_size] = '\0';
    }else{
        msg.value_size = 0;
    }

    err = msgsnd(msgQid, &msg, sizeof(msg_t)-sizeof(long), 0);
}

