#!/usr/bin/python
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

import os,argparse,subprocess
import sys,random,string,time
import xml.etree.ElementTree as ET
from header_parser import *
from abigail_core import *
from globals_api_management import *
import globals_api_management
import fnmatch,tempfile,shutil

## TODO : Chaining during check mode for minor versions with grd
## TODO : Add integrity check for unique module names - Completed
## TODO : Enforce golden reference dumps are checked in for all modules.
## TODO : Enforce check - 1.x can be stable only if all x-1 versions are stable.
## TODO : Add support for generating golden dumps on module level
## TODO : Same component state during IIC - Completed
## TODO : Add a safe-guard mechanism for interfaces , some excluded function list &
## ask user confirmation whenever new function is added.
## TODO : Do not allow user to transition to next greater state until minor x-1 is at that state.
## TODO : Block user from updating x-1 version if x version is detected for minor versioning
## TODO : Add a check to make sure minor version is not updated.
## TODO : Check for stale copy after generating GRD
## TODO : Generate more detailed report
## TODO : Treat new module additions as extensions.
## TODO : Handle module removal case as all modules should not be mandatorily compiled.

Version = 1.0
ROOT_PATH = None
API_METADATA_GOLDEN = "am_golden"
allowed_component_states = ["development","compatible","stable"]
supported_platforms = ["LA","LE"]
grd_constant = "golden_reference_dumps"
component_state_file_name = "component_state"
minor_version_component_compatibility_map = {}
IIC_MODE = False
STANDALONE_MODULE = "standalone_module"
skip_cd_check = False
# Branching Flags
FRESH_START = 0
MINOR_UPDATE = 1
MAJOR_UPDATE = 2
NO_UPDATE = 3

COMPONENT_EXCEPTION_LIST = ["uwb-vndr.lnx","computervision-fastcv.lnx"]

def exit_func():
  sys.exit(1)

def check_attribute_and_return_value(element,attribute,optional=False):
  if attribute in element.attrib:
    element_value = element.attrib[attribute]
    if not element_value:
      logging.error("XML Attribute Value not present :" + attribute)
      exit_func()
    else:
      return element_value
  else:
    if optional:
      return ""
    else:
      logging.error("XML Attribute not present :" + attribute)
      exit_func()

def parse_api_management_configuration(file_path):
  configurations = {}
  xml_root_node = ET.parse(file_path).getroot()
  print(xml_root_node.tag)
  if xml_root_node.tag == "api_management_configuration":
    configurations_nodes = list(xml_root_node)
    print(configurations_nodes)
    ## Currently look only for function-pointer types
    for configuration_node in configurations_nodes:
      if configuration_node.tag == "configuration" and \
      configuration_node.attrib["name"] == "function-pointer":
        print(configuration_node.attrib["name"])
        ## List all modules and corresponding function pointers
        module_nodes = list(configuration_node)
        module_function_pointer_mapping = {}
        for module_node in module_nodes:
          function_pointer_nodes = list(module_node)
          fp_name_wrapper_mapping = {}
          for function_pointer in function_pointer_nodes:
            fp_name_wrapper_mapping[function_pointer.attrib["name"]] = function_pointer.attrib["wrapper"]
          module_function_pointer_mapping[module_node.attrib["name"]] = fp_name_wrapper_mapping
      configurations[configuration_node.attrib["name"]] = module_function_pointer_mapping
  return configurations

def read_cd_xml_and_prepare_dict(cd_project_list,platform_type,symbol_path):
  techpack_cd_dict = {}
  for cd_project in cd_project_list:
    cd_dir = cd_project["path"]
    cd_file_found = False
    for dirpath, dirs, files in os.walk(cd_dir):
      for filename in files:
        fname = os.path.join(dirpath,filename)
        if filename.lower() == "cd.xml":
          api_configurations = {}
          api_conf = os.path.join(dirpath,"api_management_conf.xml")
          if os.path.isfile(api_conf):
            api_configurations = parse_api_management_configuration(api_conf)
            print(api_configurations)
          cd_file_found = True
          xml_root_node = ET.parse(fname).getroot()
          if xml_root_node.tag == "component":
            cd_object = {}
            conf_child = list(xml_root_node)[0]
            if conf_child.tag == "configuration":
              cd_object["name"] = check_attribute_and_return_value(conf_child,"name")
              cd_object["version"] = check_attribute_and_return_value(conf_child,"version")
              cd_object["state"] = check_attribute_and_return_value(conf_child,"state")
              cd_object["grd"] = cd_project["grd"]
              api_metadata_tags_list = xml_root_node.findall(".//api_metadata")
              api_metadata = None
              if api_metadata_tags_list:
                api_metadata = api_metadata_tags_list[0]
              else:
                logging.info("api_metadata tag is not found. Ignoring cd : " + fname)
                continue
              ## let's hard code this value so that we can enforce naming convention for api_metadata & this value
              ## can be removed.
              cd_api_metadata_file_path = check_attribute_and_return_value(api_metadata,"file_path")
              if not cd_api_metadata_file_path == "api_metadata.xml":
                logging.error("QIIFA: Api metadata file supported value is - api_metadata.xml")
                logging.error("QIIFA: It is an optional parameter and can be removed.api_metadata.xml \
                can be created at cd level , it will be automatically picked up.")
                exit_func()
              api_metadata_file_path = os.path.join(cd_dir,"api_metadata.xml")
              if not os.path.isfile(api_metadata_file_path):
                logging.error("QIIFA: api_metadata file not found!")
                exit_func()
              interface_ready = str(check_attribute_and_return_value(api_metadata,"intf_ready",True)).lower()
              grd_path = os.path.join(get_component_version_golden_path(cd_object,"",platform_type),API_METADATA_GOLDEN)
              grd_available = os.path.isfile(grd_path)
              if IIC_MODE:
                if interface_ready == "false" or ( not grd_available and not interface_ready == "true" ):
                  logging.warning("QIIFA : Skipping Component as Interface is not ready.")
                  logging.warning(str(cd_object["name"]))
                  cd_file_found = True
                  continue
              cd_object["api_metadata_file_path"] = api_metadata_file_path
              api_metadata_dict = parse_api_metadata(api_metadata_file_path,platform_type,symbol_path);
              cd_object["api_metadata_dict"] = api_metadata_dict
              cd_object["api_management_conf"] = api_configurations
              techpack_cd_dict[cd_object["name"]] = cd_object
              logging.info("CD file found :" + fname)
              cd_file_found = True
              break
          else:
            continue
      if cd_file_found:
        break
    if not cd_file_found:
      logging.error("CD file not found for :" + str(cd_dir))
      if not skip_cd_check:
        exit_func()
  return techpack_cd_dict


def check_violation_and_add_in_violation_list(path,violation_list_messages):
  if not os.path.isfile(path):
    violation_list_messages.append("QIIFA: File not accessible/present : " + path)

