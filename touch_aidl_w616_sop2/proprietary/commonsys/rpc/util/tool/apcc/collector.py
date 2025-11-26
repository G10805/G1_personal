#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""

from io import TextIOWrapper
import re

from commondef import CommonDef
from helper import *

def gather_array_implementation_info(proto_file_path: str,
                                     proto_line: str,
                                     message_type: str,
                                     message_name: str,
                                     config: dict,
                                     repeated_type: CommonDef.R_TYPE) -> Tuple[dict, bool]:
    """
    Gather info about type bytes or any repeated fields. Whether it is array or not,
    whether it is length-specific array or not.

    Args:
        proto_file_path: Path of the proto file
        proto_line: Line of this bytes type field
        message_type: Message type of which this field is in
        message_name: Message name of which this field is in
        config: Configurations
        repeated_type: Whether searching for bytes field or a repeated field

    Returns:
        A dictionary containing implementation info and whether it is nullable.
    """
    proto_file_name = proto_file_path.split("/")[-1].split("\\")[-1]
    aidl_name = proto_file_name.split(".")[0]
    aidl_file_path = os.path.join(config["aidlPath"], aidl_name + config["aidlSuffix"])

    field_name = ""
    field_type = ""
    count_pattern = ""
    nullable_pattern = ""
    match_count_pattern = None
    match_nullable_pattern = None

    field_dict = {}
    if "InterfaceCfm" == message_type:
        # Interface Cfm is self-defined
        # Do nothing, directly set type as vector
        field_dict.update({"aidlType": "vector"})
        return (field_dict, False)
    # For field in a parcelable
    # nullable is before type declaration
    elif CommonDef.CLASS == message_type:
        # Need proto_line, no proto_message
        # For byte type field within a class
        # byte[] field_name;        -> vector
        # byte[2] field_name;       -> array    (length specified)
        # byte[/*6*/] field_name;   -> vector

        # For class type field within a class
        # android.hardware.wifi.NanBandSpecificConfig[3] bandSpecificConfig;
        proto_line_entries = proto_line.split(" ")
        if CommonDef.R_TYPE.BYTES == repeated_type:
            field_type = "bytes"
            if "optional" == proto_line_entries[0]:
                field_name = proto_line_entries[2]
            else:
                field_name = proto_line_entries[1]
            count_pattern = r".*byte\s*\[(.*)\]\s*" + field_name + r";"
            nullable_pattern = r".*" + config["aidlNullableAnnotation"] + \
                               r"\s+byte.*" + field_name + r";"
        elif CommonDef.R_TYPE.CLASS == repeated_type:
            field_type = proto_line_entries[1].split(".")[-1]
            if field_type in CommonDef.TYPE_MAPPING_AIDL.keys():
                field_type = CommonDef.TYPE_MAPPING_AIDL[field_type]
            field_name = proto_line_entries[2]
            count_pattern = r".*" + field_type + r"\s*\[(.*)\]\s*" + field_name + r";"
            nullable_pattern = r".*" + config["aidlNullableAnnotation"] + \
                               r"\s+" + field_type + r".*" + field_name + r";"

    # Req, Cfm, Ind messages should consider crossing lines
    # Req & Ind for input parameters
    # nullable is between in/ out and type declaration
    elif config["requestSuffix"] == message_type or \
         config["indicateSuffix"] == message_type:
        # Need proto_line, need proto_message
        # For Req message
        # void reject(in byte[] peerAddress);
        # void setMacAddress(in byte[6] mac);
        # void removeClient(in byte[/*6*/] peerAddress, in boolean isLegacyClient);

        # For Ind message, like Reqs
        # void onDeviceFound(in byte[] srcAddress, in byte[] p2pDeviceAddress,
        #         in byte[] primaryDeviceType, in String deviceName, in WpsConfigMethods configMethods,
        #         in byte deviceCapabilities, in P2pGroupCapabilityMask groupCapabilities,
        #         in byte[] wfdDeviceInfo);
        # void onDeviceLost(in byte[] p2pDeviceAddress);
        proto_line_entries = proto_line.split(" ")
        aidl_method_name = message_name[0].lower() + message_name[1:]
        if CommonDef.R_TYPE.BYTES == repeated_type:
            field_type = "bytes"
            field_name = proto_line_entries[1]
            count_pattern = r".*" + aidl_method_name + \
                            r".*\(.*byte\s*\[(.*)\]\s*" + field_name + r".*\).*"
            nullable_pattern = r".*" + aidl_method_name + \
                               r".*\(.*" + config["aidlNullableAnnotation"] + \
                               r"\s+byte.*" + field_name + r".*\).*"
        elif CommonDef.R_TYPE.CLASS == repeated_type:
            field_type = proto_line_entries[1].split(".")[-1]
            if field_type in CommonDef.TYPE_MAPPING_AIDL.keys():
                field_type = CommonDef.TYPE_MAPPING_AIDL[field_type]
            field_name = proto_line_entries[2]
            count_pattern = r".*" + aidl_method_name + \
                            r".*\(.*" + field_type + r"\s*\[(.*)\]\s*" + \
                            field_name + r".*\).*"
            nullable_pattern = r".*" + aidl_method_name + \
                               r".*\(.*" + config["aidlNullableAnnotation"] + \
                               r"\s+" + field_type + r".*" + field_name + r".*\).*"
    # Cfm for output return value
    elif config["confirmSuffix"] == message_type:
        # No need proto_line, need proto_message
        # For Cfm message
        # byte[] getDeviceAddress();
        # byte[] getSsid(in byte[] peerAddress);
        # byte[6] getFactoryMacAddress();
        # int read(out byte[] data, int timeoutInMs);
        proto_line_entries = proto_line.split(" ")
        aidl_method_name = message_name[0].lower() + message_name[1:]
        if CommonDef.R_TYPE.BYTES == repeated_type:
            field_type = "bytes"
            count_return_pattern = r".*byte\s*\[(.*)\]\s*" + aidl_method_name + r".*\(.*\).*"
            count_param_pattern = r".*" + aidl_method_name + \
                            r".*\(.*byte\s*\[(.*)\]\s*" + field_name + r".*\).*"
            count_pattern = r"(" + count_return_pattern + r"|" + count_param_pattern + r")"
            nullable_return_pattern = r".*" + config["aidlNullableAnnotation"] + r"\s+byte.*\s+" + \
                                      aidl_method_name + r".*\(.*\).*"
            nullable_param_pattern = r".*" + aidl_method_name + \
                                     r".*\(.*" + config["aidlNullableAnnotation"] + \
                                     r"\s+byte.*" + field_name + r".*\).*"
            nullable_pattern = r"(" + nullable_return_pattern + r"|" + nullable_param_pattern + r")"
        elif CommonDef.R_TYPE.CLASS == repeated_type:
            field_type = proto_line_entries[1].split(".")[-1]
            if field_type in CommonDef.TYPE_MAPPING_AIDL.keys():
                field_type = CommonDef.TYPE_MAPPING_AIDL[field_type]
            count_return_pattern = r".*" + field_type + r"\s*\[(.*)\]\s*" + \
                                   aidl_method_name + r".*\(.*\).*"
            count_param_pattern = r".*" + aidl_method_name + \
                                  r".*\(.*" + field_type + r"\s*\[(.*)\]\s*" + \
                                  field_name + r".*\).*"
            count_pattern = r"(" + count_return_pattern + r"|" + count_param_pattern + r")"
            nullable_return_pattern = r".*" + config["aidlNullableAnnotation"] + r"\s+" + \
                                      field_type + r".*\s+" + aidl_method_name + r".*\(.*\).*"
            nullable_param_pattern = r".*" + aidl_method_name + \
                                     r".*\(.*" + config["aidlNullableAnnotation"] + \
                                     r"\s+" + field_type + r".*" + field_name + r".*\).*"
            nullable_pattern = r"(" + nullable_return_pattern + r"|" + nullable_param_pattern + r")"

    # Type "char" is not present in proto, mapped as "int32"
    # So "int32" in proto maps back to "int" and "char" in AIDL
    if "int" == field_type:
        count_pattern = count_pattern.replace("int", r"(int|char)")
        nullable_pattern = nullable_pattern.replace("int", r"(int|char)")
    match_count_pattern = re.compile(count_pattern)
    match_nullable_pattern = re.compile(nullable_pattern)
    with open(aidl_file_path, "r", encoding=config["encoding"]) as f:
        assert match_count_pattern is not None
        assert match_nullable_pattern is not None
        found = False
        optional = False
        while not found:
            line = f.readline()
            if not line:
                if not found:
                    str = "Field of {} not found in {}"\
                          .format(field_type, aidl_name + config["aidlSuffix"])
                    str += "\nproto_line: {}\nmessage_type: {}\nmessage_name: {}\nrepeated_type: {}"\
                           .format(proto_line, message_type, message_name, repeated_type)
                    str += "\nSearch pattern: {}".format(count_pattern)
                    raise ValueError(str)
                break
            # For methods, concatenate method signature crossing lines first, then match
            if "(" in line:
                while True:
                    if ")" in line:
                        break
                    line += f.readline()

            line = line.strip()
            line = line.replace("\n", "")
            match_count = match_count_pattern.match(line)
            if None == match_count:
                continue
            else:
                # Match found the field line
                found = True
                assert match_count is not None
                # ('int[] getChipIds();', 'int', '', None, None)
                # ('String[] getBridgedInstances();', '', None)
                # ('byte[6] getFactoryMacAddress();', '6', None)
                # First set to last, which is char
                last = match_count.groups()[-1]
                # length not found, but match does exist.
                # This branch is only caused by (int|char)
                # Fall back to second-last: int
                if None == last:
                    last = match_count.groups()[-2]
                if None == last:
                    field_dict.update({"aidlType": "vector"})
                else:
                    length = last.strip()
                    if "" == length:
                        # vector
                        field_dict.update({"aidlType": "vector"})
                    elif "/*" in length:
                        # vector
                        field_dict.update({"aidlType": "vector"})
                    else:
                        array_length = int(length)
                        field_dict.update({"aidlType": "array"})
                        field_dict.update({"length": array_length})
                match_nullable = match_nullable_pattern.match(line)
                if None == match_nullable:
                    field_dict.update({"nullable": False})
                else:
                    field_dict.update({"nullable": True})
                    optional = True
    return (field_dict, optional)

