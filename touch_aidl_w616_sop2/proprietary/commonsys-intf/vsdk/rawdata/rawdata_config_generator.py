# -*- coding: utf-8 -*-
#Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

import argparse
import logging
import os
import sys
import time
import xml.etree.ElementTree as ET
from xml.dom import minidom

QSSI_EXT_manifest_file = "snap_combined_manifest.xml"

def generate_rawdata_config(input_qssi_manifest, output_rawdata_config):
  if input_qssi_manifest is not None:
    xml_file = ET.parse(input_qssi_manifest).getroot()
  else:
    qssi_ext_manifest_path = os.getcwd() + "/.repo/manifests/" + QSSI_EXT_manifest_file
    if os.path.exists(qssi_ext_manifest_path):
        xml_file = ET.parse(qssi_ext_manifest_path).getroot()
    else:
        xml_file = ET.parse(os.getcwd() + "/.repo/manifest.xml").getroot()

  root = minidom.Document()
  xml_data = root.createElement('data')
  root.appendChild(xml_data)
  xml_rawdata = root.createElement('rawdata')
  xml_data.appendChild(xml_rawdata)
  xml = root.createElement('allowlist')
  xml_data.appendChild(xml)

  non_frozen_prjs_fd = open(os.getcwd() + "/vendor/qcom/configs/QSSI_prjs_config/non-frozen-prjs.txt", 'r')
  non_frozen_prjs = non_frozen_prjs_fd.readlines()
  non_frozen_prjs_list = []
  for non_frozen_prj in non_frozen_prjs:
    non_frozen_prjs_list.append(non_frozen_prj.strip())

  for prj in xml_file.findall('project'):

    if prj.get("path") is not None:
      src_path = prj.get("path")
    else:
      src_path = prj.get("name")

    if src_path not in non_frozen_prjs_list:
      continue

    prj_element = root.createElement('project')
    prj_element.setAttribute('src_path', src_path)

    for symlink in prj.findall("linkfile"):
      symlink_element = root.createElement('symlink')
      symlink_element.setAttribute('src_path', symlink.get("src"))
      symlink_element.setAttribute('link_path', symlink.get("dest"))
      prj_element.appendChild(symlink_element)

    xml.appendChild(prj_element)

    for copyfile in prj.findall("copyfile"):
      file_element = root.createElement('file')
      file_element.setAttribute('src_path', src_path + "/" + copyfile.get("src"))
      file_element.setAttribute('dest_path', copyfile.get("dest"))
      xml.appendChild(file_element)

  xml_str = root.toprettyxml(indent ="\t")
  with open(output_rawdata_config, "w") as f:
    f.write(xml_str)

def generate_frozen_prjs_manifest(input_qssi_manifest, output_frozen_prjs_manifest):
  frozen_prjs_fd = open(os.getcwd() + "/vendor/qcom/configs/QSSI_prjs_config/frozen-prjs.txt", 'r')
  frozen_prjs = frozen_prjs_fd.readlines()
  frozen_prjs_list = []
  for frozen_prj in frozen_prjs:
    frozen_prjs_list.append(frozen_prj.strip())

  if input_qssi_manifest is not None:
    root = ET.parse(input_qssi_manifest).getroot()
  else:
    qssi_ext_manifest_path = os.getcwd() + "/.repo/manifests/" + QSSI_EXT_manifest_file
    if os.path.exists(qssi_ext_manifest_path):
        root = ET.parse(qssi_ext_manifest_path).getroot()
    else:
        root = ET.parse(os.getcwd() + "/.repo/manifest.xml").getroot()

  for prj in root.findall('project'):
    if prj.get("path") is not None:
      src_path = prj.get("path")
    else:
      src_path = prj.get("name")

    if src_path not in frozen_prjs_list:
      root.remove(prj)

  for default in root.findall('default'):
    root.remove(default)

  xml_str = minidom.parseString(ET.tostring(root)).toprettyxml(indent=" ")
  with open(output_frozen_prjs_manifest, "w") as f:
    f.write(xml_str)

if __name__ == '__main__':
  logging_format = '%(asctime)s - %(filename)s - %(levelname)-8s: %(message)s'
  logging.basicConfig(level=logging.INFO, format=logging_format, datefmt='%Y-%m-%d %H:%M:%S')

  parser = argparse.ArgumentParser()
  parser.add_argument("--input_qssi_manifest", dest='input_qssi_manifest',
                  help="Indicate QSSI manifest file to generate rawdata snapshot config.", required=False)
  parser.add_argument("--output_frozen_prjs_manifest", dest='output_frozen_prjs_manifest',
                  help="Frozen projects fragment manifest xml output file.", required=True)
  parser.add_argument("--output_rawdata_config", dest='output_rawdata_config',
                  help="Rawdata output config file for non-frozen projects.", required=True)
  args = parser.parse_args()
  generate_rawdata_config(args.input_qssi_manifest, args.output_rawdata_config)
  generate_frozen_prjs_manifest(args.input_qssi_manifest, args.output_frozen_prjs_manifest)
