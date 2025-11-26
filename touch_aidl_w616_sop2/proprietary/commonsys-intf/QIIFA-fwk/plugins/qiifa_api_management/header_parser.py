#!/usr/bin/python
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.
import sys,re
import os,argparse,subprocess
import xml.etree.ElementTree as ET
import logging
import tempfile,shutil
from globals_api_management import *
import globals_api_management
ABIGAIL_TOOLS = None
IGNORE_INTERNAL_API = ["operator=","DECLARE_META_INTERFACE","interface_cast"]
API_MANAGEMENT_PLUGIN_DEPENDENCIES = True
try:
  import CppHeaderParser
except ImportError:
  API_MANAGEMENT_PLUGIN_DEPENDENCIES = False
##TODO: Virtual functions

def argument_integrity(args):
  if args.header is not None:
    if not os.path.isdir(args.header):
      return False

  if not os.path.isfile(args.elf):
    return False

  if args.exfn is not None:
    if not os.path.isfile(args.exfn):
      return False

  if args.fn_list is not None:
    if not os.path.isfile(args.fn_list):
      return False

  return True

def check_header_file_integrity_broken(header_file):
  integrity_broken = False
  header_file_name = str(header_file)
  try:
    header_file_name = str(header_file).split("/")[-1].split("_",1)[1]
  except:
    logging.info("Header file split failed.")
  header_file_handler = open(header_file,'r')
  if not header_file_handler:
    logging.error("Header file not found : " + header_file_name)
    sys.exit(-1)
  header_file_lines = header_file_handler.readlines()
  for idx,line in enumerate(header_file_lines):
    if "#if" in str(line) and "__cplusplus" in str(line):
      continue

    if "#ifdef" in str(line) or "#if" in str(line) or "ifndef" in str(line):
      if "ifndef" in str(line):
        next_line = header_file_lines[idx + 1]
        if "define" in next_line and line.split()[1] == next_line.split()[1]:
          continue
        else:
          integrity_broken = True
          message = "Header Integrity Check Failed. ifndef in header file" \
          + header_file_name + "at line number : " + str(idx)
          logging.error(message)
          return integrity_broken

      if "#ifdef" in str(line) and "__cplusplus" in str(line):
        continue
        next_1_line= header_file_lines[idx + 1]
        if "extern" in next_1_line or "}" in next_1_line:
          next_2_line = header_file_lines[idx + 2]
          if "#endif" in next_2_line:
            continue
          else:
            integrity_broken = True
            message = "Header Integrity Check Failed (#ifdef) " + header_file_name
            logging.error(message)
            return integrity_broken
        else:
          integrity_broken = True
          message = "Header Integrity Check Failed  (#ifdef) " + header_file_name
          logging.error(message)
          return integrity_broken
      message = "Header Integrity Check Failed. #ifdef or #if in header file" \
        + header_file_name + " at line number : " + str(idx)
      integrity_broken = True
      logging.error(message)
      return integrity_broken
  message = "Header Integrity Check is successfull ! for  " + header_file_name
  #logging.info(message)
  return integrity_broken

def detect_function_pointer(variable_list,fp_mapping):
  for var in variable_list:
    if int(var["pointer"]) == 1:
      pointer_type = var["type"].strip().replace(" ","")
      pointer_name = var.get("name")
      fp_config_pointer = None
      if fp_mapping is not None and pointer_name is not None:
        fp_config_pointer = fp_mapping.get(pointer_name)
      pattern = ".*\(\*.*\)\(.*\)"
      if re.search(pattern,pointer_type) and fp_config_pointer is None:
        return True
  return False

def detect_nested_classes_functions(cppHeader,header_file_name):
  cons_dest = False
  for header_class in cppHeader.classes:
    public_function_list =[]
    method_dict = cppHeader.classes[header_class]["methods"]
    for method_type_key in method_dict:
      if method_type_key == "public":
        methods = method_dict[method_type_key]
        if "::" in header_class and len(methods) > 0:
          for method in methods:
            if not bool(method["constructor"]) and not bool(method["destructor"]):
              logging.error(header_file_name)
              logging.error("QIIFA: Nested classes/structures are not supported.")
              logging.error("Please add symbol functions in interfaces")
              return True
  return False

