#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""

from collections import deque
import json
import os
import random
import string
import sys
from typing import Tuple

from commondef import CommonDef

HELPER_COMPATIBLE_VERSION = (3, 9)

def check_aidl_interface(file_path: str, config: dict, common_def: CommonDef) -> bool:
    """
    Check whether a given AIDL file defines an interface.
    Also update AIDL related suffices.

    Args:
        file_path: Relative path of the AIDL file
        config: Configurations
        common_def: Common definitions

    Returns:
        True if this is an interface file.
    """
    interface_file = False
    with open(file_path, "r", encoding=config["encoding"]) as f:
        for line in f.readlines():
            if interface_file:
                break
            if "package" == line[:len("package")]:
                if "" == common_def.aidl_using_prefix:
                    package_name = line.strip().split(" ")[1].split(";")[0] + "."
                    aidl_using = package_name.replace(".", "::")
                    common_def.aidl_using_prefix = "aidl::{}".format(aidl_using)
                    common_def.aidl_include_prefix = \
                        common_def.aidl_using_prefix.replace("::", "/")
                continue
            for keyword in config["aidlInterfaceKeywords"]:
                if keyword == line[:len(keyword)]:
                    interface_file = True
                    break
    return interface_file

def get_pseudo_base_interface_name(interfaces: list, config: dict, common_def: CommonDef) -> str:
    """
    Deduce the root base interface name within a module.
    The returned name is used only when "BASE_INTF" is not set.
    e.g. Module wifi -> IWifi, bluetooth -> IBluetoothHci, wifi.supplicant -> ISupplicant

    Args:
        interfaces: A list containing all interfaces within the module
        config: Configurations

    Returns:
        Pseudo root base interface name.
    """
    i_list = []
    for i_name in interfaces:
        callback = False
        for keyword in config["callbackKeywords"]:
            if keyword in i_name:
                callback = True
                break
        if not callback:
            i_list.append(i_name)
    pesudo_root_name = "I{}".format(common_def.package_prefix.split(".")[-2].capitalize())
    i_list.sort()
    i_root_name = ""
    for each in i_list:
        if pesudo_root_name in each:
            i_root_name = each
    return i_root_name

def check_callback(interface_name: str, common_def: CommonDef) -> bool:
    """
    Check whether a given interface belongs to a callback.

    Args:
        interface_name: Interface to be checked
        common_def: Common definitions

    Returns:
        Whether a callback or not.
    """
    callback = False
    for elem in common_def.callback_keywords:
        if elem in interface_name:
            callback = True
    return callback

def check_interface_message(type_name: str, common_def: CommonDef) -> bool:
    """
    Check whether a given field type belongs to an interface.

    Args:
        type_name: Field type name
        common_def: Common definitions

    Returns:
        Whether an interface or not
    """
    interface = False
    if type_name.split(".")[-1] in common_def.interface_set:
        interface = True
    return interface

def deduce_type_name(field_type: str,
                     package_name: str, cached_type: str,
                     common_def: CommonDef) -> Tuple[str, str]:
    """
    Deduce the full type name.

    Args:
        field_type: Type name to be analyzed
        package_name: Upper level full name
        cached_type: A type cashed by previous type definition
        common_def: Common definitions

    Returns:
        A tuple of full type name and cached type.
    """
    f_type = ""
    if field_type not in common_def.PRIMITIVE_TYPES_WITH_BYTES:
        if "." not in field_type:
            if "" == cached_type:
                f_type = "{}.{}".format(package_name, field_type)
            else:
                f_type = cached_type
                cached_type = ""
        else:
            f_type = field_type
    else:
        f_type = field_type
    return (f_type, cached_type)

