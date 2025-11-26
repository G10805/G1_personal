#!/usr/bin/python
# -*- coding: utf-8 -*-
#Copyright (c) 2022 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

'''
Import standard python modules
'''
import sys, os, json, math, fnmatch
import re
from plugins.qiifa_hidl_checker import xmltodict

import xml.etree.ElementTree as ET
from collections import OrderedDict

'''
Import local utilities
'''
from qiifa_util.util import UtilFunctions, Variables, Constants, Logger

'''
Import plugin_interface base module 18
'''
from plugin_interface import plugin_interface
try:
    import networkx as nx
    import matplotlib.pyplot as plt
except ImportError:
    GENERATE_GRAPH = False
else:
    GENERATE_GRAPH = True

LOG_TAG = "api_dep_plugin"
ROOT_PATH = Constants.croot
grd_constant = "golden_reference_dumps"
master_api_dep_dict = {}
plugin_state_enforce = False
test_source = False


def parse_cd_xml(self, cd_project_list):
    api_dep_dict = {}
    # print(cd_project_list)

    # iterate over each file in file lies
    for xml_file in cd_project_list:
        # open the cd-xml
        fd = open(xml_file)
        # convert it to a dict
        cd_xml_dict = xmltodict.parse(fd.read())

        # grab the component name
        cd_name = cd_xml_dict['component']['configuration']['@name']
        # grab the component version
        cd_vers = cd_xml_dict['component']['configuration']['@version']
        try:
            splitversion = cd_vers.split('.')
            newver = 1
            count = 0
            for splits in splitversion:
                if splits.isdigit():
                    if count == 0:
                        newver = int(splits)
                    elif count == 1:
                        if (int(splits)>0):
                            newver =  float(newver) * (float(splits)/10)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,'api dependency check', parse_cd_xml.__name__, "Please check the version of the component"+cd_xml_dict['component']['configuration']['@name']+" and Please contact qiifa-squad")
                count = count+1
        except:
               UtilFunctions.print_violations_on_stdout(LOG_TAG,'api dependency check', parse_cd_xml.__name__, "Please check the version of the component"+cd_xml_dict['component']['configuration']['@name']+" and Please contact qiifa-squad")
        cd_vers = float(newver)

        api_dep_dict[cd_name] = OrderedDict()
        api_dep_dict[cd_name]['Version'] = cd_vers
        # create a list for this cd's dependencies
        dep_list = []
        # check if there are any dependencies
        if cd_xml_dict['component'] is not None:
            if type(cd_xml_dict['component']) is OrderedDict:
                if 'dependencies' in cd_xml_dict['component'].keys():
                    if cd_xml_dict['component']['dependencies'] is not None:
                        # check if there are multiple dependencies
                        if (type(cd_xml_dict['component']['dependencies']['dependency'])) is list:
                            # for each dependency, grab the component name and version
                            for dep in cd_xml_dict['component']['dependencies']['dependency']:
                                # grab the component name
                                component = dep['@component']
                                # grab the component version
                                version   = dep['@version']
                                # if there is an or, then generate multple name-version combos
                                if '||' in version:
                                    version = version.split('||')
                                elif '>=' in version:
                                    version = version.split('>=')
                                elif '>' in version:
                                    version = version.split('>')
                                # store name-version combos as a list, since there may be multple versions
                                version_list = [component]
                                # print(version)
                                if type(version) is list:
                                    for ver in version:
                                        try:
                                            version_list.append(float(ver.strip().encode()))
                                        except Exception as e:
                                            Logger.logStdout.error(e)
                                            Logger.logStdout.error("Version Syntax problem. Please check the version")
                                else:
                                    version_list.append(float(version.strip().encode()))
                                # add to the dependency list
                                dep_list.append(version_list)
                        else:
                            # only a single dependency
                            component = cd_xml_dict['component']['dependencies']['dependency']['@component']
                            version   = cd_xml_dict['component']['dependencies']['dependency']['@version']

                            # if there is an or, then generate multple name-version combos
                            if '||' in version:
                                version = version.split('||')
                            # store name-version combos as a list, since there may be multple versions
                            version_list = [component]
                            if type(version) is list:
                                for ver in version:
                                    version_list.append(float(ver.strip().encode()))
                            else:
                                version_list.append(float(version.strip().encode()))
                            # add to the dependency list
                            dep_list.append(version_list)

                    # add the key:value combo to the api_dep_xml_dict
        api_dep_dict[cd_name]['Dependency'] = dep_list
    master_api_dep_dict.update(api_dep_dict)
    return api_dep_dict