def is_exceptional_function(func,warn=0):
  return check_if_function_is_static(func,warn) or check_if_function_is_inline(func,warn) or is_template_function(func,warn)

def check_integrity_broken_for_header_functions(cppHeader,header_file_name,fp_mapping):
  if detect_function_pointer_in_functions(cppHeader,header_file_name,fp_mapping) or \
  detect_nested_classes_functions(cppHeader,header_file_name):
    return True
  else:
    return False

def detect_function_pointer_in_functions(cppHeader,header_file_name,fp_mapping):
  function_pointer_detected = False
  if detect_function_pointer(cppHeader.variables,fp_mapping):
    message = "Function Pointer detected in Header file : " +  str(header_file_name) +\
    " Please make sure corresponding functions are added as interface for integrity."
    logging.error(message)
    function_pointer_detected = True

  for _class in cppHeader.classes:
    public_properties_list = cppHeader.classes[_class]["properties"]["public"]
    protected_properties_list = cppHeader.classes[_class]["properties"]["protected"]
    if detect_function_pointer(public_properties_list,fp_mapping) or \
    detect_function_pointer(protected_properties_list,fp_mapping):
      message = "Function Pointer detected in Header file :" +  str(header_file_name) +\
      ". Please make sure corresponding functions are added as interface for integrity."
      logging.error(message)
      function_pointer_detected = True
      break
  return function_pointer_detected

def parse_header_files(header_folder,fp_mapping=None):
  header_failure = False
  total_files = 0
  successfull_parsed_files = 0
  successfull_integrity_files = 0
  error_parsed_files = 0
  error_integrity_files = 0
  function_pointer_header_file_list = []
  function_list = []
  failure_header_list = []
  public_class_function_list_mapping = {}
  integrity_header_failure_list = []
  for dirpath, dirs, files in os.walk(header_folder):
    for filename in files:
      fname = os.path.join(dirpath,filename)
      if fname.endswith('.h'):
        total_files+=1
        header_file = fname
        header_file_name = str(header_file)
        try:
          header_file_name = str(header_file).split("/")[-1].split("_",1)[1]
        except:
          logging.info("Header file split failed.")
        try:
          cppHeader = CppHeaderParser.CppHeader(header_file)
        except Exception as e:
          logging.error(e)
          logging.error ("Error parsing header file : " + header_file)
          error_parsed_files += 1
          failure_header_list.append(header_file_name)
          continue

        successfull_parsed_files += 1
        function_map = {}
        integrity_broken = check_header_file_integrity_broken(header_file)
        integrity_broken_functions = check_integrity_broken_for_header_functions(cppHeader, header_file_name,fp_mapping)
        if integrity_broken or integrity_broken_functions:
          integrity_header_failure_list.append(header_file_name)
          error_integrity_files += 1
        else:
          message = "Header Integrity Check is successfull ! for  " + header_file_name
          logging.info(message)
          successfull_integrity_files += 1

        ## Classes
        for header_class in cppHeader.classes:
          ## Do not include any functions from nested classes.
          if "::" in header_class:
            continue
          public_function_list =[]
          method_dict = cppHeader.classes[header_class]["methods"]
          for method_type_key in method_dict:
            if method_type_key == "public" or method_type_key == "protected":
              methods = method_dict[method_type_key]
              for method in methods:
                if is_exceptional_function(method,1):
                  continue
                method_name = method["name"]
                if method_name in IGNORE_INTERNAL_API:
                  continue
                if method["namespace"]:
                  if not method_name in function_map:
                    header_function_metadata_list = []
                    function_map[method_name] = header_function_metadata_list
                  (function_map[method_name]).append(method)
                public_function_list.append(method)

          public_class_function_list_mapping[str(header_class)] = public_function_list

        for func in cppHeader.functions:
          if is_exceptional_function(func,1):
            continue
          method_name = func["name"]
          if method_name in IGNORE_INTERNAL_API:
            continue
          function_list.append(func)
          if func["namespace"]:
            if not method_name in function_map:
              header_function_metadata_list = []
              function_map[method_name] = header_function_metadata_list
            (function_map[method_name]).append(func)

  logging.info ("Header Integrity Check")
  logging.info ("    Total Header Files : " + str(total_files))
  logging.info ("    Header Files successfully passed Integrity Check : " + str(successfull_integrity_files))
  logging.info ("    Header Files failed Integrity Check : " + str(error_integrity_files))

  if integrity_header_failure_list:
    header_failure = True
    for header in integrity_header_failure_list:
      logging.info (header)

  logging.info ("Header Parser Check")
  logging.info ("    Total Header Files : " + str(total_files))
  logging.info ("    Total Header Files successfully Parsed : " + str(successfull_parsed_files))
  logging.info ("    Failures in parsing : " + str(error_parsed_files))
  if failure_header_list:
    header_failure = True
    for header in failure_header_list:
      logging.info(header)

  if function_pointer_header_file_list:
    logging.warning("Function Pointers found for below files.")
    logging.warning("Please add corresponding functions as interface.")
    for header_file in function_pointer_header_file_list:
      logging.warning(header_file)

  if header_failure:
    logging.error("Error Encountered in processing header files. Exiting")
    sys.exit(-1)
  return function_list,public_class_function_list_mapping,function_map