def handle_library_symbol_path_value(module,platform_type,symbol_path):
  module_childs = list(module)
  symbol_library_path = None
  if "path" in module.attrib:
    for child in module_childs:
      if child.tag == "platform":
        logging.error("Format not supported module path & platform element are not supported")
        exit_func()
    module_path = get_path_before_appending(module.attrib["path"])
    symbol_library_path = os.path.join(symbol_path,module_path)
  else:
    ## We need to find platform tag specific to type
    for child in module_childs:
      if child.tag == "platform" and "path" in child.attrib:
        if child.attrib["type"] == platform_type:
          module_path = get_path_before_appending(child.attrib["path"])
          symbol_library_path = os.path.join(symbol_path,module_path)
          break
  if symbol_library_path is None:
    logging.error("Unable to find path element for module : " + module.attrib["name"])
    exit_func()
  else:
    return symbol_library_path

def get_path_before_appending(path):
  if path.startswith('/'):
    return path[1:]
  else:
    return path

def handle_header_path_value(header,platform_type):
  header_childs = list(header)
  header_path = None
  if "path" in header.attrib:
    for child in header_childs:
      if child.tag == "platform":
        logging.error("Format not supported header path & platform element are not supported")
        exit_func()
    if platform_type == "LA":
      tmp_path = header.attrib["path"]
      header_path = os.path.join(ROOT_PATH,get_path_before_appending(header.attrib["path"]))
    elif platform_type == "LE":
      header_path = os.path.join(ROOTFS_PATH,get_path_before_appending(header.attrib["path"]))
  else:
    for child in header_childs:
      if child.tag == "platform" and "path" in child.attrib:
        if child.attrib["type"] == platform_type:
          if platform_type == "LA":
            header_path = os.path.join(ROOT_PATH,get_path_before_appending(child.attrib["path"]))
          elif platform_type == "LE":
            header_path = os.path.join(ROOTFS_PATH,get_path_before_appending(child.attrib["path"]))
          break
  if header_path is None:
    logging.error("Unable to find path element for header : " + header.attrib["file"])
    exit_func()
  else:
    return header_path

def check_for_xml_schema(xml_element,platform_specific_schema,api_metadata_file):
  if platform_specific_schema:
    if not "path" in xml_element.attrib:
      logging.error("QIIFA: path attribute not found. File : " + api_metadata_file)
      exit_func()
    childs = list(xml_element)
    for child in childs:
      if child.tag == "platform":
        logging.error("QIIFA: platform tag is not supported for this xml schema. \
        Please check document.File : " + api_metadata_file)
        exit_func()
  else:
    if "path" in xml_element.attrib:
      logging.error("QIIFA: path attribute is not supported for this xml schema.File : " + api_metadata_file)
      exit_func()
    childs = list(xml_element)
    platform_tag = False
    for child in childs:
      if child.tag == "platform":
        platform_tag = True
    if not platform_tag:
      logging.error("QIIFA: platform tag not found.File : " + api_metadata_file)
      exit_func()

def check_sanity_of_apimetadata(modules_list_xml,api_metadata_file):
  modules_platform_list = []
  platform_schema = False
  for modules in modules_list_xml:
    if "platform" in modules.attrib:
      platform_schema = True
      modules_platform_list.append(modules.attrib["platform"])
      module_list_xml = list(modules)
      ## platform tag should not be present and path element is needed
      for module in module_list_xml:
        check_for_xml_schema(module,True,api_metadata_file)
        header_list = list(module)
        for header in header_list:
          check_for_xml_schema(header,True,api_metadata_file)
    else:
      module_list_xml = list(modules)
      ## platform tag should not be present and path element is needed
      for module in module_list_xml:
        check_for_xml_schema(module,False,api_metadata_file)
        header_plat_list = list(module)
        for header_plat in header_plat_list:
          if not header_plat.tag == "platform":
            check_for_xml_schema(header_plat,False,api_metadata_file)
  if platform_schema and sorted(supported_platforms) != sorted(modules_platform_list):
        logging.error("Invalid api_metadata schema, <modules> not defined for all the supported platform")
        logging.error("API metadata file path : "+str(api_metadata_file))
        logging.error("Supported Platforms : "+str(supported_platforms))
        logging.error("Defined Platforms : "+str(modules_platform_list))
        exit_func()

def check_if_module_is_compiled(module_name,module_path):
  return True

def parse_api_metadata(api_metadata_file,platform_type,symbol_path):
  violation_list_messages = []
  if api_metadata_file:
    if os.path.isfile(api_metadata_file):
      xml_root_node = ET.parse(api_metadata_file).getroot()
      api_metadata_dict = {}
      api_metadata_dict["file"] = api_metadata_file
      modules_list_xml = xml_root_node.findall(".//modules")
      check_sanity_of_apimetadata(modules_list_xml,api_metadata_file)
      if not modules_list_xml:
        logging.error("No modules found in Api metadata!! Exiting!")
        exit_func()
      modules_containers = []
      for modules in modules_list_xml:
        common_comp_diff_intf = False
        module_list_xml = list(modules)
        modules_container={}
        modules_container["platform"] = check_attribute_and_return_value(modules,"platform",True)
        if modules_container["platform"] and modules_container["platform"] != platform_type:
          continue
        module_list =[]
        for module in module_list_xml:
          module_dict = {}
          module_type = check_attribute_and_return_value(module,"type",True)
          module_dict["type"] = module_type
          module_dict["name"] = check_attribute_and_return_value(module,"name")
          module_dict["project"] = check_attribute_and_return_value(module,"project")
          module_dict["path"] = handle_library_symbol_path_value(module,platform_type,symbol_path)
          check_violation_and_add_in_violation_list(module_dict["path"],violation_list_messages)
          symbol_file_value = check_attribute_and_return_value(module,"symbol_file",True)
          if symbol_file_value:
            module_dict["symbol_file"] = os.path.join(ROOT_PATH,symbol_file_value)
            check_violation_and_add_in_violation_list(module_dict["symbol_file"],violation_list_messages)
          else:
            module_dict["symbol_file"] = symbol_file_value
          if not module_type == STANDALONE_MODULE:
            header_list_xml = list(module)
            header_list =[]
            for header in header_list_xml:
              if header.tag =="platform":
                continue
              header_dict = {}
              header_dict["file"] = check_attribute_and_return_value(header,"file")
              header_dict["path"] = handle_header_path_value(header,platform_type)
              interface_xml_list = list(header)
              interface_list =[]
              for interface in interface_xml_list:
                if interface.tag == "platform":
                  continue
                interface_dict = {}
                interface_dict["namespace"] = check_attribute_and_return_value(interface,"namespace",True)
                interface_dict["class"] = check_attribute_and_return_value(interface,"class",True)
                interface_dict["symbol"] = check_attribute_and_return_value(interface,"symbol",True)
                interface_dict["name"] = check_attribute_and_return_value(interface,"name")
                interface_list.append(interface_dict)
              header_dict["interfaces"] = interface_list
              header_list.append(header_dict)
              if not len(header_dict["interfaces"]):
                check_violation_and_add_in_violation_list(header_dict["path"],violation_list_messages)
            module_dict["headers"] = header_list
          module_list.append(module_dict)
        modules_container["module_list"] = module_list
        modules_containers.append(modules_container)
      api_metadata_dict["modules_container_list"] = modules_containers
      if violation_list_messages:
        for violation_message in violation_list_messages:
          logging.error(violation_message + "\n")
        exit_func()
      return api_metadata_dict
    else:
      logging.error("Api Metadata not found : " + api_metadata_file)
      exit_func()

