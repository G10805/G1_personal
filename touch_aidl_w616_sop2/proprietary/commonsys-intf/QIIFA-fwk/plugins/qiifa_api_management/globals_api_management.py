#!/usr/bin/python
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.
import os
import logging
ABIGAIL_TOOLS = None
MANIFEST_PATH = None
PWMANIFEST_PATH = None
CRITICAL_WARNINGS = []
SUPPORTED_IIC_MODES = ["enforce","warning","enforce_qc_dir"]
IIC_ENFORCE_MODE = 0
IIC_WARN_MODE = 1
IIC_ENFORCE_QC_DIR_MODE = 2
IIC_WARN_MODE = 0
ENV = None

def init_logging():
  logging_format = '%(asctime)s - %(filename)s - %(levelname)-8s: %(message)s'
  logging.basicConfig(format='%(asctime)s,%(msecs)d %(levelname)-8s [%(filename)s:%(lineno)d] %(message)s',
    datefmt='%Y-%m-%d:%H:%M:%S',
    level=logging.DEBUG)

def init_qiifa_tools(qtools):
  global ABIGAIL_TOOLS,ENV
  ABIGAIL_TOOLS = os.path.join(qtools,"abigail-tools")
  env = os.environ.copy()
  ld_path = env.get("LD_LIBRARY_PATH","")
  ld_path = str(ld_path) + ":" + ABIGAIL_TOOLS
  env["LD_LIBRARY_PATH"] = ld_path
  ENV = env

def init_globals(root,qtools):
  global MANIFEST_PATH
  global PWMANIFEST_PATH
  init_qiifa_tools(qtools)
  MANIFEST_PATH = os.path.join(root,".repo/manifests/default.xml")
  PWMANIFEST_PATH = os.path.join(root,".repo/manifests/pwmanifest.xml")
  #MANIFEST_PATH = os.path.join(root,"manifest.xml")