def update_int32_type(proto_file_path: str,
                      proto_line: str,
                      message_type: str,
                      message_name: str,
                      config: dict) -> str:
    """
    Deduce whether int32 is mapped back to AIDL int or char.
    This method is introduced since both AIDL "int" and AIDL "char" are mapped to Proto "int32".

    Args:
        proto_file_path: Path of the proto file
        proto_line: Line of this int32 field
        message_type: Whether for class or method
        message_name: Container name of this field. For method would be method name, for class would be class name.
        config: Configurations

    Returns:
        True AIDL type of Proto int32. (int or char)
    """
    proto_file_name = proto_file_path.split("/")[-1].split("\\")[-1]
    aidl_name = proto_file_name.split(".")[0]
    aidl_file_path = os.path.join(config["aidlPath"], aidl_name + config["aidlSuffix"])

    field_name = ""
    repeated = False

    proto_line_entries = proto_line.split(" ")
    if "repeated" == proto_line_entries[0]:
        field_name = proto_line_entries[2]
        repeated = True
    else:
        field_name = proto_line_entries[1]

    match_pattern = ""
    compiled_pattern = None

    if CommonDef.CLASS == message_type:
        if repeated:
            match_pattern = r".*int.*\[.*\].*" + field_name
        else:
            match_pattern = r".*int.*" + field_name
    elif config["requestSuffix"] == message_type or \
         config["indicateSuffix"] == message_type:
        message_name = message_name[0].lower() + message_name[1:]
        if repeated:
            match_pattern = r".*" + message_name + r".*int.*\[.*\].*" + field_name
        else:
            match_pattern = r".*" + message_name + r".*int.*" + field_name
    elif config["confirmSuffix"] == message_type:
        message_name = message_name[0].lower() + message_name[1:]
        if repeated:
            match_pattern = r".*int.*\[.*\].*" + message_name + r".*"
        else:
            match_pattern = r".*int.*" + message_name + r".*"

    compiled_pattern = re.compile(match_pattern)

    map_type = "int32"
    if config["statusKeyword"] == aidl_name:
        return map_type
    with open(aidl_file_path, "r", encoding=config["encoding"]) as f:
        assert compiled_pattern is not None
        found = False
        while not found:
            line = f.readline()
            if not line:
                if not found:
                    print("Type int mapping not found, mapped as char")
                    map_type = "char"
                break
            if "(" in line:
                while True:
                    if ")" in line:
                        break
                    line += f.readline()

            line = line.strip()
            line = line.replace("\n", "")
            matched = compiled_pattern.match(line)
            if None == matched:
                continue
            else:
                found = True
    return map_type