def parse_manifest_xml():
  if not os.path.exists(globals_api_management.MANIFEST_PATH):
    logging.error("QIIFA : Manifest Path doesn't exists.")
    logging.info("QIIFA : Checking PW Manifest Path")
    if not os.path.exists(globals_api_management.PWMANIFEST_PATH):
      logging.error("QIIFA : PW Manifest Path doesn't exists. Exiting")
      exit_func()
    globals_api_management.MANIFEST_PATH = globals_api_management.PWMANIFEST_PATH
  xml_root_node = ET.parse(globals_api_management.MANIFEST_PATH).getroot()
  project_list = xml_root_node.findall(".//project")
  cd_regular_exp = "qc/components/*/cd"
  cd_project_list = []
  for project in project_list:
    if fnmatch.fnmatch(project.attrib["name"],cd_regular_exp):
      project_dict = {}
      project_key = project.attrib["name"].split("/")[2]
      if project_key in COMPONENT_EXCEPTION_LIST:
        logging.warn("Skipping exceptioned component : "+str(project_key))
        continue
      project_dict["name"] = project_key
      project_dict["path"] = os.path.join(ROOT_PATH,project.attrib["path"])
      project_dict["grd"]  = os.path.join(project_dict["path"],grd_constant)
      cd_project_list.append(project_dict)
  return cd_project_list

def locate_cd(component_name,cd_project_list):
  for project in cd_project_list:
    if project["name"] == component_name:
      return [project]
  logging.error("QIIFA : CD file not found for Component!")
  logging.error(component_name)
  exit_func()

def get_cd_project_list(scan_qc_dir=False):
  ## All cd project have fixed path i.e. qc/<component>/cd
  if not scan_qc_dir:
    return parse_manifest_xml()
  logging.info("Sanning all qc dir components")
  qc_dir_path = os.path.join(ROOT_PATH,"qc")
  cd_project_list = []
  if os.path.exists(qc_dir_path):
    component_dirs = os.listdir(qc_dir_path)
    for component in component_dirs:
      if not os.path.isdir(os.path.join(qc_dir_path,component)):
        logging.warning("Unexpected file found in qc dir" + str(component))
        continue
      project_dict = {}
      if component in COMPONENT_EXCEPTION_LIST:
        logging.warn("Skipping exceptioned component : "+str(component))
        continue
      project_dict["name"] = component
      project_dict["path"] = os.path.join(qc_dir_path,component,"cd")
      project_dict["grd"]  = os.path.join(project_dict["path"],grd_constant)
      cd_project_list.append(project_dict)
    return cd_project_list
  else:
    logging.error("QIIFA : qc top level path not found for locating cd.")
    exit_func()

def prepare_cd_and_api_metadata_dict(platform_type,symbol_path,component_name=None,scan_qc_dir=False):
  if scan_qc_dir:
    cd_project_list = get_cd_project_list(scan_qc_dir=True)
  else:
    cd_project_list = get_cd_project_list(scan_qc_dir=False)
  if component_name is not None:
    cd_project_list = locate_cd(component_name,cd_project_list)
  techpack_dict = read_cd_xml_and_prepare_dict(cd_project_list,platform_type,symbol_path)
  if not len(techpack_dict):
    logging.warn("No component found")
    logging.warn("Ensure that projects are configured in manifest, ignore if no components are configured")
    logging.warn("Exiting qiifa_api_management")
  return techpack_dict

## Golden Reference Dumps Directory Structure
## - golden_reference_dumps
## -- LA
## --- <Component_name>
## ----arch_64
## ----- version_n.x
## ------ <module_name>.xml
## ----- version_n.x-1
## ------ <module_name>.xml
## ----- version_n.x-2
## ------ <module_name>.xml
## ----arch_32
## ----- version_n.x
## ------ <module_name>.xml
## ----- version_n.x-1
## ------ <module_name>.xml
## ----- version_n.x-2
## ------ <module_name>.xml
## -- LE
## --- <Component_name>
## ----arch_64
## ----- version_n.x
## ------ <module_name>.xml
## ----- version_n.x-1
## ------ <module_name>.xml
## ----- version_n.x-2
## ------ <module_name>.xml
## ----arch_32
## ----- version_n.x
## ------ <module_name>.xml
## ----- version_n.x-1
## ------ <module_name>.xml
## ----- version_n.x-2
## ------ <module_name>.xml

def get_component_version_golden_path(component,api_metadata_platform,platform_type):
  component_name = component["name"]
  component_version = component["version"]
  component_golden_reference_path = component["grd"]
  component_version_folder = "version_" + component_version
  folder_hierarchy_path = None
  folder_hierarchy_path = os.path.join(component_golden_reference_path,platform_type,\
  component_name,component_version_folder)
  return folder_hierarchy_path

