/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <errno.h>

using std::ifstream;
using std::string;
using std::stringstream;
using std::vector;

using LineHandler = std::function<void(string&)>;

#define SPACE           " "

#define SPACES          "    "
#define SPACES_2        "        "
#define SPACES_3        "            "
#define SPACES_4        "                "
#define SPACES_5        "                    "

#define NEW_LINE        "\n"

#ifdef _WIN32
#define PATH_DELIM_CHAR '\\'
#else
#define PATH_DELIM_CHAR '/'
#endif

#define MAX_LINE_SIZE   1024

#define AIDL_PATH_PARAM         "--aidl-path"
#define PROTO_PATH_PARAM        "--proto-path"
#define GEN_PATH_PARAM          "--gen-path"
#define GEN_HAL_STATUS_PARAM    "--gen-hal-status"
#define MAKEFILE_TYPE_PARAM     "--makefile-type"
#define REMOVE_FILE_PARAM       "--remove-file"
#define PACKAGE_NAME            "--package-name"

bool is_line_start_with(const string& line, const string& str);

bool is_line_end_with(const string& line, const string& str);

bool contain_char(const string& str, char c);

bool contain_string(const string& str, const string& substr);
bool contain_string(const vector<string>& str, const string& s);

bool contain_prefix(const string& str, const string& prefix);

bool contain_suffix(const string& str, const string& suffix);

const string& get_string(const vector<string>& str, const string& suffix);

bool is_empty_string(const string& str);

bool split_string(const string& str, char delim, vector<string>& result, bool clear = true);

bool search_keyword(const string& line, const string& key, string& value);

bool exist_path(const string& path);

bool create_dir(const string& path);

bool create_dir(const string& srcPath, const char *relativePath, string& dstPath);

bool create_dir(const string& srcPath, const string& relativePath);

bool create_new_file(const string& path, const string& fileName, const stringstream& ss);

bool create_new_file(const string& fileName, const stringstream& ss);

bool create_file(const string& fileName, const stringstream& ss);

bool create_file(const string& fileName, const string& str);

void delete_file(const string& fileName);

bool exist_file(const string& fileName);

bool exist_file(const string& path, const string& fileName);

void convert_first_char_to_upper(string& str);

void convert_first_char_to_lower(string& str);

string concat_string(const string& str1, const string& str2);

string concat_string(const string& str1, char delim, const string& str2);

string concat_string(const string& prefix, const string& str, const string& suffix);

void replace_string(string& str, const string& key, const string& value, bool all = true);

string update_string(const string& str, const string& key, const string& value);

string new_string(const string& str, char delim, const string& new_delim);

string get_file_name_stem(const string& fileName, const string& suffixName);

string get_full_file_name(const string& path, const string& fileName);

string get_sub_string(const string& str, char delim);

string get_sub_string(const string& str, const string& pattern, char delim);

string get_sub_string(const string& str, const string& prefix);

string get_string(const string& str, const char *prefix, const char *pattern, const char *suffix);

string get_string(char delim = '"', const string& prefix = "", const string& suffix = "");

void copy_string(vector<string>& dst, const vector<string>& src);

bool is_same_string(const string& str1, const string& str2);

void trim(string& str);

void transform_string_to_upper(string& str);

void transform_string_to_lower(string& str);

string get_path(const string& parentPath, const string& relativePath);

string get_format_string_in_upper(const string& str, char delim = '_');

string get_format_string_in_lower(const string& str, char delim = '_');

string get_string_upper_letters(const string& str);

bool is_line_skipped(const string& line, bool& is_comment_begin_found);

void remove_comment_in_line(string& line);

bool open_file(const string& fileName, ifstream& fs);

string get_hex_string(uint16_t val);

string get_align_string(const string& str, int width = 30);

string get_std_include_string(const string& typeName);

string get_using_string(const char *ns = "std", const string& typeName = "");

void add_module_begin(const string& prefix, stringstream& ss);

void add_module_end(stringstream& ss);
void add_module_end(const string& prefix, stringstream& ss);

void add_def_val_line(const char* prefix, const string& defName, const string& valName, stringstream& ss);

string cpp_type_ref(const string& cppType);
string cpp_type_const_ref(const string& cppType);

string cpp_type_const(const string& cppType);

string cpp_type_ptr(const string& cppType);
string cpp_type_ref_ptr(const string& cppType);
string cpp_type_deref(const string& cppType);

string unused_param_name(const string& paramName);

string struct_ptr_field(const string& structPtrVal, const string& structFieldName);
string struct_field(const string& structVal, const string& structFieldName);

bool get_key_value(const string& line, const string& key, string& value, char delim = ';');

string get_pair_type(const string& keyType, const string& valueType);

string get_pair_type_def(const string& keyType, const string& valueType, const string& key, const string& value);

bool exist_string(const vector<string>& vec, const string& str);

void remove_comment(string& str);

uint32_t get_max_str_len(const vector<string>& str, uint32_t& maxLen);

bool valid_str(const string& str);

bool has_terminate_char(const string& str, char ch = ';');

bool is_decimal(const string& str);
bool get_decimal_val(const string& str, int32_t& val);
bool is_hex(const string& str);
bool get_hex_val(const string& str, int32_t& val);
bool get_number_val(const string& str, int32_t& val);

void dump_string(const char* info, const vector<string>& vec);
