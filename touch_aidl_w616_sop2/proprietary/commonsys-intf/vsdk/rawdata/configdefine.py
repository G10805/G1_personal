#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

import os
import processdefine
import sys
from xml.etree import ElementTree as ET
import xml.dom.minidom


# Check the configuration rule
def check_config_rule(rule):
    if rule.nodeName == 'project':
        src = rule.getAttribute('src_path').strip()
        des = rule.getAttribute('dest_path').strip()
        if src:
            if des:
                processdefine.project_define.append((des, src, des))
            else:
                processdefine.project_define.append((src, src, des))

            for nod in rule.childNodes:
                if nod.nodeName == 'symlink':
                    if des:
                        check_additional_info(nod, des)
                    else:
                        check_additional_info(nod, src)

    elif rule.nodeName == 'file':
        src = rule.getAttribute('src_path').strip()
        des = rule.getAttribute('dest_path').strip()
        if src:
            if des:
                processdefine.file_define.append((des, src, des))
            else:
                processdefine.file_define.append((src, src, des))
            for nod in rule.childNodes:
                if nod.nodeName == 'symlink':
                    if des:
                        check_additional_info(nod, None, des)
                    else:
                        check_additional_info(nod, None, src)


# Check the additional info for the rule
def check_additional_info(nod, *base):
    src = nod.getAttribute('src_path').strip()
    link = nod.getAttribute('link_path').strip()
    if link:
        if len(base) - 1:
            if src:
                processdefine.link_define.append((link, os.path.relpath(src, os.path.dirname(link))))
            else:
                processdefine.file_define.append((link, link, None))
        else:
            if src:
                processdefine.link_define.append((link, os.path.relpath(os.path.join(base[0], src), os.path.dirname(link))))
            else:
                processdefine.file_define.append((link, link, None))


# Check the struct of configuration
def check_conf(path):
    try:
        root = xml.dom.minidom.parse(path)
    except (OSError, xml.parsers.expat.ExpatError) as e:
        raise print("Configuration err:", e)
    return root.documentElement


# Check the configuration define
def check_def(package_config_path):
    config_define = check_conf(package_config_path)

    raw_data = config_define.getElementsByTagName('rawdata')
    for rule_define in raw_data[0].childNodes:
        check_config_rule(rule_define)

    allow_list = config_define.getElementsByTagName('allowlist')
    for rule_define in allow_list[0].childNodes:
        check_config_rule(rule_define)