def generate_golden_references(grd_arg,symbol_path,platform_type):
  techpack_dict = {}
  if grd_arg == "all":
    component_name = None
  else:
      ## Update only for a particular component
      ## TODO : Add error check if component name in cd.xml doesn;t matches folder name
    component_name = grd_arg
  techpack_dict = prepare_cd_and_api_metadata_dict(platform_type,symbol_path,component_name)
  if not len(techpack_dict):
    logging.warn("QIIFA : No component found to generate Golden reference dumps!!")
  else:
    if grd_arg == "all":
      print_information_for_components(techpack_dict)
    integrity_flag = run_integrity_check_on_techpack_cd_and_api_metadata(techpack_dict)
    for component_key in techpack_dict:
      component_obj = techpack_dict[component_key]
      component_golden_reference_path = component_obj["grd"]
      component_name = component_obj["name"]
      component_version = component_obj["version"]
      component_state = component_obj["state"]
      component_version_folder = "version_" + component_version
      ## Create hierarchy for generating reference dumps.
      api_metadata_dict = component_obj["api_metadata_dict"]
      api_metadata_platform = api_metadata_dict["modules_container_list"][0]["platform"]
      folder_hierarchy_path = get_component_version_golden_path(component_obj,api_metadata_platform,platform_type)
      ## Check if path exists and if yes then what is the component state
      component_state_path = os.path.join(folder_hierarchy_path)
      component_state_grd = None

      #1: Do not update golden reference if component in stable mode and component marker is not in stable state.
      ## Handled in authenticate_component_state_transition

      ## Let;s validate if golden reference dumps already exists or getting created first time for this version.
      ## To verify if Golden dumps are successfully generated atleast once we can rely on api metadata file existence.
      ## grd == golden_reference_dumps
      ## This path is constant for every component and mandated by QIIFA
      #2: Handle case when first time golden_reference_dumps are created.
      if not os.path.isfile(os.path.join(folder_hierarchy_path,API_METADATA_GOLDEN)):
        ## We want to validate below cases if golden dumps already existing.
        #3: development mode : Verify with n.x-1 version and check for compatiblity (Show warnings)
        #run_minor_version_compatibility(component,platform_type,arch_type)
        #4: compatible mode  : Verify New dump of n.x is compatible with older one which is already present
        #5: compatible mode  : Verify dump is compatible with x-1 version
        #if component_state == "compatible":
        # folder_path = os.path.join(component_golden_reference_path,platform_type,component_name,arch_type,component_version_folder)
        ## Let;s create temp folder and copy all reference dumps.
        #temp_dir_path = tempfile.mkdtemp(prefix="qiifa_dumps_")
        #try:
          # generate_reference_dumps_for_component(component_obj,temp_dir_path,platform_type)
          #run_abi_compatibility_check(folder_path,temp_dir_path)
          #distutils.dir_util.copy_tree(temp_dir_path,folder_path)
        #finally:
          # distutils.dir_util.remove_tree(temp_dir_path)
        # Handle Compatible state GRD generation
        if component_state != allowed_component_states[0]:
          logging.error("First time grd generation is only allowed in development state")
          exit_func()
        else:
          generate_reference_dumps_for_component(component_obj,folder_hierarchy_path,platform_type,component_state)
        ## Copy reference API's snapshot
        shutil.copy(component_obj["api_metadata_file_path"],os.path.join(folder_hierarchy_path,API_METADATA_GOLDEN))
      else:
        #6: Throw error if downgrade is detected Stable - > Compatible -> Development
        if not authenticate_component_state_transition(component_state,folder_hierarchy_path):
          logging.error("QIIFA:Component state transition is not allowed for component : " + component_name)
          exit_func()
        if component_state == allowed_component_states[1]:
          tmp_component_dump_path = tempfile.mkdtemp(prefix="qiifa_dumps_")
          skip_state_check=1
          generate_reference_dumps_for_component(component_obj,tmp_component_dump_path,platform_type,component_obj["state"])
          output,module_integrity_map = run_interface_integrity_check_for_component(component_obj,\
          tmp_component_dump_path,platform_type,skip_state_check)
          if output == 2:
            logging.error("Grd Generation failed as components's new state is incompatible")
            shutil.rmtree(tmp_component_dump_path, ignore_errors=True)
            exit_func()
          else:
            shutil.rmtree(folder_hierarchy_path, ignore_errors=True)
            shutil.copytree(tmp_component_dump_path,folder_hierarchy_path)
          shutil.rmtree(tmp_component_dump_path, ignore_errors=True)
        else:
          generate_reference_dumps_for_component(component_obj,folder_hierarchy_path,platform_type,component_state)
        ## Copy reference API's snapshot
        shutil.copy(component_obj["api_metadata_file_path"],os.path.join(folder_hierarchy_path,API_METADATA_GOLDEN))
    logging.info("QIIFA : Successfully generated all Golden reference dumps!!")

def authenticate_component_state_transition(component_state_current,folder_hierarchy_path):
  component_state_file_path = os.path.join(folder_hierarchy_path,component_state_file_name)
  file_handler = open(component_state_file_path,"r")
  component_state =  file_handler.readline().strip()
  file_handler.close()
  if component_state in allowed_component_states:
    new_component_state = component_state_current
    if new_component_state == "stable" and component_state == "stable":
      logging.error("QIIFA : Cannot update golden reference for already stable state component.")
      exit_func()
    if component_state == new_component_state:
      return True
    else:
      if new_component_state == "development":
        return False
      elif new_component_state == "compatible" and component_state == "stable":
        return False
      return True
  else:
    logging.error("QIIFA: Component state is not supported." + component_state)
    exit_func()

def run_abi_compatibility_check(golden_path,current_path):
  map_golden_modules_dumps = {}
  map_current_modules_dumps = {}
  for dirpath, dirs, files in os.walk(golden_path):
    for filename in files:
      fname = os.path.join(dirpath,filename)
      if fname.endswith('.xml'):
        map_golden_modules_dumps[filename] = fname
  for dirpath, dirs, files in os.walk(current_path):
    for filename in files:
      fname = os.path.join(dirpath,filename)
      if fname.endswith('.xml'):
        map_current_modules_dumps[filename] = fname
  module_integrity_output = []
  module_integrity_map = {}
  for module in map_current_modules_dumps:
    output,message = generate_diff_for_reference_module(map_golden_modules_dumps[module],map_current_modules_dumps[module])
    module_integrity_output.append(output)
    module_integrity_map[module] = [output,message]
  if sum(module_integrity_output) == 0:
    return 0,module_integrity_map
  elif max(module_integrity_output) == 1:
    return 1,module_integrity_map
  elif max(module_integrity_output) == 2:
    return 2,module_integrity_map

def is_elf_32_or_64_bit(elf_file_path):
  command = "file " + elf_file_path
  try:
    output = subprocess.check_output(command, stderr=subprocess.STDOUT, shell=True)
    if "64-bit" in str(output):
      return "arch64"
    elif "32-bit" in str(output):
      return "arch32"
    logging.error("QIIFA: ELF type not supported :" + output)
    exit_func()
  except subprocess.CalledProcessError as e:
    logging.error("Error" + ' exit code: {}'.format(e.returncode))
    logging.error(str(e.output))
    exit_func()

def check_if_modules_get_generated_successfully(folder_path,module_count):
  file_dir_list = os.listdir(folder_path)
  generated_files = 0
  for file_dir in file_dir_list:
    file_dir_path = os.path.join(folder_path,file_dir)
    if os.path.isfile(file_dir_path) and file_dir.endswith(".xml"):
      generated_files = generated_files + 1
  if not generated_files == module_count:
    logging.error("Modules generated is not equal to modules defined in api_metadata")
    logging.error(str(folder_path))
    exit_func()

def generate_golden_references_for_standalone_module(elf_file,dump_file):
  generate_dumps_using_full_module(elf_file,dump_file)