def gather_contained_type_info(fd: TextIOWrapper,
                               file_path: str,
                               parent_name: str,
                               type_info: dict,
                               common_def: CommonDef,
                               config: dict, message_type: str) -> str:
    """
    Gather class, enum, or union defined within an interface.

    Args:
        fd: File descriptor
        file_path: Proto file path
        parent_name: Parent message name
        type_info: Type info dictionary
        common_def: Common definitions
        config: Configurations
        message_type: Message type for which contains this field

    Returns:
        Full name of the type.
    """
    type_detail = {}
    fields = {}
    contained = True
    line = fd.readline().strip()
    type_name = line.split(" ")[1]
    union = False
    if CommonDef.MESSAGE == line[:len(CommonDef.MESSAGE)]:
        # Strip trailing "\n"
        type_detail.update({"typeCategory": CommonDef.CLASS})
        # Discard starting "{"
        line = fd.readline()
        while True:
            pos = fd.tell()
            line = fd.readline()
            repeated = False
            optional = False
            if "}" == line.strip()[0]:
                break
            line_entries = line.strip().split(" ")
            if CommonDef.ONEOF == line_entries[0]:
                union = True
                fd.seek(pos)
                gather_oneof(fd,
                             "{}.{}".format(parent_name, type_name),
                             type_info, common_def, contained)
                continue
            elif "repeated" == line_entries[0]:
                field_type = line_entries[1]
                field_name = line_entries[2]
                repeated = True
            elif "optional" == line_entries[0]:
                field_type = line_entries[1]
                field_name = line_entries[2]
                optional = True
            else:
                field_type = line_entries[0]
                field_name = line_entries[1]
                repeated = False
            (field_type, cached_type) = \
                deduce_type_name(field_type, parent_name, "", common_def)
            if common_def.INT32_TYPE == field_type:
                field_type = update_int32_type(file_path, line.strip(),
                                               CommonDef.CLASS, type_name, config)
            field_dict = {"type": field_type,
                          "repeated": repeated,
                          "nullable": optional}
            # optional field is handled within bytes & repeated
            if "bytes" == field_type:
                (bytes_info, optional) = \
                    gather_array_implementation_info(file_path, line.strip(),
                                                     message_type,
                                                     "", config,
                                                     CommonDef.R_TYPE.BYTES)
                field_dict.update(bytes_info)
            if repeated:
                (repeated_info, optional) = \
                    gather_array_implementation_info(file_path, line.strip(),
                                                     message_type,
                                                     "", config,
                                                     CommonDef.R_TYPE.CLASS)
                field_dict.update(repeated_info)
            fields.update({field_name: field_dict})
        if not union:
            type_detail.update({"fields": fields})
            type_detail.update({"contained": contained})
            type_info.update({"{}.{}".format(parent_name, type_name): type_detail})
    elif CommonDef.ENUM == line[:len(CommonDef.ENUM)]:
        # Consume enum entries
        while True:
            line = fd.readline()
            if "}" == line.strip()[0]:
                break
        type_detail.update({"typeCategory": CommonDef.ENUM})
        type_detail.update({"contained": contained})
        type_info.update({"{}.{}".format(parent_name, type_name): type_detail})
    return "{}.{}".format(parent_name, type_name)

