#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""

"""
AIDL Proto Class Converter (APCC)

Fill AIDL classes into proto buffer classes, and vice versa.
"""

import argparse
import copy
import os
import sys

# Not generate pyc cache
sys.dont_write_bytecode = True

MIN_RUNTIME_VERSION = (3, 5)

print("Python version:\n{}\n".format(sys.version))

current_version = sys.version_info

if current_version < MIN_RUNTIME_VERSION:
    raise RuntimeError("This tool must use Python {}.{} and above."\
                       .format(MIN_RUNTIME_VERSION[0], MIN_RUNTIME_VERSION[1]))

from collector import *
from commondef import *
from generator import Generator
from helper import *

def merge_interface_message(interface_info: dict, common_def: CommonDef, config: dict) -> None:
    """
    Merge callback messages into base interface.

    Args:
        interface_info: Interface info dictionary

    Returns:
        None
    """
    i_name_set = interface_info.keys()
    interface_pair_list = []
    callback_list = []
    # Pick callbacks
    for i_name in i_name_set:
        for keyword in common_def.callback_keywords:
            if keyword in i_name:
                callback_list.append({"callbackName": i_name,
                                      "callbackKeyword": keyword})
                break
    for each_callback in callback_list:
        c_name = each_callback["callbackName"]
        c_keyword = each_callback["callbackKeyword"]
        interface_pair_list.clear()
        interface_pair_list.append(c_name)
        req_cfm_methods_dict = {}
        paired_interface = False
        # Check if a paired base interface exists
        try:
            req_cfm_methods_dict = \
                interface_info[c_name[:-len(c_keyword)]]["methods"]
            interface_pair_list.append(c_name[:-len(c_keyword)])
            interface_info[c_name[:-len(c_keyword)]]\
                .update({"pairedInterfaces": interface_pair_list.copy()})
            paired_interface = True
        except:
            # Only callback
            print("Asymmetric callback found when merging interfaces: {}"\
                  .format(c_name))
        ind_methods_dict = interface_info[c_name]["methods"]
        for (m_name, m_info) in ind_methods_dict.items():
            req_cfm_methods_dict.update({m_name: m_info})
        # Interface only having callback
        if not paired_interface:
            # Strip initial "I"
            i_name = c_name[:-len(c_keyword)]
            i_base_name = c_name.split(".")[-1][1:][:-len(c_keyword)]
            interface_info.update({
                i_name: {
                    "interfaceBaseName": i_base_name,
                    "interfaceType": config["indicateSuffix"],
                    "pairedInterfaces": interface_pair_list.copy(),
                    "methods": ind_methods_dict
                }
            })
        interface_info.pop(c_name)
    # Unify paired interface field for interfaces which do not have paired callback
    for non_callback in i_name_set:
        interface_pair_list.clear()
        try:
            paired_list = interface_info[non_callback]["pairedInterfaces"]
        except:
            print("Adding pairedInterface field for {}".format(non_callback))
            interface_pair_list.append(non_callback)
            interface_info[non_callback].update({"pairedInterfaces": interface_pair_list.copy()})

def generate_impl_files(interface_info: dict,
                        type_info: dict,
                        common_def: CommonDef,
                        config: dict) -> None:
    """
    Generate header files (.h) and implementation files (.cpp) for current module.

    Args:
        interface_info: Interface info dictionary
        type_info: Type info dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    generator = Generator(interface_info, type_info, common_def, config)
    generator.generate_impl_files()

def generate_macro_define_file(interface_info: dict,
                               common_def: CommonDef,
                               config: dict) -> None:
    """
    Generate ID definition macros for messages.

    Args:
        interface_info: Interface info dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    generator = Generator(interface_info, {}, common_def, config)
    generator.generate_macro_define_file()

def generate_bp(common_def: CommonDef, config: dict) -> None:
    """
    Generate blueprint file of this module to build the message.

    Args:
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    generator = Generator({}, {}, common_def, config)
    generator.generate_bp()

def generate_makefile(interface_info: dict,
                      common_def: CommonDef, config: dict) -> None:
    """
    Generate Makefile of this module to build the message.

    Args:
        interface_info: Interface info dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    generator = Generator(interface_info, {}, common_def, config)
    generator.generate_make_file()