def parse_header_and_generate_symbol_file(elf_file,header_folder,exfn=None,symbol_file_path=None,fp_mapping=None):
  total_files = 0
  successfull_files = 0
  error_files = 0
  function_list,public_class_function_map,func_header_metadata_map = parse_header_files(header_folder,fp_mapping)
  excluded_function_list =[]
  if exfn:
    excluded_function_list = read_functions_from_function_file(exfn)
  generate_symbol_file_and_enforced_functions(elf_file,function_list,public_class_function_map,\
    func_header_metadata_map,excluded_function_list,symbol_file_path)

def check_function_is_present_in_list(function_name,fn_list,namespace=None,class_name=None):
  if not fn_list:
    ## List is empty we don;t need to exclude any functions
    return False
  for function in fn_list:
    if function["name"]==function_name:
      if class_name is None and namespace is None:
        return True
      elif class_name is None and namespace is not None:
        if function["namespace"] == namespace:
          return True
      elif class_name is not None and namespace is not None:
        if function["class_name"] == class_name and function["namespace"] == namespace:
          return true
      elif class_name is not None and namespace is None:
        if function["class_name"] == class_name:
          return True
  return False

def check_if_function_is_virtual(function_metadata):
  if bool(function_metadata.get("pure_virtual")) or bool(function_metadata.get("virtual")):
    return True
  return False

def check_if_function_is_static(function_metadata,warn=0):
  if bool(function_metadata.get("static")):
    if warn:
      logging.warning("   Static function "+str(function_metadata.get("name"))+" will be ignored for integrity check")
    return True
  return False

def is_template_function(function_metadata,warn=0):
  if bool(function_metadata.get("template")):
    if warn:
      logging.warning("   Template function "+str(function_metadata.get("name"))+" will be ignored for integrity check")
    return True
  return False

def check_if_function_is_inline(function_metadata,warn=0):
  if bool(function_metadata.get("inline")):
    if warn:
      logging.warning("   Inline function "+str(function_metadata.get("name"))+" will be ignored for integrity check")
    return True
  return False

def check_if_function_is_const(function_metadata):
  if bool(function_metadata.get("const")):
    return True
  return False