def sanitize_type_info(type_info: dict,
                       common_def: CommonDef) -> None:
    """
    Sanitize type fields with actual existing full package name.
    This method is to fix the issue caused by recording full name as type.

    Args:
        type_info: Class type dictionary
        common_def: Common definitions

    Returns:
        None
    """
    type_set = set(type_info.keys())
    for (type_name, type_value) in type_info.items():
        if CommonDef.CLASS == type_value["typeCategory"]:
            for field_info in type_value["fields"].values():
                type_recorded = field_info["type"]
                if type_recorded in common_def.PRIMITIVE_TYPES_WITH_BYTES:
                    continue
                else:
                    if type_recorded in type_set:
                        continue
                    else:
                        correct_type = type_recorded
                        # Trim the last sub-package since it does not actually exist
                        iteration_retry = 0
                        while True:
                            if common_def.SANITIZE_MAX_ITERATION < iteration_retry:
                                raise ValueError("Type info mismatch, exceeding max retry for \nfield_type: {}\nin type: {}"\
                                                 .format(type_recorded, type_name))
                            type_content = correct_type.split(".")
                            if 2 < len(type_content): 
                                correct_type = "{}.{}"\
                                               .format(".".join(type_content[:-2]),
                                                       type_content[-1])
                            else:
                                correct_type = "{}.{}"\
                                               .format("".join(type_content[:-2]),
                                                       type_content[-1])
                            iteration_retry += 1
                            if correct_type in type_set:
                                break
                        field_info.update({"type": correct_type})
        elif CommonDef.UNION == type_value["typeCategory"]:
            for (field_name, field_type) in type_value["fields"].items():
                type_recorded = field_type
                if type_recorded in common_def.PRIMITIVE_TYPES_WITH_BYTES:
                    continue
                else:
                    if type_recorded in type_set:
                        continue
                    else:
                        correct_type = type_recorded
                        iteration_retry = 0
                        while True:
                            if common_def.SANITIZE_MAX_ITERATION < iteration_retry:
                                raise ValueError("Type info mismatch, exceeding max retry for \nfield_type: {}\nin type: {}"\
                                                 .format(type_recorded, type_name))
                            type_content = correct_type.split(".")
                            if 2 < len(type_content):
                                correct_type = "{}.{}"\
                                               .format(".".join(type_content[:-2]),
                                                       type_content[-1])
                            else:
                                correct_type = "{}.{}"\
                                               .format("".join(type_content[:-2]),
                                                       type_content[-1])
                            iteration_retry += 1
                            if correct_type in type_set:
                                break
                        type_value["fields"].update({field_name: correct_type})

def sanitize_interface_params(type_info: dict,
                              interface_info: dict,
                              common_def: CommonDef) -> None:
    """
    Sanitize parameter types with actual existing full package name.
    This method is to fix the issue caused by re-ordering full name as method name.

    Args:
        type_info: Class type dictionary
        interface_info: Interface info dictionary
        common_def: Common definitions

    Returns:
        None
    """
    type_set = set(type_info.keys())
    interface_set = set(interface_info.keys())
    for i_info in interface_info.values():
        for (m_name, m_info) in i_info["methods"].items():
            p_names = list(m_info["params"].keys())
            p_infos = list(m_info["params"].values())
            delete_indices = []
            for index in range(len(p_names)):
                type_recorded = p_infos[index]["type"]
                if type_recorded in common_def.PRIMITIVE_TYPES_WITH_BYTES:
                    continue
                else:
                    if type_recorded in type_set:
                        continue
                    elif type_recorded in interface_set:
                        delete_indices.append(index)
                    else:
                        correct_type = type_recorded
                        # Trim the last sub-package since it does not actually exist
                        iteration_retry = 0
                        while True:
                            if common_def.SANITIZE_MAX_ITERATION < iteration_retry:
                                raise ValueError("Type info mismatch, exceeding max retry for type: {}\nin method: {}"\
                                                 .format(type_recorded, m_name))
                            type_content = correct_type.split(".")
                            if 2 < len(type_content):
                                correct_type = "{}.{}"\
                                                .format(".".join(type_content[:-2]),
                                                        type_content[-1])
                            else:
                                correct_type = "{}.{}"\
                                               .format("".join(type_content[:-2]),
                                                       type_content[-1])
                            iteration_retry += 1
                            if correct_type in type_set:
                                break
                            if correct_type in interface_set:
                                delete_indices.append(index)
                                break
                        p_infos[index].update({"type": correct_type})
            # Remove param field if is interface
            if 0 != len(delete_indices):
                for delete_index in delete_indices:
                    p_names[delete_index] = ""
                    p_infos[delete_index].clear()
            updated_info = {}
            for update_index in range(len(p_names)):
                if "" != p_names[update_index]:
                    updated_info.update({p_names[update_index]: p_infos[update_index]})
            m_info["params"].clear()
            m_info["params"].update(updated_info)