def gather_oneof(fd: TextIOWrapper,
                 type_name: str,
                 type_info: dict,
                 common_def: CommonDef,
                 contained: bool) -> None:
    """
    Gather ProtoBuffer oneof information in class message.

    Args:
        fd: File descriptor
        type_name: Message type name
        type_info: Type info dictionary
        common_def: Common definitions

    Returns:
        None
    """
    type_detail = {}
    line = fd.readline().strip()
    oneof_name = line.split(" ")[1]
    type_detail.update({"typeCategory": CommonDef.UNION})
    # Discard starting "{"
    line = fd.readline()
    fields = {}
    while True:
        # oneof never contains "repeated"
        line = fd.readline()
        if "}" == line.strip()[0]:
            break
        line_entries = line.strip().split(" ")
        field_type = line_entries[0]
        field_name = line_entries[1]
        (field_type, cached_type) = \
            deduce_type_name(field_type, type_name, "", common_def)
        fields.update({field_name: field_type})
    type_detail.update({"contained": contained})
    type_detail.update({"oneofName": oneof_name})
    type_detail.update({"fields": fields})
    type_info.update({type_name: type_detail})

def gather_class_type_info(file_path: str, type_info: dict,
                           common_def: CommonDef, config: dict) -> None:
    """
    Gather ProtoBuffer type information in class messages & enums.

    Args:
        file_path: Relative path of the proto type message file
        type_info: Type info dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    # Assume there is no line comment in message/ enum body
    type_name = ""
    package_name = ""
    type_detail = {}
    fields = {}
    union = False
    with open(file_path, "r", encoding=config["encoding"]) as f:
        while True:
            line = f.readline()
            if not line:
                break
            if "package" == line.strip()[:len("package")]:
                package_name = line.strip().split(" ")[1][:-1]
                continue
            if CommonDef.MESSAGE == line[:len(CommonDef.MESSAGE)]:
                # Strip trailing "\n"
                type_name = line.strip().split(" ")[1]
                type_detail.update({"typeCategory": CommonDef.CLASS})
                # Discard starting "{"
                line = f.readline()
                field_name = ""
                field_type = ""
                cached_type = ""
                while True:
                    repeated = False
                    optional = False
                    pos = f.tell()
                    line = f.readline()
                    if "}" == line[0]:
                        break
                    line_entries = line.strip().split(" ")
                    # Sub-type defined in message
                    if CommonDef.ENUM == line_entries[0] or \
                       CommonDef.MESSAGE == line_entries[0]:
                        f.seek(pos)
                        cached_type = \
                            gather_contained_type_info(f,
                                                       file_path,
                                                       "{}.{}"\
                                                       .format(package_name, type_name),
                                                       type_info,
                                                       common_def, config,
                                                       CommonDef.CLASS)
                        continue
                    elif CommonDef.ONEOF == line_entries[0]:
                        union = True
                        f.seek(pos)
                        gather_oneof(f,
                                     "{}.{}".format(package_name, type_name),
                                     type_info, common_def, False)
                        continue
                    #  Normal fields
                    if "repeated" == line_entries[0]:
                        field_type = line_entries[1]
                        field_name = line_entries[2]
                        repeated = True
                    elif "optional" == line_entries[0]:
                        field_type = line_entries[1]
                        field_name = line_entries[2]
                        optional = True
                    else:
                        field_type = line_entries[0]
                        field_name = line_entries[1]
                    (field_type, cached_type) = \
                        deduce_type_name(field_type, package_name,
                                         cached_type, common_def)
                    if CommonDef.INT32_TYPE == field_type:
                        field_type = update_int32_type(file_path, line.strip(),
                                                       CommonDef.CLASS, type_name, config)
                    field_dict = {"type": field_type,
                                  "repeated": repeated,
                                  "nullable": optional}
                    if "bytes" == field_type:
                        (bytes_info, optional) = \
                            gather_array_implementation_info(file_path, line.strip(),
                                                             CommonDef.CLASS,
                                                             "", config,
                                                             CommonDef.R_TYPE.BYTES)
                        field_dict.update(bytes_info)
                    if repeated:
                        (repeated_info, optional) = \
                            gather_array_implementation_info(file_path, line.strip(),
                                                             CommonDef.CLASS,
                                                             "", config,
                                                             CommonDef.R_TYPE.CLASS)
                        field_dict.update(repeated_info)
                    fields.update({field_name: field_dict})
                if not union:
                    type_detail.update({"fields": fields})
                type_detail.update({"contained": False})
            elif CommonDef.ENUM == line[:len(CommonDef.ENUM)]:
                type_name = line.strip().split(" ")[1]
                # Strip trailing "\n"
                type_detail.update({"typeCategory": CommonDef.ENUM})
                type_detail.update({"contained": False})
    # Union is handle already
    if not union:
        type_info.update({"{}.{}".format(package_name, type_name): type_detail})

def gather_interface_type_info(file_path: str, type_info: dict,
                               common_def: CommonDef, config: dict) -> None:
    """
    Gather ProtoBuffer type information of an interface message.

    Args:
        file_path: Relative path of the proto interface message file
        type_info: Class type dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    interface_name = file_path.split("/")[-1].split("\\")[-1].split(".")[0]
    type_name = ""
    package_name = ""
    type_detail = {}
    field_count = 0
    with open(file_path, "r", encoding=config["encoding"]) as f:
        # Find nested interface message
        while True:
            line = f.readline()
            if not line:
                break
            # Package name is the same for all interface files
            if "package" == line[:len("package")]:
                package_name = line.strip().split(" ")[1][:-1]
                continue
            # This method should be called on nested interface messages
            if CommonDef.MESSAGE == line[:len(CommonDef.MESSAGE)]:
                message_name = line.strip().split(" ")[1]
                if interface_name == message_name:
                    f.readline()
                    break
        while True:
            pos = f.tell()
            line = f.readline()
            if not line:
                break
            # Skip all messages and enums
            if CommonDef.MESSAGE == line.strip()[:len(CommonDef.MESSAGE)]:
                while True:
                    line = f.readline()
                    if CommonDef.ONEOF == line.strip().split()[0]:
                        while True:
                            line = f.readline()
                            if "}" == line.strip()[0]:
                                line = f.readline()
                                break
                    if "}" == line.strip()[0]:
                        f.readline()
                        break
            elif CommonDef.ENUM == line.strip()[:len(CommonDef.ENUM)]:
                while True:
                    line = f.readline()
                    if "}" == line.strip()[0]:
                        f.readline()
                        break
            else:
                f.seek(pos)
                type_name = interface_name
                field_name = ""
                field_type = ""
                repeated = False
                optional = False
                cached_type = ""
                fields = {}
                type_detail.update({"typeCategory": CommonDef.CLASS})
                while True:
                    line = f.readline()
                    if "}" == line.strip()[0]:
                        break
                    line_entries = line.strip().split(" ")
                    if "repeated" == line_entries[0]:
                        field_type = line_entries[1]
                        field_name = line_entries[2]
                        repeated = True
                    elif "optional" == line_entries[0]:
                        field_type = line_entries[1]
                        field_name = line_entries[2]
                        optional = True
                    else:
                        field_type = line_entries[0]
                        field_name = line_entries[1]
                    (field_type, cached_type) = \
                        deduce_type_name(field_type, package_name,
                                         cached_type, common_def)
                    field_dict = {"type": field_type,
                                  "repeated": repeated,
                                  "nullable": optional}
                    if "bytes" == field_type:
                        (bytes_info, optional) = \
                            gather_array_implementation_info(file_path, line.strip(),
                                                             CommonDef.CLASS,
                                                             "", config,
                                                             CommonDef.R_TYPE.BYTES)
                        field_dict.update(bytes_info)
                    if repeated:
                        (repeated_info, optional) = \
                            gather_array_implementation_info(file_path, line.strip(),
                                                             CommonDef.CLASS,
                                                             "", config,
                                                             CommonDef.R_TYPE.CLASS)
                        field_dict.update(repeated_info)
                    fields.update({field_name: field_dict})
                    field_count += 1
                type_detail.update({"fields": fields})
    # Only include this interface message when field exists
    if 0 < field_count:
        type_detail.update({"contained": False})
        type_info.update({"{}.{}".format(package_name, type_name): type_detail})

