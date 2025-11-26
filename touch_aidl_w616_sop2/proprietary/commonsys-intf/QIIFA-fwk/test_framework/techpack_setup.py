#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

"""This class will setup the techpack plugin to be able to run for test_framework.

Can run this file after a new libTestCase is added and will produce
golden lsdump for the new test case.

"""

import json
import sys
import os

from utils import run_cmd, copy_file


class TECHPACKSetup(object):

    _tmp_dir = '/var/tmp'
    _techpack_tc_libs_name = 'techpack_tc_libs'
    _ref_libtestcase_name = 'libTechPack'
    _unittest_conf_name = 'techpackage-techpack.json'

    _so_files_path = 'out/target/product/qssi/vendor/lib64'
    _qiifa_main_path = 'vendor/qcom/proprietary/commonsys-intf/QIIFA-fwk'
    _techpack_config_path = os.path.join(_qiifa_main_path,
                                    'plugins/qiifa_abi_checker/abi_config.json')
    _techpack_tc_libs_path = os.path.join(_qiifa_main_path,
                                     'test_framework/techpack_tc_libs')
    _ref_libtestcase_path = os.path.join(_techpack_tc_libs_path,
                                         _ref_libtestcase_name)
    _golden_dir = 'vendor/qcom/proprietary/commonsys-intf/QIIFA-cmd/' \
                  'abi-dumps/techpack-unittest'
    _target_file = 'device/qcom/pineapple/pineapple.mk'

    def __init__(self, build_path):
        self._build_path = build_path
        self._file_path = os.path.abspath(os.path.dirname(__file__))

    def _abs_path(self, path):
        """Add the path to croot to the given path."""

        abs_path = os.path.join(self._build_path, path)
        return abs_path

    def main(self):
        """Runs the various parts of the techpack setup."""
        self.update_target_mk()
        self.add_unittest_config_to_techpack_config()
        if not self.is_updated():
            self.copy_techpack_tc_libs_to_tmp()
            self.create_ref_copies()
            edited = self.run_mm()
            if edited:
                self.compile_from_croot()
            self.restore_original_techpack_tc_libs()

    def update_target_mk(self):
        """Update target.mk files with PRODUCT_PACKAGES.
        """
        commands_lst = ['more ' + self._abs_path(self._target_file) + ' | grep libTechPack | wc -l']
        target_output = run_cmd(commands_lst)['stdout']
        if '0' in target_output:
            for root, dirs, files in os.walk(self._abs_path(self._techpack_tc_libs_path),
                                             topdown=False):
                file_object = open(self._abs_path(self._target_file), 'a')
                for tc_dir in dirs:
                    file_object.write('\nPRODUCT_PACKAGES += ' + tc_dir)
                file_object.close()
        return True

    def is_updated(self):
        """Determines if the setup should be run."""

        golden_dirs = []

        # Walk through the unittest golden dir and find the golden lsdumps libs
        level = 0
        for _, dirs, _ in os.walk(self._abs_path(self._golden_dir)):
            if level == 2:
                golden_dirs = set(dirs)
                break
            level += 1

        # This will be true during very first techpack setup
        if level != 2:
            return False

        # Iterates through the tc dirs and checks if golden copies exist
        for _, dirs, _ in os.walk(self._abs_path(self._techpack_tc_libs_path),
                                  topdown=False):
            for tc_dir in dirs:
                if tc_dir not in golden_dirs:
                    return False
        return True

    def copy_techpack_tc_libs_to_tmp(self):
        """Create a copy of techpack_tc_libs within the _tmp_dir."""

        techpack_tc_libs_path = self._abs_path(self._techpack_tc_libs_path)
        cmd_lst = ['rm -r {}/{}'.format(self._tmp_dir, self._techpack_tc_libs_name),
                   'cp -r {} {}'.format(techpack_tc_libs_path, self._tmp_dir)]
        run_cmd(cmd_lst)

    def create_ref_copies(self):
        """Reset all libTestCase libs to the contents of the
        golden reference.
        """

        cp_files = []
        ref_tc_abs_path = self._abs_path(self._ref_libtestcase_path)
        for root, dirs, files in os.walk(ref_tc_abs_path, topdown=False):
            files.remove('Android.bp')
            cp_files = files

        for root, dirs, files in os.walk(self._abs_path(self._techpack_tc_libs_path),
                                         topdown=False):
            for tc_dir in dirs:
                for cp_file in cp_files:
                    f_path = os.path.join(ref_tc_abs_path, cp_file)
                    dir_abs_path = os.path.join(root, tc_dir)
                    copy_file(f_path, dir_abs_path)

    def run_mm(self):
        """Run mm on the libTestCase libs that don't have corresponding .so
        files and add them to qiifa_unittest_config.json.
        """

        edited = False
        so_file_dirs = set()
        for _, _, files in os.walk(self._abs_path(self._so_files_path),
                                   topdown=False):
            for so_file in files:
                so_file_dirs.add(so_file[:-3])

        for root, dirs, _ in os.walk(self._abs_path(self._techpack_tc_libs_path),
                                     topdown=False):
            for tc_dir in dirs:
                conf_edited = self.add_so_to_unittest_config(tc_dir)
                edited = edited or conf_edited

        # Copy the edited version of the unittest_config to tmp_dir
        self.copy_unittest_config_to_tmp()

        # Create the golden unittest directory if it doesn't already exist
        unittest_edited = self.create_unittest_dir()
        return edited or unittest_edited

    def copy_unittest_config_to_tmp(self):
        """Copy the edited version of unittest_config to the copy of
        techpack_tc_libs within _tmp_dir.
        """

        test_conf_path = self._abs_path(os.path.join(self._techpack_tc_libs_path,
                                                     self._unittest_conf_name))
        techpack_tc_tmp_path = os.path.join(self._tmp_dir,
                                       self._techpack_tc_libs_name)
        cmd_lst = ['cp {} {}'.format(test_conf_path, techpack_tc_tmp_path)]
        run_cmd(cmd_lst)

    def create_unittest_dir(self):
        """Create the unittest dir for the golden lsdumps."""

        golden_dir_abs_path = os.path.join(self._build_path, self._golden_dir)
        try:
            os.mkdir(golden_dir_abs_path)
        except OSError:
            return False
        return True

    def add_unittest_config_to_techpack_config(self):
        """Add the the qiifa_unittest_config.json path to
        techpackages_json_file_list.
        """

        with open(self._abs_path(self._techpack_config_path), 'r') as fp:
            techpack_config = json.load(fp)
            test_conf_path = os.path.join(self._techpack_tc_libs_path,
                                          self._unittest_conf_name)
            for d in techpack_config:
                key_str = 'techpacakages_json_file_list'
                if key_str in d:
                    d[key_str] = []
                    if test_conf_path not in d[key_str]:
                        d[key_str].append(test_conf_path)
                    d['enabled'] = "true"

        with open(self._abs_path(self._techpack_config_path), 'w') as fp:
            json.dump(techpack_config, fp, indent=2)

    def add_so_to_unittest_config(self, tc_dir):
        """Given test case directory, add the .so file attached
        to the dir to qiifa_unittest_config.json.
        """

        test_conf_path = self._abs_path(os.path.join(self._techpack_tc_libs_path,
                                                     self._unittest_conf_name))
        with open(test_conf_path, 'r') as fp:
            test_config = json.load(fp)
            lib_lst = test_config['library_list']
            entry = '{}'.format(tc_dir)
            if entry not in lib_lst:
                lib_lst.append(entry)
                lib_lst.sort()  # Sort to keep .so files in named order
            else:
                # If no change to the file, then simply return False
                return False
            test_config['library_list'] = lib_lst

        with open(test_conf_path, 'w') as fp:
            json.dump(test_config, fp, indent=2)
        return True

    def compile_from_croot(self):
        """Compile the build from croot."""

        comp_cmd = 'bash build.sh -j64 dist --target_only' \
                   ' --tech_package=techpackage-techpack:golden'
        cmd_lst = ['cd {}'.format(self._build_path),
                   'source build/envsetup.sh',
                   'lunch pineapple-userdebug',
                   comp_cmd]
        run_cmd(cmd_lst)

    def restore_original_techpack_tc_libs(self):
        """Restore the original version of techpack_tc_libs from within
        _tmp_dir to within test_framework.
        """

        techpack_tc_libs_abs_path = self._abs_path(self._techpack_tc_libs_path)
        cmd_lst = ['rm -r "{}"'.format(techpack_tc_libs_abs_path),
                   'cp -r "{}/{}" "{}"'.format(self._tmp_dir,
                                               self._techpack_tc_libs_name,
                                               self._file_path)]
        run_cmd(cmd_lst)


if __name__ == '__main__':
    if len(sys.argv) == 2:
        LIB_NAME = sys.argv[1]
        setup = TECHPACKSetup(LIB_NAME)
        setup.main()
    else:
        print('Error. Need croot path as arg.')