def check_used(v_name: str, v_type: str,
               stack: deque, used_set: set,
               top_level: bool) -> None:
    """
    Check whether this type is already processed.

    Args:
        v_name: Variable name
        v_type: Variable type
        stack: Proto variable type stack
        used_set: Set used for record
        top_level: Whether this parameter is a top-level one or sub-level contained one
        common_def: Common definitions

    Returns:
        None
    """
    if v_type not in used_set:
        used_set.add(v_type)
        param_entry = {"pName": v_name, "pType": v_type, "topLevel": top_level}
        # Push to last
        if top_level:
            stack.append(param_entry)
        # Push to first
        else:
            stack.appendleft(param_entry)

def get_common_interface_name(common_def: CommonDef, config: dict) -> str:
    """
    Get common interface name. Format is a concatenation of "PackageName, "Msg", "Common"".

    Args:
        common_def: Common definitions
        config: Configurations
    """
    interface_name = "{}{}{}{}"\
                     .format(common_def.package_prefix,
                             common_def.base_intf,
                             config["interfaceFileNameSuffix"].capitalize(),
                             "Common")
    return interface_name

def convert_const_upper(m_name: str) -> str:
    """
    Convert the given method name into an const style string,
    each word is separated by "_".

    Args:
        m_name: Method name

    Returns:
        Const style string.
    """
    str = list()
    for char in m_name:
        if "a" <= char <= "z":
            str.append(char.upper())
        elif "A" <= char <= "Z":
            str.append("_")
            str.append(char)
        else:
            str.append(char)
    return "".join(str)

def summary_type_name(p_type: str,
                      type_info: dict,
                      common_def: CommonDef,
                      config: dict) -> str:
    """
    Decide the correct type name

    Args:
        p_type: Type name
        type_info: Class type dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        CPP type name
    """
    cpp_name = ""
    package_list = p_type.split(".")
    if p_type == common_def.common_status_full_name:
        cpp_name = package_list[-1] + config["typedefSuffix"]
    elif type_info[p_type]["contained"]:
        cpp_name = "::".join(package_list[-2:])
    else:
        cpp_name = package_list[-1]
    return cpp_name

def get_param_short_name(i_base_name: str, prefix: str) -> str:
    """
    Get short name of the interface parameter.

    e.g. "Wifi" -> "", "WifiChip" -> "WifiChip", "WifiStaIface" -> "StaIface".

    Args:
        i_base_name: Interface base name
        prefix: Package prefix

    Returns:
        Short name for this interface.
    """
    minor_name = prefix.split(".")[-2].capitalize()

    name = ""

    if i_base_name.startswith(minor_name):
        name = i_base_name[len(minor_name):]
        index = 0
        cap_index = 0
        # Strip until the second capitalized section
        for char in name:
            if "A" <= char <= "Z":
                if 2 == cap_index:
                    name = name[:index]
                    break
                cap_index += 1
            index += 1
    return name

def deduce_repeated_type(field_dict: dict, mapped_type: str) -> str:
    """
    Decide the correct field type of this repeated field.
    Should be whether std::vector or std::array.

    Args:
        field_dict: Field detail dictionary
        mapped_type: C++ mapped type

    Returns:
        Type declaration string.
    """
    type_keyword = ""
    inner_keyword = ""
    if CommonDef.VECTOR == field_dict["aidlType"]:
        type_keyword = CommonDef.VECTOR
        inner_keyword = mapped_type
    elif CommonDef.ARRAY == field_dict["aidlType"]:
        type_keyword = CommonDef.ARRAY
        inner_keyword = "{}, {}".format(mapped_type, field_dict["length"])
    section = "{}<{}>".format(type_keyword, inner_keyword)
    return section

