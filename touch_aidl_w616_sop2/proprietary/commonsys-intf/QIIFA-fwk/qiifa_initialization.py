#!/usr/bin/python
# -*- coding: utf-8 -*-
#Copyright (c) 2020 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

import os,argparse,textwrap,json
import xml.etree.ElementTree as ET
import xml.dom.minidom
from plugins.qiifa_abi_checker.cd_wrapper import read_techpackdata_from_CD_XML

ABI_CONFIG_JSON = "commonsys-intf/QIIFA-fwk/plugins/qiifa_abi_checker/abi_config.json"

def read_qiifa_techpackage(path):
    qiifa_techpackage_dict = {}
    tech_package_json_file_path=os.path.join(os.getenv("ANDROID_BUILD_TOP"),path)
    if os.path.exists(tech_package_json_file_path):
      tech_package_json_file_handler = open(tech_package_json_file_path,'r')
      qiifa_techpackage_dict = json.load(tech_package_json_file_handler)
    else:
      print("Abichecker : Abichecker tech_packagejson file not present.")
    return qiifa_techpackage_dict

def read_abi_config():
    abi_config_dict = {}
    abi_config_file = os.path.join(os.getenv("QCPATH"),ABI_CONFIG_JSON)
    if os.path.exists(abi_config_file):
      abi_config_file_handler = open(abi_config_file,'r')
      abi_config_dict = json.load(abi_config_file_handler)
    else:
      print ("Abichecker : Abichecker config file not present.")
    return abi_config_dict

def abi_config_checker(config,type):
  if config["abi_type"] == type and config["enabled"] == "true":
    return True
  else:
    return False

def add_abi_enabled_libs_from_techpackage_list(config, abi_enabled_library_list,arg_techpackage):
    #if not "tech_package=" in arg_techpackage:
    #   print("""Warning : Not a valid tech package argument :
    #           Tech pack generation will be skipped !!
    #          Should use tech_package=<tech_pack_name1>,<tech_pack_name2>""")
    #return
    #arg_techpackage = arg_techpackage.split("tech_package=")[1]
    qiifa_build_config_path = os.getenv("QIIFA_BUILD_CONFIG")
    qiifa_config_dir = os.path.dirname(qiifa_build_config_path)
    if(not os.path.exists(qiifa_config_dir)):
        os.makedirs(qiifa_config_dir)
    techpack_info = open(qiifa_config_dir + "/techpack_info.txt","w")
    techpack_info.writelines(str(arg_techpackage))
    techpacakages_json_file_list = config["techpacakages_json_file_list"]
    tech_package_list = arg_techpackage.split(",")
    for techpacakages_json_file in techpacakages_json_file_list:
        tech_json = read_qiifa_techpackage(techpacakages_json_file)
        ## Support for reading qiifa techpack data from CD
        tech_json = read_techpackdata_from_CD_XML(tech_json)
        for tech_package in tech_package_list:
            if(tech_json["techpackage_name"] == tech_package):
                ## Add libs in abi_enabled_library_list
                ## We are considering only Module names in abi_enabled_library_list
                for lib in tech_json["library_list"]:
                    abi_enabled_library_list.append(lib)

def add_abi_enabled_libs_from_sphal_list(config, abi_enabled_library_list):
    sp_hal_library_list = config["sp_hal_library_list"]
    for sp_hal_lib in sp_hal_library_list:
        abi_enabled_library_list.append(sp_hal_lib)

def write_abi_enabled_library_list_in_xmlfile(abi_enabled_library_list):
    print(abi_enabled_library_list)
    root = ET.Element("abilibs")
    for library in abi_enabled_library_list:
        library_subelement  = ET.SubElement(root,"library")
        library_subelement.text = str(library)
    qiifa_build_config_path = os.getenv("QIIFA_BUILD_CONFIG")
    if(not os.path.exists(os.path.dirname(qiifa_build_config_path))):
        os.makedirs(os.path.dirname(qiifa_build_config_path))
    xml_tree = ET.ElementTree(root)
    with open (qiifa_build_config_path, "wb") as files :
         xml_tree.write(files)
    dom = xml.dom.minidom.parse(qiifa_build_config_path)
    pretty_xml_as_string = dom.toprettyxml()
    with open (qiifa_build_config_path, "wb") as files :
        files.write(pretty_xml_as_string.encode())

def qiifa_initialization(arg_techpackage):
    abi_config_dict = read_abi_config()
    abi_enabled_library_list = []
    if abi_config_dict:
        for config in abi_config_dict:
            if abi_config_checker(config,"techpackage"):
                if not arg_techpackage is None:
                    add_abi_enabled_libs_from_techpackage_list(config, abi_enabled_library_list, arg_techpackage)
            if abi_config_checker(config,"sphal"):
                add_abi_enabled_libs_from_sphal_list(config, abi_enabled_library_list)
    write_abi_enabled_library_list_in_xmlfile(abi_enabled_library_list)

def main():
     parser = argparse.ArgumentParser(
             formatter_class=argparse.RawDescriptionHelpFormatter,
             description=textwrap.dedent(
                 '''Tech Package Argument by , seperated'''))
     parser.add_argument("tech_package",type=str, nargs='?')
     args = parser.parse_args()
     arg_techpackage = args.tech_package
     ## If no techpackage argument is provided then run the script
     ## in sphal mode.Consider it as default mode.
     qiifa_initialization(arg_techpackage)

if __name__ == '__main__':
     main()