def generate_symbol_file_and_enforced_functions(elf_file,fn_list_h,public_class_function_map_h,
  func_header_metadata_map,ex_fn_list,symbol_file_path):
  symbol_not_found_list = []
  function_symbol_list_elf,class_function_list_elf,namespace_functions_map_elf = getSymbolList(elf_file)
  final_function_symbol_map={}
  for function_l in fn_list_h:
    function_h = function_l["name"]
    symbol_found = False
    for function_elf in function_symbol_list_elf:
      function_name_elf = function_elf["name"]
      if function_h == function_name_elf:
        ## Check if function has namespace
        if function_h in func_header_metadata_map:
          functions_header_metadata =  func_header_metadata_map[function_h]
          for function in functions_header_metadata:
            namespace_value = str(function["namespace"])
            suffix = "::"
            if suffix in namespace_value and namespace_value.endswith(suffix):
              namespace_value = namespace_value[:-len(suffix)]
            functions_object_list = namespace_functions_map_elf[namespace_value]
            for function_obj in functions_object_list:
              if namespace_value == function_obj["namespace"]:
                if not check_function_is_present_in_list(str(function_h),ex_fn_list,namespace_value):
                  final_function_symbol_map[function_obj["symbol"]] = function_h
                  symbol_found = True
                  break
        else:
          if not check_function_is_present_in_list(str(function_h),ex_fn_list):
            final_function_symbol_map[function_elf["symbol"]] = function_h
            symbol_found = True
          else:
            symbol_found = True
    if not symbol_found:
      if check_if_function_is_virtual(function_l):
        logging.error("Virtual function symbol not found : " + function_h)
      symbol_not_found_list.append(function_h)

  ##TODO : Optimize break
  #print public_class_function_map_h

##namespace Foo

##Class X {

##function A

##}

##Class Y {
##function A

##}

##namspace Bar

##class X {

##function A

##}

##[A:Foo:Symbol123, A:Bar:Symbol456, A:Foo:Symbol789]
##[A:X:Symbol123 , A:X:Symbol456 , A:Y:Symbol789]
  ##Identify Foo:X:A
  for class_name_h in public_class_function_map_h:
    class_function_list_h = public_class_function_map_h[class_name_h]
    for function_l in class_function_list_h:
      function_h = function_l["name"]
      symbol_found = False
      ## Check if function has namespace
      if function_h in func_header_metadata_map:
        functions_header_metadata =  func_header_metadata_map[function_h]
        for function in functions_header_metadata:
          namespace_value = function["namespace"]
          suffix = "::"
          if suffix in namespace_value and namespace_value.endswith(suffix):
            namespace_value = namespace_value[:-len(suffix)]
          functions_object_list = namespace_functions_map_elf[namespace_value]
          for function_obj in functions_object_list:
            if namespace_value == function_obj["namespace"]:
              if not check_function_is_present_in_list(str(function_h),ex_fn_list,namespace_value,class_name_h):
                for class_elf in class_function_list_elf:
                  if class_elf == class_name_h:
                    function_elf_list = class_function_list_elf[class_elf]
                    for function_elf in function_elf_list:
                     if function_h == function_elf["name"]:
                       if function_obj["symbol"] == function_elf["symbol"]:
                         final_function_symbol_map[function_obj["symbol"]] = function_h
                         symbol_found = True
                         break
      else:
        if not check_function_is_present_in_list(function_h,ex_fn_list,class_name_h):
          for class_elf in class_function_list_elf:
            if class_elf == class_name_h:
              function_elf_list = class_function_list_elf[class_elf]
              for function_elf in function_elf_list:
                if function_h == function_elf["name"]:
                  final_function_symbol_map[function_elf["symbol"]] = class_name_h +"::"+function_h
                  symbol_found = True
        else:
          symbol_found = True
      if not symbol_found:
        if check_if_function_is_virtual(function_l):
          logging.error("Virtual function symbol not found : " + function_h)
        symbol_not_found_list.append(class_name_h +"::"+function_h)

  if symbol_not_found_list:
    logging.error ("Error:Header Information and ELF file is Incompatible")
    logging.error ("Error:Symbol Infomation is missing for functions in elf : "+str(elf_file))
    logging.error (symbol_not_found_list)
    sys.exit(1)

  logging.info(elf_file)
  for symbol in final_function_symbol_map:
    logging.info(final_function_symbol_map[symbol] + "  :   " + symbol)
  write_symbols_in_abi_file(final_function_symbol_map,symbol_file_path)

