#!/usr/bin/python
# -*- coding: utf-8 -*-
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

'''
Import standard python modules
'''
import sys,os,logging

'''
Import local utilities
'''
from qiifa_util.util import UtilFunctions, Variables, Constants
from . import globals_api_management

'''
Import plugin_interface base module
'''
CURR_DIR = os.path.dirname(os.path.abspath(__file__))
sys.path.append(CURR_DIR)

from plugin_interface import plugin_interface
from plugins.qiifa_api_management.qiifa_api_management import _qiifa_IIC
from plugins.qiifa_api_management.header_parser import API_MANAGEMENT_PLUGIN_DEPENDENCIES

def func_start_qiifa_api_management(self,scan_all_componenet=False):
  ## Do not run for QSSI
  if scan_all_componenet:
    iic_type = globals_api_management.SUPPORTED_IIC_MODES[globals_api_management.IIC_ENFORCE_QC_DIR_MODE]
  else:
    iic_type = globals_api_management.SUPPORTED_IIC_MODES[globals_api_management.IIC_ENFORCE_MODE]
  symbols_path = os.path.join(Constants.out_path,"symbols")
  print("QIIFA API Manangement Invoked!! iic_type",iic_type)
  output = _qiifa_IIC(Constants.croot,iic_type,symbols_path, \
  os.path.join(Constants.qiifa_current_cmd_dir,"compatibility_report.txt"),Constants.qiifa_tools_path)

class MyPlugin(plugin_interface):
  def __init__(self):
    pass

  def register(self):
    return Constants.API_MANAGEMENT_SUPPORTED_CREATE_ARGS

  def config(self, QIIFA_type=None, libsPath=None, CMDPath=None):
    pass

  def generateGoldenCMD(self, libsPath=None, storagePath=None, create_args_lst=None):
    pass

  def IIC(self, **kwargs):
    if Constants.IS_CUSTOMER_VARIANT and not os.path.isdir(Constants.qiifa_tools_path):
       print("Skipping QIIFA API Manangement for Customer Build")
       return
    if API_MANAGEMENT_PLUGIN_DEPENDENCIES:
      if kwargs.get('arg_tp_build'):
        if kwargs.get('scan_all_component') and kwargs.get('scan_all_component') == "1":
          logging.info("Scanning all qiifa component")
          func_start_qiifa_api_management(self,scan_all_componenet=True)
        else:
          logging.info("Skipping all qiifa component scan")
          func_start_qiifa_api_management(self)
    else:
      print("Skipping QIIFA API Manangement, Python dependencies not met")
