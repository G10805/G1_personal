#!/usr/bin/python
# -*- coding: utf-8 -*-
#Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

import argparse
import logging
import os
import sys
from subprocess import check_call
import xml.etree.ElementTree
from xml.dom import minidom
from configstore import configparser

'''
Script Versioning:

Version 1.0:
  - Supports three modes:
      fetch_vsdk_configs  Fetches the vsdk configs corresponding to the given
                          QSSI AU input.
      compile_vsdk        Triggers the compilation of VSDK. Use this after
                          fetching the vsdk configs through 'fetch_vsdk_configs'
                          option.
      fetch_and_compile   Fetches the vsdk configs corresponding to the given
                          QSSI AU input & triggers vsdk compilation
'''
__version__ = '1.0'

logger = logging.getLogger(__name__)

VSDK_CONFIGS_PATH = "vendor/qcom/configs/"
QSSI_to_VSDK_mapping_file = "QSSI_to_VSDK_mapping.xml"
VSDK_OPTIONS_oem_file = "./configs/vsdk_options.xml"

def RunCommand(command):
  cmd = command.split()
  logging.info("Running: " + str(cmd))
  check_call(cmd)

def compile_vsdk(ws_root, arg_jobs, lunch, disabled_vsdk_snapshots, qssi_manifest):
  logging.info("Triggering the compilation of vsdk..")

  # Ensure vsdk configs dir exists before we proceed with compilation.
  vsdk_configs_path = ws_root + "/" + VSDK_CONFIGS_PATH
  if not os.path.exists(vsdk_configs_path) or len(os.listdir(vsdk_configs_path)) == 0:
    logging.error("VSDK configs un-available, please fetch it first with - 'fetch_vsdk_configs' option, exiting..")
    sys.exit(1)

  if arg_jobs is None:
      # Default number of jobs is 8.
      jobs = "8"
  else:
      jobs = arg_jobs

  vsdk_cmd = "make dist " + "-j" + jobs

  if disabled_vsdk_snapshots is not None:
    disabled_vsdk_snapshots_list = disabled_vsdk_snapshots.split(',')
  else:
    disabled_vsdk_snapshots_list = []

  snapshots_enabled_list = ""
  if "vendor" not in disabled_vsdk_snapshots_list:
    vsdk_cmd = vsdk_cmd + "  vndk VNDK_SNAPSHOT_BUILD_ARTIFACTS=true vendor-snapshot"
    snapshots_enabled_list = snapshots_enabled_list + "vndk,vendor"

  if "recovery" not in disabled_vsdk_snapshots_list:
    vsdk_cmd = vsdk_cmd + "  recovery-snapshot"
    snapshots_enabled_list = snapshots_enabled_list + ",recovery"

  if "ramdisk" not in disabled_vsdk_snapshots_list:
    vsdk_cmd = vsdk_cmd + "  ramdisk-snapshot"
    snapshots_enabled_list = snapshots_enabled_list + ",ramdisk"

  if "host" not in disabled_vsdk_snapshots_list:
    vsdk_cmd = vsdk_cmd + "  snapshots"
    snapshots_enabled_list = snapshots_enabled_list + ",host"

  if "otatools" not in disabled_vsdk_snapshots_list:
    vsdk_cmd = vsdk_cmd + "  otatools-package"
    snapshots_enabled_list = snapshots_enabled_list + ",otatools"

  if "rawdata" not in disabled_vsdk_snapshots_list:
    snapshots_enabled_list = snapshots_enabled_list + ",rawdata"

  if "java" not in disabled_vsdk_snapshots_list:
    snapshots_enabled_list = snapshots_enabled_list + ",java"

  build_cmd = ["/bin/bash", "-c", "source build/envsetup.sh && lunch " + lunch + " && " + vsdk_cmd]

  logging.info("Running: " + str(build_cmd))
  check_call(build_cmd)

  if "otatools" not in disabled_vsdk_snapshots_list:
    RunCommand("cp out/target/product/" + lunch.replace("-userdebug",'').replace("-user", '') + "/otatools.zip out/dist/")

  RunCommand("cp .repo/manifest.xml out/dist/qssi_manifest.xml")

  if "rawdata" not in disabled_vsdk_snapshots_list:
    logging.info("Creating Rawdata Snapshot..")
    if qssi_manifest is not None:
        RunCommand("python3 vendor/qcom/proprietary/commonsys-intf/vsdk/rawdata/rawdata_config_generator.py \
            --input_qssi_manifest=" + qssi_manifest + " \
            --output_frozen_prjs_manifest=" + os.getcwd() + "/out/dist/rawdata_frozen_prjs_manifest.xml \
            --output_rawdata_config=" + os.getcwd() + "/out/dist/rawdata_config.xml")
    else:
        RunCommand("python3 vendor/qcom/proprietary/commonsys-intf/vsdk/rawdata/rawdata_config_generator.py \
            --output_frozen_prjs_manifest=" + os.getcwd() + "/out/dist/rawdata_frozen_prjs_manifest.xml \
            --output_rawdata_config=" + os.getcwd() + "/out/dist/rawdata_config.xml")

    RunCommand("python3 vendor/qcom/proprietary/commonsys-intf/vsdk/rawdata/rawdata_builder.py --defi " + os.getcwd() + "/out/dist/rawdata_config.xml")

  # Create vsdk xml with snapshots enabled info.
  root = minidom.Document()
  xml = root.createElement('QVSDK')
  root.appendChild(xml)
  snapshots_element = root.createElement('Snapshots')
  snapshots_element.setAttribute('Enabled', snapshots_enabled_list)
  xml.appendChild(snapshots_element)
  xml_str = root.toprettyxml(indent ="\t")
  with open(ws_root + "/out/dist/vsdk_metadata.xml", "w") as f:
    f.write(xml_str)

  logging.info("VSDK compilation successfully completed !")