def generate_reference_dumps_for_component(component,folder_hierarchy_path,platform_type,component_state):
  api_metadata_dict = component["api_metadata_dict"]
  api_management_conf = component["api_management_conf"].get("function-pointer")
  modules_container_list = api_metadata_dict["modules_container_list"]
  module_list = None
  for modules_container in modules_container_list:
    if modules_container["platform"] and not modules_container["platform"] == platform_type:
      continue
    else:
      module_list = modules_container["module_list"]
      break

  if not os.path.isdir(folder_hierarchy_path):
    os.makedirs(folder_hierarchy_path)
  state_file = os.path.join(folder_hierarchy_path,component_state_file_name)
  fout = open(state_file, "w")
  fout.write(component_state)
  fout.close()
  unique_module_names_list = []
  for module in module_list:
    ## Priority for Dump generation.
    #1 If Symbol file is provided then all sections are ignored (Headers & Interfaces)
    #2 If Symbol file is not provided then all header files are considered.
    #3 If header file has interface section then header file is ignored and only
    ## functions defined in interfaces are considered.
    module_name = module["name"]
    api_management_conf_module = None
    if api_management_conf is not None:
      api_management_conf_module = api_management_conf.get(module_name)
    if not module_name in unique_module_names_list:
      unique_module_names_list.append(module_name)
    else:
      logging.error("Module names not unique ! : " + module_name)
      logging.error(api_metadata_dict["file"])
      exit_func()
    symbol_file = module["symbol_file"]
    elf_file = module["path"]
    ## Check if elf_file is 32 bit or 64 bit
    lib_arch_type = is_elf_32_or_64_bit(elf_file)
    dump_file = None
    if not lib_arch_type == "arch32":
      dump_file = os.path.join(folder_hierarchy_path,str(module_name) + ".xml")
    else:
      dump_file = os.path.join(folder_hierarchy_path,str(module_name) + "_" + lib_arch_type + ".xml")
    if symbol_file:
      if os.path.isfile(symbol_file):
        ## Handle #1 If Symbol file is provided then all sections are ignored (Headers & Interfaces)
        generate_reference_dumps_using_symbols(elf_file,dump_file,symbol_file)
      else:
        logging.error("Symbol file is not accessible : " + symbol_file)
    else:
      ## Write handling for standalone modules, currently to cover proto library case
      if module["type"] == STANDALONE_MODULE:
        generate_golden_references_for_standalone_module(elf_file,dump_file)
      else:
        ## Pull Header list
        headers = module["headers"]
        header_file_list = []
        header_interface_list =[]
        if headers:
          for header in headers:
            if header["interfaces"]:
              header_interface_list.extend(header["interfaces"])
            else:
              header_file_list.append(header["path"])
        ## Handle condition when no interfaces are present for any headers.
        if not header_interface_list:
          temp_dir_path = tempfile.mkdtemp(prefix="qiifa_headers_")
          try:
            ### Copy all headers in tmp folder
            for header_file in header_file_list:
              random_string = ''.join(random.choice(string.ascii_lowercase) for i in range(5))
              shutil.copy(header_file,os.path.join(temp_dir_path,random_string+ "_"+header_file.split("/")[-1]))
            generate_reference_dumps_using_headers(elf_file,dump_file,temp_dir_path,api_management_conf_module)
          finally:
            shutil.rmtree(temp_dir_path, ignore_errors=True)
        else:
          ### Aggregate header and interface
          ### Copy header_file_list in tmp folder and generate symbol file
          temp_dir_path = tempfile.mkdtemp(prefix="qiifa_headers_")
          try:
            ### Copy all headers in tmp folder
            symbol_file_path = os.path.join(temp_dir_path,"symbol.temp")
            if header_file_list:
              for header_file in header_file_list:
                random_string = ''.join(random.choice(string.ascii_lowercase) for i in range(5))
                shutil.copy(header_file,os.path.join(temp_dir_path,random_string+ "_"+header_file.split("/")[-1]))
              parse_header_and_generate_symbol_file(elf_file,temp_dir_path,None,symbol_file_path,api_management_conf_module)
            ### Generate temp.txt with interfaces object
            temp_file = os.path.join(temp_dir_path,"function.temp")
            generate_function_file_for_interfaces(temp_file,header_interface_list)
            ### append_symbols_list_with_function_file
            append_symbols_list_with_function_file(elf_file,symbol_file_path,temp_file)
            append_interfaces_symbol_list(symbol_file_path,header_interface_list)
            generate_reference_dumps_using_symbols(elf_file,dump_file,symbol_file_path)
          finally:
            shutil.rmtree(temp_dir_path, ignore_errors=True)
  check_if_modules_get_generated_successfully(folder_hierarchy_path,len(module_list))

def append_interfaces_symbol_list(symbol_file_path,header_interface_list):
  file_handler = None
  if os.path.isfile(symbol_file_path):
    file_handler = open(symbol_file_path,'a')
  else:
    file_handler = open(symbol_file_path,'w')
    file_handler.write("[abi_symbol_list]\n")
  for interface in header_interface_list:
    symbol = interface["symbol"]
    if symbol:
      file_handler.write(str(symbol) + "\n")
  file_handler.close()

def generate_function_file_for_interfaces(file_path,header_interface_list):
  file_handler = open(file_path, 'w')
  for interface in header_interface_list:
    ##format namespace#class$function-name
    function_info = None
    interface_namespace = interface["namespace"]
    interface_class = interface["class"]
    interface_name = interface["name"]
    interface_symbol = interface["symbol"]
    ## Do not bother about function name if symbol name is
    ## mentioned , it can be directly added to symbol file.
    if interface_symbol:
      continue
    if interface_namespace is not None and interface_namespace:
      function_info = interface_namespace
    if interface_class is not None and interface_class:
      if function_info is not None:
        function_info = function_info +"#"+ interface_class
      else:
        function_info = interface_class
    if interface_namespace and not interface_class:
      function_info = function_info + "#$" + interface_name
    else:
      if function_info is None:
        function_info = interface_name
      else:
        function_info = function_info + "$" + interface_name
    file_handler.write(function_info+"\n")
  file_handler.close()

def is_valid_version(version):
    version_regex = "^[0-9]+\.[0-9]+((\.{1}[^.$ \n]+)+)?$"
    if re.search(version_regex, version):
        return True
    else:
        return False

def get_major_version(version):
    if not is_valid_version(version):
        return None
    return str(version).split('.')[0]

def get_version_suffix(version):
    if not is_valid_version(version):
        return None
    split_version = str(version).split('.')
    if len(split_version) == 2 :
        return None
    elif len(split_version) == 3:
        return str(version).split('.')[2]

def get_minor_version(version):
    if not is_valid_version(version):
        return None
    return str(version).split('.')[1]

def is_major_update(initial_version,current_version):
    if initial_version == None or initial_version == "":
        if is_valid_version(current_version):
          return 1
        else:
          return 0
    if not is_valid_version(initial_version) or not is_valid_version(current_version):
        return 0
    initial_major_version = get_major_version(initial_version)
    current_major_version = get_major_version(current_version)
    initial_version_suffix = get_version_suffix(initial_version)
    current_version_suffix = get_version_suffix(current_version)
    if initial_version_suffix != current_version_suffix:
        logging.error("Version suffix is not consistent initial version is "+initial_version+" current version is "+current_version)
        return 0
    version_diff = int(current_major_version)-int(initial_major_version)
    if get_minor_version(current_version) != "0":
        logging.error("Major update should have minor version 0 current version is "+current_version)
        return 0
    return version_diff == 1

def is_minor_update(initial_version,current_version):
    if initial_version == None or initial_version == "":
        return 0
    if not is_valid_version(initial_version) or not is_valid_version(current_version):
        return 0
    if int(get_major_version(current_version)) - int(get_major_version(initial_version)):
        return 0
    if get_version_suffix(current_version) != get_version_suffix(initial_version):
       logging.error("Version suffix is not consistent initial version is "+initial_version+" current version is "+current_version)
       return 0
    initial_minor_version = get_minor_version(initial_version)
    current_minor_version = get_minor_version(current_version)
    version_diff = int(current_minor_version)-int(initial_minor_version)
    if version_diff == 1:
        return 1
    else:
        logging.error("Invalid minor_update from "+str(initial_version)+" to "+str(current_version))
        return 0

def is_minor_version_history_consistent(versions):
    versions.sort()
    for i in range(len(versions)):
        if i != 0:
            if not is_minor_update(versions[i-1],versions[i]):
                return 0
    return 1

def get_branch_type(grd_versions,current_version):
    if len(grd_versions) == 0:
        return FRESH_START
    if is_minor_version_history_consistent(grd_versions):
        grd_versions.sort()
        latest_grd_version = grd_versions[-1]
        if latest_grd_version == current_version:
            return NO_UPDATE
        elif is_minor_update(latest_grd_version,current_version):
            return MINOR_UPDATE
        elif is_major_update(latest_grd_version,current_version):
            return MAJOR_UPDATE
        else:
            return None
    else:
        logging.error("Invalid history minor version history")
        return None

