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

class Tester:
    def __init__(self, type_info: dict, interface_info: dict,
                 common_def: CommonDef, config: dict) -> None:
        self.type_info = type_info
        self.interface_info = interface_info
        self.common_def = common_def
        self.config = config
        self.used_type = set()

    def add_test_method_declaration(self, i_base_name: str, m_name: str) -> str:
        """
        Add corresponding test method declaration.

        Args:
            i_base_name: Interface base name
            m_name: Method name

        Returns:
            Test method declaration
        """
        method_name = "T_{}".format(convert_const_upper(i_base_name + m_name)[1:])
        return "static bool {}()\n{{\n".format(method_name)

    def add_test_top_level_regular_entry(self, v_name: str, v_dict: dict,
                                         stack: deque,
                                         tab_level: str) -> str:
        """
        Add initialization code for a top-level regular entry.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Cached variable type stack
            tab_level: Tab indention level

        Returns:
            Top-level regular entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        if v_type in self.interface_info.keys():
            mapped_type = CommonDef.TYPE_MAPPING_CPP[self.config["instanceIdProtoType"]]
            section += "{}{} {} = {};\n"\
                       .format(tab_level, mapped_type, v_name,
                               get_random(self.config["instanceIdProtoType"]))
        elif v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{} {} = {};\n"\
                       .format(tab_level, CommonDef.TYPE_MAPPING_CPP[v_type],
                               v_name, get_random(v_type))
        elif "bytes" == v_type:
            section += "{}{} {};\n"\
                       .format(tab_level, deduce_bytes_type(v_dict, self.config), v_name)
            section += fill_array(v_name, v_dict, "", True,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)
        else:
            formal_type_name = ""
            if CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
                if self.type_info[v_type]["contained"]:
                    formal_type_name = "::".join(v_type.split(".")[-2:])
                else:
                    formal_type_name = v_last
                section += "{}{} {} = {}({});\n"\
                           .format(tab_level, formal_type_name,v_name,
                                   formal_type_name, self.config["enumDefault"])
            elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"]:
                check_used(v_name, v_type, stack, self.used_type, True)
                if v_last == self.config["statusKeyword"]:
                    formal_type_name = self.config["statusKeyword"] + \
                                       self.config["typedefSuffix"]
                else:
                    if self.type_info[v_type]["contained"]:
                        formal_type_name = "::".join(v_type.split(".")[-2:])
                    else:
                        formal_type_name = v_last
                section += "{}{} {};\n".format(tab_level, formal_type_name, v_name)
                helper_name = get_test_helper_name(v_type)
                section += "{}{}({});\n".format(tab_level, helper_name, v_name)
        return section

    def add_test_top_level_repeated_entry(self, v_name: str, v_dict: dict,
                                          stack: deque, tab_level: str) -> str:
        """
        Add initialization code for a top-level repeated entry.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Cached variable type stack
            tab_level: Tab indention level

        Returns:
            Top-level repeated entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        variable_type = ""
        # Should not be pseudoCfm
        formal_type_name = ""
        if v_type in self.common_def.PRIMITIVE_TYPES:
            formal_type_name = CommonDef.TYPE_MAPPING_CPP[v_type]
            variable_type = deduce_repeated_type(v_dict, formal_type_name)
            section += "{}{} {};\n".format(tab_level, variable_type, v_name)
            section += fill_array(v_name, v_dict, "", True,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)
        # Should not be bytes
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            if self.type_info[v_type]["contained"]:
                formal_type_name = "::".join(v_type.split(".")[-2:])
            else:
                formal_type_name = v_last
            variable_type = deduce_repeated_type(v_dict, formal_type_name)
            section += "{}{} {};\n".format(tab_level, variable_type, v_name)
            section += fill_array(v_name, v_dict, "", True,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)

        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack, self.used_type, True)
            if self.type_info[v_type]["contained"]:
                formal_type_name = "::".join(v_type.split(".")[-2:])
            else:
                formal_type_name = v_last
            variable_type = deduce_repeated_type(v_dict, formal_type_name)
            section += "{}{} {};\n".format(tab_level, variable_type, v_name)
            section += fill_array(v_name, v_dict, "", True,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)
        return section

    def add_test_regular_entry(self, v_name: str, v_dict: dict,
                               stack: deque, p_name: str, p_type:str,
                               tab_level: str) -> str:
        """
        Add initialization code for a non top-level regular entry.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Cached variable type stack
            p_name: Parent variable name
            p_type: Parent variable type
            tab_level: Tab indention level

        Returns:
            Regular entry content.
        """
        section = ""
        v_type = v_dict["type"]
        v_last = v_type.split(".")[-1]
        # v_type would be primitive types, but parent type is interface class
        if p_type in self.interface_info.keys():
            section += "{}{}.{} = {};\n"\
                       .format(tab_level, p_name, v_name, get_random(v_type))
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{}.{} = {};\n"\
                       .format(tab_level, p_name, v_name, get_random(v_type))
        elif "bytes" == v_type:
            section += fill_array(v_name, v_dict, p_name, False,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)
        else:
            formal_type_name = v_last
            if CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
                if self.type_info[v_type]["contained"]:
                    formal_type_name = "::".join(v_type.split(".")[-2:])
                section += "{}{}.{} = {}({});\n"\
                           .format(tab_level, p_name, v_name,
                                   formal_type_name, self.config["enumDefault"])
            elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
                 CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
                check_used(v_name, v_type, stack, self.used_type, False)
                helper_name = get_test_helper_name(v_type)
                section += "{}{}({}.{});\n"\
                           .format(tab_level, helper_name, p_name, v_name)
        return section

    def add_test_repeated_entry(self, v_name: str, v_dict: dict,
                                stack: deque, p_name: str, tab_level: str) -> str:
        """
        Add initialization code for a non top-level repeated entry.

        Args:
            v_name: Variable name
            v_dict: Variable info dictionary
            stack: Cached variable type stack
            p_name: Parent variable name
            tab_level: Tab indention level

        Returns:
            Regular entry content.
        """
        section = ""
        v_type = v_dict["type"]
        # Should not be pseudoCfm
        if v_type in self.common_def.PRIMITIVE_TYPES:
            section += fill_array(v_name, v_dict, p_name, False,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)
        # Should not be bytes
        elif CommonDef.ENUM == self.type_info[v_type]["typeCategory"]:
            section += fill_array(v_name, v_dict, p_name, False,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)
        elif CommonDef.CLASS == self.type_info[v_type]["typeCategory"] or \
             CommonDef.UNION == self.type_info[v_type]["typeCategory"]:
            check_used(v_name, v_type, stack, self.used_type, False)
            section += fill_array(v_name, v_dict, p_name, False,
                                  self.type_info, self.config,
                                  self.common_def, tab_level)
        return section

    def add_test_entry(self, v_name: str, v_type_dict: dict,
                       top_level: bool, stack: deque,
                       p_name: str, p_type: str) -> str:
        """
        Add one test entry for specified field.

        Args:
            v_name: Variable name
            v_type_dict: Variable type dictionary
            top_level: Whether this variable is at top-level
            stack: Proto variable type stack
            p_name: Parent class name (Only for helper methods)
            p_type: Parent class type (Only for helper methods)
        """
        section = ""
        repeated = v_type_dict["repeated"]
        tab_level = self.common_def.TAB
        if top_level:
            if repeated:
                section += \
                    self.add_test_top_level_repeated_entry(v_name, v_type_dict,
                                                           stack, tab_level)
            else:
                section += \
                    self.add_test_top_level_regular_entry(v_name,
                                                          v_type_dict,
                                                          stack, tab_level)
        else:
            if repeated:
                section += \
                    self.add_test_repeated_entry(v_name, v_type_dict,
                                                 stack, p_name, tab_level)
            else:
                section += \
                    self.add_test_regular_entry(v_name, v_type_dict,
                                                stack,
                                                p_name, p_type, tab_level)
        return section

    def add_prepare_section(self, i_base_name: str,
                            m_name: str, m_params: dict, stack: deque) -> str:
        """
        Generate whole initialization section plus serialize method invocation.

        Args:
            i_base_name: Interface base name
            m_name: Method name
            m_params: Method parameters dictionary
            stack: Stack used for cache handling

        Returns:
            Initialization section.
        """
        section = ""
        interface_cfm = False
        initialization_section = ""
        for (param_name, param_info) in m_params.items():
            param_type = param_info["type"]
            if param_type.split(".")[-1] == self.config["statusKeyword"]:
                interface_cfm = True
            initialization_section += \
                self.add_test_entry(param_name, param_info,True, stack, "", "")

        payload_line = "{}vector<uint8_t> payload;\n".format(self.common_def.TAB)
        log_payload = ", payload.size()"
        param_list = ", ".join(list(m_params.keys()))
        param_list += ", payload"
        if 0 == len(m_params) or (1 == len(m_params) and interface_cfm):
            initialization_section = ""
            param_list = "payload"
            # i_base_name = ""
            log_payload = ""
        section += initialization_section
        section += payload_line
        section += "{}bool msgSerializeResult = {}{}{}({});\n"\
                   .format(self.common_def.TAB,
                           i_base_name,
                           self.config["serializeMethodKeyword"],
                           m_name, param_list)
        section += "{}ALOGD(\"%s: msgSerializeResult : %d,  output size: %d\", __func__, msgSerializeResult{});\n"\
                   .format(self.common_def.TAB, log_payload)
        return section

    def add_verify_entry(self, p_name: str, p_dict: dict, multiple: bool) -> str:
        """
        Add one verification entry for this method.

        Args:
            i_base_name: Interface base name
            p_name: Parameter name
            p_dict: Parameter info dictionary
            multiple: Whether the belonged method has other parameters

        Returns:
            One verification entry.
        """
        section = deduce_compare_method(p_name, p_dict,
                                        self.type_info, self.interface_info,
                                        self.config, self.common_def, multiple)
        return section

    def add_verify_section(self, i_base_name: str,
                           m_name: str, m_params: dict) -> str:
        """
        Add verification section for a method.

        Args:
            i_base_name: Interface base name
            m_name: Method name
            m_params: Method parameter dictionary

        Returns:
            Verification section.
        """
        section = ""
        interface_cfm = False
        param_list = "payload.data(), payload.size()"
        parse_method_name = "{}{}{}"\
                            .format(i_base_name,
                                    self.config["parseMethodKeyword"], m_name)

        verify_section = ""
        if 1 < len(m_params):
            for (p_name, p_dict) in m_params.items():
                verify_section += \
                    self.add_verify_entry(p_name, p_dict, True)
        elif 1 == len(m_params):
            p_list = list(m_params.items())
            p_name = p_list[0][0]
            p_dict = p_list[0][1]
            p_type = p_dict["type"]
            if p_type.split(".")[-1] == self.config["statusKeyword"]:
                interface_cfm = True
            verify_section += \
                self.add_verify_entry(p_name, p_dict, False)

        if 0 == len(m_params) or (1 == len(m_params) and interface_cfm):
            verify_section = ""
        else:
            param_list += ", param"
        section += "{}bool msgParseResult = {}({});\n"\
                   .format(self.common_def.TAB, parse_method_name, param_list)
        section += verify_section
        return section

    def add_parse_variable_regular(self, p_dict: dict) -> str:
        """
        Add regular parse variable declaration.

        Args:
            p_dict: Parameter info dictionary

        Returns:
            One line of parse variable declaration.
        """
        section = ""
        p_type = p_dict["type"]
        p_type_last = p_type.split(".")[-1]
        field_type = ""
        if p_type in self.interface_info.keys():
            field_type = CommonDef.TYPE_MAPPING_CPP[self.config["instanceIdProtoType"]]
        elif p_type in self.common_def.PRIMITIVE_TYPES:
            field_type = CommonDef.TYPE_MAPPING_CPP[p_type]
        elif "bytes" == p_type:
            field_type = deduce_bytes_type(p_dict, self.config)
        else:
            if p_type_last == self.config["statusKeyword"]:
                field_type = self.config["statusKeyword"] + self.config["typedefSuffix"]
            else:
                if self.type_info[p_type]["contained"]:
                    field_type = "::".join(p_type.split(".")[-2:])
                else:
                    field_type = p_type_last
        section += "{}{} {};\n".format(self.common_def.TAB, field_type,
                                       self.config["parseIntermediateVariableName"])
        return section

    def add_parse_variable_array(self, p_dict: dict) -> str:
        """
        Add parse variable declaration for an array variable.

        Args:
            p_dict: Parameter info dictionary

        Returns:
            One line of parse variable declaration.
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
        section += "{}{} {};\n".format(self.common_def.TAB, field_type,
                                       self.config["parseIntermediateVariableName"])
        return section

    def add_parse_variable_declaration(self, i_base_name: str,
                                       m_name: str, m_params: dict) -> str:
        """
        Add parse variable declaration for a method message.

        Args:
            i_base_name: Interface base name;
            m_name: Method name
            m_params: Method parameter dictionary

        Returns:
            Parse variable declaration.
        """
        param_declaration = "\n"
        interface_cfm = False
        if 1 < len(m_params):
            param_declaration += "{}{}{}{} {};\n"\
                                 .format(self.common_def.TAB, m_name,
                                         get_param_short_name(i_base_name,
                                                              self.common_def.package_prefix),
                                         self.config["typedefSuffix"],
                                         self.config["parseIntermediateVariableName"])
        elif 1 == len(m_params):
            param_values = list(m_params.values())
            param_type = param_values[0]["type"]
            repeated = param_values[0]["repeated"]
            if param_type.split(".")[-1] == self.config["statusKeyword"]:
                interface_cfm = True
            else:
                if repeated:
                    param_declaration += \
                        self.add_parse_variable_array(param_values[0])
                else:
                    param_declaration += \
                        self.add_parse_variable_regular(param_values[0])
        if 0 == len(m_params) or (1 == len(m_params) and interface_cfm):
            param_declaration = ""
        return param_declaration

    def add_test_method_body(self, i_base_name: str, m_name: str,
                             m_params: dict, stack: deque) -> str:
        """
        Add actual test method body. Top entry interface.

        Args:
            i_base_name: Interface base name
            m_name: Method name
            m_params: Corresponding parameters of the method
            stack: Proto variable type stack

        Returns:
            Method body of the test method
        """
        section = ""
        section += \
            self.add_prepare_section(i_base_name, m_name, m_params, stack)
        section += \
            self.add_parse_variable_declaration(i_base_name,
                                                m_name, m_params)
        section += \
            self.add_verify_section(i_base_name,
                                    m_name, m_params)
        section += \
            "{}return {};\n}}".format(self.common_def.TAB,
                                      str(self.config["booleanDefault"]).lower())
        return section

    def add_status_helper_method(self, s_type: str) -> str:
        """
        Add general initialization for HalStatus structure.

        Args:
            status_info: Status structure dictionary

        Returns:
            Status helper method section
        """
        method_section = "static void {}({}& status)\n{{\n"\
                         .format(get_test_helper_name(s_type),
                                 self.config["statusKeyword"] + \
                                 self.config["typedefSuffix"])
        for f_name in self.type_info[s_type]["fields"]:
            f_type_dict = self.type_info[s_type]["fields"][f_name]
            method_section += \
                self.add_test_entry(f_name, f_type_dict,
                                    False, deque(), "status", s_type)
        method_section += "}"
        return method_section

    def add_test_union_entry(self, v_type: str, v_name: str, v_dict: dict,
                             stack: deque):
        """
        Add initialization code for a union type. Field will be chosen randomly.

        Args:
            v_type: Union type name
            v_name: Parent variable name (Union variable name)
            v_dict: Union type dictionary
            stack: Cached variable type stack

        Returns:
            Initialization entry for a union type.
        """
        section = ""
        type_ref = v_type.split(".")[-1]
        if self.type_info[v_type]["contained"]:
            type_ref = "::".join(v_type.split(".")[-2:])
        tab_level = self.common_def.TAB
        field_names = list(v_dict["fields"].keys())
        field_types = list(v_dict["fields"].values())
        index = random.randrange(0, len(field_names))
        chosen_type = field_types[index]
        chosen_name = field_names[index]
        if chosen_type in self.common_def.PRIMITIVE_TYPES:
            section += "{}{}.set<{}::{}>({});\n"\
                       .format(tab_level, v_name, type_ref,
                               chosen_name,
                               get_random(chosen_type))
        # May need to handle bytes
        elif "bytes" == chosen_type:
            pass
        else:
            formal_type_name = chosen_type.split(".")[-1]
            if CommonDef.ENUM == self.type_info[chosen_type]["typeCategory"]:
                if self.type_info[chosen_type]["contained"]:
                    formal_type_name = "::".join(chosen_type.split(".")[-2:])
                section += "{}{}.set<{}::{}>(({}) {});\n"\
                           .format(tab_level, v_name, type_ref,
                                   chosen_name, formal_type_name,
                                   self.config["enumDefault"])
            elif CommonDef.CLASS == self.type_info[chosen_type]["typeCategory"] or \
                 CommonDef.UNION == self.type_info[chosen_type]["typeCategory"]:
                if self.type_info[chosen_type]["contained"]:
                    formal_type_name = "::".join(chosen_type.split(".")[-2:])
                check_used(chosen_name, chosen_type, stack, self.used_type, False)
                helper_name = get_test_helper_name(chosen_type)
                temp_variable_name = chosen_name + \
                                     self.config["parseIntermediateVariableName"].capitalize()
                section += "{}{} {};\n"\
                           .format(tab_level, formal_type_name, temp_variable_name)
                section += "{}{}({});\n"\
                           .format(tab_level, helper_name, temp_variable_name)
                section += "{}{}.set<{}::{}>({});\n"\
                           .format(tab_level, v_name, type_ref,
                                   chosen_name, temp_variable_name)
        return section

    def add_test_helper_methods(self, stack: deque) -> str:
        """
        Add initialization helper methods.

        Args:
            stack: Proto variable type stack
        """
        helper_methods = deque()
        while 0 != len(stack):
            # First out
            param_entry = stack.popleft()
            p_name = param_entry["pName"]
            p_type = param_entry["pType"]
            # Not handling type HalStatus
            if p_type == self.common_def.package_prefix + self.config["statusKeyword"]:
                continue
            p_top_level = param_entry["topLevel"]
            cpp_type_name = \
                summary_type_name(p_type, self.type_info, self.common_def, self.config)
            param_fields = ""
            param_fields = "{}& {}".format(cpp_type_name, p_name)
            method_section = "static void {}({})\n{{\n"\
                             .format(get_test_helper_name(p_type), param_fields)
            if CommonDef.CLASS == self.type_info[p_type]["typeCategory"]:
                for f_name in self.type_info[p_type]["fields"]:
                    f_type_dict = self.type_info[p_type]["fields"][f_name]
                    method_section += self.add_test_entry(f_name, f_type_dict,
                                                          False, stack,
                                                          p_name, p_type)
            elif CommonDef.UNION == self.type_info[p_type]["typeCategory"]:
                p_dict = self.type_info[p_type]
                method_section += self.add_test_union_entry(p_type, p_name, p_dict, stack)
            method_section += "}"
            if p_top_level:
                helper_methods.append(method_section)
            else:
                helper_methods.appendleft(method_section)
        section = "\n\n".join(helper_methods)
        return section

    def add_test_method(self, i_base_name: str,
                        m_name: str, m_info: dict,
                        test_method_list: list) -> str:
        """
        Add test code main section and corresponding initialization helper methods of this method.

        Args:
            i_base_name: Interface base name
            m_name: Method name
            m_info: A dictionary containing corresponding method info
            test_method_list: List containing all test methods

        Returns:
            Main test method and every helper methods of this test method.
        """
        section = ""
        method_declaration = \
            self.add_test_method_declaration(i_base_name, m_name)
        test_method_list.append(method_declaration.split(" ")[-1].split("\n")[0])
        method_body = ""
        stack = deque()
        method_body = \
            self.add_test_method_body(i_base_name, m_name, m_info["params"], stack)

        helper_methods = \
            self.add_test_helper_methods(stack)
        # Discard empty message or message only having interface Cfm,
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

    def generate_test_code(self, i_base_name: str, i_info: dict,
                           test_method_list: list) -> str:
        """
        Generate test code of the current interface.

        Args:
            i_name: Interface base name
            i_info: Interface info dictionary
            test_method_list: List containing all test methods

        Returns:
            Test code of current interface.
        """
        section = ""
        for (method_name, method_info) in i_info["methods"].items():
            section += self.add_test_method(i_base_name, method_name,
                                            method_info, test_method_list)
        return section