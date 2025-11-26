#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (c) 2022 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.


"""This class methods that edit api_dep files for specific test cases.

IMPORTANT:
 - The sys.path should contain path to
   vendor/qcom/proprietary/commonsys-intf/QIIFA-fwk/ dir to use xmltodict
 - This class must contain an implementation of all the tc_method
   variables found within test_cases.json.
 - tc_kwargs should be the only normal parameter when defining a
   method within this class.
 - Methods can have params that contain default values.
 - Any params that are used should be placed within tc_method_kwargs
   within test_cases.json and retrieved within a try catch statement
   from the tc_kwargs dictionary.
 - The abs_file_path key is added to all tc_kwargs dicts within
   api_dep_compatibility.py.
 - The abs_file_path key is the file_path variable within test_cases.json

"""

import os
import sys

sys.path.append('../')

import plugins.qiifa_hidl_checker.xmltodict as xmltodict
from collections import OrderedDict


class ApiDepTCMethods(object):
    def __init__(self):
        pass

    ########################## helper functions ###############################
    def check_file_alter(self, altered, msg):
        # Raise an exception if method doesn't edit file
        if not altered:
            raise Exception(msg)

    def tc_kwargs_error(self, msg):
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

    ############################# helper functions to modify XML ###################################
    def replace_dependency_version(self, tc_kwargs, xml=None, altered=False):
        """replaces dependency version with given one for specified component and specified file"""

        try:
            file_path = tc_kwargs['file_path']
            # Found within tc_method_kwargs variable within test_cases.json
            component = tc_kwargs['component']
            new_ver = tc_kwargs['new_dep_ver']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing file_path, component, or new_dep_ver.')

        # read xml from file_path if not passes as argument
        xml = self.read_xml_from_file(file_path, xml)

        # read and edit contents of a file
        if isinstance(xml['component']['dependencies']['dependency'], list):
            for dep in xml['component']['dependencies']['dependency']:
                if dep['@component'] == component:
                    altered = True
                    dep['@version'] = new_ver

        else:
            if xml['component']['dependencies']['dependency']['@component'] == component:
                altered = True
                xml['component']['dependencies']['dependency']['@version'] = new_ver

        # Raise an exception if method doesn't edit file
        self.check_file_alter(altered, msg='Signature not found. File hasn\'t been edited.')

        # Write the edited contents of the file
        self.write_xml_file(file_path, xml)
        return xml

    def replace_configuration_version(self, tc_kwargs, xml=None, altered=False):
        """replaces configuration version to given one specified name and file"""

        try:
            file_path = tc_kwargs['file_path']
            # Found within tc_method_kwargs variable within test_cases.json
            name = tc_kwargs['name']
            new_ver = tc_kwargs['new_config_ver']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing file_path, name, or new_config_ver.')

        # read xml from abs_file_path if not passes as argument
        xml = self.read_xml_from_file(file_path, xml)

        # read and edit contents of a file
        if xml['component']['configuration']['@name'] == name:
            altered = True

        xml['component']['configuration']['@version'] = new_ver

        # Raise an exception if method doesn't edit file
        self.check_file_alter(altered, msg='Signature not found. File hasn\'t been edited.')

        # Write the edited contents of the file
        self.write_xml_file(file_path, xml)
        return xml

    def delete_xml(self, filepath):
        """deletes the specified XML in the file path"""

        if os.path.exists(filepath):
            os.remove(filepath)
        else:
            self.tc_kwargs_error(msg='file to delete does not exist')

        if os.path.exists(filepath):
            self.tc_kwargs_error(msg='file that was deleted still exists')

    def new_dep(self, args, xml=None):
        """creates a new dependency in specified filepath file using arguments in args dictionary"""

        try:
            file_path = args["file_path"]
            # Found within tc_method_kwargs variable within test_cases.json
            component = args["component"]
            version = args["ver"]
        except KeyError:
            self.tc_kwargs_error(msg='args missing file_path,component, or version.')

        # read XML if not it passes as argument
        xml = self.read_xml_from_file(file_path, xml)

        new_dep = OrderedDict(
            [(u'dependency', OrderedDict([(u'@type', u'generic'), (u'@component', u'TP-B'), (u'@version', u'2.0')]))])
        new_dep['dependency']['@component'] = component
        new_dep['dependency']['@version'] = version

        if xml['component']['dependencies'] is not None:
            copy = xml['component']['dependencies']['dependency']
            if not isinstance(copy, list):
                copy = [copy]
            copy.append(new_dep['dependency'])
            xml['component']['dependencies']['dependency'] = copy
        else:
            xml['component']['dependencies'] = new_dep

        self.write_xml_file(file_path, xml)

    ############################# test cases code ###################################
    def dep_2_0_config_2_0(self, tc_kwargs):
        """Don't alter any files since default is dependency 2.0 configuration 2.0"""
        pass

    def dep_2_0_config_2_1(self, tc_kwargs):
        """no change to dependency since default is 2.0 replaces configuration version to 2.1"""

        try:
            file_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, or file_b arguments.')

        rcv_kwargs['file_path'] = file_path + 'TP-B.xml'

        self.replace_configuration_version(rcv_kwargs)

    def dep_2_0_config_3_0(self, tc_kwargs):
        """no change to dependency since default is 2.0 replaces configuration version to 3.0"""

        try:
            file_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, or file_b arguments.')

        rcv_kwargs['file_path'] = file_path + 'TP-B.xml'

        self.replace_configuration_version(rcv_kwargs)

    def dep_2_0_config_3_1(self, tc_kwargs):
        """no change to dependency since default is 2.0 replaces configuration version to 3.1"""

        try:
            file_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, or file_b arguments.')

        rcv_kwargs['file_path'] = file_path + 'TP-B.xml'

        self.replace_configuration_version(rcv_kwargs)

    def dep_2_1_config_2_0(self, tc_kwargs):
        """replaces dependency version to 2.1 no change to configuration since default is 2.0"""

        try:
            file_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rdv_kwargs = tc_kwargs['file_a']['mod_cur_dep']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, or file_a arguments.')

        rdv_kwargs['file_path'] = file_path + 'TP-A.xml'

        self.replace_dependency_version(rdv_kwargs)

    def dep_2_1_config_3_0(self, tc_kwargs):
        """replaces dependency version to 2.1 and configuration version to 3.0"""

        try:
            files_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rdv_kwargs = tc_kwargs['file_a']['mod_cur_dep']
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, file_a or file_b arguments.')

        rdv_kwargs['file_path'] = files_path + 'TP-A.xml'
        rcv_kwargs['file_path'] = files_path + 'TP-B.xml'

        self.replace_dependency_version(rdv_kwargs)
        self.replace_configuration_version(rcv_kwargs)

    def dep_2_0_config_1_0(self, tc_kwargs):
        """no change to dependency since default is 2.0 replaces configuration version to 1_0"""

        try:
            file_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, or file_b arguments.')

        rcv_kwargs['file_path'] = file_path + 'TP-B.xml'

        self.replace_configuration_version(rcv_kwargs)

    def dep_2_3_or_2_8_config_2_1(self, tc_kwargs):
        """replaces dependency version to 2.3 || 2.8 and configuration version to 2.1"""

        try:
            files_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rdv_kwargs = tc_kwargs['file_a']['mod_cur_dep']
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path, file_a or file_b arguments.')

        rdv_kwargs['file_path'] = files_path + 'TP-A.xml'
        rcv_kwargs['file_path'] = files_path + 'TP-B.xml'

        self.replace_dependency_version(rdv_kwargs)
        self.replace_configuration_version(rcv_kwargs)

    def dep_2_3_or_2_8_config_2_5(self, tc_kwargs):
        """replaces dependency version to 2.3 || 2.8 and configuration version to 2.5"""

        try:
            files_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rdv_kwargs = tc_kwargs['file_a']['mod_cur_dep']
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, file_a or file_b arguments.')

        rdv_kwargs['file_path'] = files_path + 'TP-A.xml'
        rcv_kwargs['file_path'] = files_path + 'TP-B.xml'

        self.replace_dependency_version(rdv_kwargs)
        self.replace_configuration_version(rcv_kwargs)

    def dep_2_3_or_2_8_config_2_9(self, tc_kwargs):
        """replaces dependency version to 2.3 || 2.8 and configuration version to 2.9"""

        try:
            files_path = tc_kwargs['abs_files_path']
            # Found within tc_method_kwargs variable within test_cases.json
            rdv_kwargs = tc_kwargs['file_a']['mod_cur_dep']
            rcv_kwargs = tc_kwargs['file_b']['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, file_a or file_b arguments.')

        rdv_kwargs['file_path'] = files_path + 'TP-A.xml'
        rcv_kwargs['file_path'] = files_path + 'TP-B.xml'

        self.replace_dependency_version(rdv_kwargs)
        self.replace_configuration_version(rcv_kwargs)

    def dep_xml_del(self, tc_kwargs):
        """deletes the dependency file while keeping the dependency"""

        try:
            file_path = tc_kwargs['abs_files_path']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_file_path_b.')

        self.delete_xml(file_path + 'TP-B.xml')

    def interdep_test_case_1(self, tc_kwargs):
        """TP-A dependency on TP-B 2.0 | TP-B version 3.0 | Result Fail
        TP-A dependency on TP-C 2.0 | TP-C version 2.1 | Result Pass"""

        try:
            files_path = tc_kwargs['abs_files_path']
            new_dep = tc_kwargs["file_a"]["new_dep"]
            rcv_b_kwargs = tc_kwargs["file_b"]['mod_cur_config']
            rcv_c_kwargs = tc_kwargs["file_c"]['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, new_dep, file_b, or file_c Arguments.')

        new_dep["file_path"] = files_path + 'TP-A.xml'
        rcv_b_kwargs["file_path"] = files_path + 'TP-B.xml'
        rcv_c_kwargs["file_path"] = files_path + 'TP-C.xml'

        self.new_dep(new_dep)
        self.replace_configuration_version(rcv_b_kwargs)
        self.replace_configuration_version(rcv_c_kwargs)

    def interdep_test_case_2(self, tc_kwargs):
        """TP-A dependency on TP-B 2.1 | TP-B version 2.0 | Result Fail
        TP-A dependency on TP-C 2.0 || 2.5 | TP-C version 2.3 | Result Pass"""

        try:
            files_path = tc_kwargs['abs_files_path']
            new_dep = tc_kwargs["file_a"]["new_dep"]
            rdv_a_kwargs = tc_kwargs["file_a"]['mod_cur_dep']
            rcv_c_kwargs = tc_kwargs["file_c"]['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, new_dep, file_b, or file_c Arguments.')

        new_dep["file_path"] = files_path + 'TP-A.xml'
        rdv_a_kwargs["file_path"] = files_path + 'TP-A.xml'
        rcv_c_kwargs["file_path"] = files_path + 'TP-C.xml'

        self.new_dep(new_dep)
        self.replace_dependency_version(rdv_a_kwargs)
        self.replace_configuration_version(rcv_c_kwargs)

    def interdep_test_case_3(self, tc_kwargs):
        """TP-A dependency on TP-B 2.0 | TP-B version 3.1 | Result Fail
        TP-A dependency on TP-C 2.1 || 3.1 | TP-C version 3.3 | Result Pass"""

        try:
            files_path = tc_kwargs['abs_files_path']
            new_dep = tc_kwargs["file_a"]["new_dep"]
            rcv_b_kwargs = tc_kwargs["file_b"]['mod_cur_config']
            rcv_c_kwargs = tc_kwargs["file_c"]['mod_cur_config']
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, new_dep, file_b, or file_c Arguments.')

        new_dep["file_path"] = files_path + 'TP-A.xml'
        rcv_b_kwargs["file_path"] = files_path + 'TP-B.xml'
        rcv_c_kwargs["file_path"] = files_path + 'TP-C.xml'

        self.new_dep(new_dep)
        self.replace_configuration_version(rcv_b_kwargs)
        self.replace_configuration_version(rcv_c_kwargs)

    def interdep_test_case_4(self, tc_kwargs):
        """TP-A dependency on TP-B 2.3 | TP-B version 2.5 | Result Pass
        TP-A dependency on TP-C 2.5 | TP-C version 2.3 | Result Fail
        TP-B dependency on TP-C 3.0 | TP-C version 2.3 | Result Fail
        TP-C dependency on TP-A 2.7 || 3.1 | TP-A version 3.0 | Result Fail"""

        try:
            files_path = tc_kwargs['abs_files_path']

            new_dep_a = tc_kwargs["file_a"]["new_dep"]
            rcv_a_kwargs = tc_kwargs["file_a"]["mod_cur_config"]
            rdv_a_kwargs = tc_kwargs["file_a"]["mod_cur_dep"]

            rcv_b_kwargs = tc_kwargs["file_b"]['mod_cur_config']
            new_dep_b = tc_kwargs["file_b"]["new_dep"]

            rcv_c_kwargs = tc_kwargs["file_c"]['mod_cur_config']
            new_dep_c = tc_kwargs["file_c"]["new_dep"]
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, new_dep, file_b, or file_c Arguments.')

        rdv_a_kwargs["file_path"] = files_path + 'TP-A.xml'

        rcv_a_kwargs["file_path"] = files_path + 'TP-A.xml'
        rcv_b_kwargs["file_path"] = files_path + 'TP-B.xml'
        rcv_c_kwargs["file_path"] = files_path + 'TP-C.xml'

        new_dep_a["file_path"] = files_path + 'TP-A.xml'
        new_dep_b["file_path"] = files_path + 'TP-B.xml'
        new_dep_c["file_path"] = files_path + 'TP-C.xml'

        self.replace_dependency_version(rdv_a_kwargs)

        self.replace_configuration_version(rcv_a_kwargs)
        self.replace_configuration_version(rcv_b_kwargs)
        self.replace_configuration_version(rcv_c_kwargs)

        self.new_dep(new_dep_a)
        self.new_dep(new_dep_b)
        self.new_dep(new_dep_c)

    def interdep_test_case_5(self, tc_kwargs):
        """TP-A dependency on TP-B 2.5 | TP-B version 2.7 | Result Pass
        TP-A dependency on TP-C 3.0 || 3.5 | TP-C version 3.3 | Result Pass
        TP-A dependency on TP-D 2.5 | TP-D Missing | Result Fail
        TP-B dependency on TP-A 3.0 | TP-A version 2.3 | Result Fail
        TP-C dependency on TP-B 2.9 | TP-B 2.7 | Result Fail
        TP-C dependency on TP-D 2.1 | TP-D Missing | Result Fail"""

        try:
            files_path = tc_kwargs['abs_files_path']

            new_dep_a_on_c = tc_kwargs["file_a"]["new_dep_c"]
            new_dep_a_on_d = tc_kwargs["file_a"]["new_dep_d"]
            rcv_a_kwargs = tc_kwargs["file_a"]["mod_cur_config"]
            rdv_a_kwargs = tc_kwargs["file_a"]["mod_cur_dep"]

            rcv_b_kwargs = tc_kwargs["file_b"]['mod_cur_config']
            new_dep_b = tc_kwargs["file_b"]["new_dep"]

            rcv_c_kwargs = tc_kwargs["file_c"]['mod_cur_config']
            new_dep_c_b = tc_kwargs["file_c"]["new_dep_b"]
            new_dep_c_d = tc_kwargs["file_c"]["new_dep_d"]
        except KeyError:
            self.tc_kwargs_error(msg='tc_kwargs missing abs_files_path, new_dep, file_b, or file_c Arguments.')

        rdv_a_kwargs["file_path"] = files_path + 'TP-A.xml'

        rcv_a_kwargs["file_path"] = files_path + 'TP-A.xml'
        rcv_b_kwargs["file_path"] = files_path + 'TP-B.xml'
        rcv_c_kwargs["file_path"] = files_path + 'TP-C.xml'

        new_dep_a_on_c["file_path"] = files_path + 'TP-A.xml'
        new_dep_a_on_d["file_path"] = files_path + 'TP-A.xml'
        new_dep_b["file_path"] = files_path + 'TP-B.xml'
        new_dep_c_b["file_path"] = files_path + 'TP-C.xml'
        new_dep_c_d["file_path"] = files_path + 'TP-C.xml'

        self.replace_dependency_version(rdv_a_kwargs)

        self.replace_configuration_version(rcv_a_kwargs)
        self.replace_configuration_version(rcv_b_kwargs)
        self.replace_configuration_version(rcv_c_kwargs)

        self.new_dep(new_dep_a_on_c)
        self.new_dep(new_dep_a_on_d)
        self.new_dep(new_dep_b)
        self.new_dep(new_dep_c_b)
        self.new_dep(new_dep_c_d)