def deduce_bytes_type(field_dict: dict, config: dict) -> str:
    """
    Decide the correct field type of this bytes field.
    Should be whether std::vector or std::array.

    Args:
        field_dict: Field detail dictionary
        config: Configurations

    Returns:
        Type declaration string.
    """
    type_keyword = ""
    inner_keyword = ""
    if CommonDef.VECTOR == field_dict["aidlType"]:
        type_keyword = CommonDef.VECTOR
        inner_keyword = config["bytesType"]
    elif CommonDef.ARRAY == field_dict["aidlType"]:
        type_keyword = CommonDef.ARRAY
        inner_keyword = "{}, {}".format(config["bytesType"], field_dict["length"])
    section = "{}<{}>".format(type_keyword, inner_keyword)
    return section

def deduce_string_handling_name(field_dict: dict, config: dict,
                                handling: CommonDef.H_TYPE) -> str:
    """
    Decide the correct helper method name for this byte field.
    Should be whether Vector2String, Array2String or vice versa.

    Args:
        field_dict: Field info dictionary
        config: Configurations
        handling: Whether for serialization or de-serialization

    Returns:
        Correct helper method name.
    """
    keyword_dict = config["repeatedHandlingKeywords"][field_dict["aidlType"]]
    method_name = ""
    if CommonDef.H_TYPE.SERIALIZE == handling:
        method_name = keyword_dict["serialize"]
    elif CommonDef.H_TYPE.DE_SERIALIZE == handling:
        method_name = keyword_dict["parse"]
    return method_name

def fill_top_level_array(v_name: str, v_dict: dict,
                         type_info: dict, config: dict,
                         common_def: CommonDef, tab_level: str) -> str:
    """
    Initialization section to fill a top-level array.

    Args:
        v_name: Variable name
        v_dict: Variable info dictionary
        type_info: Type info dictionary
        config: Configuration dictionary
        common_def: Common definitions
        tab_level: Tab indention level

    Returns:
        Initialization section to fill a top-level array.
    """
    section = ""
    v_type = v_dict["type"]
    length = CommonDef.TEST_VECTOR_LENGTH
    if CommonDef.ARRAY == v_dict["aidlType"]:
        length = v_dict["length"]
    elif CommonDef.VECTOR == v_dict["aidlType"]:
        section += "{}{}.resize({});\n".format(tab_level, v_name, length)
    section += "{}for (int index = 0; index < {}; index++)\n{}{{\n"\
               .format(tab_level, length, tab_level)
    if v_type in common_def.PRIMITIVE_TYPES_WITH_BYTES:
        section += "{}{}[index] = {};\n"\
                   .format(tab_level + common_def.TAB, v_name, get_random(v_type))
    else:
        if CommonDef.ENUM == type_info[v_type]["typeCategory"]:
            field_type = ""
            if type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_type.split(".")[-1]
            section += "{}{}[index] = {}({});\n"\
                       .format(tab_level + common_def.TAB,
                               v_name, field_type, config["enumDefault"])
        elif CommonDef.CLASS == type_info[v_type]["typeCategory"] or \
             CommonDef.CLASS == type_info[v_type]["typeCategory"]:
            section += "{}{}({}[index]);\n"\
                       .format(tab_level + common_def.TAB,
                               get_test_helper_name(v_type), v_name)
    section += "{}}}\n".format(tab_level)
    return section