def get_vsdk_configs_SHA(vsdk_configs_path, qssi_au):
  pre_vsdk_configs_sha = None
  mapping_list = xml.etree.ElementTree.parse(vsdk_configs_path + "/" + QSSI_to_VSDK_mapping_file).getroot().findall('mapping')
  for mapping in mapping_list:
    if qssi_au == mapping.get("QSSI_AU"):
      if mapping.get("vsdk_configs_sha") is not None:
        pre_vsdk_configs_sha = mapping.get("vsdk_configs_sha")
    elif pre_vsdk_configs_sha is not None:
      return pre_vsdk_configs_sha
  if pre_vsdk_configs_sha is not None:
    return pre_vsdk_configs_sha
  logging.error("Could not find the vsdk configs corresponding to the QSSI AU provided, please cross check if the QSSI AU provided is a valid one (i.e. it should be mapped to a released VSDK).")
  sys.exit(1)

def fetch_vsdk_configs(ws_root, vsdk_configs_params, qssi_au):
  vsdk_configs_path = ws_root + "/" + VSDK_CONFIGS_PATH
  logging.info("QSSI AU = " + qssi_au)
  logging.info("Fetching the vsdk configs to : " + vsdk_configs_path)
  RunCommand("rm -rf " + vsdk_configs_path)
  if vsdk_configs_params is not None:
      RunCommand("git clone " + vsdk_configs_params + " " + vsdk_configs_path)
  else:
      RunCommand("git clone git://git-android.quicinc.com/platform/vendor/qcom-proprietary/vsdk-configs -b vsdk.lnx.13.0 " + vsdk_configs_path)

  os.chdir(vsdk_configs_path)
  RunCommand("git reset --hard " + get_vsdk_configs_SHA(vsdk_configs_path, qssi_au))
  os.chdir(ws_root)
  logging.info("Vsdk configs successfully fetched to: " + vsdk_configs_path)