def write_symbols_in_abi_file(final_function_symbol_map,symbol_file_path=None):
  file_path = "abi_symbol_list"
  if symbol_file_path is not None:
    file_path = symbol_file_path
  f = open(file_path, 'w')
  f.write("[abi_symbol_list]\n")
  for symbol in final_function_symbol_map:
    f.write(symbol+"\n")
  f.close()
  logging.info ("File Generated :" + file_path)


def get_namespace_child(namespace,namespace_name):
  child = list(namespace)[0]
  if child.tag == "namespace-decl":
    namespace_name = namespace_name + "::" + child.attrib["name"]
    return get_namespace_child(child,namespace_name)
  else:
    return namespace,namespace_name

def getSymbolList(elf_library):
  ## TODO Add scratch folder Handling across api_management
  ## TODO Handle global across python files
  ## Invoke abidw to generate xml and then load that xml to get symbol list.
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
    logging.error(e.output)
    shutil.rmtree(temp_dir_path, ignore_errors=True)
    sys.exit(-1)
  finally:
    shutil.rmtree(temp_dir_path, ignore_errors=True)
  function_list = xml_root_node.findall(".//function-decl")
  function_name_symbol_list = []
  class_function_mapping ={}

  namespace_list = xml_root_node.findall(".//namespace-decl")
  namespace_functions_map = {}
  for namespace in namespace_list:
    namespace_name = namespace.attrib["name"]
    last_namespace_element, namespace_name = get_namespace_child(namespace,namespace_name)
    function_declaration_list = last_namespace_element.findall(".//function-decl")
    func_obj_list = []
    temp_list = []
    for function in function_declaration_list:
      temp_list.append(function.attrib["name"])
      func_obj = {}
      if "elf-symbol-id" in function.attrib:
        func_obj["symbol"] = function.attrib["elf-symbol-id"]
      else:
        continue
      func_obj["name"] = function.attrib["name"]
      func_obj["namespace"] = namespace_name
      func_obj_list.append(func_obj)
    if namespace_name in namespace_functions_map:
      ## Join List
      existing_func_list = namespace_functions_map[namespace_name]
      existing_func_list.extend(func_obj_list)
    else:
      namespace_functions_map[namespace_name] = func_obj_list
  for func in function_list:
    function_name = str(func.attrib["name"])
    if "elf-symbol-id" in func.attrib:
      function_name_symbol_list.append({"name":function_name,"symbol":func.attrib["elf-symbol-id"]})
    else:
      continue

  class_list = xml_root_node.findall(".//class-decl")
  for class_obj in class_list:
    function_declaration_list = class_obj.findall(".//function-decl")
    function_list = []
    for function in function_declaration_list:
      function_obj ={}
      try:
        function_obj["name"] = function.attrib["name"]
        function_obj["symbol"]= function.attrib["elf-symbol-id"]
      except KeyError:
        continue
      function_list.append(function_obj)
    class_function_mapping[class_obj.attrib["name"]] = function_list
  return function_name_symbol_list,class_function_mapping,namespace_functions_map

def read_functions_from_function_file(function_file):
  ##format namespace#class$function-name
  function_list = []
  file_handler = open(function_file,'r')
  if not file_handler:
    logging.error ("File not found : " + function_file)
    sys.exit(1)
  file_lines = file_handler.readlines()
  for line in file_lines:
    line_strip = line.strip()
    function_obj = {}
    ##Handle case for namespace and function seperately
    if "#$" in line:
      line_split_namespace_func = line.strip().split("#$")
      function_obj["namespace"] = line_split_namespace_func[0]
      function_obj["class_name"] = None
      function_obj["name"] = line_split_namespace_func[1]
    else:
      if "$" in line and "#" in line:
        line_split_namespace_classfunc = line.strip().split("#")
        class_function_split = line_split_namespace_classfunc[1].split("$")
        function_obj["namespace"] = line_split_namespace_classfunc[0]
        function_obj["class_name"] = class_function_split[0]
        function_obj["name"] = class_function_split[1]
      elif "$" in line and "#" not in line:
        class_function_split = line.strip().split("$")
        function_obj["namespace"] = None
        function_obj["class_name"] = class_function_split[0]
        function_obj["name"] = class_function_split[1]
      elif "#" not in line and "$" not in line:
        function_obj["namespace"] = None
        function_obj["class_name"] = None
        function_obj["name"] = line.strip()
    function_list.append(function_obj)
  return function_list