def fill_lower_level_array(v_name: str, v_dict: dict,
                           p_name: str, type_info: dict, config: dict,
                           common_def: CommonDef, tab_level: str) -> str:
    """
    Initialization section to fill a lower-level array.

    Args:
        v_name: Variable name
        v_dict: Variable info dictionary
        p_name: Parent variable name
        type_info: Type info dictionary
        config: Configuration dictionary
        common_def: Common definitions
        tab_level: Tab indention level

    Returns:
        Initialization section to fill a lower-level array.
    """
    section = ""
    v_type = v_dict["type"]
    length = CommonDef.TEST_VECTOR_LENGTH
    optional_prepend = ""
    optional_append = ""
    optional = False
    resize_line = ""
    if CommonDef.ARRAY == v_dict["aidlType"]:
        length = v_dict["length"]
    elif CommonDef.VECTOR == v_dict["aidlType"]:
        resize_line = "{}{}.{}.resize({});\n"\
                      .format(tab_level, p_name, v_name, length)
    section += "NULLABLE_PREPEND{}{}for (int index = 0; index < {}; index++)\n{}{{\n"\
               .format(resize_line, tab_level, length, tab_level)
    if v_type in CommonDef.PRIMITIVE_TYPES:
        section += "{}{}.{}[index] = {};\n"\
                   .format(tab_level + common_def.TAB,
                           p_name, v_name, get_random(v_type))
    elif "bytes" == v_type:
        if v_dict["nullable"]:
            optional = True
            optional_prepend = "{}{} {};\n"\
                .format(tab_level, deduce_bytes_type(v_dict, config),
                        config["parseIntermediateVariableName"])
            section += "{}{}[index] = {};\n"\
                       .format(tab_level * 2, config["parseIntermediateVariableName"],
                               get_random(v_type))
            optional_append = "{}{}.{} = {};\n"\
                              .format(tab_level, p_name, v_name,
                                      config["parseIntermediateVariableName"])
        else:
            section += "{}{}.{}[index] = {};\n"\
                       .format(tab_level + common_def.TAB,
                               p_name, v_name, get_random(v_type))
    else:
        if CommonDef.ENUM == type_info[v_type]["typeCategory"]:
            field_type = ""
            if type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_type.split(".")[-1]
            section += "{}{}.{}[index] = {}({});\n"\
                       .format(tab_level + common_def.TAB,
                               p_name, v_name, field_type, config["enumDefault"])
        elif CommonDef.CLASS == type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == type_info[v_type]["typeCategory"]:
            field_type = ""
            if type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_type.split(".")[-1]
            if v_dict["nullable"]:
                temp_vector = config["parseIntermediateVariableName"]
                resize_line = "{}{}.resize({});\n".format(tab_level, temp_vector, length)
                section = "NULLABLE_PREPEND{}{}for (int index = 0; index < {}; index++)\n{}{{\n"\
                          .format(resize_line, tab_level, length, tab_level)
                optional_prepend = "{}vector<std::optional<{}>> {};\n"\
                                   .format(tab_level, field_type, temp_vector)
                temp_name = v_name.lower() + config["parseIntermediateVariableName"]
                section += "{}{} {};\n"\
                           .format(tab_level * 2, field_type, temp_name)
                section += "{}{}({});\n"\
                           .format(tab_level * 2, get_test_helper_name(v_type), temp_name)
                section += "{}{}[index] = {};\n"\
                           .format(tab_level * 2, temp_vector, temp_name)
                optional_append = "{}{}.{} = {};\n"\
                                  .format(tab_level, p_name, v_name, temp_vector)
            else:
                section += "{}{}({}.{}[index]);\n"\
                           .format(tab_level + common_def.TAB,
                                   get_test_helper_name(v_type), p_name, v_name)
    section += "{}}}\nNULLABLE_APPEND".format(tab_level)
    section = section.replace("NULLABLE_PREPEND", optional_prepend)
    section = section.replace("NULLABLE_APPEND", optional_append)
    if optional:
        # Half chance not fill the optional field at all
        # This is to emulate the actual "null" situation
        if not config["forceNonNullable"] and get_random_bool():
            section = "{}// {}.{}\n".format(tab_level, p_name, v_name)
    return section

