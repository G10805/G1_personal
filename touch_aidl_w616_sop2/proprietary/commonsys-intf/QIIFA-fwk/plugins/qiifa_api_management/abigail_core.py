#!/usr/bin/python
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.
import os,argparse,subprocess
import sys
import logging
import globals_api_management
from header_parser import *
import tempfile,shutil


### 0  - COMPATIBLE
### 1  - ABIDIFF_ERROR (Internal Abidiff error)
### 3  - ABIDIFF_USAGE_ERROR (If usage error then it will set ABIDIFF_ERROR bit as well)
### 4  - ABIDIFF_ABI_CHANGE
### 8  - ABIDIFF_ABI_INCOMPATIBLE_CHANGE (Rare condition)
### 12 - ABIDIFF_ABI_INCOMPATIBLE_CHANGE and ABIDIFF_ABI_CHANGE (Both bits will be set)
abidiff_return_values = [0,1,3,4,8,12]

def is_grd_argument_valid(args):
  if not args.elf:
    logging.error("--elf argument missing, mandatory in grd mode, check usage help -h")
    return False
  if args.header and args.symbol:
    logging.error("Either header or symbol path is acceptable in GRD mode")
    return False
  if not os.path.isfile(args.elf) :
    logging.error("--elf arg file does not exist : "+str(args.elf))
    return False
  if args.header:
    if not os.path.isdir(args.header):
      logging.error("Header directory not accessible ! : "+str(args.header))
      return False
  if args.symbol:
    if not os.path.isfile(args.symbol):
      logging.error("Symbol file not accessible ! : "+str(args.symbol))
      return False
  return True

def is_diff_argument_valid(args):
  if args.symbol or args.header or args.golden_ref or args.elf:
    logging.error("Symbol/Heder/golden_ref/ELF arguments are not allowed with diff option")
    return False
  if len(args.diff_list)!=2:
    logging.error("Exactly 2 positional arguments need in diff mode, check usage help -h")
    return False
  previous_reference = args.diff_list[0]
  current_reference = args.diff_list[1]
  if not os.path.isfile(previous_reference):
    logging.error("Invalid --diff args, not a valid file : "+str(previous_reference))
    return False
  if not os.path.isfile(current_reference):
    logging.error("Invalid --diff args, not a valid file : "+str(current_reference))
    return False
  return True

def is_argument_valid(args):
  if not os.path.isdir(args.qiifa_tools):
    logging.error("--qiifa_tools arg is invalid , no such directory : "+str(args.qiifa_tools))
    return False
  if not args.golden_ref and not args.diff_list:
    logging.error("Abigail core script runs either in golden reference or diff mode")
    logging.error("Atleast one of the arguments is required to use this script")
    return False
  if args.golden_ref and args.diff_list:
    logging.error("Golden reference and diff mode cannot be used together.")
    return False
  return True

def is_access_upgraded(prev_type, new_type):
  if prev_type == "public":
    return 0
  elif prev_type == "private":
    return 1
  elif new_type == "public":
    return 1
  elif new_type == "private":
    return 0

def get_access_states(line):
  words = line.strip().split(" ")
  prev_type = words[5]
  new_type = words[7]
  prev_type = prev_type.lstrip("'").rstrip("'")
  new_type = new_type.lstrip("'").rstrip("'")
  return prev_type,new_type

def harmless_iic_check(txt):
  txtlines = txt.split("\n")
  accesss_specifier_regex = re.compile(r'access changed from')
  redundant_report_regex = re.compile(r'as reported earlier')
  enum_addition_regex = re.compile(r'enumerator insertion:')
  redundant_change_count = 0
  change_count = 0
  addition_count = 0
  for line in txtlines:
    if re.search(accesss_specifier_regex,line):
      prev_type, new_type = get_access_states(line)
      if is_access_upgraded(prev_type, new_type):
        addition_count+=1
      else:
        change_count+=1
    elif re.search(enum_addition_regex,line):
      addition_count += 1
    elif re.search(redundant_report_regex,line):
      redundant_change_count += 1
  return change_count,addition_count

