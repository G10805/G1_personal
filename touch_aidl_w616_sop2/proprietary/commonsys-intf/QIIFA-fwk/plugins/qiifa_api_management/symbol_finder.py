#!/usr/bin/python
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.
import os,argparse,subprocess
import xml.etree.ElementTree as ET
from globals_api_management import *
import globals_api_management
import fnmatch,tempfile,shutil

def print_function_symbol_mapping(elf_library,function_list):
  temp_dir_path = tempfile.mkdtemp(prefix="qiifa_scratch_")
  temp_file_path = os.path.join(temp_dir_path,"abi_symbols")
  xml_root_node = None
  try:
    command = os.path.join(globals_api_management.ABIGAIL_TOOLS,"abidw ") + str(elf_library) + " > " + temp_file_path
    ##TODO Handle error condition
    subprocess.check_output(command, env=globals_api_management.ENV,stderr=subprocess.STDOUT, shell=True)
    xml_root_node = ET.parse(temp_file_path).getroot()
  except subprocess.CalledProcessError as e:
    logging.error("Error" + ' exit code: {}'.format(e.returncode))
    #logging.error(e.output)
    shutil.rmtree(temp_dir_path, ignore_errors=True)
    sys.exit(-1)
  finally:
    shutil.rmtree(temp_dir_path, ignore_errors=True)
  xml_function_list = xml_root_node.findall(".//function-decl")
  for func_name in function_list:
    func_found = False
    for xml_func in xml_function_list:
      if func_name == xml_func.attrib.get("name"):
        logging.info("Function Name : " + func_name + "  Symbol : " + xml_func.attrib.get("elf-symbol-id"))
        func_found = True
    if not func_found:
      logging.warning("Symbol not found for : " + func_name)

def read_functions_return_list(function_file):
  function_list =[]
  if os.path.isfile(function_file):
    file_handler = open(function_file,"r")
    for line in file_handler.readlines():
      function_list.append(line.strip())
    return function_list
  else:
    logging.error("File not present : " + function_file)
    sys.exit(-1)

def main():
  parser = argparse.ArgumentParser(description='Symbol Finder Script')
  required_group = parser.add_argument_group('Mandatory Arguments')
  required_group.add_argument("--elf", help="",type=str,dest="elf",required=True)
  required_group.add_argument("--qiifa_tools", help="Abs Path to QIIFA tools dir",type=str,dest="qtools",required=True)
  required_group.add_argument("--fn", help="Functions file",type=str,dest="func_file",required=True)
  args = parser.parse_args()
  init_logging()
  init_qiifa_tools(args.qtools)
  function_list = read_functions_return_list(args.func_file)
  print_function_symbol_mapping(args.elf,function_list)
if __name__ == "__main__":
  main()