def fill_array(v_name: str, v_dict: dict, p_name: str,
               top_level: bool, type_info: dict, config: dict,
               common_def: CommonDef, tab_level: str) -> str:
    """
    Initialize and fill an array variable.

    Args:
        v_name: Variable name
        v_dict: Variable info dictionary
        p_name: Parent variable name
        top_level: Whether this variable is at the top-level
        type_info: Type info dictionary
        config: Configuration dictionary
        common_def: Common definitions
        tab_level: Tab indention level

    Returns:
        Initialization section to fill the array.
    """
    section = ""
    if top_level:
        section += \
            fill_top_level_array(v_name, v_dict, type_info,
                                 config, common_def, tab_level)
    else:
        section += \
            fill_lower_level_array(v_name, v_dict, p_name,
                                   type_info, config, common_def, tab_level)
    return section

def check_out_path_exists(config: dict, path: str) -> None:
    """
    Check if the given out path exists.
    If not, create all intermediate directories with a specific privilege mode.

    Args:
        config: Configurations
        path: Path to be validated

    Returns:
        None
    """
    if not os.path.exists(path):
        print("Out path \"{}\" does not exist, create using privilege {}"\
              .format(path, config["outMode"]))
        # Remove current umask then set it back,
        # as makedirs() honers current execution environment over "mode=" parameter
        original_mask = os.umask(0)
        os.makedirs(path,
                    mode=int(config["outMode"], base=8),
                    exist_ok=True)
        os.umask(original_mask)

def delete_generated(config: dict) -> None:
    """
    Delete any existing ".h" header file or ".cpp" implementation file.

    Args:
        config: Configurations

    Returns:
        None
    """
    if not os.path.exists(config["outPath"]):
        raise ValueError("Out directory does not exist")
    files = os.listdir(config["outPath"])
    for file in files:
        if config["headerFileSuffix"] == file[-2:]\
            or config["sourceFileSuffix"] == file[-4:]:
            os.remove(file)
            print("Delete {}".format(file))
    try:
        os.removedirs(config["outPath"])
        print("Folder {} removed", config["outPath"])
    except:
        print("Other files exist under out directory. Delete them manually")
    

def dump_json(file_path: str, arg_dict: dict, encoding: str) -> None:
    """
    Dump a dictionary into a given file.

    Args:
        file_path: Destination path
        arg_dict: The dictionary to be dumped
        encoding: Encoding

    Returns:
        None
    """
    with open(file_path, "w", encoding=encoding) as f:
        f.write(json.dumps(arg_dict, indent=4, separators=(",", ": ")))

def load_json(json_path: str) -> dict:
    """
    Load a given JSON file into a dictionary.

    Args:
        json_path: JSON file path

    Returns:
        JSON dictionary.
    """
    json_dict = {}
    if not os.path.exists(json_path):
        raise ValueError("JSON file does not exist")
    with open(json_path, "r", encoding="utf-8") as f:
        json_dict = json.loads(f.read())
    return json_dict

def convert_package_name(str: str) -> str:
    """
    Convert package name into C++ namespace style.

    e.g. "wifi.hostapd." -> "wifi::hostapd::"

    Args:
        str: Package name string to be converted

    Returns:
        C++ style reference.
    """
    reference = []
    for char in str:
        if "." == char:
            reference.append("::")
        else:
            reference.append(char)
    return "".join(reference)

def get_file_name(interface_base_name: str,
                  file_name_suffix: str, file_type_suffix: str) -> str:
    """
    Get proper file name used for headers or implementation files.

    Args:
        interface_base_name: Interface base name
        file_name_suffix: Suffix used in file name
        file_type_suffix: File suffix

    Returns:
        File name.
    """
    file_name = "{}_{}{}"\
                .format(convert_const_upper(interface_base_name)[1:].lower(),
                        file_name_suffix,
                        file_type_suffix)
    return file_name

def get_test_helper_name(v_type: str) -> str:
    """
    Get test code helper method name of this class type.
    """
    return "INIT{}".format(convert_const_upper(v_type.split(".")[-1]))

def get_random_bool() -> bool:
    """
    Generate a random bool value.
    """
    return bool(random.getrandbits(1))

def get_random_int() -> int:
    """
    Generate a random type int value.
    """
    return random.getrandbits(CommonDef.TEST_RANDOM_BIT_LENGTH)