def gather_type_info(file_path: str,
                     type_info: dict,
                     common_def: CommonDef,
                     config: dict,
                     category: CommonDef.M_TYPE) -> None:
    """
    Gather ProtoBuffer type information in a file.

    Args:
        file_path: Relative path of the proto type message file
        type_info: Type info dictionary
        common_def: Common definitions
        config: Configurations
        category: Whether for class messages or interface messages

    Returns:
        None
    """
    if CommonDef.M_TYPE.CLASS == category:
        gather_class_type_info(file_path, type_info,
                               common_def, config)
    elif CommonDef.M_TYPE.INTERFACE == category:
        gather_interface_type_info(file_path, type_info,
                                   common_def, config)

def gather_method_param_info(fd: TextIOWrapper,
                             file_path: str,
                             parent_name: str,
                             common_def: CommonDef,
                             config: dict,
                             message_type: str,
                             method_base_name: str) -> dict:
    """
    Gather interface method parameters info.

    Args:
        fd: File descriptor
        file_path: Proto file path
        parent_name: Parent message name, normally the name of the interface
        common_def: Common definitions
        config: Configurations
        message_type: Message type for which contains this field
        method_base_name: Method base name for which contains this entry

    Returns:
        (params, empty_cfm): A tuple containing a parameter dictionary
        and whether this should be an empty Cfm message.
    """
    params = {}
    while True:
        line = fd.readline().strip()
        if "}" == line[0]:
            break
        elif "{" == line[0]:
            continue
        line_entries = line.split(" ")
        repeated = False
        optional = False
        # For field of interface message
        if "optional" == line_entries[0]:
            param_type = line_entries[1]
            param_name = line_entries[2]
            optional = True
        elif "repeated" == line_entries[0]:
            param_type = line_entries[1]
            param_name = line_entries[2]
            repeated = True
        else:
            param_type = line_entries[0]
            param_name = line_entries[1]
        # Whether this type is defined in this interface
        (param_type, cached_type) = \
            deduce_type_name(param_type, parent_name, "", common_def)
        if CommonDef.INT32_TYPE == param_type:
            param_type = update_int32_type(file_path, line.strip(),
                                           message_type, method_base_name, config)
        param_dict = {"type": param_type,
                      "repeated": repeated,
                      "nullable": optional}
        if "bytes" == param_type:
            (bytes_info, optional) = \
                gather_array_implementation_info(file_path, line.strip(),
                                                 message_type,
                                                 method_base_name, config,
                                                 CommonDef.R_TYPE.BYTES)
            param_dict.update(bytes_info)
        if repeated:
            (repeated_info, optional) = \
                gather_array_implementation_info(file_path, line.strip(),
                                                 message_type,
                                                 method_base_name, config,
                                                 CommonDef.R_TYPE.CLASS)
            param_dict.update(repeated_info)
        if not check_interface_message(param_type, common_def):
            params.update({param_name: param_dict})
        else:
            if not check_callback(param_type, common_def):
                params.update({param_name: param_dict})
    return params

