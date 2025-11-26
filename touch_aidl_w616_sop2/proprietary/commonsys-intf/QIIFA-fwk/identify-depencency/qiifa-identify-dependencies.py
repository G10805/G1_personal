#!/usr/bin/python
# -*- coding: utf-8 -*-
#Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

"""
The script will print out the public libraries.
Logic:
1) Find all the makefiles within the given library (.bp and .mk)
2) Grep the LOCAL_MODULES (for .mk) and name: (for .bp) from the makefiles to get all the local modules
3) Iterate through the libraries found within the makefiles
4) Run the ninja command on each library
5) Determine if the library is public or not by looking through whitelist
6) Return list of all the public libraries among the LOCAL_MODULES

IMPORTANT:
- Must run source build/envsetup.sh and lunch <product>-userdebug before running script
- Must change the _output_whitelist_path and _input_whitelist_path to the path of where the whitelists are located.
- Must run from top level of vendor

HOW TO RUN EXAMPLE:
python new_find_public_libs.py --library_path camera-projects.json --whitelists camera --recommended android-s-recommended.json
"""

import subprocess
import argparse
import json
import time
import os
import re


class FindPublicLib:

    _out_path = os.getenv("OUT")
    _out_path_split = out_path.split("/")
    _product_found_index = 0
    _count = 0
    _qiifa_out_path_target_value = ""
    for _out_path_word in _out_path_split:
        if _out_path_word == "product":
            product_found_index = _count
        if _count == ( product_found_index + 1):
            if _out_path_word.find("qssi"):
                _qiifa_out_path_target_value = "qssi"
            else:
                _qiifa_out_path_target_value = _out_path_word
        _count=_count+1

    # Path to this file
    _file_path = os.path.dirname(os.path.abspath(__file__))

    # The ninja command
    _out_dir = 'out/target/product/' + _qiifa_out_path_target_value + '/vendor/lib64'
    _cmd_prefix = 'prebuilts/build-tools/linux-x86/bin/ninja -f out/combined-' + _qiifa_out_path_target_value + '.ninja -t query '

    # Path to the ninja logs directory
    _ninja_log_dir = os.path.join(_file_path, 'ninja_logs')

    def __init__(self, lib_path, whitelists, output_whitelist, input_whitelist):
        self._output_whitelist = output_whitelist
        self._input_whitelist = input_whitelist
        self._lib_path, self._whitelists  = lib_path, whitelists

    def _load_whitelist(self, whitelist_path):
        """Loads the whitelisted libraries from the json file."""
        with open(whitelist_path, 'r') as fp:
            whitelist = json.load(fp)
            return whitelist

    # Linux commands
    def _run_ninja_cmd(self, lib_name):
        """Runs the ninja command and returns the output."""
        out = ''
        lib_path = self._find_lib_path(lib_name)
        ninja_cmd = self._cmd_prefix + self._out_dir + lib_path
        if lib_path:
            out = self._run_cmd(ninja_cmd)
        return out

    def _find_lib_path(self, lib_name):
        """Calls find command within the out_dir for the library."""
        cmd = 'find . -name ' + lib_name + '.so'
        out_dir_abs_path = os.path.join(self._file_path, self._out_dir)
        lib_path = self._run_cmd(cmd, out_dir_abs_path)
        return lib_path[1:]

    def _run_cmd(self, cmd, cwd=None):
        """Runs the give command and returns the output."""
        if cwd:
            process = subprocess.Popen('/bin/bash', stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=cwd)
            out, err = process.communicate(cmd)
        else:
            process = subprocess.Popen('/bin/bash', stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            out, err = process.communicate(cmd)
        return str(out)

    # Finding the local modules for bp and mk files
    def _parse_find_cmd(self, find_output):
        """Parse the find cmd for the bp and mk files."""
        files = []
        for f in find_output.split('\n'):
            if len(f) > 0:
                f_path = os.path.join(self._lib_path, f[2:])
                files.append(f_path)
        return files

    def _find_modules_helper(self, file_ext, mod_keyword, split_keyword):
        """A helper method that contains the same logic for bp and mk files."""
        modules = []
        makefiles = self._run_cmd('find . -name "*.' + file_ext + '"', cwd=self._lib_path)
        for f_path in self._parse_find_cmd(makefiles):
            local_module = self._run_cmd('grep "' + mod_keyword + '" ' + f_path)
            if len(local_module) > 0 and '#' not in local_module:
                lib = local_module.split(split_keyword)[1].strip().split('\n')[0]
                modules.append(lib)
        return modules

    def _find_modules(self):
        """Finds all the modules within bp and mk files within the library_path."""
        modules = set()
        extensions = ['mk', 'bp']
        mod_keywords = ['LOCAL_MODULE ', 'name: ']
        split_keywords = [' := ', '"']
        for ext in extensions:
            ind = extensions.index(ext)
            mods = self._find_modules_helper(ext, mod_keywords[ind], split_keywords[ind])
            for mod in mods:
                modules.add(mod)
        return modules

    # Output library parsing
    def _create_output_lib_set(self, output):
        """Parse the output of ninja command and create a set of the the output libraries."""
        lib_set = set()
        outputs_flag = False
        for line in output.split('\n'):
            if not outputs_flag:
                if 'outputs:' in line:
                    outputs_flag = True
            else:
                lib = line.strip()
                if len(lib) > 0:
                    lib_set.add(lib)
        return lib_set

    def _is_public_lib(self, lib_name, output_lib_set):
        """Determines if a library is public from its output libraries."""
        match_set = set()
        for whitelist in self._whitelists:
            for wl_lib in self._output_whitelist[whitelist]:
                if '*' in wl_lib:
                    wl_lib = self._insert_wildcard(wl_lib)

                # Check if the library itself is within whitelist
                if re.search(wl_lib, lib_name):
                    return False

                # Check if any output libraries of the lib are within the whitelist
                for out_lib in output_lib_set:
                    if re.search(wl_lib, out_lib):
                        match_set.add(out_lib)

                # If all the libs within output_lib_set are within t
                if len(match_set) == len(output_lib_set):
                    return False
        return True

    # Input libarary parsing
    def _create_input_lib_set(self, output):
        """Parse the output of ninja command and create a set of the the output libraries."""
        lib_set = set()
        for line in output.split('\n'):
            if '||' in line and len(line) > 0:
                lib = line.split('||')[1].strip()
                lib_set.add(lib)
            elif 'outputs:' in line:
                break
        return lib_set

    def _get_dependencies(self, input_lib_set):
        """Determine the dependencies from the input libraries."""
        dep_set = input_lib_set
        match_set = set()
        for whitelist in self._whitelists:
            for wl_lib in self._input_whitelist[whitelist]:
                if '*' in wl_lib:
                    wl_lib = self._insert_wildcard(wl_lib)

                # Check if any output libraries of the lib are within the whitelist
                for in_lib in dep_set:
                    if re.search(wl_lib, in_lib):
                        match_set.add(in_lib)

                # Remove the whitelisted libraries within the match_set
                dep_set -= match_set
        return dep_set

    # Main method helpers
    def _insert_wildcard(self, strng):
        """Insert '.' wildcard before any '*' within a string for regex and prevent regex insertions."""
        special_chars = ['.', '+', '?', '^', '$']
        regex = []
        for s in strng:
            if s == '*':
                regex.append('.')
            elif s in special_chars:
                regex.append('\\')
            regex.append(s)
        return ''.join(regex)

    def _log_ninja_output(self, output, log_dir, f_name):
        """Log the given output at the given log directory under the file name."""
        if not os.path.exists(log_dir):
            os.mkdir(log_dir)

        # Create a separate folder for the library within the ninja_log directory
        lib_path_lst = self._lib_path.split('/')
        lib_name = lib_path_lst[-1] if lib_path_lst[-1] else lib_path_lst[-2]
        lib_folder_path = os.path.join(log_dir, lib_name)
        if not os.path.exists(lib_folder_path):
            os.mkdir(lib_folder_path)

        # Write the output to the log file
        log_path = os.path.join(lib_folder_path, f_name)
        with open(log_path, 'w') as fp:
            fp.write(output)

    def main(self):
        """Run grep on the library_path to find the local_modules and returns the public libraries within that dir."""
        public_lib_set = set()
        dep_lib_set = set()

        # Find all the modules contained within the various .bp and .mk makefiles
        #print('finding local mdoules')
        local_module_libs = self._find_modules()
        for lib in local_module_libs:
            # Run ninja command on library
            ninja_out = self._run_ninja_cmd(lib)
            self._log_ninja_output(ninja_out, self._ninja_log_dir, lib + '_ninja.log')

            # Input and output parsing
            out_lib_set = self._create_output_lib_set(ninja_out)
            in_lib_set = self._create_input_lib_set(ninja_out)

            # Determine if library is public
            is_pub_lib = self._is_public_lib(lib, out_lib_set)
            if is_pub_lib:
                public_lib_set.add(lib)

            # Add depdendencies of the library to dep_lib_set
            dep_set = self._get_dependencies(in_lib_set)
            for dep in dep_set:
                dep_lib_set.add(dep)

        return public_lib_set, dep_lib_set



class FindPublicLibs:

    # Path to this file
    _file_path = os.path.dirname(os.path.abspath(__file__))

    # Both output and input whitelist paths
    _output_whitelist_path = os.path.join(_file_path, 'output_whitelist.json')
    _input_whitelist_path = os.path.join(_file_path, 'input_whitelist.json')

    def __init__(self):
        # Output and Input specific whitelists
        self._output_whitelist = self._load_json(self._output_whitelist_path)
        self._input_whitelist = self._load_json(self._input_whitelist_path)

        self._lib_path, self._whitelists, self._recommended  = self._parse_arguments()

        self._lib_paths_lst = self._load_json(self._lib_path)
        self._rec_lst = self._load_json(self._recommended)

    # Init method helpers
    def _parse_arguments(self):
        """Parses the command line arguments."""
        parser = argparse.ArgumentParser()
        parser.add_argument('--library_path', nargs=1, type=str, help='the path to the json file that contains the libraries to be run.', required=True)
        parser.add_argument('--whitelists', nargs='+', type=str, help='the whitelists to be used. Supported whitelists: ' + ', '.join(list(set(self._output_whitelist) - {'standard'})))
        parser.add_argument('--recommended', nargs=1, type=str, help='the path to the json file that contains the recommended library versions.', required=True)
        args = parser.parse_args()

        lib_path = args.library_path[0]
        recommended = args.recommended[0]
        whitelists = args.whitelists if args.whitelists is not None else []
        whitelists.append('standard')

        # If unidentified whitelist is entered, the script throws an error
        for whitelist in [self._output_whitelist, self._input_whitelist]:
            for wl in whitelists:
                if wl not in whitelist:
                    parser.error(wl + ' not found within the whitelists. Supported whitelists: ' + ', '.join(list(set(self._output_whitelist) - {'standard'})))
        return lib_path, whitelists, recommended

    def _load_json(self, json_path):
        """Loads the libraries paths from the json file."""
        with open(json_path, 'r') as fp:
            lib_paths = json.load(fp)
            return lib_paths

    def _find_outdated_versions(self, dep_set):
        outdated_lst = []
        for lib in self._rec_lst:
            lib_lst = lib.split('@')
            lib_name = lib_lst[0]
            lib_version = lib_lst[1][:-3]
            for dep in dep_set:
                dep_lib = dep.split('/')[-1]
                if '@' in dep_lib:
                    dep_lib_lst = dep_lib.split('@')
                    dep_lib_name = dep_lib_lst[0]
                    dep_lib_version = dep_lib_lst[1][:-3]

                    if lib_name == dep_lib_name and dep_lib_version < lib_version:
                        outdated_lst.append(dep)
        return outdated_lst

    def _print_results(self, pub_lib_set, dep_lib_set, outdated_versions):
        """Print the results of the script that include the public libraries and the dependencies."""
        print('Library Paths: ' + '\n' + ', '.join(self._lib_paths_lst))
        print('\n')

        # Print the public libraries
        print('Public Libraries:')
        for lib in pub_lib_set:
            print(lib)
        print('\n')

        # Print the dependencies
        print('Dependencies:')
        for lib in dep_lib_set:
            print(lib)
        print('\n')

        # Print the outdated dependency versions
        print('Warning: Outdated versions used ' )
        for outdated in outdated_versions:
            print(outdated)


    def main(self):
        """Run FindPublicLib().main() on each of the libraries within the json file that contains the libraries."""
        public_lib_set = set()
        dep_lib_set = set()
        for lib_path in self._lib_paths_lst:
            find_pub_lib = FindPublicLib(lib_path, self._whitelists, self._output_whitelist, self._input_whitelist)
            temp_pub_lib_set, temp_dep_lib_set = find_pub_lib.main()

            # Add the public and dependency libraries to the main list and set
            for lib in temp_pub_lib_set:
                public_lib_set.add(lib)
            for dep in temp_dep_lib_set:
                dep_lib_set.add(dep)

        outdated_versions = self._find_outdated_versions(dep_lib_set)
        self._print_results(public_lib_set, dep_lib_set, outdated_versions)


if __name__ == '__main__':
    start = time.time()
    FindPublicLibs().main()
    print('\ntotal time taken: ' + str(time.time() - start))