def check_api_dependencies(self,api_dep_dict):

    # iterate over the api_dep_xml_dict keys and values
    failure = False
    failure_reason = ''
    for key in api_dep_dict.keys():
        # for each dependency in the 'value' column
        for dependency in api_dep_dict[key]['Dependency']:
            # dependency name is the first element in the sub-list, the following indices  are acceptable versions
            dep_name = dependency[0]
            # check if the dependency is listed in the api_dep_dict, if not its a Warning
            if api_dep_dict.get(dep_name) is not None:
                # supported version from the cd gathered
                sup_ver = api_dep_dict[dep_name]['Version']
                # our dict value is a list of lists, check the versions for each dependency name ie indices (1-n)
                if not any(version == sup_ver for version in dependency[1:]):
                    # If no exact match, Check if major number matches and throw warning if they do.
                    if not any(math.floor(version) == math.floor(sup_ver) and version < sup_ver for version in dependency[1:]):
                        # Major number does not match, therefor this is a failure
                        failure = True
                        reason = '\nComponent {} dependency {} version(s) {} not in version list.'.format(key,dep_name,dependency[1:])
                        failure_reason += reason
                    else:
                        Logger.logStdout.warning('ONLY version major number match for CD {} w/ dependency on {}'.format(key,dep_name,dependency[1:]))
                        Logger.logStdout.warning('Current {} Version {}, Dependent on Version {}'.format(dep_name, sup_ver, dependency[1:]))
            else:
                failure = True
                reason = '\n ERROR CD {} has dependency on {} which does not have a corresponding CD file'.format(key,dep_name,dependency[1:])
                # Logger.logStdout.info(reason)
                failure_reason += reason

    # Print out the failure messages if there is a failure
    if failure:
        failure_reason = '\n' + failure_reason + '\n'
        if plugin_state_enforce:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,'api dependency check', check_api_dependencies.__name__, failure_reason,)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,'api dependency check', check_api_dependencies.__name__, failure_reason, False)