def get_random_long() -> int:
    """
    Generate a random type long equivalent value.
    """
    # Always use a number not smaller than MAX_INT to simulate as long
    return random.randint(CommonDef.INT_LIMIT[1], CommonDef.LONG_LIMIT[1])

# In fact "float" in Python has CPP "double", Java "double" precision
def get_random_float() -> float:
    """
    Generate a random type float value.
    """
    # Use [0, 1] for simplicity
    return random.uniform(0, 1)

def get_random_byte() -> bytes:
    """
    Generate a random byte.
    """
    # random.randbytes is introduced version 3.9 onwards
    if sys.version_info < HELPER_COMPATIBLE_VERSION:
        # Place holder return value
        return bytes([0])
    else:
        return random.randbytes(1)

def get_random_byte_str() -> str:
    """
    Generate a string representation of a byte.
    """
    length = 2
    hex_pool = string.hexdigits
    return "".join(random.choice(hex_pool) for i in range(length)).lower()

def get_random_string() -> str:
    """
    Generate a random string.
    """
    length = random.randint(1, CommonDef.TEST_RANDOM_STRING_MAX_LENGTH)
    base = string.ascii_letters + string.digits
    return "".join(random.choice(base) for i in range(length))

def get_random_char() -> str:
    """
    Generate a random character.
    """
    base = string.ascii_letters + string.digits
    return random.choice(base)

def get_random(t: str):
    """
    Generate a random value for a specific primitive type.

    Args:
        t: Type name

    Returns:
        A random bool, int, float, string, byte, or None
    """
    if "bool" == t:
        return str(get_random_bool()).lower()
    elif "int32" == t:
        return get_random_int()
    elif "int64" == t:
        return get_random_long()
    elif "float" == t or "double" == t:
        return get_random_float()
    elif "string" == t:
        return "\"{}\"".format(get_random_string())
    elif "bytes" == t:
        if sys.version_info < HELPER_COMPATIBLE_VERSION:
            return "0x{}".format(get_random_byte_str())
        else:
            return "0x{:0>2x}".format(int(get_random_byte()[0]))
    elif "uint32" == t:
        return "0x{:0>2x}".format(int(get_random_byte()[0]))
    elif "char" == t:
        return "\'{}\'".format(get_random_char())

def deduce_regular_compare_method(p_name: str, p_dict: dict,
                                  type_info: dict, interface_info: dict,
                                  config: dict, common_def: CommonDef,
                                  multiple: bool) -> str:
    """
    Retrieve the compare method line of a regular parameter field.

    Args:
        p_name: Parameter name
        p_dict: Parameter info dictionary
        type_info: Type info dictionary
        interface_info: Interface info dictionary
        config: Configurations
        common_def: Common definitions
        multiple: Whether the belonged method has other parameters

    Returns:
        Compare method line of this parameter.
    """
    section = ""
    p_type = p_dict["type"]
    p_last = p_type.split(".")[-1]
    mapped_type = ""
    method_invoc = ""
    parent_name = config["parseIntermediateVariableName"]
    if multiple:
        parent_name += ".{}".format(p_name)

    if p_last == config["statusKeyword"]:
        # typedef -> buf
        mapped_type = config["statusKeyword"] + config["typedefSuffix"]
        method_invoc = "{}(&{}, &{}, sizeof({}), \"{} ({})\")"\
                       .format(common_def.COMPARE_BUF, p_name,
                               parent_name, mapped_type, p_name, mapped_type)
    elif p_type in interface_info.keys():
        # interface message, defined as "instanceIdProtoType" -> buf
        mapped_type = CommonDef.TYPE_MAPPING_CPP[config["instanceIdProtoType"]]
        method_invoc = "{}(&{}, &{}, sizeof({}), \"{} ({})\")"\
                       .format(common_def.COMPARE_BUF, p_name,
                               parent_name, mapped_type, p_name, mapped_type)
    elif p_type in CommonDef.HEADER_PRIMITIVE_TYPES:
        # primitive type -> buf
        mapped_type = CommonDef.TYPE_MAPPING_CPP[p_type]
        method_invoc = "{}(&{}, &{}, sizeof({}), \"{} ({})\")"\
                       .format(common_def.COMPARE_BUF, p_name,
                               parent_name, mapped_type, p_name, mapped_type)
    elif "string" == p_type:
        # string -> string
        mapped_type = CommonDef.TYPE_MAPPING_CPP[p_type]
        method_invoc = "{}({}, {}, \"{} ({})\")"\
                       .format(common_def.COMPARE_STR, p_name,
                               parent_name, p_name, mapped_type)
    elif "bytes" == p_type:
        # array or vec
        mapped_type = config["bytesType"]
        macro_type = config["repeatedHandlingKeywords"]\
                     [p_dict["aidlType"]]["testCodeCompare"]
        method_invoc = "{}({}, {}, \"{} ({}<{}>)\")"\
                       .format(macro_type, p_name, parent_name,
                               p_name, p_dict["aidlType"], mapped_type)
    else:
        mapped_type = p_last
        if type_info[p_type]["contained"]:
            mapped_type = "::".join(p_type.split(".")[-2:])
        # enum -> buf
        if CommonDef.ENUM == type_info[p_type]["typeCategory"]:
            method_invoc = "{}(&{}, &{}, sizeof({}), \"{} ({})\")"\
                           .format(common_def.COMPARE_BUF, p_name,
                                   parent_name, mapped_type, p_name, mapped_type)
        elif CommonDef.CLASS == type_info[p_type]["typeCategory"]:
            method_invoc = "{}({}, {}, \"{} ({})\")"\
                           .format(common_def.COMPARE_OBJ, p_name,
                                   parent_name, p_name, mapped_type)
    section += "{}VERIFY({});\n".format(common_def.TAB, method_invoc)
    return section

