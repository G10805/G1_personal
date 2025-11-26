/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */

#include <stdlib.h>

#include "common_util.h"

void Vector2String(const vector<uint8_t>& v, string& s)
{
    s = string(v.begin(), v.end());
}

void String2Vector(const string& s, vector<uint8_t>& v)
{
    uint8_t *data = (uint8_t *) s.c_str();
    size_t length = s.size();
    v = vector<uint8_t>(data, data + length);
}

string* Int2String(int value)
{
    return new string(std::to_string(value));
}

int String2Int(void *s)
{
    string *str = static_cast<string *> (s);
    return atoi(str->c_str());
}

string* Bool2String(bool value)
{
    return Int2String(value ? 1 : 0);
}

bool String2Bool(void *s)
{
    return String2Int(s) != 0 ? true : false;
}