def evaluate_abidiff_return_code(returncode,text_output,harmless_check=0):
  if returncode in abidiff_return_values:
    if returncode < 4:
      logging.error("Abidiff encountered error or wrong arguments provided !!")
      sys.exit(-1)
    else:
      ## We need to differentiate here if it is a extension or backward-incompatible change.
      function_states  = {"Removed":0, "Changed":0 , "Added":0}
      if not harmless_check:
        list_values = str(text_output).split('\n', 1)[0].split(",")
        list_values[0] = str(list_values[0]).split(":")[1]
        list_values = list_values[:3]
        for values in list_values:
          number_function_state = values.strip().split()
          number_of_functions = int(number_function_state[0].strip())
          function_state = str(number_function_state[1]).strip()
          if function_state in function_states:
            function_states[function_state] = number_of_functions
          else:
            logging.error("Undefined function state.")
      else:
        function_states["Changed"],function_states["Added"] = harmless_iic_check(text_output)
      if function_states["Removed"] > 0 or function_states["Changed"] > 0:
        return 2

      if function_states["Added"] > 0:
        return 1
      return 0
  else:
    logging.error("Abidiff return code not found!")

### Return code for module compatiblity
### 0 - Compatible
### 1 - Extension
### 2 - Incompatible
def generate_diff_for_reference_module(golden_reference,current_reference):
  if os.path.isfile(golden_reference) and os.path.isfile(current_reference):
    command = os.path.join(globals_api_management.ABIGAIL_TOOLS,"abidiff") + " "+ golden_reference + " " + current_reference
    harmless_command = os.path.join(globals_api_management.ABIGAIL_TOOLS,"abidiff --harmless") + " "+ golden_reference + " " + current_reference
    harmless_check=0
    try:
      output = subprocess.check_output(command, env=globals_api_management.ENV,stderr=subprocess.STDOUT, shell=True)
      harmless_check=1
      subprocess.check_output(harmless_command, env=globals_api_management.ENV,stderr=subprocess.STDOUT, shell=True)
      return 0,"Compatible"
    except subprocess.CalledProcessError as e:
      logging.info("Error" + ' exit code: {}'.format(e.returncode))
      logging.info(e.output)
      return evaluate_abidiff_return_code(int(e.returncode),str(e.output),harmless_check),e.output

def append_symbols_list_with_function_file(elf,symbol_file,function_file):
  temp_dir_path = tempfile.mkdtemp(prefix="qiifa_scratch_")
  temp_file_path = os.path.join(temp_dir_path,"temp.symbol")
  try:
    generate_symbol_file_with_functions(function_file,elf,temp_file_path)
    append_symbols(symbol_file,temp_file_path)
  finally:
    shutil.rmtree(temp_dir_path, ignore_errors=True)

def append_symbols(destiation_symbol_file, source_symbol_file):
  fin = open(source_symbol_file, "r")
  fout = None
  if not os.path.isfile(destiation_symbol_file):
    fout = open(destiation_symbol_file, "w")
    fout.write("[abi_symbol_list]\n")
  else:
    fout = open(destiation_symbol_file, "a")
  for line in fin.readlines():
    if not "[abi_symbol_list]" in line:
      fout.write(line + "\n")
  fin.close()
  fout.close()

def generate_reference_dumps_using_headers(elf,output_path,header_dir,fp_mapping=None):
  ##TODO Move to centeralize scratch dir
  temp_dir_path = tempfile.mkdtemp(prefix="qiifa_scratch_")
  temp_file_path = os.path.join(temp_dir_path,"symbol.temp")
  try:
    parse_header_and_generate_symbol_file(elf,header_dir,None,temp_file_path,fp_mapping)
    generate_reference_dumps_using_symbols(elf,output_path,temp_file_path)
  finally:
    shutil.rmtree(temp_dir_path, ignore_errors=True)

def check_symbol_file_exists_and_have_content(symbol_file):
  if os.path.isfile(symbol_file):
    if os.stat(symbol_file).st_size == 0:
      logging.error("QIIFA : Symbol file is empty : " + str(symbol_file))
      sys.exit(-1)
  else:
    logging.error("QIIFA : Symbol file is not accessible : " + str(symbol_file))
    sys.exit(-1)

