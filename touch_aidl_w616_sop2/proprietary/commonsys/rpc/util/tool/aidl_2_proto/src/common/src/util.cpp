/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <string.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#ifndef EXCLUDE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "log.h"
#include "util.h"

using std::endl;
using std::ifstream;
using std::istringstream;
using std::ofstream;
using std::size_t;
using std::stringstream;

#ifndef EXCLUDE_EXPERIMENTAL_FILESYSTEM
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

static bool is_comment_begin(const string& line)
{
    return is_line_start_with(line, "/*");
}

static bool is_comment_end(const string& line)
{
    return is_line_end_with(line, "*/");
}

static bool is_constant_definition(const string& line)
{
    return contain_string(line, "const") &&
        contain_string(line, "=") &&
        contain_string(line, ";") &&
        !contain_string(line, "(") &&
        !contain_string(line, ")");
}

bool is_line_start_with(const string& line, const string& str)
{
    int strLen = str.length();
    int lineLen = line.length();
    if (lineLen < strLen)
        return false;

    return line.substr(0, strLen) == str;
}

bool is_line_end_with(const string& line, const string& str)
{
    int strLen = str.length();
    int lineLen = line.length();
    if (lineLen < strLen)
        return false;

    return line.substr(lineLen - strLen, strLen) == str;
}

bool contain_char(const string& str, char c)
{
    return str.find(c) != string::npos;
}

bool contain_string(const string& str, const string& substr)
{
    return str.find(substr) != string::npos;
}

bool contain_string(const vector<string>& str, const string& s)
{
    for (auto it: str)
    {
        if (is_same_string(it, s))
            return true;
    }
    return false;
}

bool contain_prefix(const string& str, const string& prefix)
{
    return str.find(prefix) == 0;
}

bool contain_suffix(const string& str, const string& suffix)
{
    size_t pos = str.rfind(suffix);
    return (pos != string::npos) && (pos + suffix.length() == str.length());
}

const string& get_string(const vector<string>& str, const string& suffix)
{
    const static string sEmptyString = "";
    for (vector<string>::size_type index = 0; index < str.size(); index++)
    {
        if (contain_string(str[index], suffix))
            return str[index];
    }
    return sEmptyString;
}

bool is_empty_string(const string& str)
{
    return str.empty() || (str.find_first_not_of(" \n") == string::npos);
}

bool split_string(const string& str, char delim, vector<string>& result, bool clear)
{
    istringstream iss(str);
    string token;

    if (clear)
        result.clear();

    while (getline(iss, token, delim))
    {
        result.push_back(token);
    }
    return !result.empty() ;
}

bool search_keyword(const string& line, const string& key, string& value)
{
    string str = line;
    vector<string> result;
    char delim = ' '; // divide string based on space char

    if (!split_string(str, delim, result))
        return false;

    if (result.size() < 2)
        return false;

    if (key == result[0])
    {
        /* find keyword */
        value = result[1];
        return true;
    }
    else
    {
        return false;
    }
}

bool exist_path(const string& path)
{
    std::error_code error;
    fs::path dir_path(path);
    auto file_status = fs::status(dir_path, error);
    return !error && fs::exists(file_status);
}

bool create_dir(const string& path)
{
    if (exist_path(path))
        return true;

    fs::path dir_path(path);
    try
    {
        fs::create_directory(dir_path);
    }
    catch (fs::filesystem_error& e)
    {
        loge("%s: fail to create dst path, err: %d (%s)", __func__, errno, strerror(errno));
        return false;
    }
    return true;
}

bool create_dir(const string& srcPath, const char *relativePath, string& dstPath)
{
    stringstream ss;

    if (relativePath != NULL)
    {
        logd("%s: src path: %s, relative path: %s", __func__, srcPath.c_str(), relativePath);
        ss << srcPath << relativePath;
    }
    else
    {
        logd("%s: src path: %s", __func__, srcPath.c_str());
        ss << srcPath;
    }
    dstPath = ss.str();
    return create_dir(dstPath);
}