def main():
  logging_format = '%(asctime)s - %(filename)s - %(levelname)-8s: %(message)s'
  logging.basicConfig(level=logging.INFO, format=logging_format, datefmt='%Y-%m-%d %H:%M:%S')

  # Populate oem configs if exist
  vsdk_configs_params = None
  qssi_manifest = None
  if os.path.exists(VSDK_OPTIONS_oem_file):
    configparser.update(VSDK_OPTIONS_oem_file)
    vsdk_configs_params = configparser.query("vsdk_configs_params")
    qssi_manifest = configparser.query("qssi_manifest")

  parser = argparse.ArgumentParser()
  subparsers = parser.add_subparsers(dest="selection")

  fetch_subparser = subparsers.add_parser("fetch_vsdk_configs",
                    help="Fetches the vsdk configs corresponding to the given QSSI AU input.")
  fetch_subparser.add_argument("--vsdk_configs_params", dest='vsdk_configs_params',
                    help="Option to indicate git clone params to download vsdk configs.")
  fetch_subparser.add_argument("--qssi_au", dest='qssi_au',
                    help="QSSI AU info for which the vsdk configs need to fetched.", required=True)

  compile_subparser = subparsers.add_parser("compile_vsdk",
                    help="Triggers the compilation of VSDK. Use this after fetching the vsdk configs through 'fetch_vsdk_configs' option.")
  compile_subparser.add_argument("-j", dest='jobs',
                    help="Make jobs while compiling vsdk.")
  compile_subparser.add_argument("--disabled_vsdk_snapshots", dest='disabled_vsdk_snapshots',
                    help="List of disabled snapshots when compiling vsdk.")
  compile_subparser.add_argument("--qssi_manifest", dest='qssi_manifest',
                    help="Indicate the qssi manifest file to generate rawdata snapshot.")
  compile_subparser.add_argument("--lunch", dest='lunch',
                      help="lunch name", required=True)

  fetch_and_compile_subparser = subparsers.add_parser("fetch_and_compile",
                    help="Fetches the vsdk configs corresponding to the given QSSI AU input & triggers vsdk compilation")
  fetch_and_compile_subparser.add_argument("--vsdk_configs_params", dest='vsdk_configs_params',
                    help="Option to indicate git clone params to download vsdk configs.")
  fetch_and_compile_subparser.add_argument("--qssi_au", dest='qssi_au',
                    help="QSSI AU info for which the vsdk configs need to fetched.", required=True)
  fetch_and_compile_subparser.add_argument("--disabled_vsdk_snapshots", dest='disabled_vsdk_snapshots',
                    help="List of disabled snapshots when compiling vsdk.")
  fetch_and_compile_subparser.add_argument("--qssi_manifest", dest='qssi_manifest',
                    help="Indicate the qssi manifest file to generate rawdata snapshot.")
  fetch_and_compile_subparser.add_argument("--lunch", dest='lunch',
                      help="lunch name", required=True)
  fetch_and_compile_subparser.add_argument("-j", dest='jobs',
                    help="Make jobs while compiling vsdk.")

  parser.add_argument('--version', action='version', version=__version__)
  args = parser.parse_args()

  # Get the workspace root
  ws_root = os.path.abspath(os.getcwd())

  if args.selection is not None:
    logging.info("Running the generate_vsdk script in the mode: '" + args.selection + "'")
    if args.selection != "compile_vsdk":
      if args.vsdk_configs_params is not None:
        vsdk_configs_params = args.vsdk_configs_params
    if args.selection != "fetch_vsdk_configs":
      if args.qssi_manifest is not None:
        qssi_manifest = args.qssi_manifest

  if args.selection == "fetch_vsdk_configs":
    fetch_vsdk_configs(ws_root, vsdk_configs_params, args.qssi_au)
  elif args.selection == "compile_vsdk":
    compile_vsdk(ws_root, args.jobs, args.lunch, args.disabled_vsdk_snapshots, qssi_manifest)
  elif args.selection == "fetch_and_compile":
    fetch_vsdk_configs(ws_root, vsdk_configs_params, args.qssi_au)
    compile_vsdk(ws_root, args.jobs, args.lunch, args.disabled_vsdk_snapshots, qssi_manifest)

if __name__ == '__main__':
  main()
