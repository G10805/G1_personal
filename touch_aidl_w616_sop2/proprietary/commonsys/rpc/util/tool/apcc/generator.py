#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
"""

import os
from collections import deque

from helper import *
from de_serializer import DeSerializer
from serializer import Serializer
from tester import Tester

class Generator:
    def __init__(self, interface_info: dict,
                 type_info: dict,
                 common_def: CommonDef,
                 config: dict) -> None:
        self.common_def = common_def
        self.config = config
        self.type_info = type_info
        self.interface_info = interface_info

    def add_typedef_field(self, param_name: str, param_dict: dict) -> str:
        """
        Add one typedef field.

        Args:
            param_name: Parameter name
            param_dict: Parameter dictionary

        Returns:
            String of the corresponding parameter.
        """
        declaration = ""
        p_type = param_dict["type"]
        p_type_last = p_type.split(".")[-1]
        field_type = ""
        if "bytes" == p_type:
            field_type = deduce_bytes_type(param_dict, self.config)
        # Common status struct
        elif self.config["statusKeyword"] == p_type_last:
            field_type = self.config["statusKeyword"] + self.config["typedefSuffix"]
        elif p_type in self.interface_info.keys():
            field_type = self.common_def.TYPE_MAPPING_CPP[self.config["instanceIdProtoType"]]
        else:
            if param_dict["repeated"]:
                if p_type in self.common_def.PRIMITIVE_TYPES:
                    field_type = \
                        deduce_repeated_type(param_dict,
                                             CommonDef.TYPE_MAPPING_CPP[param_dict["type"]])
                else:
                    if self.type_info[p_type]["contained"]:
                        type_name = "::".join(p_type.split(".")[-2:])
                        field_type = \
                            deduce_repeated_type(param_dict, type_name)
                    else:
                        field_type = \
                            deduce_repeated_type(param_dict, p_type_last)
            else:
                if p_type in self.common_def.PRIMITIVE_TYPES:
                    field_type = self.common_def.TYPE_MAPPING_CPP[param_dict["type"]]
                else:
                    if self.type_info[p_type]["contained"]:
                        field_type = "::".join(p_type.split(".")[-2:])
                    else:
                        field_type = p_type_last
        declaration = "{}{} {};\n".format(self.common_def.TAB, field_type, param_name)
        return declaration

    def add_typedef_struct(self, m_name: str, m_info: dict,
                           i_base_name: str) -> str:
        """
        Add new typedef struct for methods taking more than 1 parameter.

        Args:
            m_name: Method name
            m_info: Method info
            i_base_name: Interface base name

        Returns:
           String of typedef struct.
        """
        typedef = "typedef struct\n{\n"
        for (param_name, param_dict) in m_info["params"].items():
            typedef += self.add_typedef_field(param_name, param_dict)
        typedef += "}"

        # Add a short name to resolve naming conflict crossing files
        typedef += " {}{}{};"\
            .format(m_name, get_param_short_name(i_base_name,
                                                 self.common_def.package_prefix),
                                                 self.config["typedefSuffix"])
        return typedef

    def add_typedef_section(self, i_base_name: str, i_methods: dict) -> str:
        """
        Add additional struct definition for interface methods
        which contain more than one parameter.

        Args:
            i_base_name: Interface base name
            i_methods: A method dictionary

        Returns:
            String of additional struct typedef section.
        """
        section = ""

        typedef_queue = deque()
        for (method_name, method_info) in i_methods.items():
            if 1 < len(method_info["params"]):
                typedef = \
                    self.add_typedef_struct(method_name, method_info,
                                            i_base_name)
            else:
                continue
            typedef_queue.append(typedef)
        section = "\n\n".join(list(typedef_queue))
        return section

    def generate_header_file(self, i_name: str, i_info: dict) -> None:
        """
        Generate and fill ".h" header file of the given interface.

        Args:
            i_name: Interface name
            i_info: Corresponding interface info

        Returns:
            None
        """
        # Strip from index 1 as 0-th would be "_"
        header_file_name = get_file_name(i_info["interfaceBaseName"],
                                         self.config["interfaceFileNameSuffix"],
                                         self.config["headerFileSuffix"])
        header_out_dir = os.path.join(self.config["outPath"],
                                      self.config["messageSubPath"],
                                      self.config["headerFileSubPath"])
        check_out_path_exists(self.config, header_out_dir)
        if i_name == self.common_def.msg_common_interface_name:
            header_file_name = "{}{}{}".format(
                               convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                               self.config["commonMsgSuffix"],
                               self.config["headerFileSuffix"])
        header_file_path = os.path.join(header_out_dir, header_file_name)

        interface_base_name = i_info["interfaceBaseName"]
        if i_name == self.common_def.msg_common_interface_name:
            interface_base_name = \
                self.common_def.package_prefix[:-1].split(".")[-1].capitalize()

        if self.config["skipGeneration"]:
            if os.path.exists(header_file_path):
                print("Skip {}, already exists".format(header_file_path))
                return

        content = ""
        # Add disclaimer
        # Add macro
        # Add include
        # Add using

        include_headers_list = []
        for header in self.config["headerStdHeaders"]:
            include_headers_list.append("#include <{}>".format(header))
        include_headers_list.sort()
        include_namespaces_list = []
        for namespace in self.config["headerStdNamespaces"]:
            include_namespaces_list.append("#include <{}>".format(namespace))
        include_namespaces_list.sort()
        include_aidl_header_list = []
        for paired_interface in i_info["pairedInterfaces"]:
            include_aidl_header_list.append("#include <{}{}.h>"\
                                            .format(self.common_def.aidl_include_prefix,
                                                    paired_interface.split(".")[-1]))
        include_aidl_header_list.sort()
        include_aidl_header = "{}\n\n".format("\n".join(include_aidl_header_list))

        # Strip initial "I" and underscore
        include_common_header = "#include \"{}{}{}\"\n\n".format(
                                convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                                self.config["commonMsgSuffix"],
                                self.config["headerFileSuffix"])
        # Remove common header if this module uses asynchronous mode
        if not self.config["sync"]:
            include_common_header = ""
        # Clear reference for common header
        # Other headers include the common header
        if i_name == self.common_def.msg_common_interface_name:
            include_aidl_header = ""
            include_common_header = ""
            include_namespaces_list.clear()
            for namespace in self.config["commonHeaderNamespaces"]:
                include_namespaces_list.append("#include <{}>".format(namespace))
            include_namespaces_list.sort()
        include_section = "{}\n\n{}\n\n{}{}"\
                           .format("\n".join(include_headers_list),
                                   "\n".join(include_namespaces_list),
                                   include_aidl_header, include_common_header)

        using_std_list = []
        for namespace in self.config["headerUsingStds"]:
            using_std_list.append("using std::{};".format(namespace))
        using_std_list.sort()
        using_aidl_namespace = "using namespace {};\n\n"\
                               .format(self.common_def.aidl_using_prefix[:-2])
        # Clear reference for common header
        if i_name == self.common_def.msg_common_interface_name:
            using_std_list.clear()
            for namespace in self.config["commonHeaderUsingStds"]:
                using_std_list.append("using std::{};".format(namespace))
                using_std_list.sort()
            using_aidl_namespace = ""
        using_section = "{}\n\n{}\n".format("\n".join(using_std_list),
                                          using_aidl_namespace)

        # typedef struct of messages which has multiple fields
        new_typedef_section = \
            self.add_typedef_section(interface_base_name, i_info["methods"])

        if "" != new_typedef_section:
            new_typedef_section += "\n\n"

        # Add method declaration
        serialize_methods = \
            self.serializer.add_header_serialize_methods(interface_base_name,
                                                         i_info["methods"])

        # Add Parse/ De-serialize
        parse_methods = \
            self.de_serializer.add_header_parse_methods(interface_base_name,
                                                        i_info["methods"])

        content = "{}{}{}{}{}{}{}"\
                  .format(CommonDef.PROMPT,
                          CommonDef.MACRO,
                          include_section,
                          using_section,
                          new_typedef_section,
                          serialize_methods,
                          parse_methods)
        with open(os.path.normpath(header_file_path),
                  "w", encoding=self.config["encoding"]) as f:
            f.writelines(content)
        print("Generated {}".format(header_file_path))

    def generate_implementation_file(self, i_name: str, i_info: dict) -> None:
        """
        Generate and fill ".cpp" implementation file of the given interface.

        Args:
            i_name: Interface name
            i_info: Corresponding interface info

        Returns:
            None
        """
        # Start from index 1 as 0-th would be "_" from convert_const_upper()
        source_file_name = get_file_name(i_info["interfaceBaseName"],
                                         self.config["interfaceFileNameSuffix"],
                                         self.config["sourceFileSuffix"])
        source_out_dir = os.path.join(self.config["outPath"],
                                      self.config["messageSubPath"],
                                      self.config["sourceFileSubPath"])
        check_out_path_exists(self.config, source_out_dir)
        if i_name == self.common_def.msg_common_interface_name:
            source_file_name = "{}{}{}"\
                .format(convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                        self.config["commonMsgSuffix"],
                        self.config["sourceFileSuffix"])
        source_file_path = os.path.join(source_out_dir, source_file_name)

        interface_base_name = i_info["interfaceBaseName"]
        if i_name == self.common_def.msg_common_interface_name:
            interface_base_name = \
                self.common_def.package_prefix.split(".")[-2].capitalize()

        if self.config["skipGeneration"]:
            if os.path.exists(source_file_path):
                print("Skip {}, already exists".format(source_file_path))
                return

        content = ""
        # Add disclaimer
        # Add macro
        # Add include
        include_header_list = []
        for header in self.config["implAdditionalHeaders"]:
            include_header_list.append("#include \"{}\"".format(header))
        # Include corresponding .h
        header_file_name = source_file_name.replace(self.config["sourceFileSuffix"],
                                                    self.config["headerFileSuffix"])
        include_header_list.sort()
        include_header_list.append("#include \"{}\"".format(header_file_name))

        include_base_header_section = "\n".join(include_header_list) + "\n\n"
        include_pb_type_set = set()
        # Include common status proto for the common file,
        # or regular interface proto for normal interfaces
        if i_name == self.common_def.msg_common_interface_name:
            pass
        else:
            for paired_interface in i_info["pairedInterfaces"]:
                include_pb_type_set.add("#include \"{}{}\""\
                                        .format(paired_interface.split(".")[-1],
                                                self.config["protoHeaderFileSuffix"]))

        # Add Serialize
        if i_name.split(".")[-1] == self.common_def.special_interface_name:
            self.common_def.special_interface = True
        serialize_section = ""
        serialize_section = \
            self.serializer.add_source_serialize_methods(interface_base_name,
                                                         i_info["methods"])

        # Update using
        using_proto_method_list = []
        # Message wrapping
        for method in self.serializer.used_serialize_pb_message:
            using_proto_method_list.append("using {} = ::{}{};"\
                                           .format(method, self.common_def.proto_using_prefix,
                                                   self.interface_info[i_name]["methods"][method]["fullName"]))
        using_proto_method_section = ""
        # Common status handling
        if i_name == self.common_def.msg_common_interface_name:
            using_proto_method_list.clear()
            # Manually add common status proto type
            self.serializer.used_serialize_pb_type.add(self.common_def.common_status_full_name)
        else:
            using_proto_method_list.sort()
            if 0 < len(using_proto_method_list):
                using_proto_method_section = "\n".join(using_proto_method_list)
                using_proto_method_section += "\n\n"

        # Update used protoBuffer types
        using_pb_type_list = []
        for p_type in self.serializer.used_serialize_pb_type:
            full_name = ""
            include_type_name = ""
            p_type_last = p_type.split(".")[-1]
            if self.type_info[p_type]["contained"]:
                type_name = "_".join(p_type.split(".")[-2:])
                full_name = "{}::{}".format("::".join(p_type.split(".")[:-2]),
                                            type_name)
                include_type_name = p_type.split(".")[-2]
            else:
                full_name = p_type.replace(".", "::")
                include_type_name = p_type_last
            include_pb_type_set.add("#include \"{}{}\""\
                                    .format(include_type_name,
                                            self.config["protoHeaderFileSuffix"]))
            using_pb_type_list.append("using {}{} = ::{};"\
                                      .format(p_type_last,
                                              self.config["protoClassVariant"],
                                              full_name))
        using_pb_type_section = ""
        include_pb_type_section = ""
        if 0 != len(using_pb_type_list):
            using_pb_type_list.sort()
            using_pb_type_section = "\n".join(using_pb_type_list) + "\n\n"
        if 0 != len(include_pb_type_set):
            include_pb_type_list = list(include_pb_type_set)
            include_pb_type_list.sort()
            include_pb_type_section = "\n".join(include_pb_type_list) + "\n\n"

        # Add Parse/ De-serialize methods
        parse_section = ""
        parse_section = \
            self.de_serializer.add_source_parse_methods(interface_base_name,
                                                        i_info["methods"])
        self.common_def.special_interface = False

        content = "{}{}{}{}{}\n{}{}"\
                  .format(CommonDef.PROMPT,
                          include_base_header_section,
                          include_pb_type_section,
                          using_proto_method_section,
                          using_pb_type_section,
                          serialize_section,
                          parse_section)
        with open(os.path.normpath(source_file_path),
                  "w", encoding=self.config["encoding"]) as f:
            f.writelines(content)
        print("Generated {}".format(source_file_path))

    def generate_impl_files(self) -> None:
        """
        Generate header files (.h) and implementation files (.cpp) for interfaces recorded.

        Args:
            None

        Returns:
            None
        """
        self.serializer = Serializer(self.type_info, self.interface_info,
                                     self.common_def, self.config)
        self.de_serializer = DeSerializer(self.type_info, self.interface_info,
                                          self.common_def, self.config)
        for (i_name, i_info) in self.interface_info.items():
            self.generate_header_file(i_name, i_info)
            self.generate_implementation_file(i_name, i_info)

    def generate_macro_define_file(self) -> None:
        """
        Generate ID definition macros for messages.

        Args:
            None

        Returns:
            None
        """
        definition_file_name = "{}{}{}".format(
                               convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                               self.config["idDefinitionFileNameSuffix"],
                               self.config["headerFileSuffix"])
        definition_file_dir = os.path.join(self.config["outPath"],
                                           self.config["headerFileSubPath"])
        check_out_path_exists(self.config, definition_file_dir)
        definition_file_path = os.path.join(definition_file_dir, definition_file_name)

        if self.config["skipGeneration"]:
            if os.path.exists(definition_file_path):
                print("Skip {}, already exists".format(definition_file_path))
                return

        section = ""
        include_header_list = []
        for header in self.config["headerStdHeaders"]:
            include_header_list.append("#include <{}>".format(header))
        include_header_list.sort()
        include_base_header_section = "\n".join(include_header_list) + "\n\n"
        # List wrapper since int is immutable
        interface_req_index = [0]
        interface_ind_index = [0]
        interface_req_list = []
        interface_ind_list = []
        typedef_msg_name = self.common_def.package_prefix.split(".")[-2].capitalize() + "Msg"
        typedef_section = "typedef {} {};\n\n"\
                          .format(self.config["idDefinitionTypedefType"], typedef_msg_name)
        define_keyword = "#define"

        msg_offset_section = ""
        msg_offset = False
        msg_offset_name = ""
        if 0 != self.common_def.id_definition_offset:
            msg_offset = True
            print("Module having offset: {}".format(hex(self.common_def.id_definition_offset)))
            msg_offset_name = "{}_OFFSET"\
                              .format(convert_const_upper(self.common_def.base_intf[1:])[1:])
            msg_offset_section += "{} {:<76}0x{:0>4X}\n\n"\
                              .format(define_keyword, msg_offset_name,
                                      self.common_def.id_definition_offset)

        msg_common_interface_name = get_common_interface_name(self.common_def, self.config)
        for (i_name, i_info) in self.interface_info.items():
            if i_name == msg_common_interface_name:
                continue
            index = interface_req_index
            offset = 0
            offset_part = ""
            if msg_offset:
                offset_part = f" + {msg_offset_name}"
            interface_section_list = interface_req_list
            if self.config["indicateSuffix"] == i_info["interfaceType"]:
                index = interface_ind_index
                offset = self.common_def.id_definition_callback_offset
                interface_section_list = interface_ind_list
            interface_section = ""
            interface_name = self.config["idDefinitionInterfacePrefix"] +\
                             convert_const_upper(i_info["interfaceBaseName"])[1:]
            interface_base_name = ""
            type_suffix = ""
            type_suffix = self.config["requestSuffix"].upper()
            for keyword in self.config["callbackKeywords"]:
                if keyword in i_name:
                    type_suffix = self.config["indicateSuffix"].upper()
                    break
            interface_base_name = "{}_{}_BASE".format(interface_name, type_suffix)
            padding = ""
            if CommonDef.ID_NAME_MAX_LENGTH <= len(interface_base_name):
                # Plus one "space"
                padding = " \\\n{}"\
                          .format(" " * (len(define_keyword) + 1 + CommonDef.ID_NAME_MAX_LENGTH))
            interface_section = "{} {:<76}{}(({})(0x{:0>4X}{}))\n\n"\
                                .format(define_keyword, interface_base_name,
                                        padding, typedef_msg_name, offset + index[0] * 0x100, offset_part)
            index[0] += 1
            method_index = 0
            method_section = ""
            first_method = ""
            last_method = ""
            for (m_name, m_info) in i_info["methods"].items():
                # Not listing Cfm messages
                if self.config["confirmSuffix"] in m_name:
                    continue
                else:
                    padding = ""
                    const_method_name = "{}{}_{}"\
                                        .format(interface_name,
                                                convert_const_upper(m_info["baseName"]),
                                                m_info["type"].upper())
                    if CommonDef.ID_NAME_MAX_LENGTH <= len(const_method_name):
                        padding = " \\\n{}"\
                                  .format(" " * (len(define_keyword) + 1 + CommonDef.ID_NAME_MAX_LENGTH))
                    method_section += "{} {:<76}{}({})({} + 0x{:0>4X})\n"\
                                      .format(define_keyword, const_method_name, padding,
                                              typedef_msg_name, interface_base_name,
                                              method_index)
                    if 0 == method_index:
                        first_method = const_method_name
                    last_method = const_method_name
                    method_index += 1
            interface_section += method_section
            interface_section += "\n{} {:<76}({} - {} + 1)"\
                                 .format(define_keyword,
                                         interface_base_name[:-len("_BASE")] + "_COUNT",
                                         last_method, first_method)
            interface_section += "\n{} {:<76}(((t) >= {}) && ((t) <= {}))\n"\
                                 .format(define_keyword,
                                         "IS_" + interface_base_name[:-len("_BASE")] + "(t)",
                                         first_method, last_method)
            interface_section_list.append(interface_section)
        interface_req_section = "\n".join(interface_req_list)
        interface_ind_section = "\n".join(interface_ind_list)
        section = "{}\n{}".format(interface_req_section, interface_ind_section)

        content = "{}{}{}{}{}{}"\
                  .format(CommonDef.PROMPT,
                          CommonDef.MACRO,
                          include_base_header_section,
                          typedef_section,
                          msg_offset_section,
                          section)
        with open(os.path.normpath(definition_file_path),
                  "w", encoding=self.config["encoding"]) as f:
            f.writelines(content.strip())
        print("Generated {}".format(definition_file_path))

    def generate_bp(self) -> None:
        """
        Generate corresponding blueprint file.

        Args:
            None

        Returns:
            None
        """
        bp_file_name = self.config["blueprintFileName"]
        check_out_path_exists(self.config, self.config["outPath"])
        bp_file_path = os.path.join(self.config["outPath"], bp_file_name)

        if self.config["skipGeneration"]:
            if os.path.exists(bp_file_path):
                print("Skip {}, already exists".format(bp_file_path))
                return

        blueprint = self.common_def.BLUEPRINT
        package_base = self.common_def.package_prefix[:-1].replace(".", "-")
        if "" != self.common_def.provided_package_name:
            package_base = self.common_def.provided_package_name
        blueprint = blueprint.replace("__PACKAGE_BASE__", package_base)

        shared_libs = self.config["blueprintSharedLibrary"].copy()
        ndk_name = self.common_def.bp_ndk_name
        shared_libs.append(ndk_name)
        shared_libs.sort()
        for i in range(len(shared_libs)):
            shared_libs[i] = "\"{}\"".format(shared_libs[i])
        shared_libs_section = (",\n{}".format(self.common_def.TAB * 2)).join(shared_libs)
        blueprint = blueprint.replace("__USED_SHARED_LIBS__", shared_libs_section)
        static_libs = self.config["blueprintStaticLibrary"].copy()
        static_libs.append("libqti-{}-proto".format(package_base))
        static_libs.sort()
        for i in range(len(static_libs)):
            static_libs[i] = "\"{}\"".format(static_libs[i])
        static_libs_section = ",\n{}".format(self.common_def.TAB * 2).join(static_libs)
        if 0 == len(static_libs):
            static_libs_section = "\n{}".format(self.config["tabStyle"])
        blueprint = blueprint.replace("__USED_STATIC_LIBS__", static_libs_section)

        with open(os.path.normpath(bp_file_path),
                  "w", encoding=self.config["encoding"]) as f:
            f.writelines(blueprint)
        print("Generated {}".format(bp_file_path))

    def generate_make_file(self) -> None:
        """
        Generate Makefile of this module.

        Args:
            None

        Returns:
            None
        """
        check_out_path_exists(self.config, self.config["outPath"])
        makefile_path = os.path.join(self.config["outPath"],
                                     self.config["makefileName"])

        if self.config["skipGeneration"]:
            if os.path.exists(makefile_path):
                print("Skip {}, already exists".format(makefile_path))
                return

        makefile = self.common_def.MAKEFILE
        conditional_def_list = []
        conditional_logic_list = []
        common_i_name = get_common_interface_name(self.common_def, self.config)
        for (i_name, i_info) in self.interface_info.items():
            source_file_base = convert_const_upper(i_info["interfaceBaseName"])[1:]
            source_file_name = "{}_{}{}"\
                               .format(source_file_base.lower(),
                                       self.config["interfaceFileNameSuffix"],
                                       self.config["sourceFileSuffix"])
            if i_name == common_i_name:
                source_file_base = "{}{}".format(
                                   convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                                   self.config["commonMsgSuffix"])
                entry = "\nSRCS += $(SRC_DIR)/{}"\
                        .format("{}{}".format(source_file_base.lower(),
                                              self.config["sourceFileSuffix"]))
                conditional_logic_list.append(entry)
            else:
                def_entry = "CONFIG_{}".format(source_file_base)
                conditional_def_list.append("{} := n".format(def_entry))
                conditional_logic = \
                    self.common_def.CONDITIONAL_LOGIC.replace("__CONDITIONAL_DEF__", def_entry)
                conditional_logic = \
                    conditional_logic.replace("__SRC_FILE_NAME__", source_file_name)
                conditional_logic_list.append(conditional_logic)
        conditional_def_list.sort()
        conditional_def_section = "\n".join(conditional_def_list)
        makefile = makefile.replace("__CONDITIONAL_CONFIG_DEFINE__",conditional_def_section)

        conditional_logic_list.sort()
        conditional_logic_section = "\n".join(conditional_logic_list)
        conditional_logic_section += "\n"
        makefile = makefile.replace("__CONDITIONAL_CONFIG_LOGIC__",
                                    conditional_logic_section)

        package_base = self.common_def.package_prefix[:-1]
        package_base = convert_const_upper(package_base).lower()
        package_base = package_base.replace(".", "-")
        if "" != self.common_def.provided_package_name:
            package_base = self.common_def.provided_package_name
        makefile = makefile.replace("__PACKAGE_NAME__", package_base) # type: ignore

        additional_flag_list = []
        for lib in self.config["makefileLinkedLibrary"]:
            additional_flag_list.append("-l{}".format(lib))
        additional_flag_list.append("-lqti-{}-proto".format(package_base))
        additional_flag_section = " ".join(additional_flag_list)
        makefile = makefile.replace("__ADDITIONAL_LDFLAGS__", additional_flag_section)
        with open(os.path.normpath(makefile_path),
                  "w", encoding=self.config["encoding"]) as f:
            f.writelines(makefile)
        print("Generated {}".format(makefile_path))

    def generate_test_code_implementation(self, out_dir: str) -> None:
        """
        Generate test code, implementation part.

        Args:
            out_dir: Target directory

        Returns:
            None
        """
        test_file_name = "{}{}{}".format(
                         convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                         self.config["testCodeFileSuffix"],
                         self.config["sourceFileSuffix"])
        test_file_name = test_file_name.replace("-", "_")

        check_out_path_exists(self.config, out_dir)
        test_file_path = os.path.join(out_dir, test_file_name)

        if self.config["skipGeneration"]:
            if os.path.exists(test_file_path):
                print("Skip {}, already exists".format(test_file_path))
                return
        # include library log <>
        # include library <>
        include_library_list = list(self.config["testCodeHeaders"])
        include_library_list.sort()
        for i in range(len(include_library_list)):
            include_library_list[i] = "#include <{}>".format(include_library_list[i])
        include_library_section = "\n".join(include_library_list)

        # include local ".h"
        include_header_list = list(self.config["implAdditionalHeaders"])
        include_header_list.sort()
        for i in range(len(include_header_list)):
            include_header_list[i] = "#include \"{}\"".format(include_header_list[i])
        include_header_section = "\n".join(include_header_list)

        # using
        using_list = list(self.config["headerUsingStds"])
        using_list.sort()
        for i in range(len(using_list)):
            using_list[i] = "using std::{};".format(using_list[i])
        using_section = "\n".join(using_list)

        include_msg_header_list = []
        test_code_section = ""
        test_method_list = []
        for (i_name, i_info) in self.interface_info.items():
            interface_base_name = i_info["interfaceBaseName"]
            # Not include common header
            if i_name == get_common_interface_name(self.common_def, self.config):
                interface_base_name = self.common_def.package_prefix.split(".")[-2].capitalize()
                header_file_name = "{}{}{}".format(
                                   convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                                   self.config["commonMsgSuffix"],
                                   self.config["headerFileSuffix"])
            else:
                header_file_name = get_file_name(i_info["interfaceBaseName"],
                                                 self.config["interfaceFileNameSuffix"],
                                                 self.config["headerFileSuffix"])
            include_msg_header_list.append("#include \"{}\"".format(header_file_name))
            test_code_section += \
                self.tester.generate_test_code(interface_base_name, i_info, test_method_list)
        include_msg_header_list.sort()

        static_definition_section = "{}{}{}{}{}{}"\
                                    .format(self.common_def.TEST_VERIFY,
                                            self.common_def.TEST_COMPARE_STR,
                                            self.common_def.TEST_COMPARE_VEC,
                                            self.common_def.TEST_COMPARE_ARY,
                                            self.common_def.TEST_COMPARE_BUF,
                                            self.common_def.TEST_COMPARE_OBJ)
        status_helper_section = ""
        tab = self.common_def.TAB
        # Only include if communication model is synchronous
        if self.config["sync"]:
            status_helper_section = "{}\n\n"\
                .format(self.tester.add_status_helper_method(self.common_def.package_prefix + \
                                                             self.config["statusKeyword"]))
        for index in range(len(test_method_list)):
            test_method_list[index] = \
                "{}VERIFY({});".format(tab, test_method_list[index])
        test_methods_section = "\n".format(tab).join(test_method_list)
        aggregate_method_section = "static bool T_ALL()\n{{\n{}\n{}ALOGD(\"ALL {} TESTS PASSED\");\n{}return true;\n}}"\
                                   .format(test_methods_section, tab,
                                           len(test_method_list), tab)

        test_module_name = test_file_name.split(".")[0]
        test_module_name = test_module_name.replace("_", "-")
        main_section = "int main(int argc, char** argv)\n{\n"
        main_section += "{}ALOGD(\"{} starts\");\n".format(tab, test_module_name)
        main_section += "{}int count = 1;\n".format(tab)
        main_section += "{}if (1 < argc)\n{}{{\n".format(tab, tab)
        main_section += "{}count = atoi(argv[1]);\n{}}}\n"\
                        .format(tab * 2, tab)
        main_section += "{}for (int i = 0; i < count; i++)\n{}{{\n"\
                        .format(tab, tab)
        main_section += "{}if (T_ALL())\n{}{{\n".format(tab * 2, tab * 2)
        main_section += "{}ALOGD(\"LOOP #%d FINISHED AND PASSED\", i + 1);\n{}}}\n{}else\n{}{{\n"\
                        .format(tab * 3, tab * 2, tab * 2, tab * 2)
        main_section += "{}ALOGE(\"LOOP #%d FAILED\", i + 1);\n{}}}\n{}}}\n"\
                        .format(tab * 3, tab * 2, tab)
        main_section += "{}return 0;\n}}".format(tab)

        content = "{}{}\n\n{}\n\n{}\n\n{}\n{}\n{}{}{}\n\n{}"\
                  .format(self.common_def.PROMPT,
                          include_library_section,
                          include_header_section,
                          "\n".join(include_msg_header_list),
                          using_section,
                          static_definition_section,
                          status_helper_section,
                          test_code_section,
                          aggregate_method_section,
                          main_section)
        with open(os.path.normpath(test_file_path),
                  "w", encoding=self.config["encoding"]) as f:
            f.writelines(content)
        print("Generated {}".format(test_file_path))

    def generate_test_code_mk(self, out_dir: str) -> None:
        """
        Generate test code, blueprint part.

        Args:
            out_dir: Target directory

        Returns:
            None
        """
        mk_file_name = self.config["testMkFileName"]
        if self.config["disableTestMk"]:
            mk_file_name += self.config["disableSuffix"]
        check_out_path_exists(self.config, out_dir)
        mk_file_path = os.path.join(out_dir, mk_file_name)

        if self.config["skipGeneration"]:
            if os.path.exists(mk_file_path):
                print("Skip {}, already exists".format(mk_file_path))
                return

        makefile = self.common_def.TEST_MK
        package_base = self.common_def.package_prefix[:-1].replace(".", "-")
        if "" != self.common_def.provided_package_name:
            package_base = self.common_def.provided_package_name
        shared_message_lib_name = "libqti-{}-message".format(package_base)

        # separator is handled twice for consistency
        makefile_module_name = "{}{}".format(
                               convert_const_upper(self.common_def.base_intf[1:])[1:].lower(),
                               self.config["testCodeFileSuffix"])
        makefile_module_name = makefile_module_name.replace("_", "-")
        makefile = makefile.replace("__TEST_MODULE_NAME__", makefile_module_name)
        makefile_module_name = makefile_module_name.replace("-", "_")
        test_code_src_file_name = "{}{}".format(makefile_module_name,
                                                self.config["sourceFileSuffix"])
        makefile = makefile.replace("__TEST_CODE_SOURCE_FILE__", test_code_src_file_name)

        shared_libs = self.config["blueprintSharedLibrary"].copy()
        ndk_name = self.common_def.bp_ndk_name
        shared_libs.append(shared_message_lib_name)
        shared_libs.append(ndk_name)
        shared_libs.sort()
        shared_libs_section = (" \\\n{}".format(self.common_def.TAB)).join(shared_libs)
        makefile = makefile.replace("__USED_SHARED_LIBS__", shared_libs_section)

        static_libs = self.config["blueprintStaticLibrary"].copy()
        static_libs.append("libqti-{}-proto".format(package_base))
        static_libs.sort()
        static_libs_section = " \\\n{}".format(self.common_def.TAB).join(static_libs)
        makefile = makefile.replace("__USED_STATIC_LIBS__", static_libs_section)

        with open(os.path.normpath(mk_file_path),
                  "w", encoding=self.config["encoding"]) as f:
            f.writelines(makefile)
        print("Generated {}".format(mk_file_path))

    def generate_test_code(self) -> None:
        """
        Generate test code and corresponding build file of the module.
        One source file for one module.

        Args:
            None

        Returns:
            None
        """
        self.tester = Tester(self.type_info, self.interface_info,
                             self.common_def, self.config)
        out_dir = os.path.join(self.config["outPath"],
                               self.config["messageSubPath"],
                               self.config["testCodeSubPath"])
        self.generate_test_code_implementation(out_dir)
        self.generate_test_code_mk(out_dir)