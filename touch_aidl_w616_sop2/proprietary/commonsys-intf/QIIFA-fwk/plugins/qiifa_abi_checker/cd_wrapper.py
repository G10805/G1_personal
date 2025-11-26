#!/usr/bin/python
# -*- coding: utf-8 -*-
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.
import xml.etree.ElementTree as ET
import os,argparse,textwrap,json

def read_techpackdata_from_CD_XML(tech_json):
  ## Check if provided Json data has type techpack_cd_info
  try:
    if tech_json["type"] == "techpack_cd_info":
      print("JSON Format is component descriptor Information")
  except KeyError:
    return tech_json
  cd_file_list = tech_json["paths"]
  tech_json_dict = {}
  library_list =[]
  tech_json_dict["techpackage_name"] = tech_json["techpackage_name"]
  for xml_file in cd_file_list:
    cd_abs_path = os.path.join(os.getenv("ANDROID_BUILD_TOP"),xml_file)
    ## Check if path exists, we expect all CD file should be present
    if os.path.exists(cd_abs_path):
      xml_root_node = None
      try:
        xml_root_node = ET.parse(cd_abs_path)
      except ET.ParseError as e:
        print(e)
        print("Aborting Script!!")
        sys.exit(1)
      api_management_tag = xml_root_node.findall("api_management_metadata")
      api_management_file_path = os.path.join(os.getenv("ANDROID_BUILD_TOP"),api_management_tag[0].attrib["file_path"])
      if os.path.exists(api_management_file_path):
        api_management_root_node = None
        try:
          api_management_root_node =  ET.parse(api_management_file_path).getroot()
        except ET.ParseError as e:
          print(e)
          print("Aborting Script!! Parsing failed.Unable to get node of xml file.")
          sys.exit(1)
      tech_json_dict["lsdump_golden_path"] = api_management_root_node.attrib["golden_reference"]
      api_managed_module_list =  api_management_root_node.findall("modules")[0]
      for child in api_managed_module_list:
        library_list.append(child.attrib["name"])
    else:
      print("CD file doesn;t exist in path : " + cd_abs_path)
      print("Aborting!!")
      sys.exit(1)
  tech_json_dict["library_list"] = library_list
  tech_json_dict["enable"] = "true"
  return tech_json_dict