def deduce_repeated_compare_method(p_name: str, p_dict: dict,
                                   type_info: dict,
                                   config: dict, common_def: CommonDef,
                                   multiple: bool) -> str:
    """
    Retrieve the compare method line of a repeated parameter field.

    Args:
        p_name: Parameter name
        p_dict: Parameter info dictionary
        type_info: Type info dictionary
        config: Configurations
        common_def: Common definitions
        multiple: Whether the belonged method has other parameters

    Returns:
        Compare method line of this parameter.
    """
    section = ""
    p_type = p_dict["type"]
    mapped_type = p_type.split(".")[-1]
    method_invoc = ""
    parent_name = config["parseIntermediateVariableName"]
    if multiple:
        parent_name += ".{}".format(p_name)
    macro_type = config["repeatedHandlingKeywords"]\
                 [p_dict["aidlType"]]["testCodeCompare"]

    # Should not be common status
    # Should not be interface message
    # Should not be bytes
    if p_type not in CommonDef.PRIMITIVE_TYPES:
        if type_info[p_type]["contained"]:
            mapped_type = "::".join(p_type.split(".")[-2:])
    method_invoc = "{}({}, {}, \"{} ({}<{}>)\")"\
                   .format(macro_type, p_name, parent_name,
                           p_name, p_dict["aidlType"], mapped_type)
    section += "{}VERIFY({});\n".format(common_def.TAB, method_invoc)
    return section

def deduce_compare_method(p_name: str, p_dict: dict,
                          type_info: dict, interface_info: dict,
                          config: dict, common_def: CommonDef,
                          multiple: bool) -> str:
    """
    Return the correct compare macro regards to this parameter field.

    Args:
        p_name: Parameter name
        p_dict: Parameter info dictionary
        type_info: Type info dictionary
        interface_info: Interface info dictionary
        config: Configurations
        common_def: Common definitions
        multiple: Whether the belonged method has other parameters

    Returns:
        Compare method line of this parameter.
    """
    section = ""
    p_repeat = p_dict["repeated"]
    if p_repeat:
        section += deduce_repeated_compare_method(p_name, p_dict,
                                                  type_info,
                                                  config, common_def, multiple)
    else:
        section += deduce_regular_compare_method(p_name, p_dict,
                                                 type_info, interface_info,
                                                 config, common_def, multiple)
    return section