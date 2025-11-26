#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (c) 2019-2021, 2023 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.


"""This class methods that edit aidl files for specific test cases.

IMPORTANT:
 - The sys.path should contain path to
   vendor/qcom/proprietary/commonsys-intf/QIIFA-fwk/ dir to use json
 - This class must contain an implementation of all the tc_method
   variables found within test_cases.json.
 - tc_kwargs should be the only normal parameter when defining a
   method within this class.
 - Methods can have params that contain default values.
 - Any params that are used should be placed within tc_method_kwargs
   within test_cases.json and retrieved within a try catch statement
   from the tc_kwargs dictionary.
 - The abs_file_path key is added to all tc_kwargs dicts within
   aidl_compatibility.py.
 - The abs_file_path key is the file_path variable within test_cases.json

"""

import sys
import random

sys.path.append('../')
from compatibility_check import CompatibilityCheck

import json
import ast
import pandas as pd
import plugins.qiifa_hidl_checker.xmltodict as xmltodict
import pytest


class AIDLTCMethods(CompatibilityCheck):
    def __init__(self):
        super(AIDLTCMethods, self).__init__()
        self.hal_name = random.choice(self.get_hal_name())
        pass

    ########################## helper functions ###############################
    def check_file_alter(self, altered, msg):
        # Raise an exception if method doesn't edit file
        if not altered:
            raise Exception(msg)

    def tc_kwargs_error(self, msg):
        raise Exception(msg)

    def read_json_file(self, abs_file_path):
        with open(abs_file_path, 'r') as f:
            data = json.load(f)
        return data

    def write_json_file(self, abs_file_path, data):
        with open(abs_file_path, 'w') as outfile:
            json.dump(data, outfile, indent=4)

    def get_hal_name(self):
        # Function to get the list of hal names by comapring QIIFA-CMD with the skip list
        # df_json -> List of HALs from qiifa_aidl_cmd.json starting with vendor.qti
        df_json = pd.read_json(self.build_path + '/vendor/qcom/proprietary/commonsys-intf/QIIFA-cmd/aidl/qiifa_aidl_cmd_device.json')
        df_json = df_json[["name"]]
        df_json = df_json[df_json['name'].str.startswith('vendor.qti')]
        df_json = df_json.astype({"name": str})
        # df_txt -> List of HALs from the skip list
        with open(self.build_path + '/vendor/qcom/proprietary/commonsys-intf/QIIFA-cmd/aidl/qiifa_aidl_skipped_interfaces.json') as f:
            data = json.load(f)
        data = ast.literal_eval(json.dumps(data))
        df_skip = pd.DataFrame(data['ALL'], columns=['name'])
        df_skip = df_skip[df_skip['name'].str.startswith('vendor.qti')]
        df_skip = df_skip.astype({"name": str})
        return pd.concat([df_json, df_skip]).drop_duplicates(keep=False).name.values.tolist()

    def check_file_alter(self, altered, msg):
        # Raise an exception if method doesn't edit file
        if not altered:
            raise Exception(msg)

    def write_xml_file(self, abs_file_path, xml):
        # Write the edited contents of the file
        with open(abs_file_path, 'w') as fp:
            xmltodict.unparse(xml, fp)

    def read_xml_from_file(self, abs_file_path, xml):
        if not xml:
            with open(abs_file_path, 'r') as fp:
                xml = xmltodict.parse(fp)
        return xml

    ############################# main code ###################################

    def do_nothing(self, tc_kwargs):
        """Don't alter any files."""
        pass

    def modify_aidl_stability(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl stability to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify 'vintf' to 'intf' in stability for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['stability'] = "intf"

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def delete_aidl_stability(self, tc_kwargs):
        """Traverses the json dictionary and deletes a aidl stability to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Delete stability for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                del interface['stability']

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def modify_aidl_hash(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl hash to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify hash for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['hashes']["1"] = interface['hashes']["1"] + "_updated"

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def empty_aidl_hash(self, tc_kwargs):
        """Traverses the json dictionary and empties a aidl hash to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify hash for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['hashes'] = {}

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def modify_aidl_hash_metadata(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl hash to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify hash for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['hashes'][0] = interface['hashes'][0] + "_updated"

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def add_aidl_hash(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl hash to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify hash for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['hashes']['1000'] = '0dab5f7fc46756d126ac127cd6033fc242eff0aa'

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def delete_aidl_hash(self, tc_kwargs):
        """Traverses the json dictionary and deletes a aidl hash to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Delete hash for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                del interface['hashes']

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def modify_aidl_vendor_available(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl vendor_available to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify boolean value in vendor_available for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                if interface['vendor_available'] =='false':
                    interface['vendor_available'] = "true"
                else:
                    interface['vendor_available'] = "false"

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def delete_aidl_vendor_available(self, tc_kwargs):
        """Traverses the json dictionary and deletes a aidl vendor_available to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Delete vendor_available for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                del interface['vendor_available']

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def modify_aidl_types(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl types to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['types'][0] = interface['types'][0] + '_updated'

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def empty_aidl_types(self, tc_kwargs):
        """Traverses the json dictionary and empties a aidl types to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['types'] = []

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def add_aidl_types(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl types to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['types'].append("android.hardware.graphics.allocator.IAllocator")

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def delete_aidl_types(self, tc_kwargs):
        """Traverses the json dictionary and deletes a aidl types to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Delete types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                del interface['types']

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def modify_aidl_name(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl name to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in name for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['name'] = interface['name'] + '_updated'

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def modify_aidl_has_development(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl has_development to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                if interface['has_development'] == 'false':
                    interface['has_development'] = "true"
                else:
                    interface['has_development'] = "false"

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def delete_aidl_has_development(self, tc_kwargs):
        """Traverses the json dictionary and deletes a aidl has_development to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Delete types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                del interface['has_development']

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def modify_aidl_versions(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl versions to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['versions'][0] = 1000

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def empty_aidl_versions(self, tc_kwargs):
        """Traverses the json dictionary and modifies a aidl versions to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['versions'] = []

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def add_aidl_versions(self, tc_kwargs):
        """Traverses the json dictionary and adds a aidl versions to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Modify value in types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                interface['versions'].append(1000)

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    def delete_aidl_versions(self, tc_kwargs):
        """Traverses the json dictionary and deletes a aidl versions to a given hal."""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, intf_name, or new_intf_name.')

        # Read json from abs_file_path if not passes as argument
        data = self.read_json_file(abs_file_path)

        # Delete types for the given hal
        for interface in data:
            if interface['name'] == hal_name:
                del interface['versions']

        # Write json file to abs_file_path
        self.write_json_file(abs_file_path, data)
        return True

    #################################################### XML Related Test Code ####################################################

    def modify_compatibility_matrix_hal_format(self, tc_kwargs, xml=None, altered=False):
        """Traverses the xml dictionary and modify compatibility-matrix.hal.format"""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
            # Found within tc_method_kwargs variable within test_cases.json
            new_format_name = tc_kwargs['new_format']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, new_format.')

        # Read xml from abs_file_path if not passes as argument
        xml = self.read_xml_from_file(abs_file_path, xml)

        # Read and edit contents of file
        for ind, item in enumerate(xml['compatibility-matrix']['hal']):
            if 'name' in item and item['name'] == hal_name:
                xml['compatibility-matrix']['hal'][ind]['@format'] = new_format_name
                altered = True

        # Raise an exception if method doesn't edit file
        self.check_file_alter(altered, msg='hal format not modified. File hasn\'t been edited.')

        # Write the edited contents of the file
        self.write_xml_file(abs_file_path, xml)
        return xml

    def modify_compatibility_matrix_hal_name(self, tc_kwargs, xml=None, altered=False):
        """Traverses the xml dictionary and modify compatibility-matrix.hal.name"""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path.')

        # Read xml from abs_file_path if not passes as argument
        xml = self.read_xml_from_file(abs_file_path, xml)

        # Read and edit contents of file
        for ind, item in enumerate(xml['compatibility-matrix']['hal']):
            if 'name' in item and item['name'] == hal_name:
                xml['compatibility-matrix']['hal'][ind]['name'] = xml['compatibility-matrix']['hal'][ind]['name'] + '_updated'
                altered = True

        # Raise an exception if method doesn't edit file
        self.check_file_alter(altered, msg='hal name not modified. File hasn\'t been edited.')

        # Write the edited contents of the file
        self.write_xml_file(abs_file_path, xml)
        return xml

    def remove_compatibility_matrix_hal_format(self, tc_kwargs, xml=None, altered=False):
        """Traverses the xml dictionary and removes compatibility-matrix.hal.format"""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = self.hal_name
            # Found within tc_method_kwargs variable within test_cases.json
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path.')

        # Read xml from abs_file_path if not passes as argument
        xml = self.read_xml_from_file(abs_file_path, xml)

        # Read and edit contents of file
        for ind, item in enumerate(xml['compatibility-matrix']['hal']):
            if 'name' in item and item['name'] == hal_name:
                del xml['compatibility-matrix']['hal'][ind]['@format']
                altered = True

        # Raise an exception if method doesn't edit file
        self.check_file_alter(altered, msg='hal format not removed. File hasn\'t been edited.')

        # Write the edited contents of the file
        self.write_xml_file(abs_file_path, xml)
        return xml

    def modify_compatibility_matrix_hal_version(self, tc_kwargs, xml=None, altered=False):
        """Traverses the xml dictionary and modify compatibility-matrix.hal.version"""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = "vendor.qti.data.txpwrservice"
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path.')

        # Read xml from abs_file_path if not passes as argument
        xml = self.read_xml_from_file(abs_file_path, xml)

        # Read and edit contents of file
        for ind, item in enumerate(xml['compatibility-matrix']['hal']):
            if 'name' in item and item['name'] == hal_name:
                xml['compatibility-matrix']['hal'][ind]['version'] = xml['compatibility-matrix']['hal'][ind]['version'] + '0'
                altered = True

        # Raise an exception if method doesn't edit file
        self.check_file_alter(altered, msg='hal version not modified. File hasn\'t been edited.')

        # Write the edited contents of the file
        self.write_xml_file(abs_file_path, xml)
        return xml

    def remove_compatibility_matrix_hal_version(self, tc_kwargs, xml=None, altered=False):
        """Traverses the xml dictionary and removes compatibility-matrix.hal.version"""
        try:
            abs_file_path = tc_kwargs['abs_file_path']
            hal_name = "vendor.qti.data.txpwrservice"
            # Found within tc_method_kwargs variable within test_cases.json
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path.')

        # Read xml from abs_file_path if not passes as argument
        xml = self.read_xml_from_file(abs_file_path, xml)

        # Read and edit contents of file
        for ind, item in enumerate(xml['compatibility-matrix']['hal']):
            if 'name' in item and item['name'] == hal_name:
                del xml['compatibility-matrix']['hal'][ind]['version']
                altered = True

        # Raise an exception if method doesn't edit file
        self.check_file_alter(altered, msg='hal version not removed. File hasn\'t been edited.')

        # Write the edited contents of the file
        self.write_xml_file(abs_file_path, xml)
        return xml