def generate_testcode(interface_info: dict,
                      type_info: dict,
                      common_def: CommonDef,
                      config: dict) -> None:
    """
    Generate test code and corresponding build file to build the test binary.

    Args:
        interface_info: Interface info dictionary
        type_info: Type info dictionary
        common_def: Common definitions
        config: Configuration dictionary
    """
    generator = Generator(interface_info, type_info, common_def, config)
    generator.generate_test_code()

if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(description="AIDL <-> ProtocolBuffer class convertor")
    arg_parser.add_argument("-d", "--delete", action="store_true", help="Delete generated files")
    arg_parser.add_argument("-impl", "--generate_impl", action="store_true", help="Generate .h and .cpp implementation files")
    arg_parser.add_argument("-id", "--generate_id", action="store_true", help="Generate ID definitions for methods")
    arg_parser.add_argument("--aidl_path", type=str, metavar="AIDL_PATH", help="Path to aidl definitions")
    arg_parser.add_argument("--proto_path", type=str, metavar="PROTO_PATH", help="Path to proto definitions")
    arg_parser.add_argument("--out_path", type=str, metavar="OUT_PATH", help="Destination out path")
    arg_parser.add_argument("-bp", "--generate_bp", action="store_true", help="Generate blueprint file")
    arg_parser.add_argument("--ndk_name", type=str, metavar="NDK_NAME", help="NDK shared lib name")
    arg_parser.add_argument("-makefile", "--generate_makefile", action="store_true", help="Generate Makefile")
    arg_parser.add_argument("-testcode", "--generate_testcode", action="store_true", help="Generate test code")
    arg_parser.add_argument("--base_intf", type=str, metavar="BASE_INTF", help="Root base AIDL interface name")
    arg_parser.add_argument("-async", "--traffic_model", action="store_true", help="Communication model, provide if asynchronous")
    arg_parser.add_argument("--package_name", type=str, metavar="PACKAGE_INDICATION_NAME", help="Preferred package name for the module")
    arg_parser.add_argument("--msg_def_offset", type=str, metavar="MSG_DEF_OFFSET", choices=[f"0x{i}000" for i in range(1, 8)],
                            help="Offset (4-digit hex number) for message definition base, stepping is 0x1000")
    args = arg_parser.parse_args()

    common_def = CommonDef()
    config = {}

    # Change dir
    dir_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(dir_path)

    if not os.path.exists(os.path.normpath("config.json")):
        raise ValueError("Missing config file")
    else:
        config = load_json("config.json")

    if args.delete:
        delete_generated(config)
        exit(0)

    # Override paths, if provided
    if None != args.aidl_path:
        config["aidlPath"] = args.aidl_path
        print("AIDL path set to \"{}\"".format(config["aidlPath"]))
    else:
        print("AIDL path not provided, using default \"{}\"".format(config["aidlPath"]))
    if None != args.proto_path:
        config["protoPath"] = args.proto_path
        print("Proto path set to \"{}\"".format(config["protoPath"]))
    else:
        print("Proto path not provided, using default \"{}\"".format(config["protoPath"]))
    if None != args.out_path:
        config["outPath"] = args.out_path
        print("Out path set to \"{}\"".format(config["outPath"]))
    else:
        print("Out path not provided, using default \"{}\"".format(config["outPath"]))

    common_def.TAB = config["tabStyle"]

    type_info = {}
    interface_info = {}
    if None == config["aidlPath"] or None == config["protoPath"]:
        raise ValueError("\nBoth AIDL path and Proto path must be provided")

    path_aidl = config["aidlPath"]
    path_proto = config["protoPath"]
    if 0 != len(config["headerAdditionalHeaders"]):
        print("Adding additional headers")
        additional_header_list = []
        for header in config["headerAdditionalHeaders"]:
            additional_header_list.append("#include \"{}\"".format(header))
        common_def.additional_include_header = "\n{}\n\n"\
                                               .format("\n".join(additional_header_list))

    if "" == path_aidl or "" == path_proto:
        raise ValueError("\nBoth AIDL path and Proto path must be provided")

    aidl_files = os.listdir(path_aidl)

    type_list = []
    interface_list = []
    # Distinguish interfaces and types
    for each_file in aidl_files:
        # AIDL interface files always start with an "I"
        # and contains an "interface" or "oneway interface" declaration at the start of one line
        file_name = each_file.replace(config["aidlSuffix"], "")
        if "I" != each_file[0]:
            type_list.append(file_name)
        else:
            file_path = os.path.join(path_aidl, each_file)
            if check_aidl_interface(os.path.normpath(file_path), config, common_def):
                interface_list.append(file_name)
                common_def.interface_set.add(file_name)
            else:
                type_list.append(file_name)

    interface_list.sort()
    # AIDL definitions does not have status.aidl
    type_list.append(config["statusKeyword"])
    # Collect type information
    for each_type in type_list:
        file_path = os.path.join(path_proto, each_type + config["protoSuffix"])
        gather_type_info(os.path.normpath(file_path),
                         type_info, common_def, config, CommonDef.M_TYPE.CLASS)
    # Collect interface methods information, also register specific callback keyword
    for each_interface in interface_list:
        file_path = os.path.join(path_proto, each_interface + config["protoSuffix"])
        gather_interface_info(os.path.normpath(file_path),
                              interface_info, type_info, common_def, config)
        # Collect additional interface message fields for non callbacks
        if not check_callback(each_interface, common_def):
            gather_type_info(os.path.normpath(file_path),
                             type_info, common_def, config, CommonDef.M_TYPE.INTERFACE)
    common_def.base_intf = get_pseudo_base_interface_name(interface_list, config, common_def)

    if None != args.base_intf:
        print("Base root interface: {}".format(args.base_intf))
        common_def.base_intf = args.base_intf

    # If not given, APCC assumes this is a module within AOSP
    if None != args.package_name:
        if "" != args.package_name:
            print("Setting module name to {}".format(args.package_name))
            common_def.provided_package_name = args.package_name

    gather_common_status_info(path_proto, interface_info, common_def, config)

    # dump_json("info_type.json", type_info, config["encoding"])
    # dump_json("info_interface.json", interface_info, config["encoding"])

    for interface_entry in interface_info.keys():
        print("{} methods in {}: {}\n"\
              .format(len(interface_info[interface_entry]["methods"]),
                      interface_entry,
                      list(interface_info[interface_entry]["methods"].keys())))

    if args.traffic_model:
        print("This module uses asynchronous communication mode")
        config["sync"] = False
        interface_info.pop(common_def.msg_common_interface_name)
    else:
        print("Using default mode: synchronous")

    if args.msg_def_offset:
        offset_arg = int(args.msg_def_offset[2:], base=16)
        common_def.id_definition_offset = offset_arg
        print(f"Using additional definition offset: 0x{offset_arg:0>4X}")

    sanitize_type_info(type_info, common_def)
    sanitize_interface_params(type_info, interface_info, common_def)
    print("\nclass type count: {}".format(len(type_info)))
    print("interface count: {}\n".format(len(interface_info)))

    # dump_json("info_type.json", type_info, config["encoding"])
    # dump_json("info_interface.json", interface_info, config["encoding"])

    if args.generate_impl:
        print("\nGenerating .h & .cpp")
        merged_interface_info = copy.deepcopy(interface_info)
        merge_interface_message(merged_interface_info, common_def, config)
        generate_impl_files(merged_interface_info, type_info, common_def, config)

    if args.generate_id:
        print("\nGenerating ID definitions")
        generate_macro_define_file(interface_info, common_def, config)

    if args.generate_bp:
        print("\nGenerating .bp")
        if None != args.ndk_name:
            common_def.bp_ndk_name = args.ndk_name
            print("Shared NDK is {}".format(common_def.bp_ndk_name))
        else:
            raise ValueError("NDK name not provided")
        generate_bp(common_def, config)

    if args.generate_makefile:
        print("\nGenerating Makefile")
        merged_interface_info = copy.deepcopy(interface_info)
        merge_interface_message(merged_interface_info, common_def, config)
        generate_makefile(merged_interface_info, common_def, config)

    if args.generate_testcode:
        print("\nGenerating test code and build file")
        if None != args.ndk_name:
            common_def.bp_ndk_name = args.ndk_name
            print("Shared NDK is {}".format(common_def.bp_ndk_name))
        else:
            raise ValueError("NDK name not provided")
        merged_interface_info = copy.deepcopy(interface_info)
        merge_interface_message(merged_interface_info, common_def, config)
        generate_testcode(merged_interface_info, type_info, common_def, config)