#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""

from collections import deque

from commondef import CommonDef
from helper import *

class DeSerializer:
    def __init__(self, type_info: dict, interface_info: dict,
                 common_def: CommonDef, config: dict) -> None:
        self.type_info = type_info
        self.interface_info = interface_info
        self.common_def = common_def
        self.config = config
        self.used_parse_pb_type = set()
        self.used_parse_pb_message = set()

    def add_parse_method_declaration_regular(self, p_dict: dict) -> str:
        """
        Add regular field for parse method declaration.

        Args:
            p_dict: Parameter info dictionary

        Returns:
            String of parse field declaration.
        """
        section = ""
        p_type = p_dict["type"]
        p_type_last = p_type.split(".")[-1]
        field_type = ""
        optional_begin = ""
        optional_end = ""
        if p_dict["nullable"]:
            if not check_interface_message(p_type, self.common_def):
                optional_begin = "std::optional<"
                optional_end = ">"
        if p_type in self.interface_info.keys():
            field_type = CommonDef.TYPE_MAPPING_CPP[self.config["instanceIdProtoType"]]
        elif p_type in self.common_def.PRIMITIVE_TYPES:
            field_type = CommonDef.TYPE_MAPPING_CPP[p_type]
        elif "bytes" == p_type:
            field_type = deduce_bytes_type(p_dict, self.config)
        else:
            if self.type_info[p_type]["contained"]:
                field_type = "::".join(p_type.split(".")[-2:])
            else:
                field_type = p_type_last
        section = "{}{}{}& {}".format(optional_begin, field_type, optional_end,
                                      self.config["parseIntermediateVariableName"])
        return section

    def add_parse_method_declaration_array(self, p_dict: dict) -> str:
        """
        Add array field for parse method declaration.

        Args:
            p_dict: Parameter info dictionary

        Returns:
            String of parse field declaration.
        """
        section = ""
        p_type = p_dict["type"]
        field_type = ""
        if p_type in self.common_def.PRIMITIVE_TYPES:
            field_type = deduce_repeated_type(p_dict, CommonDef.TYPE_MAPPING_CPP[p_type])
        # Should not be bytes
        elif p_type not in self.common_def.PRIMITIVE_TYPES_WITH_BYTES:
            if self.type_info[p_type]["contained"]:
                type_name = "::".join(p_type.split(".")[-2:])
                field_type = deduce_repeated_type(p_dict, type_name)
            else:
                field_type = deduce_repeated_type(p_dict, p_type.split(".")[-1])
        section = "{}& {}".format(field_type, self.config["parseIntermediateVariableName"])
        return section

    def add_parse_method_declaration(self, i_base_name: str,
                                     m_name: str, m_info: dict,
                                     g_type: CommonDef.G_TYPE) -> str:
        """
        Generate parse method declaration.

        Args:
            i_base_name: Interface base name
            m_name: Method name
            m_info: Method information dictionary
            g_type: Generation type,
                    whether for header file or implementation file

        Returns:
            String of parse method declaration.
        """
        params = m_info["params"]
        param_field = ""
        interface_cfm = False
        # Typedef param
        if 1 < len(params):
            param_field = "{}{}{}& {}"\
                          .format(m_name,
                                  get_param_short_name(i_base_name,
                                                       self.common_def.package_prefix),
                                  self.config["typedefSuffix"],
                                  self.config["parseIntermediateVariableName"])
        # Only 1 param
        elif 1 == len(params):
            param_list = list(params.values())
            param_type = param_list[0]["type"]
            repeated = param_list[0]["repeated"]
            if param_type.split(".")[-1] == self.config["statusKeyword"]:
                interface_cfm = True
            else:
                if repeated:
                    param_field = \
                        self.add_parse_method_declaration_array(param_list[0])
                else:
                    param_field = \
                        self.add_parse_method_declaration_regular(param_list[0])
        method_ending = ""
        if CommonDef.G_TYPE.H == g_type:
            method_ending = ";\n\n"
        elif CommonDef.G_TYPE.CPP == g_type:
            method_ending = "\n{\n"

        param_section = "{} uint8_t* data, size_t length"\
                        .format(self.config["constKeyword"])
        declaration = ""
        if interface_cfm or (0 == len(params)):
            pass
        else:
            if CommonDef.G_TYPE.CPP == g_type:
                self.used_parse_pb_message.add(m_name)
            param_section += ", {}".format(param_field)
        declaration = "bool {}{}{}({}){}"\
                      .format(i_base_name,
                              self.config["parseMethodKeyword"],
                              m_name,
                              param_section,
                              method_ending)
        return declaration

    def add_header_parse_methods(self, i_base_name: str, i_methods: dict) -> str:
        """
        Add parse methods of the header file.

        Args:
            i_base_name: Interface base name
            i_methods: A method dictionary

        Returns:
            String of parse method declaration section.
        """
        self.used_parse_pb_message.clear()
        section = ""
        for (method_name, method_info) in i_methods.items():
            method_section = \
                self.add_parse_method_declaration(i_base_name,
                                                  method_name, method_info,
                                                  CommonDef.G_TYPE.H)
            section += method_section
        return section.strip()

    def add_parse_top_level_typedef_regular_entry(self, v_name: str, v_dict: dict,
                                                  stack: deque,
                                                  p_name: str, tab_level: str) -> str:
        """
        Add a top-level typedef struct field entry.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: typedef variable name
            tab_level: Tab indention level

        Returns:
            Top-level typedef entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        field_type = ""
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{}.{} = msg.{}();\n"\
                       .format(tab_level, p_name, v_name, v_name.lower())
        elif "bytes" == v_type:
            section += "{}{}(msg.{}(), {}.{});\n"\
                       .format(tab_level,
                               deduce_string_handling_name(v_dict, self.config,
                                                           CommonDef.H_TYPE.DE_SERIALIZE),
                               v_name.lower(), p_name, v_name)
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_parse_pb_type.add(v_type)
            if self.type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_last
            section += "{}{}.{} = ({}) msg.{}();\n"\
                       .format(tab_level, p_name, v_name,
                               field_type, v_name.lower())
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_parse_pb_type, True)
            field_type = v_last
            section += "{}Message2{}(msg.{}(), {}.{});\n"\
                       .format(tab_level, field_type,
                               v_name.lower(), p_name, v_name)
        return section

    def add_parse_top_level_typedef_repeated_entry(self, v_name: str, v_dict: dict,
                                                   stack: deque, p_name: str,
                                                   tab_level: str) -> str:
        """
        Add entry for a top-level typedef struct field.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: typedef variable name
            tab_level: Tab indention level

        Returns:
            Top-level typedef entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        field_type = ""
        size_str = "size_" + v_name.lower()
        section += "{}int {} = msg.{}_size();\n"\
                   .format(tab_level, size_str, v_name.lower())
        section += "{}if (0 < {})\n{}{{\n".format(tab_level, size_str, tab_level)
        if CommonDef.VECTOR == v_dict["aidlType"]:
            section += "{}{}.{}.resize({});\n"\
                       .format(tab_level + self.common_def.TAB,
                               p_name, v_name, size_str)
        elif CommonDef.ARRAY == v_dict["aidlType"]:
            pass
        section += "{}for (int index = 0; index < {}; index++)\n{}{{\n"\
                   .format(tab_level + self.common_def.TAB, size_str,
                           tab_level + self.common_def.TAB)
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{}.{}[index] = msg.{}(index);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               p_name, v_name, v_name.lower())
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_parse_pb_type.add(v_type)
            if self.type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_last
            section += "{}{}.{}[index] = ({}) msg.{}(index);\n"\
                       .format(tab_level + self.common_def.TAB * 2, p_name, v_name,
                               field_type, v_name.lower())
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_parse_pb_type, True)
            field_type = v_last
            section += "{}Message2{}(msg.{}(index), {}.{}[index]);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               field_type,
                               v_name.lower(), p_name, v_name)
        section += "{}}}\n{}}}\n".format(tab_level + self.common_def.TAB, tab_level)
        return section

    def add_parse_top_level_regular_entry(self, v_name: str, v_dict: dict,
                                          stack: deque,
                                          p_name: str, tab_level: str) -> str:
        """
        Add entry for a regular top-level entry.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: Intermediate variable name
            tab_level: Tab indention level

        Returns:
            Top-level regular entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{} = msg.{}();\n".format(tab_level, p_name,
                                                   v_name.lower())
            if self.common_def.special_param:
                self.common_def.special_param = False
                section = "{}if (msg.has_{}())\n{}{{\n"\
                          .format(tab_level, v_name.lower(), tab_level)
                section += "{}{} = msg.{}();\n{}}}\n"\
                           .format(tab_level * 2, p_name, v_name.lower())
        elif "bytes" == v_type:
            section += "{}{}(msg.{}(), {});\n"\
                       .format(tab_level,
                               deduce_string_handling_name(v_dict, self.config,
                                                           CommonDef.H_TYPE.DE_SERIALIZE),
                               v_name.lower(),
                               self.config["parseIntermediateVariableName"])
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_parse_pb_type.add(v_type)
            field_type = ""
            if self.type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_last
            section += "{}{} = ({}) msg.{}();\n"\
                       .format(tab_level, p_name,
                               field_type, v_name.lower())
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_parse_pb_type, True)
            field_type = ""
            param_usage = ""
            field_type = v_last
            param_usage = p_name
            section += "{}Message2{}(msg.{}(), {});\n"\
                       .format(tab_level, field_type,
                               v_name.lower(), param_usage)
        return section

    def add_parse_top_level_repeated_entry(self, v_name: str, v_dict: dict,
                                           stack: deque,
                                           p_name: str, tab_level: str) -> str:
        """
        Add entry for an array top-level entry. i.e., v_name itself is an array.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: Parent name
            tab_level: Tab indention level

        Returns:
            Top-level array entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        size_str = "size_" + v_name.lower()
        section += "{}int {} = msg.{}_size();\n"\
                   .format(tab_level, size_str, v_name.lower())
        section += "{}if (0 < {})\n{}{{\n"\
                   .format(tab_level, size_str, tab_level)
        if CommonDef.VECTOR == v_dict["aidlType"]:
            section += "{}{}.resize({});\n"\
                       .format(tab_level + self.common_def.TAB,
                               p_name, size_str)
        elif CommonDef.ARRAY == v_dict["aidlType"]:
            pass
        section += "{}for (int index = 0; index < {}; index++)\n{}{{\n"\
                   .format(tab_level + self.common_def.TAB,
                           size_str, tab_level + self.common_def.TAB)
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{}[index] = msg.{}(index);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               p_name, v_name.lower())
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_parse_pb_type.add(v_type)
            field_type = ""
            if self.type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_last
            section += "{}{}[index] = ({}) msg.{}(index);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               p_name, field_type, v_name.lower())
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_parse_pb_type, True)
            field_type = ""
            param_usage = ""
            field_type = v_last
            param_usage = p_name
            section += "{}Message2{}(msg.{}(index), {}[index]);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               field_type,
                               v_name.lower(), param_usage)
        section += "{}}}\n{}}}\n"\
                   .format(tab_level + self.common_def.TAB, tab_level)
        return section

    def add_parse_regular_entry(self, i_base_name: str,
                                v_name: str, v_dict: dict, stack: deque,
                                p_name: str, p_type: str, tab_level: str) -> str:
        """
        Add one entry for a regular field.

        Args:
            i_base_name: Interface base name
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods)
            p_name: Parent class type (Only for helper methods)
            tab_level: Tab indention level

        Returns:
            Regular entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        # v_type would be primitive types, but parent type is interface class
        if p_type in self.interface_info.keys():
            section += "{}{} = msg{}.{}();\n"\
                       .format(tab_level, p_name, i_base_name, v_name.lower())
        elif v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{}.{} = msg{}.{}();\n"\
                       .format(tab_level, p_name, v_name, i_base_name, v_name.lower())
        elif "bytes" == v_type:
            # Note: As Proto lacks an "has_" method to check whether the parent message actually contains a bytes field,
            # the general accessor will always return a valid string reference.
            # If the bytes field is indeed not serialized and wired, the accessor will return an empty string of length 0, but not NULL.
            # It is up to the caller to perform validity check.
            if v_dict["nullable"]:
                section = "{}{} {};\n"\
                          .format(tab_level,
                                  deduce_bytes_type(v_dict, self.config),
                                  self.config["parseIntermediateVariableName"])
                section += "{}{}(msg{}.{}(), {});\n"\
                           .format(tab_level,
                                   deduce_string_handling_name(v_dict, self.config,
                                                               CommonDef.H_TYPE.DE_SERIALIZE),
                                                               i_base_name, v_name.lower(),
                                                               self.config["parseIntermediateVariableName"])
                section += "{}{}.{} = {};\n"\
                           .format(tab_level, p_name, v_name,
                                   self.config["parseIntermediateVariableName"])
            else:
                section += "{}{}(msg{}.{}(), {}.{});\n"\
                       .format(tab_level,
                               deduce_string_handling_name(v_dict, self.config,
                                                           CommonDef.H_TYPE.DE_SERIALIZE),
                               i_base_name, v_name.lower(), p_name, v_name)
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_parse_pb_type.add(v_type)
            field_type = v_last
            if self.type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            section += "{}{}.{} = ({}) msg{}.{}();\n"\
                       .format(tab_level, p_name, v_name,
                               field_type,
                               i_base_name, v_name.lower())
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_parse_pb_type, False)
            field_type = v_last
            section += "{}Message2{}(msg{}.{}(), {}.{});\n"\
                       .format(tab_level, field_type,
                               i_base_name, v_name.lower(), p_name, v_name)
        return section

    def add_parse_repeated_entry(self, i_base_name: str,
                                 v_name: str, v_dict: dict, stack: deque,
                                 p_name: str, tab_level: str) -> str:
        """
        Add one entry for a repeated field.

        Args:
            i_base_name: Interface base name
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods)
            tab_level: Tab indention level

        Returns:
            Entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        size_str = "size_" + v_name.lower()
        section += "{}int {} = msg{}.{}_size();\n"\
                   .format(tab_level, size_str, i_base_name, v_name.lower())
        section += "{}if (0 < {})\n{}{{\n".format(tab_level, size_str, tab_level)
        if CommonDef.VECTOR == v_dict["aidlType"]:
            section += "{}{}.{}.resize({});\n"\
                       .format(tab_level + self.common_def.TAB,
                               p_name, v_name, size_str)
        elif CommonDef.ARRAY == v_dict["aidlType"]:
            pass
        section += "{}for (int index = 0; index < {}; index++)\n{}{{\n"\
                   .format(tab_level + self.common_def.TAB, size_str,
                           tab_level + self.common_def.TAB)
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{}.{}[index] = msg{}.{}(index);\n{}}}\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               p_name, v_name,
                               i_base_name, v_name.lower(), tab_level + self.common_def.TAB)
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_parse_pb_type.add(v_type)
            field_type = ""
            if self.type_info[v_type]["contained"]:
                field_type = "::".join(v_type.split(".")[-2:])
            else:
                field_type = v_last
            section += "{}{}.{}[index] = ({}) msg{}.{}(index);\n{}}}\n"\
                       .format(tab_level + self.common_def.TAB * 2, p_name, v_name,
                               field_type,
                               i_base_name, v_name.lower(), tab_level + self.common_def.TAB)
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_parse_pb_type, False)
            field_type = v_last
            # nullable T[] or List<T> are mapped as
            # std::optional<std::vector<std::optional<T>>>
            # wrapped in a double-optional structure
            if v_dict["nullable"]:
                section = "{}int size_{} = msg{}.{}_size();\n"\
                          .format(tab_level, v_name.lower(), i_base_name, v_name.lower())
                section += "{}if (0 < size_{})\n{}{{\n"\
                           .format(tab_level, v_name.lower(), tab_level)
                temp_vector = self.config["parseIntermediateVariableName"] + i_base_name
                section += "{}vector<std::optional<{}>> {};\n"\
                           .format(tab_level * 2, field_type, temp_vector)
                section += "{}{}.resize(size_{});\n"\
                           .format(tab_level * 2, temp_vector, v_name.lower())
                section += "{}for (int index = 0; index < size_{}; index++)\n{}{{\n"\
                           .format(tab_level * 2, v_name.lower(), tab_level * 2)
                temp_name = v_name.lower() + self.config["parseIntermediateVariableName"]
                section += "{}{} {};\n".format(tab_level * 3, field_type, temp_name)
                section += "{}Message2{}(msg{}.{}(index), {});\n"\
                           .format(tab_level * 3, field_type, i_base_name, v_name.lower(), temp_name)
                section += "{}{}[index] = {};\n{}}}\n"\
                           .format(tab_level * 3, temp_vector, temp_name, tab_level * 2)
                section += "{}{}.{} = {};\n".format(tab_level * 2, p_name, v_name, temp_vector)
            else:
                section += "{}Message2{}(msg{}.{}(index), {}.{}[index]);\n{}}}\n"\
                           .format(tab_level + self.common_def.TAB * 2,
                                   field_type,
                                   i_base_name, v_name.lower(), p_name, v_name,
                                   tab_level + self.common_def.TAB)
        section += "{}}}\n".format(tab_level)
        return section

    def add_parse_top_level_typedef_entries(self, v_name: str, v_dict: dict,
                                            stack: deque, p_name: str,
                                            tab_level: str,
                                            repeated: bool) -> str:
        """
        Add top-level typedef entries

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods).
                   Mutually exclusive with top_level
            tab_level: Tab indention level
            repeated: Whether this field is repeated or not

        Returns:
            Entry content.
        """
        section = ""
        if repeated:
            section += \
                self.add_parse_top_level_typedef_repeated_entry(v_name, v_dict,
                                                                stack, p_name,
                                                                tab_level)
        else:
            section += \
                self.add_parse_top_level_typedef_regular_entry(v_name, v_dict,
                                                               stack, p_name,
                                                               tab_level)
        return section

    def add_parse_top_level_entries(self, v_name: str, v_dict: dict,
                                    stack: deque, p_name: str,
                                    tab_level: str, repeated: bool) -> str:
        """
        Add top-level non-typedef entries

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods).
                   Mutually exclusive with top_level
            tab_level: Tab indention level
            repeated: Whether this field is repeated or not

        Returns:
            Entry content.
        """
        section = ""
        if repeated:
            section += \
                self.add_parse_top_level_repeated_entry(v_name, v_dict,
                                                        stack, p_name,
                                                        tab_level)
        else:
            section += \
                self.add_parse_top_level_regular_entry(v_name, v_dict,
                                                       stack, p_name,
                                                       tab_level)
        return section

    def add_parse_entry(self, i_base_name: str, v_name: str,
                        v_type_dict: dict, top_level: bool, stack: deque,
                        p_name: str, p_type: str, type_def: bool) -> str:
        """
        Add one entry for the given variable name and variable type.

        Args:
            i_base_name: Interface base name
            v_name: Variable name
            v_type_dict: Variable type dictionary
            top_level: Whether this entry belongs to top-level
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods).
                   Mutually exclusive with top_level
            p_name: Parent class type (Only for helper methods)
            type_def: Whether this entry is a new struct containing multiple sub fields

        Returns:
            Entry content.
        """
        section = ""
        repeated = v_type_dict["repeated"]
        tab_level = ""
        # Top level entries
        if top_level:
            tab_level = self.common_def.TAB * 2
            if type_def:
                section += \
                    self.add_parse_top_level_typedef_entries(v_name,
                                                             v_type_dict,
                                                             stack, p_name,
                                                             tab_level, repeated)
            else:
                section += \
                    self.add_parse_top_level_entries(v_name, v_type_dict,
                                                     stack,
                                                     self.config["parseIntermediateVariableName"],
                                                     tab_level, repeated)
        # Entry for helper methods
        else:
            tab_level = self.common_def.TAB
            if repeated:
                section += self.add_parse_repeated_entry(i_base_name,
                                                         v_name, v_type_dict, stack,
                                                         p_name, tab_level)
            else:
                section += self.add_parse_regular_entry(i_base_name,
                                                        v_name, v_type_dict, stack,
                                                        p_name, p_type, tab_level)
        return section

    def add_parse_method_body(self, i_base_name: str,
                              m_name: str, m_info: dict, stack: deque) -> str:
        """
        Add actual parse method body. Top entry interface.

        Args:
            i_base_name: Interface base name
            m_name: Parse method name
            m_info: Corresponding method info
            stack: Proto variable type stack

        Returns:
            Method body of the parse method.
        """
        tab_level = self.common_def.TAB
        if m_name == self.config["statusKeyword"]:
            m_name += self.config["protoClassVariant"]
        section = "{}{} msg;\n".format(tab_level, m_name)
        interface_cfm = False
        type_def = False
        parent_type_name = ""
        section += "{}if ({}(data, length, &msg))\n{}{{\n"\
                   .format(tab_level, self.config["protoParseKeyword"], tab_level)

        if 1 < len(m_info["params"]):
            type_def = True
            parent_type_name = self.config["parseIntermediateVariableName"]

        for (param_name, param_info) in m_info["params"].items():
            param_type = param_info["type"]
            if self.common_def.special_method:
                if self.common_def.special_param_name == param_name:
                    self.common_def.special_param = True
            if param_type.split(".")[-1] == self.config["statusKeyword"]:
                interface_cfm = True
            section += \
                self.add_parse_entry(i_base_name,
                                     param_name, param_info, True,
                                     stack, parent_type_name, "", type_def)
        section += "{}return true;\n{}}}\n".format(tab_level * 2, tab_level)
        section += "{}return false;\n".format(tab_level)
        # Discard empty message or message only containing interface Cfm
        if (0 == len(m_info["params"])) or \
           (1 == len(m_info["params"]) and interface_cfm):
            section = "{}return {};\n".format(tab_level,
                                              str(self.config["booleanDefault"]).lower())
        return section

    def add_parse_union_entry(self, i_base_name: str,
                              v_type: str, v_name: str, v_dict: dict,
                              stack: deque) -> str:
        """
        Add case switches for AIDL "union" type.
        This should only be used for helper methods, thus never be top-level.

        Args:
            i_base_name: Interface base name
            v_type: Union type name
            v_name: Union variable name (Only for helper methods)
            v_dict: Union type dictionary
            stack: Proto variable type stack

        Returns:
            Full case switches for a union type.
        """
        section = ""
        type_ref = v_type.split(".")[-1]
        if self.type_info[v_type]["contained"]:
                type_ref = "_".join(v_type.split(".")[-2:])
        tab_level = self.common_def.TAB
        oneof_name = v_dict["oneofName"]
        section += "{}switch (msg{}.{}_case())\n{}{{\n"\
                   .format(tab_level, i_base_name, v_dict["oneofName"], tab_level)
        for (elem_name, elem_type) in v_dict["fields"].items():
            elem_last = elem_type.split(".")[-1]
            section += "{}case {}{}::{}Case::{}:\n"\
                       .format(tab_level * 2, type_ref, self.config["protoClassVariant"],
                               oneof_name,
                               "k{}{}".format(elem_name[0].upper(), elem_name[1:]))
            if elem_type in self.common_def.PRIMITIVE_TYPES:
                section += "{}{}.set<{}::{}>(msg{}.{}());\n"\
                           .format(tab_level * 3, v_name, type_ref,
                                   elem_name, i_base_name, elem_name.lower())
            # May need to handle bytes
            elif "bytes" == elem_type:
                pass
            elif CommonDef.ENUM == self.type_info[elem_type]["typeCategory"]:
                self.used_parse_pb_type.add(elem_type)
                field_type = elem_last
                if self.type_info[elem_type]["contained"]:
                    field_type = "::".join(elem_type.split(".")[-2:])
                section += "{}{}.set<{}::{}>(({}) msg{}.{}());\n"\
                           .format(tab_level *3, v_name, type_ref,
                                   elem_name, field_type,
                                   i_base_name, elem_name.lower())
            elif CommonDef.CLASS == self.type_info[elem_type]["typeCategory"] or \
                 CommonDef.UNION == self.type_info[elem_type]["typeCategory"]:
                check_used(elem_name, elem_type, stack,
                           self.used_parse_pb_type, False)
                field_type = elem_last
                if self.type_info[elem_type]["contained"]:
                    field_type = "::".join(elem_type.split(".")[-2:])
                variable_name = self.config["parseIntermediateVariableName"]
                section += "{}{} {};\n"\
                           .format(tab_level * 3, field_type, variable_name)
                section += "{}Message2{}(msg{}.{}(), {});\n"\
                           .format(tab_level * 3, elem_last, i_base_name,
                                   elem_name.lower(), variable_name)
                section += "{}{}.set<{}::{}>({});\n"\
                           .format(tab_level * 3, v_name, type_ref,
                                   elem_name, variable_name)
            section += "{}break;\n".format(tab_level * 3)
        section += "{}default:\n{}break;\n".format(tab_level * 2, tab_level * 3)
        section += "{}}}\n".format(tab_level)
        return section

    def add_parse_helper_methods(self, i_base_name: str, stack: deque) -> str:
        """
        Add helper methods for used proto type.

        Args:
            i_base_name: Interface base name
            stack: Proto variable type stack

        Returns:
            Helper methods of used proto types.
        """
        section = ""
        helper_methods = deque()

        while 0 != len(stack):
            # First out
            param_entry = stack.popleft()
            p_name = param_entry["pName"]
            p_type = param_entry["pType"]
            p_top_level = param_entry["topLevel"]
            cpp_type_name = \
                summary_type_name(p_type, self.type_info,
                                  self.common_def, self.config)
            proto_sanitize_name = ""
            p_last = p_type.split(".")[-1]
            proto_sanitize_name = p_last + self.config["protoClassVariant"]
            field_type = ""
            param_fields = ""
            if p_type in self.interface_info.keys():
                field_type = \
                    CommonDef.TYPE_MAPPING_CPP[self.config["instanceIdProtoType"]]
            else:
                field_type = cpp_type_name
            param_fields = "{} {}& msg{}, {}& {}"\
                           .format(self.config["constKeyword"],
                                   proto_sanitize_name,
                                   i_base_name, field_type, p_name)
            if p_type == self.common_def.common_status_full_name:
                cpp_type_name = self.config["statusKeyword"]
            method_section = "static void Message2{}({})\n{{\n"\
                            .format(cpp_type_name.split("::")[-1],
                                    param_fields)
            if CommonDef.CLASS == self.type_info[p_type]["typeCategory"]:
                for f_name in self.type_info[p_type]["fields"]:
                    f_type_dict = self.type_info[p_type]["fields"][f_name]
                    method_section += \
                        self.add_parse_entry(i_base_name,
                                             f_name, f_type_dict, False,
                                             stack, p_name, p_type, False)
            elif CommonDef.UNION == self.type_info[p_type]["typeCategory"]:
                p_dict = self.type_info[p_type]
                method_section += \
                    self.add_parse_union_entry(i_base_name,
                                               p_type, p_name, p_dict, stack)
            method_section += "}"
            if p_type == self.common_def.common_status_full_name \
            and not self.config["sync"]:
                method_section = ""
            if p_top_level:
                helper_methods.append(method_section)
            else:
                helper_methods.appendleft(method_section)
        section = "\n\n".join(helper_methods)
        return section.strip()

    def add_parse_method(self, i_base_name: str,
                         m_name: str, m_info: dict) -> str:
        """
        Add parse method and belonging helper methods of
        the given interface method name and parameters.

        Args:
            i_base_name: Interface base name
            m_name: Interface method name
            m_info: A dictionary containing corresponding method info

        Returns:
            Main parse method body, and helper method(s) of the method.
        """
        section = ""
        if self.common_def.special_interface:
            if m_name == self.common_def.special_method_name:
                self.common_def.special_method = True
        method_declaration = \
            self.add_parse_method_declaration(i_base_name,
                                              m_name, m_info,
                                              CommonDef.G_TYPE.CPP)
        method_body = ""
        stack = deque()
        # Only handle non-empty and non interface cfm message
        method_body = \
            self.add_parse_method_body(i_base_name,
                                       m_name, m_info, stack)

        helper_methods = \
            self.add_parse_helper_methods(i_base_name, stack)
        # Method body does not contain trailing "}"
        if "" == method_declaration and "" == method_body:
            section = ""
            if "" != helper_methods:
                section = "{}\n\n".format(helper_methods)
        else:
            if "" != helper_methods:
                section = "{}\n\n{}{}}}\n\n"\
                          .format(helper_methods, method_declaration, method_body)
            else:
                section = "{}{}}}\n\n".format(method_declaration, method_body)
        return section

    def add_source_parse_methods(self, i_base_name: str, i_methods: dict) -> str:
        """
        Add parse methods of the implementation file.

        Args:
            i_base_name: Interface base name
            i_methods: A method dictionary of the corresponding interface

        Returns:
            String of the parse methods section.
        """
        section = ""
        self.used_parse_pb_type.clear()
        for (method_name, method_info) in i_methods.items():
            section += \
                self.add_parse_method(i_base_name, method_name, method_info)
            self.common_def.special_method = False
        return section.strip()