def get_grd_versions(grd_path,platform_type):
  if not os.path.exists(grd_path):
    logging.error("Golden reference path doesn't exists : " + grd_path)
    exit_func()
  dir_list = os.listdir(grd_path)
  versions = []
  VERSION_PREFIX = "version_"
  for dr in dir_list:
    if dr.find(VERSION_PREFIX) == -1:
      logging.error("Invalid grd version found at " + grd_path)
    else:
      versions.append(dr[len(VERSION_PREFIX):])
  return versions

def is_minor_version_history_consistent(versions):
    versions.sort()
    for i in range(len(versions)):
        if i != 0:
            if not is_minor_update(versions[i-1],versions[i]):
                return 0
    return 1

def run_minor_version_compatibility(component,platform_type):
  ## Identify minor versions.
  api_metadata_dict = component["api_metadata_dict"]
  api_metadata_platform = api_metadata_dict["modules_container_list"][0]["platform"]
  component_name = component["name"]
  component_version = component["version"]
  component_path = os.path.join(component["grd"],platform_type,component_name)
  component_version_path = os.path.join(component_path,"version_"+component_version)
  component_grd_state = get_component_grd_state(component_version_path)
  if component_grd_state != allowed_component_states[1]:
    logging.error("Invalid minor update state for component : "+str(component_name)+ ", state : "+component_grd_state)
    exit_func()
  if not os.path.exists(component_path):
    logging.error("Golden reference path doesn't exists : " + component_path)
    exit_func()

  file_dir_list = os.listdir(component_path)
  versions = []
  for dr in file_dir_list:
    versions.append(dr.split("_")[1])
  if not is_minor_version_history_consistent(versions):
    logging.error("Minor version history inconsistent for component : "+component_name)
    logging.error("Version History : "+" -> ".join(versions))
    exit_func()
  number_of_dir = len(file_dir_list)

  if number_of_dir > 1 :
    component_major_version = str(get_major_version(component["version"]))
    component_minor_version = get_minor_version(component["version"])
    component_suffix = get_version_suffix(component["version"])
    component_x_minus_1_version = int(component_minor_version)-1
    if component_x_minus_1_version >= 0:
      ## Create path for reading reference dumps.
      x_minux_1_version_string = "version_" + component_major_version + "." + str(component_x_minus_1_version)
      if component_suffix:
        x_minux_1_version_string += "." + component_suffix
      component_x_minus_1_version_path = os.path.join(component_path,x_minux_1_version_string)
      ## Consider x-1 version as golden and generate abidiff
      grd_state = get_component_grd_state(component_x_minus_1_version_path)
      if grd_state == None:
        logging.error("Failed to get GRD state for the latest minor revision : "+str(component_x_minus_1_version)+" of component : "+str(component_name))
        exit_func()
      elif grd_state != allowed_component_states[2]:
        logging.error("Minor revision only allowed in stable state")
        logging.error("n-1 revision grd state is "+str(grd_state)+" for the component : "+str(component_name))
        exit_func()
      return run_abi_compatibility_check(component_x_minus_1_version_path,component_version_path)
  else:
    return -1,{}

def run_integrity_check_on_techpack_cd_and_api_metadata(techpack_dict):
  ## check if CD state is : dev,compat and stable
  for component in techpack_dict:
    component_obj = techpack_dict[component]
    if component_obj["state"] not in allowed_component_states:
      logging.error("Component state is undefined : " + component_obj["name"])
      return False
  return True

def match_component_state(current_component_state,golden_path):
  component_state_file_path = os.path.join(golden_path,component_state_file_name)
  file_handler = open(component_state_file_path,"r")
  golden_component_state =  file_handler.readline().strip()
  file_handler.close()
  if not current_component_state == golden_component_state:
    return False
  return True

def run_interface_integrity_check_for_component(component,component_dump_path,platform_type,skip_state_check=0):
  ## Get Module list within a component and run integrity check if all
  ## required dumps are present to run abidiff
  component_name = component["name"]
  component_version = component["version"]
  component_state = component["state"]
  grd_path = os.path.join(component["grd"],platform_type,component_name)
  grd_versions = get_grd_versions(grd_path,platform_type)
  # pop the latest i.e. current version
  grd_versions.sort()
  if len(grd_versions) > 1:
    grd_versions.pop()
  branch_type = get_branch_type(grd_versions,component_version)
  minor_version_compatibility = -1
  module_integrity_map = {}
  if branch_type == MAJOR_UPDATE:
    logging.info("Major update detected for the component : "+component_name)
    logging.warn("Kindly delete GRD for the previous version to continue")
  elif branch_type == MINOR_UPDATE:
    logging.info("Minor update detected for the component : "+component_name)
    minor_version_compatibility,module_integrity_map = run_minor_version_compatibility(component,platform_type)
  elif branch_type == FRESH_START:
    logging.error("GRD not found for the component : "+component_name)
    exit_func()
  elif branch_type == NO_UPDATE:
    logging.info("IIC on same version for component : "+component_name)
  else:
    logging.error("Failed to determine branch type for the component : "+component_name)
    exit_func()

  global minor_version_component_compatibility_map
  minor_version_component_compatibility_map [component_name] = [minor_version_compatibility,module_integrity_map]
  grd_folder_path = grd_constant
  component_version_folder = "version_" + component_version
  path_to_component_golden_version = None
  api_metadata_dict = component["api_metadata_dict"]
  modules_container_list = api_metadata_dict["modules_container_list"]
  module_list = None
  api_metdata_platform_value = None
  for modules_container in modules_container_list:
    if modules_container["platform"] and not modules_container["platform"] == platform_type:
      continue
    else:
      api_metdata_platform_value = modules_container["platform"]
      module_list = modules_container["module_list"]
      break

  path_to_component_golden_version = get_component_version_golden_path(component,api_metdata_platform_value,platform_type)
  if not match_component_state(component_state,path_to_component_golden_version) and not skip_state_check:
    logging.error("Component state is different for current and golden!!")
    logging.error(component_name)
    exit_func()
  ## Enforce all lists should be equal at any point of time , irrespective of component state.
  ## This is to ensure all modules are present.
  module_list_api_metadata = []
  module_list_golden = []
  module_list_current = []
  ## Map to store filename , path mapping
  map_golden_modules_dumps = {}
  map_current_modules_dumps = {}
  for module in module_list:
    module_list_api_metadata.append(str(module["name"]))

  for dirpath, dirs, files in os.walk(path_to_component_golden_version):
    for filename in files:
      fname = os.path.join(dirpath,filename)
      if fname.endswith('.xml'):
        fname_without_extension = os.path.splitext(filename)[0]
        map_golden_modules_dumps[fname_without_extension] = fname
        module_list_golden.append(fname_without_extension)

  for dirpath, dirs, files in os.walk(component_dump_path):
    for filename in files:
      fname = os.path.join(dirpath,filename)
      if fname.endswith('.xml'):
        fname_without_extension = os.path.splitext(filename)[0]
        map_current_modules_dumps[fname_without_extension] = fname
        module_list_current.append(fname_without_extension)
  if sorted(module_list_api_metadata) == sorted(module_list_current):
    logging.info("Module Integrity matched.")
    logging.info("Execute Interface Integrity Check.")
  else:
    logging.error("Module Integrity Failed for Component - "+component_name+" due to module mismatch")
    logging.error("GRD available modules are - "+" ".join(module_list_golden))
    logging.error("Expected modules are - "+" ".join(module_list_current))
    exit_func()

  ## TODO : We need to do 32 bit handling here.
  for module in module_list_current:
    if not module in module_list_golden:
      ## This should be treated as Extension case.
      logging.error("Module Integrity Failed.")
      logging.error("New Module detected. Component Interface is Extended.")
      module_integrity_map = {}
      return 1,module_integrity_map

  for module in module_list_golden:
    if not module in module_list_current:
      logging.error("Module Integrity Failed.")
      logging.error("Module Removed. Component Interface is Broken.")
      module_integrity_map = {}
      return 2,module_integrity_map

  ## All files are available to run abidiff check, let;s loop through all modules
  ## and run abidiff check.
  module_integrity_output = []
  module_integrity_map = {}
  for module in module_list_api_metadata:
    output,message = generate_diff_for_reference_module(map_golden_modules_dumps[module],map_current_modules_dumps[module])
    module_integrity_output.append(output)
    module_integrity_map[module] = [output,message]
  if sum(module_integrity_output) == 0:
    return 0,module_integrity_map
  elif max(module_integrity_output) == 1:
    return 1,module_integrity_map
  elif max(module_integrity_output) == 2:
    return 2,module_integrity_map

