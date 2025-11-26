#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

from xml.dom.minidom import parse
import os
# define global variable which store configs
gConfigs = {}
# define whether update function has been called
gHasUpdate = False

def update(path):
    '''
    update and parse config file

    @type path: string
    @param path: config file path
    @return: True, if it update config file successfully, otherwise, False
    '''

    if(len(path) == 0):
        print("Path is empty,please enter a correct path.")
        return False

    # not exist path
    if not os.path.exists(path):
        print(path, "does not exist, please enter a correct path.")
        return False

    try:
        domTree = parse(path)
    except Exception as e:
        print(path, "parse error, please check xml file")
        return False

    # clear global variables
    gConfigs.clear()

    # parse root node
    rootNode = domTree.documentElement

    # parse config
    configNode = rootNode.getElementsByTagName("config")[0]
    # parse public node of config
    publicNodes = configNode.getElementsByTagName("public")
    for publicNode in publicNodes:
        if publicNode.hasAttribute("name"):
            if len(publicNode.childNodes) > 0:
                gConfigs[publicNode.getAttribute("name").strip()] = publicNode.childNodes[0].data

    # parse private node of config
    privateNodes = configNode.getElementsByTagName("private")
    for privateNode in privateNodes:
        if privateNode.hasAttribute("name"):
            privateName = privateNode.getAttribute("name").strip()
            itemNodes = privateNode.getElementsByTagName("item")
            for itemNode in itemNodes:
                if itemNode.hasAttribute("name"):
                    if len(itemNode.childNodes) > 0:
                        gConfigs[privateName + '.' + itemNode.getAttribute("name").strip()] = itemNode.childNodes[0].data

    global gHasUpdate
    gHasUpdate = True
    return True

def query(key, domain="public"):
    '''
    query config 

    @type key: string
    @param key: value of name attribute if public node, value of the name attribute of its item node if private node
    @type domain: string
    @param domain: no domain param or public if public node, value of name attribute if private node
    @return: query result, if it query successfully, otherwise, None
    '''
    if gHasUpdate:
        # Remove space before and after
        realKey = key.strip()
        domainStrip = domain.strip()
        if "public" != domainStrip:
            realKey = domainStrip + '.' + realKey

        if realKey in gConfigs.keys():
            return gConfigs[realKey]
        else:
            return None
    else:
        print("We keep such usage as incorrect, the update must be called first")

