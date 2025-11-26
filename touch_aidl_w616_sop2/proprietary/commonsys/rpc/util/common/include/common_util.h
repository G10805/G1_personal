/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <string>
#include <vector>

using std::array;
using std::string;
using std::vector;

/* Push int32_t input to vector, output must be vector<uint8_t> */
#define ADD_INT32_TO_VECTOR(input, output) \
    (output).push_back((uint8_t)((input) & 0xff)); \
    (output).push_back((uint8_t)(((input) >> 8) & 0xff)); \
    (output).push_back((uint8_t)(((input) >> 16) & 0xff)); \
    (output).push_back((uint8_t)(((input) >> 24) & 0xff));

/*
 * Get int32_t value from vector<uint8_t>.
 * The input must be already checked with correct length.
 * This macro is used by example:
 * int32_t result = CONVERT_INT32_FROM_VECTOR(vector, offset);
 */
#define CONVERT_INT32_FROM_VECTOR(input, offset) \
    ((int32_t)((input)[offset])) | \
    (((int32_t)((input)[offset + 1])) << 8) | \
    (((int32_t)((input)[offset + 2])) << 16) | \
    (((int32_t)((input)[offset + 3])) << 24)

void Vector2String(const vector<uint8_t>& v, string& s);

void String2Vector(const string& s, vector<uint8_t>& v);

string* Int2String(int value);

int String2Int(void *s);

string* Bool2String(bool value);

bool String2Bool(void *s);

template<size_t N>
void Array2String(const array<uint8_t, N>& a, string& s)
{
    s = string(a.begin(), a.end());
}

template<size_t N>
void String2Array(const string& s, array<uint8_t, N>& a)
{
    assert(s.size() == N);
    std::copy(s.begin(), s.end(), a.data());
}

template<size_t N>
void Array2Vector(const array<uint8_t, N>& a, vector<uint8_t>& v)
{
    v.assign(a.begin(), a.end());
}

template<size_t N>
void Vector2Array(const vector<uint8_t>& v, array<uint8_t, N>& a)
{
    assert(v.size() == N);
    std::copy(v.begin(), v.end(), a.data());
}