bool create_dir(const string& srcPath, const string& relativePath)
{
    string path;
    size_t pos = 0;

    if (relativePath.empty())
        return create_dir(srcPath);

    do
    {
        pos = relativePath.find_first_of(PATH_DELIM_CHAR, pos);
        if (pos == string::npos)
        {
            path = get_path(srcPath, relativePath);
            if (!create_dir(path))
                return false;

            break;
        }
        else
        {
            if (pos != 0)
            {
                path = get_path(srcPath, relativePath.substr(0, pos));
                if (!create_dir(path))
                    return false;                
            }
            if (pos < relativePath.length())
            {
                ++pos;   
            }
            else
            {
                path = get_path(srcPath, relativePath.substr(0, pos));
                if (!create_dir(path))
                    return false;               

                break;
            }
        }
    }
    while (1);
    return true;
}

bool create_new_file(const string& path, const string& fileName, const stringstream& ss)
{
    return create_new_file(get_full_file_name(path, fileName), ss);
}

bool create_new_file(const string& fileName, const stringstream& ss)
{
    /* delete old file */
    delete_file(fileName);
    return create_file(fileName, ss);
}

bool create_file(const string& fileName, const stringstream& ss)
{
    return create_file(fileName, ss.str());
}

bool create_file(const string& fileName, const string& str)
{
    ofstream protoFile;
    // logd("%s: file name: %s", __func__, fileName.c_str());
    protoFile.open(fileName);
    protoFile << str;
    protoFile.close();
    return true;
}

void delete_file(const string& fileName)
{
    fs::path p(fileName);
    if (fs::exists(p))
    {
        // logd("%s: remove file: %s", __func__, fileName.c_str());
        fs::remove(p);
    }
}

bool exist_file(const string& fileName)
{
    fs::path p(fileName);
    return fs::exists(p);
}

bool exist_file(const string& path, const string& fileName)
{
    return exist_file(get_full_file_name(path, fileName));
}

void convert_first_char_to_upper(string& str)
{
    char ch;

    if (is_empty_string(str))
        return;

    ch = str.at(0);
    if (isupper(ch))
        return;

    ch = (char) toupper(ch);

    /* e.g. "hal" -> "Hal" */
    str.replace(0, 1, 1, ch);
}

void convert_first_char_to_lower(string& str)
{
    char ch;

    if (is_empty_string(str))
        return;

    ch = str.at(0);
    if (islower(ch))
        return;

    ch = (char) tolower(ch);

    /* e.g. "Hal" -> "hal" */
    str.replace(0, 1, 1, ch);

}

string concat_string(const string& str1, const string& str2)
{
    stringstream ss;
    ss << str1 << str2;
    return ss.str();
}

string concat_string(const string& str1, char delim, const string& str2)
{
    stringstream ss;
    ss << str1 << delim << str2;
    return ss.str();
}

string concat_string(const string& prefix, const string& str, const string& suffix)
{
    stringstream ss;
    ss << prefix << str << suffix;
    return ss.str();
}

void replace_string(string& str, const string& key, const string& value, bool all)
{
    size_t pos = 0;

    if (is_empty_string(str) ||
        is_empty_string(key))
        return;

    do
    {
        pos = str.find(key, pos);

        if (pos == string::npos)
            break;

        str.replace(pos, key.length(), value);

        if (!all)
            break;

        pos += value.length();
    }
    while (1);
}

string update_string(const string& str, const string& key, const string& value)
{
    string new_str = str;
    replace_string(new_str, key, value, false);
    return new_str;
}

string new_string(const string& str, char delim, const string& new_delim)
{
    vector<string> result;
    stringstream ss;

    if (!split_string(str, delim, result))
        return str;

    /* e.g. "android.hardware.hal" + "." + "/" -> "android/hardware/hal" */
    ss << result[0];
    for (vector<string>::size_type index = 1; index < result.size(); index++)
    {
        ss << new_delim << result[index];
    }
    return ss.str();
}