def parse_manifest_xml_and_locate_cd(self):

    master_cd_dict = {}
    cd_regular_exp = "qc/components/*/cd"

    # Check if manifest path exist.
    MANIFEST_PATH = os.path.join(Constants.croot,".repo/manifests/default.xml")
    if not os.path.exists(MANIFEST_PATH):
        print("QIIFA : Manifest Path doesn't exists.")
    else:
        xml_root_node = ET.parse(MANIFEST_PATH).getroot()
        project_list = xml_root_node.findall(".//project")
        cd_project_list = []
        for project in project_list:
            if fnmatch.fnmatch(project.attrib["name"],cd_regular_exp):
                project_dict = {}
                project_dict["name"] = project.attrib["name"]
                project_dict["path"] = os.path.join(ROOT_PATH,project.attrib["path"])
                project_dict["grd"]  = os.path.join(project_dict["path"],grd_constant)
                cd_project_list.append(project_dict)
        master_cd_dict["VENDOR"] = cd_project_list

        #Traversing through techpack manifest files
        repo_path = os.path.join(Constants.croot,".repo/")
        for root, dirs, files in os.walk(repo_path):
            for name in dirs:
                cd_project_list = []
                if "AU_TECHPACK" in name:
                    TP_MANIFEST_PATH = os.path.join(repo_path,name,"default.xml")
                    TECHPACK_NAME = re.findall(r'(?:AU_TECHPACK_)(.*)(?:.LA(.)*[0-9]*)',name)[0][0]
                    if not os.path.exists(TP_MANIFEST_PATH):
                        print("QIIFA : Manifest Path doesn't exists.")
                        continue
                    tp_xml_root_node = ET.parse(TP_MANIFEST_PATH).getroot()
                    tp_project_list = tp_xml_root_node.findall(".//project")
                    for project in tp_project_list:
                        if fnmatch.fnmatch(project.attrib["name"],cd_regular_exp):
                            project_dict = {}
                            project_dict["name"] = project.attrib["name"]
                            project_dict["path"] = os.path.join(ROOT_PATH,project.attrib["path"])
                            project_dict["grd"]  = os.path.join(project_dict["path"],grd_constant)
                            cd_project_list.append(project_dict)
                    master_cd_dict[TECHPACK_NAME] = cd_project_list

    cd_tp_regular_exp = r'qc/(.*?).lnx/cd'
    cd_path_list=[]

    # Creating master dict OS walk through qc folder to determine list of cd folders
    if not os.path.exists(os.path.join(ROOT_PATH,"qc")):
        print("QIIFA: platform/qc folder does not exist")
    else:
        if len(os.listdir(os.path.join(ROOT_PATH,"qc"))) == 0:
            print("\n QIIFA: platform/qc folder is empty")
        else:
            for root, dirs, files in os.walk(os.path.join(ROOT_PATH,"qc")):
                if dirs and "cd" in dirs:
                    cd_path_list.append(os.path.join(root,"cd"))
    img_tp_dictionary_list = [{"name":re.findall(r'(?<=qc/).+?(?=/cd)',x)[0],
                            "path":os.path.join(ROOT_PATH,re.search(cd_tp_regular_exp,x).group(0)),
                            "grd": os.path.join(ROOT_PATH,re.search(cd_tp_regular_exp,x).group(0),grd_constant)
                            } for x in cd_path_list if re.findall(cd_tp_regular_exp,x)]
    if(len(img_tp_dictionary_list)==0):
        print("QIIFA Dependency Checker: No cd.xml files were found")
    master_cd_dict["MASTER"] = img_tp_dictionary_list
    return master_cd_dict


def output_dicts_to_json(self, build_name, api_dep_dict):

    if not os.path.isdir(os.path.join(Constants.qiifa_out_dir,"api_dependency_graphs",build_name)):
            if not UtilFunctions.pathExists(os.path.join(Constants.qiifa_out_dir, "api_dependency_graphs")):
                os.mkdir(os.path.join(Constants.qiifa_out_dir, "api_dependency_graphs"))
            os.mkdir(os.path.join(Constants.qiifa_out_dir, "api_dependency_graphs", build_name))
    json_path = os.path.join(Constants.qiifa_out_dir,"api_dependency_graphs",build_name,build_name+'_api_dep_plugin_info.json')
    if UtilFunctions.pathExists(json_path):
        os.remove(json_path)
    try:
        with open(json_path, 'w') as json_file:
            app_json = json.dump(api_dep_dict, json_file, indent=4)
    except:
        if not os.path.exists(json_path):
            print("QIIFA Dependency Checker: The {} directory does not exist".format(json_path))
        else:
            print("QIIFA Dependency Checker: There was a problem with the {} directory".format(json_path))