def generate_symbol_file_with_functions(function_file,elf,symbol_file=None):
  final_function_symbol_map={}
  symbol_not_found_list =[]
  function_list = read_functions_from_function_file(function_file)
  function_symbol_list_elf,class_function_list_elf,namespace_functions_map_elf = getSymbolList(elf)
  for function in function_list:
    symbol_found = False
    function_class_user = function["class_name"]
    if function_class_user is not None:
      for class_elf in class_function_list_elf:
        if class_elf == function_class_user:
          function_elf_list = class_function_list_elf[class_elf]
          for function_elf in function_elf_list:
            func_name = function["name"]
            if func_name == function_elf["name"]:
               ## Check if function has namespace
              if function["namespace"] is not None:
                namespace_value = function["namespace"]
                functions_object_list = namespace_functions_map_elf[namespace_value]
                for function_obj in functions_object_list:
                  if namespace_value == function_obj["namespace"]:
                    if function_obj["symbol"] == function_elf["symbol"]:
                      final_function_symbol_map[function_elf["symbol"]] = function_class_user +"::"+function["name"]
                      symbol_found = True
                      break
              else:
                final_function_symbol_map[function_elf["symbol"]] = function_class_user +"::"+function["name"]
                symbol_found = True
                break
    else:
      for function_elf in function_symbol_list_elf:
        function_name_elf = function_elf["name"]
        if function["name"] == function_name_elf:
          if function["namespace"] is not None:
            namespace_value = function["namespace"]
            functions_object_list = namespace_functions_map_elf[namespace_value]
            for function_obj in functions_object_list:
              if namespace_value == function_obj["namespace"]:
                final_function_symbol_map[function_elf["symbol"]] = function["name"]
                symbol_found = True
                break
          else:
            final_function_symbol_map[function_elf["symbol"]] = function["name"]
            symbol_found = True
            break
    if not symbol_found:
      if function_class_user is not None:
        symbol_not_found_list.append(function_class_user +"::"+function["name"])
      else:
        symbol_not_found_list.append(function["name"])

  if symbol_not_found_list:
    logging.error("Error:Header Information and ELF file is Incompatible")
    logging.error("Error:Symbol Information is missing for functions in elf : "+str(elf))
    logging.error(symbol_not_found_list)
    sys.exit(1)

  for symbol in final_function_symbol_map:
    logging.info (final_function_symbol_map[symbol] + "  :   " + symbol)
  write_symbols_in_abi_file(final_function_symbol_map,symbol_file)

def main():
  parser = argparse.ArgumentParser(description='QIIFA Header Checker Script')
  required_group = parser.add_argument_group('Mandatory Arguments')
  parser.add_argument("--header", help="",type=str,dest="header",required=False)
  required_group.add_argument("--elf", help="",type=str,dest="elf",required=True)
  required_group.add_argument("--qiifa_tools", help="",type=str,dest="qtools",required=True)
  parser.add_argument("--exclude_fn",help="",type=str,dest="exfn",required=False)
  parser.add_argument("--fn_list",help="",type=str,dest="fn_list",required=False)
  args = parser.parse_args()
  init_logging()
  init_qiifa_tools(args.qtools)
  if argument_integrity(args):
    if args.fn_list:
      generate_symbol_file_with_functions(args.fn_list,args.elf)
    else:
      parse_header_and_generate_symbol_file(args.elf,args.header,args.exfn)
  else:
    print ("Argument Error")

if __name__ == "__main__":
  main()