def gather_interface_info(file_path: str,
                          interface_info: dict,
                          type_info: dict,
                          common_def: CommonDef,
                          config: dict) -> None:
    """
    Gather interface methods information in given proto file.

    Args:
        file_path: Relative path of the proto interface message file
        interface_info: Interface info dictionary
        type_info: Class type info dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    # Strip interface name from interface file name
    interface_name = file_path.split("/")[-1].split("\\")[-1].split(".")[0]
    # Strip initial "I"
    interface_base_name = interface_name[1:]
    method_prefix = ""
    package_name = ""
    interface_fullname = ""
    for keyword in config["callbackKeywords"]:
        if keyword in interface_base_name:
            interface_base_name = interface_base_name[:-len(keyword)]
            common_def.callback_keywords.add(keyword)
            break
    methods = {}
    ind_message_count = 0
    # Retrieve interface wrapper message, if exist
    with open(file_path, "r", encoding=config["encoding"]) as f:
        while True:
            pos = f.tell()
            line = f.readline()
            if "package" == line.strip()[:len("package")]:
                # Strip ending ";"
                package_name = line.strip().split(" ")[1][:-1]
                if "" == common_def.package_prefix:
                    common_def.package_prefix = package_name + "."
                    # AIDL using & include prefix are handled 
                    # in previous AIDL differentiation section
                    common_def.proto_using_prefix = \
                        convert_package_name(package_name.lower() + ".")
                continue
            if CommonDef.MESSAGE == line[:len(CommonDef.MESSAGE)]:
                message_name = line.strip().split(" ")[1]
                # Interface child message
                if interface_name == message_name:
                    method_prefix = message_name + "_"
                    f.readline()
                    break
                # Direct messages, no wrapper interface message
                else:
                    f.seek(pos)
                    break

        interface_fullname = "{}.{}".format(package_name, interface_name)
        # Retrieve method info & parameters
        while True:
            pos = f.tell()
            line = f.readline()
            if not line:
                break
            if CommonDef.MESSAGE == line.strip()[:len(CommonDef.MESSAGE)]:
                message_name = line.strip()[len(CommonDef.MESSAGE) + 1:]
                # General class definition within interface
                method_base_name = message_name[:-3]
                method_type = message_name[-3:].lower()
                if config["requestSuffix"].lower() != method_type\
                    and config["indicateSuffix"].lower() != method_type\
                    and config["confirmSuffix"].lower() != method_type:
                    f.seek(pos)
                    gather_contained_type_info(f, file_path,
                                               interface_fullname,
                                               type_info, common_def,
                                               config, CommonDef.CLASS)
                # "req", "ind", "cfm" methods
                else:
                    if config["indicateSuffix"] == message_name[-3:]:
                        ind_message_count += 1
                    f.readline()
                    params = \
                        gather_method_param_info(f, file_path,
                                                 interface_fullname,
                                                 common_def, config,
                                                 method_type.capitalize(),
                                                 method_base_name)

                    methods.update({message_name:\
                                    {"baseName": method_base_name,
                                     "fullName": "{}{}".format(method_prefix,
                                                               message_name),
                                     "type": method_type,
                                     "params": params}})
                continue
            if CommonDef.ENUM == line.strip()[:len(CommonDef.ENUM)]:
                f.seek(pos)
                gather_contained_type_info(f,
                                           file_path,
                                           "{}.{}".format(package_name,
                                                          interface_name),
                                           type_info, common_def,
                                           config, CommonDef.ENUM)
    # "interfaceType" field is only used for ID definitions
    interface_type = config["requestSuffix"]
    if 0 != ind_message_count:
        interface_type = config["indicateSuffix"]
    interface_info.update({interface_fullname:\
                           {"interfaceBaseName": interface_base_name,\
                            "interfaceType": interface_type,\
                            "methods": methods}})

def gather_common_status_info(path_proto: str,
                              interface_info: dict,
                              common_def: CommonDef,
                              config: dict) -> None:
    """
    Collect common status structure as a method used for serialization and parsing.

    Args:
        path_proto: Proto path of the status structure
        interface_info: Interface info dictionary
        common_def: Common definitions
        config: Configurations

    Returns:
        None
    """
    file_path = os.path.join(path_proto, config["statusKeyword"] + config["protoSuffix"])
    package_name = common_def.package_prefix[:-1]
    interface_name = "{}{}{}"\
                     .format(common_def.base_intf,
                             config["interfaceFileNameSuffix"].capitalize(),
                             "Common")
    interface_fullname = "{}.{}".format(package_name, interface_name)
    # Update common definitions
    common_def.msg_common_interface_name = interface_fullname
    common_def.common_status_full_name = common_def.package_prefix + config["statusKeyword"]
    methods = {}
    with open(file_path, "r", encoding=config["encoding"]) as f:
        while True:
            line = f.readline()
            if not line:
                break
            if CommonDef.MESSAGE == line[:len(CommonDef.MESSAGE)]:
                message_name = line.strip().split(" ")[-1]
                method_base_name = message_name
                f.readline()
                params = \
                    gather_method_param_info(f, file_path, package_name,
                                             common_def, config, "InterfaceCfm", "")
                method_type = config["confirmSuffix"].lower()
                methods.update({message_name:{"baseName": method_base_name,
                                              "fullName": "{}.{}"\
                                              .format(package_name, message_name),
                                              "type": method_type,
                                              "params": params}})
                continue
    interface_type = config["confirmSuffix"]
    interface_info.update({interface_fullname:\
                           {"interfaceBaseName": interface_name[1:],\
                            "interfaceType": interface_type,\
                            "methods": methods}})