def generate_compatibility_report_for_techpack(techpack_dict,component_integrity_map,\
component_integrity_output,compatibility_report_path):
  file_handler = open(compatibility_report_path,'w')
  integrity_values = {0:"Compatible",1:"Extension",2:"Incompatible"}
  file_handler.write("QIIFA Compatibility Report\n")
  file_handler.write("\n")
  compatbility_bit = 0
  for component_key in techpack_dict:
    component_element = techpack_dict[component_key]
    component_version = component_element["version"]
    component_state = component_element["state"]
    integerity_result = component_integrity_map[component_key][0]
    modules_integrity_map = component_integrity_map[component_key][1]
    file_handler.write("Component : " + component_key + component_version + "\n")
    file_handler.write ("State : " + component_state + "\n")
    file_handler.write ("Integerity Check Result : " + integrity_values[integerity_result] + "\n")
    if not component_state == "development" and integerity_result > 0:
      compatbility_bit = 1

    if component_state == "development" and integerity_result == 1:
      file_handler.write("QIIFA : Warning : Update Golden Refernces as Extension is detected.")
    elif component_state == "development" and integerity_result == 2:
      file_handler.write("QIIFA : Warning : Backward compatibility Broken. Major Up-rev Required.")
    if component_state == "compatible" and integerity_result == 1:
      file_handler.write("QIIFA : Error : Update Golden Refernces as Extension is detected.")
    elif component_state == "compatible" and integerity_result == 2:
      file_handler.write("QIIFA : Error : Backward compatibility Broken. Major Up-rev Required.")
    elif component_state == "stable" and integerity_result == 1:
      file_handler.write("QIIFA : Error : Extension Detected. Minor Up-rev Required.")
    elif component_state == "stable" and integerity_result ==2:
      file_handler.write("QIIFA : Error : Backward compatibility Broken. Major Up-rev Required.")

    if component_key in minor_version_component_compatibility_map:
      integrity_result_and_module_integrity_list = minor_version_component_compatibility_map[component_key]
      min_version_integrity_result = integrity_result_and_module_integrity_list[0]
      min_version_module_integrity_map = integrity_result_and_module_integrity_list[1]
      if component_state == "development" and int(min_version_integrity_result) > 1:
        file_handler.write("QIIFA : Warning : Current Version is not compatible with minor Version.")
      elif ( component_state == "compatible" or component_state== "stable") and int(min_version_integrity_result) > 1:
        file_handler.write("QIIFA : Error : Current Version is not compatible with minor Version.")
        compatbility_bit = 1
      elif min_version_integrity_result == 0:
        file_handler.write("QIIFA : Info : Compatible with minor Version\n")
    file_handler.write("\n")
  file_handler.close()
  return compatbility_bit

def run_interface_integrity_check(techpack_dict,mode,platform_type,compatibility_report_path):
  #1: Compatibility with x-1 version
  #2: Warning mode for development status
  #3: Allow API addition in compatible status and update golden reference
  #4: Error out in Stable status
  ## Use tmp storage to generate dumps on the fly
  component_integrity_output = []
  component_integrity_map= {}
  for component_key in techpack_dict:
    ## Create Reference structure for current reference dumps
    ## Let's only use component-name and version flat_hieraarchy because we
    ## will be generating dumps for only latest version. So it is just better
    ## to use single level structure with component folder and all module
    ## reference dumps.
    component = techpack_dict[component_key]
    component_name_value = component["name"] + "_" + component["version"]
    temp_dir_path = tempfile.mkdtemp(prefix="qiifa_dumps_")
    component_dump_path = os.path.join(temp_dir_path,component_name_value)
    try:
      os.mkdir(component_dump_path)
      generate_reference_dumps_for_component(component,component_dump_path,platform_type,component["state"])
      output,module_integrity_map = run_interface_integrity_check_for_component(component,\
      component_dump_path,platform_type)
      component_integrity_output.append(output)
      component_integrity_map[component_key] = [output,module_integrity_map]
    finally:
      shutil.rmtree(temp_dir_path, ignore_errors=True)
  return generate_compatibility_report_for_techpack(techpack_dict,component_integrity_map,
  component_integrity_output,compatibility_report_path)

def run_integrity_check_on_args(args):
  arch_supported_values = ["arch64","arch32"]
  iic_value = args.iic

  if not args.qtools or not os.path.exists(args.qtools):
    logging.error("QIIFA Tools not accessible.")
    return False
  #if iic_value and iic_value not in iic_supported:
   # logging.error("QIIFA: IIC only support enforce and compatible.")
    #return False
  if args.platform == "LE" and args.symbol_path:
    logging.error("QIIFA: Symbol path is not supported for this platform.")
    return False
  if args.platform == "LE" and not args.rootfs_path:
    logging.error("QIIFA: Rootfs path is requried for LE platform.")
    return False
  if args.platform == "LA" and not args.symbol_path:
    logging.error("QIIFA: Symbols path is required for LA platform")
    return False
  if iic_value and not args.comp_report_path:
    logging.error("QIIFA: Compatibility Report path is required with IIC option.")
    return False
  if args.golden_ref and args.iic:
    logging.error("QIIFA: Golden Reference and IIC mode cannot be executed together.")
    return False
  if args.rootfs_path and not os.path.isdir(args.rootfs_path):
    logging.error("QIIFA: Rootfs Path is not accessible.")
    return False
  if args.workspace_root and not os.path.isdir(args.workspace_root):
    logging.error("QIIFA: Workspace root Path is not accessible.")
    return False
  if args.architecture and args.architecture not in arch_supported_values:
    logging.error("QIIFA: arch supported values are " + str(arch_supported_values))
    return False
  if args.platform not in supported_platforms:
    logging.error("QIIFA: platform supported values are " + str(supported_platforms))
    return False
  if args.symbol_path is not None and args.symbol_path:
    if not os.path.isdir(args.symbol_path):
      logging.error("QIIFA: Symbol Path is not accessible.")
  return True