def generate_dumps_using_full_module(elf,output_path):
  command = os.path.join(globals_api_management.ABIGAIL_TOOLS,"abidw --no-show-locs --no-corpus-path") + " " + elf + " > " + output_path
  try:
    output = subprocess.check_output(command,env=globals_api_management.ENV, stderr=subprocess.STDOUT, shell=True)
    logging.info("Reference dumps generated successfully : " + output_path)
  except subprocess.CalledProcessError as e:
    logging.error("Reference dumps generation failed for : " + elf_name)
    logging.error("Error" + ' exit code: {}'.format(e.returncode))
    sys.exit(-1)

def generate_reference_dumps(elf,output_path):
  elf_name = elf.split("/")[-1]
  command = os.path.join(globals_api_management.ABIGAIL_TOOLS,"abidw --no-show-locs --no-corpus-path") + " " + elf + " > " + output_path
  try:
    subprocess.check_output(command,env=globals_api_management.ENV, stderr=subprocess.STDOUT, shell=True)
    logging.info("Reference dumps generated successfully : " + output_path)
  except subprocess.CalledProcessError as e:
    logging.error("Reference dumps generation failed for : " + elf_name)
    logging.error("Error" + ' exit code: {}'.format(e.returncode))
    sys.exit(-1)

def generate_reference_dumps_using_symbols(elf,output_path,symbol):
  check_symbol_file_exists_and_have_content(symbol)
  elf_name = elf.split("/")[-1]
  logging.info("Info: Generating Reference Dumps from Symbol File")
  command = os.path.join(globals_api_management.ABIGAIL_TOOLS,"abidw --no-show-locs --no-corpus-path") + " -w "+ symbol + " " + elf + " > " + output_path
  try:
    subprocess.check_output(command,env=globals_api_management.ENV, stderr=subprocess.STDOUT, shell=True)
    logging.info("Reference dumps generated successfully : " + output_path)
  except subprocess.CalledProcessError as e:
    logging.error("Reference dumps generation failed for : " + elf_name)
    logging.error("Error" + ' exit code: {}'.format(e.returncode))
    sys.exit(-1)

def generate_grd(args):
  if not is_grd_argument_valid(args):
    sys.exit(-1)
  if args.symbol:
    generate_reference_dumps_using_symbols(args.elf,args.golden_ref,args.symbol)
  elif args.header:
    generate_reference_dumps_using_headers(args.elf,args.golden_ref,args.header)
  else:
    generate_reference_dumps(args.elf,args.golden_ref)

def generate_abidiff(args):
  if not is_diff_argument_valid(args):
    sys.exit(-1)
  previous_reference = args.diff_list[0]
  current_reference = args.diff_list[1]
  return generate_diff_for_reference_module(previous_reference,current_reference)

def get_abigail_arg_parser():
  parser = argparse.ArgumentParser(description='QIIFA Abigail Core Script')
  required_group = parser.add_argument_group('Supported modes')
  required_group.add_argument("--elf", help="elf file path",type=str,dest="elf",required=False)
  required_group.add_argument("--symbol_file",help="symbol file path",type=str,dest="symbol",required=False)
  required_group.add_argument("--header", help="header directory path",type=str,dest="header",required=False)
  required_group.add_argument("--golden_ref", help="grd output file path",type=str,dest="golden_ref",required=False)
  required_group.add_argument("--diff", help="<pevious reference> <current reference>",nargs='*',dest="diff_list",required=False)
  required_group.add_argument("--qiifa_tools", help="QIIFA tools folder path",type=str,dest="qiifa_tools",required=True)
  return parser

def main():
  parser=get_abigail_arg_parser()
  args = parser.parse_args()
  logging.basicConfig(level=logging.INFO)
  if not is_argument_valid(args):
    logging.error("Arguments Integrity Check Failed")
    sys.exit(-1)
  globals_api_management.init_qiifa_tools(args.qiifa_tools)
  if args.golden_ref:
    generate_grd(args)
  elif args.diff_list:
    output,message = generate_abidiff(args)
    sys.exit(output)

if __name__ == "__main__":
  main()


