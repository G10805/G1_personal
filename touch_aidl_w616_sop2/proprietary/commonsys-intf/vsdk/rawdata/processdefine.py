#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

import logging
import os
import shutil
import sys
import subprocess
import tempfile

buf_define = []
file_define = []
link_define = []
project_define = []


# Handle the running buffer
def config_buf(handle_proc, clean_buf):
    buf_info = add_temp()
    try:
        handle_proc(buf_info)
    finally:
        if clean_buf:
            clean_temp()
        else:
            logging.info("buf: " + buf_info)


# Manage the temp config
def add_temp():
    new_buf = tempfile.mkdtemp()
    buf_define.append(new_buf)
    return new_buf


# remove the temp
def clean_temp():
    for buf in buf_define:
        if os.path.isdir(buf):
            shutil.rmtree(buf, ignore_errors=True)
        else:
            os.remove(buf)
    buf_define.clear()


# Check the result
def check_call(cmd):
    subprocess.check_call(cmd, shell=True)


# Handle the file
def file_proc(buf_input):
    base = os.path.abspath(os.curdir)
    buf = os.path.join(buf_input, "dir")

    for rule_project in project_define:
        path = os.path.join(base, rule_project[1])
        if rule_project[2]:
            project_buf = os.path.join(buf, rule_project[2])
        else:
            project_buf = os.path.join(buf, rule_project[1])
        if os.path.isdir(path):
            if not os.path.exists(project_buf):
                logging.info("Processing project: " + rule_project[1])
                shutil.copytree(path, project_buf, symlinks=True, ignore=shutil.ignore_patterns(".git"))
    for rule_file in file_define:
        path = os.path.join(base, rule_file[1])
        if rule_file[2]:
            project_buf = os.path.join(buf, rule_file[2])
        else:
            project_buf = os.path.join(buf, rule_file[1])
        if os.path.isfile(path) or os.path.islink(path):
            if not os.path.exists(project_buf):
                des_folder = os.path.dirname(project_buf)
                if not os.path.exists(des_folder):
                    os.makedirs(des_folder, exist_ok=True)
                if os.path.isdir(des_folder):
                    logging.info("Processing copyfile: " + rule_file[1])
                    shutil.copyfile(path, project_buf, follow_symlinks=False)

    for rule_link in link_define:
        if rule_link[0] and rule_link[1]:
            linpat = os.path.join(buf, rule_link[0])
            link_folder = os.path.dirname(linpat)
            if not (os.path.exists(linpat) or os.path.islink(linpat)):
                if not os.path.exists(link_folder):
                    os.makedirs(link_folder, exist_ok=True)
                logging.info("Processing symlink: " + rule_link[0] + " -> " + rule_link[1])
                os.symlink(rule_link[1], linpat)

    outpath = os.path.join(base, 'out', 'dist')
    if not os.path.exists(outpath):
        os.makedirs(outpath)

    if os.path.isdir(outpath):
        buf_file = os.path.join(outpath, "raw-qssi.zip")
        cwd_info = os.getcwd()
        if buf is not None:
            os.chdir(buf)
        sys_call_info = "zip -qry "
        sys_call_info += buf_file
        sys_call_info += " *"

        try:
            if os.path.exists(buf_file):
                os.remove(buf_file)
            logging.info("Preparing raw-qssi.zip...")
            check_call(sys_call_info)
        finally:
            if buf is not None:
                os.chdir(cwd_info)


# Check the status of the definition
def prepare_definition():
    file_define.sort()
    link_define.sort()
    project_define.sort()

    if len(project_define) + len(file_define) + len(link_define):
        config_buf(lambda buf_info: file_proc(buf_info), True)


# load the process
def pack_process():
    prepare_definition()