def output_dependency_graph(self, build_name, api_dep_dict):
    G = nx.DiGraph()

    for key in api_dep_dict.keys():
        for dependency in api_dep_dict[key]['Dependency']:
            dep_name = dependency[0]
            #if build_name != "master":
                # print([key, dep_name, dependency[1:]])
            G.add_edge(key, dep_name, version=dependency[1:])

    color_map = []
    for node in G:
        if node in ["vsdk.lnx","qcom-devices.lnx","kernel.lnx","android-core.lnx","sdllvm_arm_14.lnx","sepolicy.vndr.lnx"]:
            color_map.append('#ffb300')
        else:
            if api_dep_dict.get(node):
                color_map.append('#31ff03')
            else:
                color_map.append('#CC00CC')
    pos = nx.spring_layout(G, k=2.5, scale = 7, center=[5,5])
    plt.figure(7,figsize=(56,56))
    nx.draw_networkx(G, pos=pos, arrows=True, node_size=10000, font_size=8, node_color=color_map)

    edge_labels = nx.get_edge_attributes(G,'version')
    nx.draw_networkx_edge_labels(G, pos=pos, edge_labels=edge_labels, font_size=20)

    graph_path = os.path.join(Constants.qiifa_out_dir, "api_dependency_graphs", build_name, build_name+'_api_dep_plugin_graph.png')

    plt.savefig(graph_path, bbox_inches='tight')
    #Clearing the graph and plot
    plt.clf()
    G.clear()

def print_populated_dicts(self):
    # print our the generated dict
    print('Populated CD_XML Dependency Dict')
    for key in master_api_dep_dict.keys():
        print(key, master_api_dep_dict[key]['Dependency'])

    print('\nPopulated CD_XML Version Dict')
    for key in master_api_dep_dict.keys():
        print(key, master_api_dep_dict[key]['Version'])



def check_plugin_state(self):
    global plugin_state_enforce
    plugin_state_enforce = Variables.is_enforced & Variables.api_dep_enforce


def parse_graph_for_build(self, build_name, cd_project_list):
    if cd_project_list:
        Logger.logInternal.info('cd list found for {}\n: {}'.format(build_name,cd_project_list))
    else:
        Logger.logInternal.info('API Dependency Plugin: No CD XML files found')
        return
    # parse our cd-xmls and generate our dict "api_dep_xml_dict"
    api_dep_dict_each = parse_cd_xml(self, cd_project_list)

    output_dicts_to_json(self, build_name, api_dep_dict_each)

    if GENERATE_GRAPH:
        #print("\nGenerating graph for {}\n".format(build_name))
        output_dependency_graph(self, build_name, api_dep_dict_each)
    else:
        Logger.logInternal.info('API Dependency Plugin: No graph generated, import error')

def func_start_qiifa_api_dep_checker(self):

    check_plugin_state(self)
    master_cd_dict = {}
    if test_source:
        cd_project_list = []
        cd_project_list = os.listdir(test_source)
        for i in range(len(cd_project_list)):
            cd_project_list[i] = os.path.join(test_source, cd_project_list[i])
        parse_graph_for_build(self,"APTTest" , cd_project_list)
    else:
        master_cd_dict = parse_manifest_xml_and_locate_cd(self)
        for key in master_cd_dict:
            cd_project_list = []
            build_name = key
            output_dict_project_list = master_cd_dict[key]
            for items in output_dict_project_list:
                path = os.path.join(items['path'],'cd.xml')
                if os.path.exists(path):
                    cd_project_list.append(path)
            parse_graph_for_build(self, build_name, cd_project_list)

    # validate our dependencies
    check_api_dependencies(self,master_api_dep_dict)

class qiifa_api_dep:
    def __init__(self):
         pass
    start_qiifa_api_dep_checker = func_start_qiifa_api_dep_checker

'''
plugin class implementation
plugin class is derived from plugin_interface class
Name of plugin class MUST be MyPlugin
'''
class MyPlugin(plugin_interface):
    def __init__(self):
        pass

    def register(self):
        return Constants.DEPENDENCY_SUPPORTED_CREATE_ARGS

    def config(self, QIIFA_type=None, libsPath=None, CMDPath=None):
        pass

    def generateGoldenCMD(self, libsPath=None, storagePath=None, create_args_lst=None):
        pass

    def IIC(self, **kwargs):
        global test_source
        # Disabling plugin temporarily
        return

        if kwargs.get('arg_test_source'):
            test_source = kwargs.get('arg_test_source')

        if kwargs.get('arg_tp_build'):
            func_start_qiifa_api_dep_checker(self)
        else:
            Logger.logInternal.info('api_dependency_checker: not ran, only runs for techpack builds')