def get_component_grd_state(grd_path):
  state_file_path = os.path.join(grd_path,component_state_file_name)
  if not os.access(state_file_path,os.R_OK):
    logging.error("State File missing at "+grd_path)
    return ""
  file_handler = open(state_file_path,"r")
  platform_state  = None
  platform_state =  file_handler.readline().strip()
  return platform_state

def check_component_state_sync(techpack_dict):
    component_in_sync = bool(1)
    unsupported_platform_found = bool(0)
    for component_key in techpack_dict:
         component = techpack_dict[component_key]
         logging.info("Running component sync check for the component : "+str(component["name"]))
         component_state = component["state"]
         grd_root_path = component["grd"]
         if not os.path.isdir(grd_root_path):
            logging.error("No GRD found, cannot run IIC without GRD: "+str(component["name"]))
            logging.error("Exiting")
            exit_func()
         grd_contents = os.listdir(grd_root_path)
         available_platforms = grd_contents
         available_platforms.sort()
         for platform in available_platforms:
            if platform not in supported_platforms:
                unsupported_platform_found = bool(1)
                logging.error("Unsupported platform "+platform)
                continue
            grd_path = get_component_version_golden_path(component,"",platform)
            state_file_path = os.path.join(grd_path,component_state_file_name)
            if not os.access(state_file_path,os.R_OK):
                logging.error("State File missing for platform at "+state_file_path)
                component_in_sync = bool(0)
                continue
            file_handler = open(state_file_path,"r")
            platform_state =  file_handler.readline().strip()
            if platform_state != component_state:
                component_in_sync = bool(0)
                logging.error("Component state inconsistency identified")
                logging.error("Component Name = "+component["name"]+" Platform = "+platform)
                logging.error("Component State = "+component_state+ " Platform State = "+platform_state)
                logging.error("Kindly update the Golden reference dumps to match the component state than re-run IIC check")
    if unsupported_platform_found:
        logging.error("Unsupported platforms found")
    if not component_in_sync:
        logging.error("Component state not in sync across all platform")
    if unsupported_platform_found or not component_in_sync:
        logging.error("Aborting...")
        exit_func()
    logging.info("Component State Check Successfull")

def print_information_for_components(techpack_dict):
  logging.info("Components: \n")
  for component_key in techpack_dict:
    component = techpack_dict[component_key]
    logging.info(component["name"] + component["version"]+"\n")

def log_results(start_time,report_path,output):
  logging.info("QIIFA : Interface Integrity Check executed Successfully !!")
  logging.info("QIIFA : Check Detailed report !!  : " + report_path)
  logging.info("--- Execution Time :  %s seconds ---" % (time.time() - start_time))
  logging.info("IIC Output: " + str(output))

def evaluate_iic(iic_arg,platform,symbol_path,report_path):
  techpack_dict = {}
  if iic_arg in globals_api_management.SUPPORTED_IIC_MODES:
    if iic_arg == globals_api_management.SUPPORTED_IIC_MODES[globals_api_management.IIC_ENFORCE_QC_DIR_MODE]:
      techpack_dict = prepare_cd_and_api_metadata_dict(platform,symbol_path,scan_qc_dir=True)
    else:
      techpack_dict = prepare_cd_and_api_metadata_dict(platform,symbol_path)
  else:
    techpack_dict = prepare_cd_and_api_metadata_dict(platform,symbol_path,iic_arg)
  if not len(techpack_dict):
    return "0"
  integrity_flag = run_integrity_check_on_techpack_cd_and_api_metadata(techpack_dict)
  check_component_state_sync(techpack_dict)
  output = run_interface_integrity_check(techpack_dict,iic_arg,platform,report_path)
  return output

def _qiifa_IIC(wroot,iic_type,symbols,report_path,qtools):
  start_time = time.time()
  platform = "LA"
  global IIC_MODE,skip_cd_check
  IIC_MODE = True
  skip_cd_check = True
  init_logging()
  global ROOT_PATH
  ROOT_PATH = wroot
  init_globals(ROOT_PATH,qtools)
  output = evaluate_iic(iic_type,platform,symbols,report_path)
  log_results(start_time,report_path,output)
  if str(output) != "0":
    exit_func()
  return output

def prepare_arg_parser():
  parser = argparse.ArgumentParser(description='QIIFA Api Management Script')
  required_group = parser.add_argument_group('Mandatory Arguments')
  required_group.add_argument("--platform", help="Supported : [LA,LE]",type=str,dest="platform",required=True)
  required_group.add_argument("--wroot", help="Workspace root path",type=str,dest="workspace_root",required=True)
  required_group.add_argument("--qiifa_tools", help="Abs Path to QIIFA tools dir",type=str,dest="qtools",required=True)
  parser.add_argument("--rootfs", help="Option is applicable for LE",type=str,dest="rootfs_path",required=False)
  parser.add_argument("--golden_ref", help="Generates Golden Reference",type=str,dest="golden_ref",required=False)
  parser.add_argument("--iic", help="Interface Integrity check Supported : [enforce,warning,component_name]",type=str,dest="iic",required=False)
  parser.add_argument("--symbol", help="LA Symbol folder path",type=str,dest="symbol_path",required=False)
  parser.add_argument("--cr", help="Path to output Compatibility Report",type=str,dest="comp_report_path",required=False)
  parser.add_argument("--arch", help="Supported : [arch64,arch32]",type=str,dest="architecture",required=False)
  parser.add_argument("--skip_cd_check", dest="skip_cd_check",required=False,action='store_true')
  return parser

def main():
  start_time = time.time()
  arg_parser = prepare_arg_parser()
  args = arg_parser.parse_args()
  init_logging()
  if not run_integrity_check_on_args(args):
    logging.error("QIIFA: Argument Integrity Check Failed")
    exit_func()
  global ROOT_PATH,ROOTFS_PATH,IIC_MODE,skip_cd_check
  ROOT_PATH = args.workspace_root
  ROOTFS_PATH = args.rootfs_path
  skip_cd_check = args.skip_cd_check
  if args.platform == "LE":
    args.symbol_path = args.rootfs_path
  if args.iic is not None and args.iic:
    IIC_MODE = True
  init_globals(ROOT_PATH,args.qtools)
  platform = args.platform
  architecture = args.architecture
  if args.golden_ref is not None and args.golden_ref:
    generate_golden_references(args.golden_ref,args.symbol_path,args.platform)
  if args.iic is not None and args.iic:
    ### output values 0 & 1
    ### 0 : Compatible
    ### 1 : Extension or Backward compatibility broken
    output = evaluate_iic(args.iic,platform,args.symbol_path,args.comp_report_path)
    log_results(start_time,args.comp_report_path,output)
    if str(output) != "0":
      exit_func()
    return output

if __name__ == "__main__":
  main()
