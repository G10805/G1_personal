#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (c) 2022 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.


"""This class runs the workflow for api_dep test cases.

IMPORTANT:
 - ApiDepCompatibility class must contain an implementation of all the
   tc_method variables found within test_cases.json

SCRIPT LOGIC:
 - A copy of the files to be edited indicated within the
   test_case_kwargs, is placed within the copy_dir
 - The appropriate tc_method indicated within test_case_kwargs is
   dynamically called within api_dep_tc_methods
 - qiifa_main --type api_dep is run and the result of the test case
   is found
 - Finally, the instance of the file within copy_dir is used to
   restore the file back to original state
"""

import os

from compatibility_check import CompatibilityCheck
from api_dep_tc_methods import ApiDepTCMethods
from utils import run_cmd, copy_file


class API_DEPCompatibility(CompatibilityCheck):
    _copy_dir_name = 'copy_dir'

    def __init__(self):
        super(API_DEPCompatibility, self).__init__()
        self._api_dep_tc_methods = ApiDepTCMethods()

    def compatibility(self, test_case_kwargs):
        """Determines if the library of the given test case
        is backwards compatible or not.
        """
        try:
            files = test_case_kwargs['files']
            post_check = test_case_kwargs.get('post_check')
        except KeyError:
            raise Exception('Test case missing file_path.')

        abs_path_to_files = os.path.join(self.build_path, files)

        # Copy the file to be edited for the test case into copy_dir
        copy_files_path = self._copy_files_to_copy_dir(abs_path_to_files)

        try:
            # call_tc_method will dynamically call the appropriate tc_method
            self._call_tc_method(abs_path_to_files, test_case_kwargs)
        except Exception:
            # Restore the original file that was altered
            self._restore_files(abs_path_to_files, copy_files_path)
            raise

        # Edit file and find the return val of running test case

        result = self._type_api_dep_result(post_check)

        # Restore the original file that was altered
        self._restore_files(abs_path_to_files, copy_files_path)
        return result

    def _call_tc_method(self, abs_path_to_files, test_case_kwargs):
        """Dynamically calls the appropriate tc_method and
        passes along the tc_kwargs.
        """

        try:
            tc_method = test_case_kwargs['tc_method']
            tc_method_kwargs = test_case_kwargs['tc_method_kwargs']
            tc_method_kwargs['abs_files_path'] = abs_path_to_files
        except KeyError:
            raise Exception('Test case missing tc_method or tc_method kwargs.')

        try:
            # Dynamically get and run tc_method from the ApiDepTCMethods class
            method = getattr(self._api_dep_tc_methods, tc_method)
            method(tc_method_kwargs)
        except AttributeError:
            raise Exception('Given tc_method is not currently implemented.')

    def _copy_files_to_copy_dir(self, files_path):
        """Copy the file to the copy_dir directory before running
        test cases alter the contents.
        """

        test_framework_path = os.path.dirname(os.path.realpath(__file__))
        copy_dir_path = os.path.join(test_framework_path, self._copy_dir_name)

        if not os.path.exists(copy_dir_path):
            os.mkdir(copy_dir_path)

        for file in os.listdir(files_path):
            source = files_path + '/' + file
            destination = copy_dir_path + '/' + file
            copy_file(source, destination)

        return copy_dir_path

    def _restore_files(self, file_path, copy_path):
        """Restore the given file original state of the file before
        the test cases.
        """
        for file in os.listdir(copy_path):
            source = copy_path + '/' + file
            destination = file_path + '/' + file
            copy_file(source, destination)

    def _type_api_dep_result(self, post_check):
        """Run python qiifa_main.py --type api_dep to determine
        if test case results in 0 (pass) or anything else (fail).
        if post_check is set it will check stout for post check"""

        commands_lst = ['cd ' + self.build_path, 'cd vendor/qcom/proprietary/commonsys-intf/QIIFA-fwk/',
                        'python qiifa_main.py --type api_dep --techpack_build --enforced 1 --test_source '
                        + self.build_path +
                        '/vendor/qcom/proprietary/commonsys-intf/QIIFA-fwk/test_framework/api_dep_files']

        # Get the return value of the qiifa_main --type api_dep subprocess
        output = run_cmd(commands_lst)

        result = 0

        if post_check:
            for s in post_check:
                if s not in output['stdout']:
                    result = 1

        else:
            result = output['returncode']

        return 'pass' if result == 0 else 'fail'