string get_file_name_stem(const string& fileName, const string& suffixName)
{
    /* e.g. "IHal.aidl", ".aidl" -> "IHal" */
    return fileName.substr(0, fileName.find(suffixName));
}

string get_full_file_name(const string& path, const string& fileName)
{
    return get_path(path, fileName);
}

string get_sub_string(const string& str, char delim)
{
    size_t begin_pos = str.find_first_of(delim);
    if (begin_pos != string::npos)
    {
        size_t end_pos = str.find_last_of(delim);
        if (end_pos != string::npos)
        {
            /* e.g. "IHal.proto" -> IHal.proto */
            return str.substr(begin_pos + 1, end_pos - begin_pos - 1);
        }
    }
    return str;
}

string get_sub_string(const string& str, const string& pattern, char delim)
{
    string sub_str = str;;
    size_t pos = str.find(pattern);
    /* e.g. str: "android.hardware.hal.IHal", pattern: "android.hardware.hal", delim: '.' => "IHal" */
    if (pos != string::npos)
    {
        pos = str.find_first_not_of(delim, pos + pattern.length());
        sub_str = str.substr(pos, string::npos);
    }
    return sub_str;
}

string get_sub_string(const string& str, const string& prefix)
{
    return (str.find(prefix) == 0) ? str.substr(prefix.length(), string::npos) : "";
}

string get_string(const string& str, const char *prefix, const char *pattern, const char *suffix)
{
    string sub_str = str;
    string prefix_str(prefix);
    string pattern_str(pattern);
    string suffix_str(suffix);

    size_t pos = str.find(prefix_str);

    /* e.g. str: "IHalEventCallback", prefix_str: "I" => "HalEventCallback" */
    if (pos != string::npos)
    {
        sub_str = str.substr(prefix_str.length(), string::npos);
        /* e.g. "HalEventCallback", suffix_str: "Callback" => "HalEvent" */
        pos = sub_str.find(suffix_str);
        if (pos != string::npos)
        {
            sub_str = sub_str.substr(0, pos);
        }
        /* e.g. "HalEvent", pattern: "Event" ==> "Hal" */
        pos = sub_str.find(pattern_str);
        if (pos != string::npos)
        {
            sub_str = sub_str.substr(0, pos);
        }
    }
    return sub_str;
}

string get_string(char delim, const string& prefix, const string& suffix)
{
    stringstream ss;
    ss << delim << prefix << suffix << delim;
    return ss.str();
}

void copy_string(vector<string>& dst, const vector<string>& src)
{
    for (auto it = src.begin() ; it != src.end(); ++it)
    {
        dst.push_back(*it);
    }
}

bool is_same_string(const string& str1, const string& str2)
{
    return !str1.compare(str2);
}

void trim(string& str)
{
    if (str.empty())
        return;

    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
    /* erase last char '\r'*/
    if (!str.empty())
    {
        size_t end_pos = str.length() - 1;
        if (str[end_pos] == '\r')
            str.erase(end_pos, 1);
    }
}

