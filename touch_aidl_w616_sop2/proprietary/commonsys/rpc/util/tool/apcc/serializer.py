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

class Serializer:
    def __init__(self, type_info: dict, interface_info: dict,
                 common_def: CommonDef, config: dict) -> None:
        self.type_info = type_info
        self.interface_info = interface_info
        self.common_def = common_def
        self.config = config
        self.used_serialize_pb_message = set()
        self.used_serialize_pb_type = set()

    def add_serialize_method_declaration_regular(self, p_name: str,
                                                 p_dict: dict) -> str:
        """
        Add regular field for serialize method declaration.

        Args:
            p_name: Parameter name
            p_dict: Parameter info dictionary

        Returns:
            String of serialize field declaration.
        """
        p_type = p_dict["type"]
        section = ""
        p_type_last = p_type.split(".")[-1]
        field_type = ""
        optional_begin = ""
        optional_end = ""
        if p_dict["nullable"]:
            if not check_interface_message(p_type, self.common_def):
                optional_begin = "std::optional<"
                optional_end = ">"
        if p_type in self.interface_info.keys():
            field_type = self.common_def.TYPE_MAPPING_CPP[self.config["instanceIdProtoType"]]
        elif p_type in self.common_def.HEADER_PRIMITIVE_TYPES:
            field_type += optional_begin + CommonDef.TYPE_MAPPING_CPP[p_type] + optional_end
        elif "string" == p_type:
            field_type += "{} {}{}{}&"\
                          .format(self.config["constKeyword"], optional_begin,
                                  CommonDef.TYPE_MAPPING_CPP[p_type], optional_end)
        elif "bytes" == p_type:
            field_type += "{} {}{}{}&"\
                          .format(self.config["constKeyword"], optional_begin,
                                  deduce_bytes_type(p_dict, self.config), optional_end)
        else:
            type_category = self.type_info[p_type]["typeCategory"]
            if CommonDef.ENUM == type_category:
                if self.type_info[p_type]["contained"]:
                    field_type = "::".join(p_type.split(".")[-2:])
                else:
                    field_type = p_type_last
                field_type = "{}{}{}".format(optional_begin, field_type, optional_end)
            if CommonDef.CLASS == type_category or \
               CommonDef.UNION == type_category:
                # Interface Cfm message
                if p_type_last == self.config["statusKeyword"]:
                    field_type = "{} {}&".format(self.config["constKeyword"],
                                                 self.config["statusKeyword"] + \
                                                 self.config["typedefSuffix"])
                else:
                    type_name = p_type_last
                    if self.type_info[p_type]["contained"]:
                        type_name = "::".join(p_type.split(".")[-2:])
                    field_type += "{} {}{}{}&"\
                                  .format(self.config["constKeyword"], optional_begin,
                                          type_name, optional_end)
        section = "{} {}".format(field_type, p_name)
        return section

    def add_serialize_method_declaration_array(self, p_name: str,
                                               p_dict: dict) -> str:
        """
        Add array fields for method declaration.

        Args:
            p_name: Parameter name
            p_dict: Parameter info dictionary

        Returns:
            String of field declaration.
        """
        p_type = p_dict["type"]
        section = ""
        field_type = ""
        if p_name.split(".")[-1] == self.config["statusKeyword"]:
            field_type = "{}&"\
                .format("{}{}".format(self.config["statusKeyword"],
                                      self.config["typedefSuffix"]))
        else:
            if p_type in self.common_def.PRIMITIVE_TYPES:
                field_type = "{} {}&"\
                    .format(self.config["constKeyword"],
                            deduce_repeated_type(p_dict,
                                                 CommonDef.TYPE_MAPPING_CPP[p_type]))
            # Should not be bytes
            if p_type not in self.common_def.PRIMITIVE_TYPES_WITH_BYTES:
                if self.type_info[p_type]["contained"]:
                    type_name = "::".join(p_type.split(".")[-2:])
                    field_type = "{} {}&"\
                        .format(self.config["constKeyword"],
                                deduce_repeated_type(p_dict, type_name))
                else:
                    field_type = "{} {}&"\
                        .format(self.config["constKeyword"],
                                deduce_repeated_type(p_dict, p_type.split(".")[-1]))
        section = "{} {}".format(field_type, p_name)
        return section

    def add_serialize_method_declaration(self, i_base_name: str,
                                         m_name: str,
                                         m_info: dict,
                                         g_type: CommonDef.G_TYPE) -> str:
        """
        Add serialize method declarations.

        Args:
            i_base_name: Interface base name
            m_name: Method name
            m_info: Method information dictionary
            g_type: Whether for header file or implementation file

        Returns:
            String of method declaration.
        """
        param_list = []
        params = m_info["params"]
        param_field = ""
        interface_cfm = False
        for (param_name, param_dict) in params.items():
            if self.common_def.special_method:
                if param_name == self.common_def.special_param_name:
                    self.common_def.special_param = True
            param_type = param_dict["type"]
            repeated = param_dict["repeated"]
            if param_type.split(".")[-1] == self.config["statusKeyword"]:
                interface_cfm = True
            if repeated:
                param_list.append(self.add_serialize_method_declaration_array(param_name,
                                                                              param_dict))
            else:
                param_list.append(self.add_serialize_method_declaration_regular(param_name,
                                                                                param_dict))
        param_field = ", ".join(param_list)

        method_ending = ""
        if CommonDef.G_TYPE.H == g_type:
            method_ending = ";\n\n"
        elif CommonDef.G_TYPE.CPP == g_type:
            method_ending = "\n{\n"

        param_content = "vector<uint8_t>& payload"
        declaration = ""
        # Not including empty message or interface return
        if (0 == len(param_list)) or (1 == len(param_list) and interface_cfm):
            pass
        else:
            if CommonDef.G_TYPE.CPP == g_type:
                self.used_serialize_pb_message.add(m_name)
            param_content = "{}, ".format(param_field) + param_content
        declaration = "bool {}{}{}({}){}"\
                      .format(i_base_name,
                              self.config["serializeMethodKeyword"],
                              m_name,
                              param_content,
                              method_ending)
        return declaration

    def add_serialize_top_level_regular_entry(self, i_base_name: str,
                                              v_name: str,
                                              v_dict: dict,
                                              stack: deque,
                                              tab_level: str) -> str:
        """
        Add entry for a top-level regular field.

        Args:
            i_base_name: Interface base name
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            tab_level: Tab indention level

        Returns:
            Top-level regular entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}msg{}.set_{}({});\n"\
                       .format(tab_level, i_base_name, v_name.lower(), v_name)
            if self.common_def.special_param:
                self.common_def.special_param = False
                section = "{}if ({}.has_value())\n{}{{\n".format(tab_level, v_name, tab_level)
                section += "{}msg{}.set_{}({}.value());\n{}}}\n"\
                           .format(tab_level * 2, i_base_name, v_name.lower(), v_name, tab_level)
        elif "bytes" == v_type:
            name_str = v_name + self.config["stringSuffix"]
            section += "{}string {};\n".format(tab_level, name_str)
            section += "{}{}({}, {});\n"\
                       .format(tab_level,
                               deduce_string_handling_name(v_dict,
                                                           self.config,
                                                           CommonDef.H_TYPE.SERIALIZE),
                               v_name, name_str)
            section += "{}msg{}.set_{}({});\n"\
                       .format(tab_level, i_base_name, v_name.lower(), name_str)
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_serialize_pb_type.add(v_type)
            section += "{}msg{}.set_{}(({}) {});\n"\
                       .format(tab_level, i_base_name, v_name.lower(),
                               v_last + self.config["protoClassVariant"], v_name)
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_serialize_pb_type, True)
            assigned_type = ""
            reference_type = ""
            assigned_type = v_last + self.config["protoClassVariant"]
            reference_type = v_last
            section += "{}{}* {} = msg{}.mutable_{}();\n"\
                           .format(tab_level,
                                   assigned_type,
                                   v_name + self.config["protoClassVariant"].lower(),
                                   i_base_name, v_name.lower())
            section += "{}{}2Message({}, *{});\n"\
                       .format(tab_level, reference_type,
                               v_name,
                               v_name + self.config["protoClassVariant"].lower())
        return section

    def add_serialize_top_level_repeated_entry(self, i_base_name: str,
                                               v_name: str,
                                               v_type: str,
                                               stack: deque,
                                               tab_level: str) -> str:
        """
        Add entry for a top-level array field.

        Args:
            i_base_name: Interface base name
            v_name: Variable name
            v_type: Variable type
            stack: Proto variable type stack
            tab_level: Tab indention level

        Returns:
            Top-level array entry content.
        """
        section = ""
        v_last = v_type.split(".")[-1]
        size_str = "size_" + v_name.lower()
        section += "{}int {} = {}.size();\n".format(tab_level, size_str, v_name)
        section += "{}if (0 < {})\n{}{{\n".format(tab_level, size_str, tab_level)
        section += "{}for (int index = 0; index < {}; index++)\n{}{{\n"\
                   .format(tab_level + self.common_def.TAB,
                           size_str, tab_level + self.common_def.TAB)
        # Should not be bytes
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}msg{}.add_{}({}[index]);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               i_base_name, v_name.lower(), v_name)
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_serialize_pb_type.add(v_type)
            section += "{}msg{}.add_{}(({}) {}[index]);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               i_base_name, v_name.lower(),
                               v_last + self.config["protoClassVariant"], v_name)
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_serialize_pb_type, True)
            field_type = ""
            field_type = v_type.split(".")[-1] + self.config["protoClassVariant"]
            section += "{}{} *{} = msg{}.add_{}();\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               field_type,
                               v_name + self.config["protoClassVariant"].lower(),
                               i_base_name, v_name.lower())
            section += "{}{}2Message({}[index], *{});\n"\
                       .format(tab_level + self.common_def.TAB * 2, v_last, v_name,
                               v_name + self.config["protoClassVariant"].lower())
        section += "{}}}\n{}}}\n".format(tab_level + self.common_def.TAB, tab_level)
        return section

    def add_serialize_regular_entry(self, i_base_name: str,
                                    v_name: str, v_dict: dict,
                                    stack: deque,
                                    p_name: str, p_type: str,
                                    tab_level: str) -> str:
        """
        Add entry for a regular field.

        Args:
            i_base_name: Interface base name
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods)
            p_type: Parent class type (Only for helper methods)
            tab_level: Tab indention level

        Returns:
            Regular entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        # v_type would be primitive types, but parent type is interface class
        if p_type in self.interface_info.keys():
            section += "{}msg{}.set_{}({});\n"\
                       .format(tab_level, i_base_name, v_name.lower(), p_name)
        elif v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}msg{}.set_{}({}.{});\n"\
                       .format(tab_level, i_base_name, v_name.lower(), p_name, v_name)
        elif "bytes" == v_type:
            if v_dict["nullable"]:
                section += "{}if ({}.{}.has_value())\n{}{{\n"\
                           .format(tab_level, p_name, v_name, tab_level)
                section += "{}string {};\n".format(tab_level * 2, v_name)
                section += "{}{}({}.{}.value(), {});\n"\
                           .format(tab_level * 2,
                                   deduce_string_handling_name(v_dict, self.config,
                                                               CommonDef.H_TYPE.SERIALIZE),
                                   p_name, v_name, v_name)
                section += "{}msg{}.set_{}({});\n{}}}\n"\
                           .format(tab_level * 2, i_base_name, v_name.lower(), v_name, tab_level)
            else:
                section += "{}string {};\n".format(tab_level, v_name)
                section += "{}{}({}.{}, {});\n"\
                           .format(tab_level,
                                   deduce_string_handling_name(v_dict, self.config,
                                                               CommonDef.H_TYPE.SERIALIZE),
                                   p_name, v_name, v_name)
                section += "{}msg{}.set_{}({});\n"\
                           .format(tab_level, i_base_name, v_name.lower(), v_name)
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_serialize_pb_type.add(v_type)
            section += "{}msg{}.set_{}(({}) {}.{});\n"\
                       .format(tab_level, i_base_name, v_name.lower(),
                               v_last + self.config["protoClassVariant"],
                               p_name, v_name)
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_serialize_pb_type, False)
            reference_type = ""
            param_usage = ""
            field_type = v_last + self.config["protoClassVariant"]
            field_name = v_name + self.config["protoClassVariant"].lower()
            reference_type = v_last
            param_usage = "{}.{}".format(p_name, v_name)
            section += "{}{}* {} = msg{}.mutable_{}();\n"\
                           .format(tab_level, field_type,
                                   field_name,
                                   i_base_name, v_name.lower())
            section += "{}{}2Message({}, *{});\n"\
                       .format(tab_level, reference_type,
                               param_usage, field_name)
        return section

    def add_serialize_repeated_entry(self, i_base_name: str,
                                     v_name: str, v_type_dict: dict,
                                     stack: deque,
                                     p_name: str, tab_level: str) -> str:
        """
        Add entry for a repeated field.

        Args:
            i_base_name: Interface base name
            v_name: Variable name
            v_type_dict: Variable type dictionary
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods)
            tab_level: Tab indention level

        Returns:
            Array entry content.
        """
        section = ""
        v_type = v_type_dict["type"]
        v_last = v_type.split(".")[-1]
        size_str = "size_" + v_name.lower()
        section += "{}int {} = {}.{}.size();\n"\
                   .format(tab_level, size_str, p_name, v_name)
        section += "{}if (0 < {})\n{}{{\n".format(tab_level, size_str, tab_level)
        section += "{}for (int index = 0; index < {}; index++)\n{}{{\n"\
                   .format(tab_level + self.common_def.TAB,
                           size_str, tab_level + self.common_def.TAB)
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}msg{}.add_{}({}.{}[index]);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               i_base_name, v_name.lower(), p_name, v_name)
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            self.used_serialize_pb_type.add(v_type)
            section += "{}msg{}.add_{}(({}) {}.{}[index]);\n"\
                       .format(tab_level + self.common_def.TAB * 2,
                               i_base_name, v_name.lower(),
                               v_last + self.config["protoClassVariant"],
                               p_name, v_name)
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack,
                       self.used_serialize_pb_type, False)
            field_type = v_last + self.config["protoClassVariant"]
            field_name = v_name + self.config["protoClassVariant"].lower()
            if v_type_dict["nullable"]:
                section = "{}if ({}.{}.has_value())\n{}{{\n"\
                          .format(tab_level, p_name, v_name, tab_level)
                section += "{}int size_{} = {}.{}.value().size();\n"\
                           .format(tab_level * 2, v_name.lower(), p_name, v_name)
                section += "{}if (0 < size_{})\n{}{{\n"\
                           .format(tab_level * 2, v_name.lower(), tab_level * 2)
                section += "{}for (int index = 0; index < size_{}; index++)\n{}{{\n"\
                           .format(tab_level * 3, v_name.lower(), tab_level * 3)
                section += "{}{} *{} = msg{}.add_{}();\n"\
                           .format(tab_level * 4, field_type, field_name, i_base_name, v_name.lower())
                section += "{}{}2Message({}.{}.value()[index].value(), *{});\n"\
                           .format(tab_level * 4, v_last, p_name, v_name, field_name)
                section += "{}}}\n".format(tab_level * 3)
            else:
                section += "{}{} *{} = msg{}.add_{}();\n"\
                           .format(tab_level + self.common_def.TAB * 2,
                                   field_type,
                                   field_name,
                                   i_base_name, v_name.lower())
                section += "{}{}2Message({}.{}[index], *{});\n"\
                           .format(tab_level + self.common_def.TAB * 2, v_last, p_name,
                                   v_name, field_name)

        section += "{}}}\n{}}}\n".format(tab_level + self.common_def.TAB, tab_level)
        return section

    def add_serialize_entry(self, i_base_name: str,
                            v_name: str, v_type_dict: dict,
                            top_level: bool, stack: deque,
                            p_name: str, p_type: str) -> str:
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
            p_type: Parent class type (Only for helper methods).

        Returns:
            Entry content.
        """
        section = ""
        repeated = v_type_dict["repeated"]
        v_type = v_type_dict["type"]
        tab_level = self.common_def.TAB
        # Entry for top-level interface
        if top_level:
            if repeated:
                section += \
                    self.add_serialize_top_level_repeated_entry(i_base_name,
                                                                v_name, v_type,
                                                                stack, tab_level)
            else:
                section += \
                    self.add_serialize_top_level_regular_entry(i_base_name,
                                                               v_name, v_type_dict,
                                                               stack, tab_level)
        # Entry for helper methods
        else:
            if repeated:
                section += \
                    self.add_serialize_repeated_entry(i_base_name, v_name,
                                                      v_type_dict, stack, p_name, tab_level)
            else:
                section = \
                    self.add_serialize_regular_entry(i_base_name, v_name,
                                                     v_type_dict, stack,
                                                     p_name, p_type, tab_level)
        return section

    def add_serialize_union_entry(self, i_base_name: str,
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
            type_ref = "::".join(v_type.split(".")[-2:])
        tab_level = self.common_def.TAB
        section += "{}switch({}.{}())\n{}{{\n"\
                   .format(tab_level, v_name,
                           self.config["aidlUnionGetTagKeyword"], tab_level)
        for (elem_name, elem_type) in v_dict["fields"].items():
            elem_last = elem_type.split(".")[-1]
            section += "{}case {}::{}:\n".format(tab_level *2, type_ref, elem_name)
            if elem_type in self.common_def.PRIMITIVE_TYPES:
                section += "{}msg{}.set_{}({}.get<{}::{}>());\n"\
                           .format(tab_level * 3, i_base_name,
                                   elem_name.lower(), v_name, type_ref, elem_name)
            # May need to handle byte(s)
            elif "bytes" == elem_type:
                pass
            elif CommonDef.ENUM == self.type_info[elem_type]["typeCategory"]:
                self.used_serialize_pb_type.add(elem_type)
                section += "{}msg{}.set_{}(({}) {}.get<{}::{}>());\n"\
                           .format(tab_level * 3, i_base_name,
                                   elem_name.lower(),
                                   elem_last + self.config["protoClassVariant"],
                                   v_name, type_ref, elem_name)
            elif CommonDef.CLASS == self.type_info[elem_type]["typeCategory"] or \
                 CommonDef.UNION == self.type_info[elem_type]["typeCategory"]:
                check_used(elem_name, elem_type, stack,
                           self.used_serialize_pb_type, False)
                field_type = elem_last + self.config["protoClassVariant"]
                field_name = elem_name + self.config["protoClassVariant"].lower()
                reference_type = elem_last
                section += "{}{}* {} = msg{}.mutable_{}();\n"\
                           .format(tab_level * 3, field_type,
                                   field_name,
                                   i_base_name, elem_name.lower())
                section += "{}{}2Message({}.get<{}::{}>(), *{});\n"\
                           .format(tab_level * 3, reference_type,
                                   v_name, type_ref, elem_name, field_name)
            section += "{}break;\n".format(tab_level * 3)
        section += "{}}}\n".format(tab_level)
        return section  

    def add_serialize_helper_methods(self, i_base_name: str, stack: deque) -> str:
        """
        Add helper methods for used proto type.

        Args:
            i_base_name: Interface base name
            stack: Proto variable type stack

        Returns:
            Helper methods to fill used proto types.
        """
        helper_methods = deque()
        while 0 != len(stack):
            # First out
            param_entry = stack.popleft()
            p_name = param_entry["pName"]
            p_type = param_entry["pType"]
            p_top_level = param_entry["topLevel"]
            cpp_type_name = \
                summary_type_name(p_type, self.type_info, self.common_def, self.config)
            proto_sanitize_name = ""
            p_last = p_type.split(".")[-1]
            proto_sanitize_name = p_last + self.config["protoClassVariant"]
            param_fields = ""
            if p_type in self.interface_info.keys():
                param_fields = "{} {}, {}& msg{}"\
                               .format(CommonDef.TYPE_MAPPING_CPP[self.config["instanceIdProtoType"]],
                                       p_name,
                                       proto_sanitize_name,
                                       i_base_name)
            else:
                param_fields = "{} {}& {}, {}& msg{}"\
                               .format(self.config["constKeyword"],
                                       cpp_type_name,
                                       p_name,
                                       proto_sanitize_name,
                                       i_base_name)
            if p_type == self.common_def.common_status_full_name:
                cpp_type_name = self.config["statusKeyword"]
            method_section = "static void {}2Message({})\n{{\n"\
                             .format(cpp_type_name.split("::")[-1], param_fields)
            if CommonDef.CLASS == self.type_info[p_type]["typeCategory"]:
                for f_name in self.type_info[p_type]["fields"]:
                    f_type_dict = self.type_info[p_type]["fields"][f_name]
                    method_section += self.add_serialize_entry(i_base_name,
                                                               f_name, f_type_dict,
                                                               False, stack,
                                                               p_name, p_type)
            elif CommonDef.UNION == self.type_info[p_type]["typeCategory"]:
                p_dict = self.type_info[p_type]
                method_section += self.add_serialize_union_entry(i_base_name,
                                                                 p_type, p_name,
                                                                 p_dict, stack)
            method_section += "}"
            if p_type == self.common_def.common_status_full_name \
            and not self.config["sync"]:
                method_section = ""
            # Follow a first-call-first-appear order
            if p_top_level:
                helper_methods.append(method_section)
            else:
                helper_methods.appendleft(method_section)
        section = "\n\n".join(helper_methods)
        return section.strip()

    def add_serialize_method_body(self, i_base_name: str,
                                  m_name: str, m_params: dict, stack: deque) -> str:
        """
        Add actual serialize method body. Top entry interface.

        Args:
            i_base_name: Interface base name
            m_name: Serialize method name
            m_params: Corresponding parameters of the method
            stack: Proto variable type stack

        Returns:
            Method body of the serialize method.
        """
        section = ""
        interface_cfm = False
        if m_name == self.config["statusKeyword"]:
            m_name += self.config["protoClassVariant"]
        section += "{}{} msg{};\n".format(self.common_def.TAB, m_name, i_base_name)
        for (param_name, param_info) in m_params.items():
            if self.common_def.special_method:
                if param_name == self.common_def.special_param_name:
                    self.common_def.special_param = True
            param_type = param_info["type"]
            if param_type.split(".")[-1] == self.config["statusKeyword"]:
                interface_cfm = True
            section += self.add_serialize_entry(i_base_name,
                                                param_name, param_info,
                                                True, stack, "", "")
        section += "{}return {}(&msg{}, payload);\n"\
                   .format(self.common_def.TAB,
                           self.config["protoSerializeKeyword"], i_base_name)
        section += "}"

        # Not handling empty message or messages containing only interface Cfm
        if (0 == len(m_params)) or (1 == len(m_params) and interface_cfm):
            section = "{}return {};\n}}".format(self.common_def.TAB,
                                                str(self.config["booleanDefault"]).lower())
        return section

    def add_source_serialize_method(self, i_base_name: str,
                                    m_name: str, m_info: dict) -> str:
        """
        Add serialize method, and belonging helper methods
        of the given interface method name and parameters.

        Args:
            i_base_name: Interface base name
            m_name: Method name
            m_info: A dictionary containing corresponding method info

        Returns:
            Main serialize method and every helper methods of this serialize method.
        """
        # Helper methods, method_declaration+{, method body + }
        section = ""
        if self.common_def.special_interface:
            m_name_base = m_name[:-3]
            if m_name_base.lower() == self.common_def.special_method_name.lower():
                self.common_def.special_method = True
        method_declaration = \
            self.add_serialize_method_declaration(i_base_name,
                                                  m_name, m_info,
                                                  CommonDef.G_TYPE.CPP)
        method_body = ""
        stack = deque()
        # Only handle non-empty and non interface cfm message
        method_body += \
            self.add_serialize_method_body(i_base_name,
                                           m_name, m_info["params"], stack)

        helper_methods = \
            self.add_serialize_helper_methods(i_base_name, stack)
        # Discard empty message or messages only having interface Cfm
        if "" == method_declaration and "" == method_body:
            section = ""
            if "" != helper_methods:
                section = "{}\n\n".format(helper_methods)
        else:
            if "" != helper_methods:
                section = "{}\n\n{}{}\n\n"\
                          .format(helper_methods, method_declaration, method_body)
            else:
                section = "{}{}\n\n".format(method_declaration, method_body)
        return section

    def add_header_serialize_methods(self, i_base_name: str, i_methods: dict) -> str:
        """
        Add serialize methods of the header file.

        Args:
            i_base_name: Interface base name
            i_methods: A method dictionary

        Returns:
            String of serialize method declaration section.
        """
        self.used_serialize_pb_message.clear()
        section = ""
        for (method_name, method_info) in i_methods.items():
            method_section = \
                self.add_serialize_method_declaration(i_base_name,
                                                      method_name,
                                                      method_info,
                                                      CommonDef.G_TYPE.H)
            section += method_section
        return section

    def add_source_serialize_methods(self, i_base_name: str, i_methods: dict) -> str:
        """
        Add serialize methods of the implementation file.

        Args:
            i_base_name: Interface base name
            i_methods: A method dictionary

        Returns:
            String of the serialize methods section.
        """
        section = ""
        self.used_serialize_pb_type.clear()
        self.used_serialize_pb_message.clear()
        for (method_name, method_info) in i_methods.items():
            section += self.add_source_serialize_method(i_base_name,
                                                        method_name,
                                                        method_info)
            self.common_def.special_method = False
        return section