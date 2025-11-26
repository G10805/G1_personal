#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (c) 2023 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.


"""This class runs the workflow for aidl test cases.

IMPORTANT:
 - AIDLTCMethods class must contain an implementation of all the
   tc_method variables found within test_cases.json

SCRIPT LOGIC:
 - A copy of the file to be edited indicated within the
   test_case_kwargs, is placed within the copy_dir
 - The appropriate tc_method indicated within test_case_kwargs is
   dynamically called within aidl_tc_methods
 - qiifa_main --type aidl is run and the result of the test case
   is found
 - Finally, the instance of the file within copy_dir is used to
   restore the file back to original state
"""

import shutil
import os

from compatibility_check import CompatibilityCheck
from aidl_vendor_tc_methods import AIDL_VENDORTCMethods
from utils import run_cmd, copy_file


class AIDL_VENDORCompatibility(CompatibilityCheck):

    _copy_dir_name = 'copy_dir'

    def __init__(self):
        super(AIDL_VENDORCompatibility, self).__init__()
        self._aidl_tc_methods = AIDL_VENDORTCMethods()

    def compatibility(self, test_case_kwargs):
        """Determines if the library of the given test case
        is backwards compatible or not.
        """

        try:
            file_path = test_case_kwargs['file_path']
            post_check = test_case_kwargs.get('post_check')
        except KeyError:
            raise Exception('Test case missing file_path.')

        abs_path_to_file = os.path.join(self.build_path, file_path)

        # Copy the file to be edited for the test case into copy_dir
        copy_file_path = self._copy_file_to_copy_dir(abs_path_to_file)

        try:
            # call_tc_method will dynamically call the appropriate tc_method
            self._call_tc_method(abs_path_to_file, test_case_kwargs)
        except Exception:
            # Restore the original file that was altered
            self._restore_file(abs_path_to_file, copy_file_path)
            raise

        # Edit file and find the return val of running type aidl for test case
        result = self._type_aidl_result(post_check)

        # Restore the original file that was altered
        self._restore_file(abs_path_to_file, copy_file_path)
        return result

    def _call_tc_method(self, abs_file_path, test_case_kwargs):
        """Dynamically calls the appropriate tc_method and
        passes along the tc_kwargs.
        """

        try:
            tc_method = test_case_kwargs['tc_method']
            tc_method_kwargs = test_case_kwargs['tc_method_kwargs']
            tc_method_kwargs['abs_file_path'] = abs_file_path
        except KeyError:
            raise Exception('Test case missing tc_method or tc_method kwargs.')

        try:
            # Dynamically get and run tc_method from the AIDLTCMethods class
            method = getattr(self._aidl_tc_methods, tc_method)
            method(tc_method_kwargs)
        except AttributeError:
            raise Exception('Given tc_method is not currently implemented.')

    def _copy_file_to_copy_dir(self, file_path):
        """Copy the file to the copy_dir directory before running
        test cases alter the contents.
        """

        curr_file_path = os.path.dirname(os.path.realpath(__file__))
        copy_dir_path = os.path.join(curr_file_path, self._copy_dir_name)
        if not os.path.exists(copy_dir_path):
            os.mkdir(copy_dir_path)

        copy_file_name = file_path.split('/')[-1] + '_copy'
        copy_file_path = os.path.join(copy_dir_path, copy_file_name)
        copy_file(file_path, copy_file_path)
        return copy_file_path

    def _restore_file(self, file_path, copy_path):
        """Restore the given file original state of the file before
        the test cases.
        if post_check is set it will check stout for post check"""

        result = copy_file(copy_path, file_path)
        return result

    def _type_aidl_result(self, post_check):
        """Run python qiifa_main.py --type aidl to determine
        if test case results in 0 (pass) or anything else (fail).
        """

        commands_lst = ['cd ' + self.build_path,
                        'cd vendor/qcom/proprietary/commonsys-intf/QIIFA-fwk/',
                        'python qiifa_main.py --type aidl']

        # Get the return value of the qiifa_main --type aidl subprocess
        ret_val = 0

        if post_check:
            for s in post_check:
                if s not in run_cmd(commands_lst)['stdout']:
                    ret_val = 1
        else:
            ret_val = run_cmd(commands_lst)['returncode']

        # Check if python error is occuring during AIDL execution
        if 'Traceback' in run_cmd(commands_lst)['stderr']:
            print(run_cmd(commands_lst)['stderr'])
            return 'python error'

        return 'pass' if ret_val == 0 else 'fail'