void transform_string_to_upper(string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void transform_string_to_lower(string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
}

string get_path(const string& parentPath, const string& relativePath)
{
    stringstream ss;
    ss << parentPath << PATH_DELIM_CHAR << relativePath;
    return ss.str();
}

string get_format_string_in_upper(const string& str, char delim)
{
    string new_str = get_format_string_in_lower(str, delim);
    transform_string_to_upper(new_str);
    return new_str;
}

string get_format_string_in_lower(const string& str, char delim)
{
    vector<string> v;
    stringstream ss;

    if (is_empty_string(str))
    {
        return "";
    }

    for (size_t index = 0; index < str.length(); index++)
    {
        char ch = str[index];
        if (isupper(ch))
        {
            ch = (char) tolower(ch);
            if (!ss.str().empty())
            {
                v.push_back(ss.str());
                ss.clear();
                ss.str("");
            }
        }
        ss << ch;
    }
    if (!ss.str().empty())
    {
        v.push_back(ss.str());
    }

    ss.clear();
    ss.str("");

    ss << v[0];

    size_t size = v.size();
    if (size >= 2)
    {
        for (vector<string>::size_type index = 1; index < size; index++)
        {
            ss << delim << v[index];
        }
    }
    /* e.g. "HalMod" -> "hal_mod" */
    return ss.str();
}

string get_string_upper_letters(const string& str)
{
    stringstream ss;
    if (str.empty())
    {
        return "";
    }
    for (size_t i = 0; i < str.length(); i ++)
    {
        char ch = str[i];
        if ((ch >= 'A') && (ch <= 'Z'))
        {
            ss << ch;
        }
    }
    return ss.str();
}

/* empty line, constant line and comment line shall be skipped */
bool is_line_skipped(const string& line, bool& is_comment_begin_found)
{
    if (is_empty_string(line))
        return true;

    /* const value is not needed by protobuf */
    if (is_constant_definition(line))
        return true;

    if (!is_comment_begin_found)
    {
        if (is_comment_begin(line))
        {
            is_comment_begin_found = true;
            if (is_comment_end(line))
            {
                is_comment_begin_found = false;
            }
            return true;
        }
    }
    else
    {
        if (is_comment_end(line))
        {
            is_comment_begin_found = false;
        }
        return true;
    }
    return false;
}

void remove_comment_in_line(string& line)
{
    int comment_start_index = line.find("/*");
    int comment_end_index = line.find("*/");
    int comment_length = comment_end_index - comment_start_index + 2;
    line = line.replace(comment_start_index, comment_length, "");
}

bool open_file(const string& fileName, ifstream& fs)
{
    fs.open(fileName.c_str(), std::ios::in | std::ios::binary);
    if (!fs.is_open())
    {
        loge("%s: fail to open file %s, err: %d (%s)", __func__, fileName.c_str(), errno, strerror(errno));
        return false;
    }
    return true;
}

string get_hex_string(uint16_t val)
{
    stringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << val;
    return ss.str();
}

string get_align_string(const string& str, int width)
{
    stringstream ss;
    ss << std::setw(width) << std::setiosflags(std::ios::left) << str;
    return ss.str();    
}

string get_std_include_string(const string& typeName)
{
    stringstream ss;
    /* e.g. "#include <vector>" */
    ss << "#include " << '<' << typeName << '>';
    return ss.str();
}

string get_using_string(const char *ns, const string& typeName)
{
    stringstream ss;
    /* e.g. "using std::vector;" */
    ss << "using " << ns << "::" << typeName << ';';
    return ss.str();
}

void add_module_begin(const string& prefix, stringstream& ss)
{
    ss << prefix << '{' << endl;
}

void add_module_end(stringstream& ss)
{
    add_module_end("", ss);
}

void add_module_end(const string& prefix, stringstream& ss)
{
    ss << prefix << '}' << endl;
}

void add_def_val_line(const char* prefix, const string& defName, const string& valName, stringstream& ss)
{
    ss << prefix << defName << " = " << valName << ';' << endl;
}

string cpp_type_ref(const string& cppType)
{
    stringstream ss;
    ss << cppType << "&";
    return ss.str();
}

string cpp_type_const_ref(const string& cppType)
{
    stringstream ss;
    ss << "const" << " " << cppType << "&";
    return ss.str();
}

string cpp_type_const(const string& cppType)
{
    stringstream ss;
    ss << "const" << " " << cppType;
    return ss.str();
}

string cpp_type_ptr(const string& cppType)
{
    stringstream ss;
    ss << cppType << "*";
    return ss.str();
}

string cpp_type_ref_ptr(const string& cppType)
{
    stringstream ss;
    ss << "&" << cppType;
    return ss.str();
}

string cpp_type_deref(const string& cppType)
{
    stringstream ss;
    ss << "*" << cppType;
    return ss.str();
}

string unused_param_name(const string& paramName)
{
    stringstream ss;
    ss << "/* " << paramName << " */";
    return ss.str();
}

string struct_ptr_field(const string& structPtrVal, const string& structFieldName)
{
    stringstream ss;
    ss << structPtrVal << "->" << structFieldName;
    return ss.str();
}

string struct_field(const string& structVal, const string& structFieldName)
{
    stringstream ss;
    ss << structVal << "." << structFieldName;
    return ss.str();
}

bool get_key_value(const string& line, const string& key, string& value, char delim)
{
    string str;
    vector<string> result;
    size_t length = 0;

    /* e.g. line: "package android.hardware.hal;"  key: "package" */
    if (!search_keyword(line, key, str))
        return false;

    if (contain_char(str, delim))
    {
        /* e.g. "android.hardware.hal;" -> "android.hardware.hal" */
        if (!split_string(str, delim, result))
            return false;

        value = result[0];
        return true;
    }

    length = str.length();
    /* exclude suffix of new line */
    if (str[length-1] == 0x0D)
    {
        length -= 1;
    }
    value = str.substr(0, length);
    return true;
}

string get_pair_type(const string& keyType, const string& valueType)
{
    stringstream ss;
    /* e.g. pair<T, V> */
    ss << "pair" << '<' << keyType << ", " << valueType << '>';
    return ss.str();
}

string get_pair_type_def(const string& keyType, const string& valueType, const string& key, const string& value)
{
    stringstream ss;
    /* e.g. pair<T, V>(k, v) */
    ss << get_pair_type(keyType, valueType) << '(' << key << ", " << value << ')';
    return ss.str();
}

bool exist_string(const vector<string>& vec, const string& str)
{
    for (vector<string>::size_type index = 0; index < vec.size(); index++)
    {
        if (contain_suffix(vec[index], str))
            return true;
    }
    return false;
}

void remove_comment(string& str)
{
    size_t begin_pos = str.find_first_of("/*");
    size_t end_pos = str.find_last_of("*/");
    if ((begin_pos != string::npos) &&
        (end_pos != string::npos))
    {
        // logd("%s: + str: %s, len: %d", __func__, str.c_str(), str.length());
        str.erase(begin_pos, end_pos - begin_pos + 1);
        // logd("%s: - str: %s, len: %d", __func__, str.c_str(), str.length());
    }
}

uint32_t get_max_str_len(const vector<string>& str, uint32_t& maxLen)
{
    if (!str.empty())
    {
        for (auto it: str)
        {
            if (it.length() > maxLen)
                maxLen = it.length();
        }
    }
    return maxLen;
}

bool valid_str(const string& str)
{
    return !is_empty_string(str);
}

bool has_terminate_char(const string& str, char ch)
{
    return !str.empty() && (*(str.rbegin()) == ch);
}

bool is_decimal(const string& str)
{
    /* in case it's a negative number */
    std::size_t pos = str[0] == '-' ? 1 : 0;
    return str.find_first_not_of("0123456789", pos) == string::npos;
}

bool get_decimal_val(const string& str, int32_t& val)
{
    if (is_decimal(str))
    {
        val = std::stoi(str, nullptr, 10);
        return true;
    }
    return false;
}

bool is_hex(const string& str)
{
    size_t pos = str.find("0x");
    if (pos != string::npos)
    {
        string val = str.substr(pos + 2);
        return !val.empty() &&
            (val.find_first_not_of("0123456789abcdefABCDEF") == string::npos);
    }
    return false;
}

bool get_hex_val(const string& str, int32_t& val)
{
    if (is_hex(str))
    {
        val = std::stoi(str, nullptr, 16);
        return true;
    }
    return false;
}

bool get_number_val(const string& str, int32_t& val)
{
    return get_decimal_val(str, val) ||
        get_hex_val(str, val);
}

void dump_string(const char* info, const vector<string>& vec)
{
    logd("%s: info: %s, vec size: %d", __func__, info, vec.size());
    for (vector<string>::size_type index = 0; index < vec.size(); index++)
    {
        logd("%s: (%d/%d): %s", __func__, index + 1, vec.size(), vec[index].c_str());
    }
}
