
#!/usr/bin/python
# -*- coding: utf-8 -*-
#Copyright (c) 2021 Qualcomm Technologies, Inc.
#All Rights Reserved.
#Confidential and Proprietary - Qualcomm Technologies, Inc.

'''
Import standard python modules
'''
import sys,os,json,shutil,traceback

'''
Import local utilities
'''
from qiifa_util.util import UtilFunctions, Variables, Constants, Logger

'''
Import plugin_interface base module
'''
from plugin_interface import plugin_interface
try:
    import plugins.qiifa_hidl_checker.xmltodict as xmltodict
except Exception as e:
    traceback.print_exc()
    Logger.logStdout.error(e)
    Logger.logStdout.error("Please check the xmltodict library. Please contact qiifa-squad")
    sys.exit(1)

from collections import OrderedDict

sys.dont_write_bytecode = True

LOG_TAG = "aidl_plugin"
module_info_dict ={}
aidl_metadata_dict = {}
aidl_skipped_lst = {}
plugin_state_warning = False
FCM_GLOBAL_DICT={"device":"device","current":0}
FCM_XML_DICT = {}
FCM_MAX_VAL = 0
FCM_MIN_VAL = 100
FCM_MAX_KEY = ""
FCM_VENDOR_TL = 0
DEVICES_FCM_MAX_KEY = ""
XML_HAL_NOTFND = []
MD_HAL_NOTFND = {}
VNDR_HAL_NOTFND = {}
qiifa_aidl_chipset_dvlpmnt_enbl = []

def load_info_from_JSON_file(path):
    '''
    Parse the JSON file given in the path into a list of dictionaries.
    '''
    info_dict = [] #The json file would always be a list of dictionaries
    filename = os.path.basename(path)
    try:
        info_file_handle = open(path, "r")
        info_dict = json.load(info_file_handle)
        info_file_handle.close()
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "AIDL file is not present in "+path
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,load_info_from_JSON_file.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,load_info_from_JSON_file.__name__,reason)
            sys.exit(1)
    return info_dict

def aidl_checker_main_create(self,
                             flag,
                             arg_create_type,
                             arg_intf_name=None):
    Logger.logStdout.info("Running qiifa golden db generator... \n")
    if not check_plugin_state():
        Logger.logStdout.warning("AIDL Plugin state doen't match supported values. Exiting now.")
        return
    if plugin_disabled():
        Logger.logStdout.warning("AIDL Plugin is disabled. Exiting now.")
        return
    ''' rarely the case'''
    if not flag == "golden":
        Logger.logStdout.info("Unexpected create flag! \n")
        sys.exit(1)
    if Constants.qiifa_out_path_target_value != "qssi":
        reason = "QIIFA golden db generator not supported on vendor Side. Please use only on System Side"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA golden db generator",aidl_checker_main_create.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA golden db generator",aidl_checker_main_create.__name__,reason)

    initialize_aidl_plugin_globals()
    if arg_create_type != "aidl_lst_gen":
        if len(aidl_metadata_dict) == 0:
            reason="Error while parsing txt files"
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA golden db generator",aidl_checker_main_create.__name__,reason,False)
            else:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA golden db generator",aidl_checker_main_create.__name__,reason)
                sys.exit(1)
        for directory in Constants.paths_of_xml_files:
            if UtilFunctions.pathExists(directory):
                buildfcmpath(directory)
                if (arg_intf_name != None):
                    if arg_create_type in Constants.AIDL_SUPPORTED_CREATE_ARGS and arg_intf_name == None:
                        Logger.logStdout.error("Interface name option needs to be provided \n")
                        Logger.logStdout.info("python qiifa_main.py -h \n")
                        sys.exit(1)
                    read_xml_file_from_dict(FCM_GLOBAL_DICT,directory,arg_intf_name,FCM_MAX_VAL)
                    if len(FCM_XML_DICT) == 0 :
                        reason="Interface name is not found in the compatibility matrix. Please add it to the corresponding compatibility matrix"
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,arg_intf_name,json_create_for_create.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,arg_intf_name,json_create_for_create.__name__,reason)
                            sys.exit(1)
                else:
                    read_xml_file_from_dict(FCM_GLOBAL_DICT,directory)
        chk_dup_lst(aidl_metadata_dict,Constants.aidl_metadata_file_path)
        '''
            Generating the /QIIFA_cmd/aidl if the folder does not exist
        '''
        while not os.path.isdir(Constants.qiifa_aidl_db_root):
            genopdir(Constants.qiifa_aidl_db_root)
        #the case where user wants to generate the full cmd
        #not recommended :)

        if arg_intf_name == None and arg_create_type == Constants.AIDL_SUPPORTED_CREATE_ARGS[0]:
            Logger.logStdout.info("Running full aidl cmd generation.. \n")
            Logger.logStdout.warning("Not a recommended operation..\n")
            json_create_for_create(Constants.qiifa_aidl_db_root, FCM_XML_DICT, aidl_metadata_dict, "all")
        #at this point it must be a single intf cmd generation
        else:
            Logger.logStdout.info("Running aidl cmd generation with option: " + arg_create_type +", intf name: " + arg_intf_name+"\n")
            json_create_for_create(Constants.qiifa_aidl_db_root, FCM_XML_DICT, aidl_metadata_dict, arg_create_type, arg_intf_name)
        Logger.logInternal.info ("Success")
    elif arg_create_type == "aidl_lst_gen":
        for directory in Constants.paths_of_xml_files:
            if UtilFunctions.pathExists(directory):
                buildfcmpath(directory)
                newhidlaidllst(directory,FCM_MAX_VAL)
        Logger.logInternal.info ("Success")

def newhidlaidllst(xml_file_root,target_level = None):
    hidl_dict={}
    aidl_dict={}
    hidl_lst=[]
    aidl_lst=[]
    total_hal_lst=[]
    try:
        if target_level == None:
            target_level = FCM_MAX_VAL
        if "current" in FCM_GLOBAL_DICT.keys():
            target_level = target_level-1
        for root,dirs,files in os.walk(xml_file_root):
            for file in files:
                if file.startswith("compatibility_matrix."):
                    file_path = os.path.join(root,file)
                    fname_split = ((file.replace(".xml","")).split("."))[-1]
                    if (fname_split.isdigit() and int(fname_split)>=target_level) or (fname_split=='current' or fname_split=='device'):
                        xml_fd = open(file_path)
                        try:
                            xml_files_dict = xmltodict.parse(xml_fd.read())
                            if len(xml_files_dict)>0:
                                if type(xml_files_dict) is OrderedDict or type(xml_files_dict) is dict:
                                    if type(xml_files_dict[u'compatibility-matrix']) is OrderedDict or type(xml_files_dict[u'compatibility-matrix']) is dict:
                                        if type(xml_files_dict[u'compatibility-matrix'][u'hal']) is OrderedDict or type(xml_files_dict[u'compatibility-matrix'][u'hal']) is dict:
                                            if '@format' in xml_files_dict['compatibility-matrix']['hal'].keys():
                                                if xml_files_dict['compatibility-matrix']['hal'][u'name'] not in total_hal_lst:
                                                    total_hal_lst.append(xml_files_dict['compatibility-matrix']['hal'][u'name'])
                                                if xml_files_dict['compatibility-matrix']['hal'][u'@format'] == 'aidl' and xml_files_dict['compatibility-matrix']['hal'][u'name'] not in aidl_dict.keys():
                                                    if u'version' in xml_files_dict['compatibility-matrix']['hal'].keys():
                                                        aidl_dict[xml_files_dict['compatibility-matrix']['hal'][u'name']]={u'version':str(xml_files_dict['compatibility-matrix']['hal'][u'version'])}
                                                    else:
                                                        aidl_dict[xml_files_dict['compatibility-matrix']['hal'][u'name']]={u'version':"1"}
                                                elif xml_files_dict['compatibility-matrix']['hal'][u'@format'] == 'hidl' and xml_files_dict['compatibility-matrix']['hal'][u'name'] not in hidl_dict.keys():
                                                    hidl_dict[xml_files_dict['compatibility-matrix']['hal'][u'name']]={}
                                        elif type(xml_files_dict[u'compatibility-matrix'][u'hal']) is list:
                                            for hal_list in xml_files_dict[u'compatibility-matrix'][u'hal']:
                                                if hal_list[u'name'] not in total_hal_lst:
                                                    total_hal_lst.append(hal_list[u'name'])
                                                if hal_list[u'@format'] == 'aidl' and hal_list[u'name'] not in aidl_dict.keys():
                                                    if u'version' in hal_list.keys():
                                                        aidl_dict[hal_list[u'name']]={u'version':str(hal_list[u'version'])}
                                                    else:
                                                        aidl_dict[hal_list[u'name']] = {u'version':"1"}
                                                elif hal_list[u'@format'] == 'hidl' and hal_list[u'name'] not in hidl_dict.keys():
                                                    hidl_dict[hal_list[u'name']] = {}
                        except Exception as e:
                            traceback.print_exc()
                            reason = "Please verify the XML File, invalid syntax has been used."
                            Logger.logStdout.error(e)
                            UtilFunctions.print_violations_on_stdout("AIDL Plugin",fname_split,newhidlaidllst.__name__,reason)
                        xml_fd.close()
                else:
                    file_path = os.path.join(root,file)
                    xml_fd = open(file_path)
                    try:
                        xml_files_dict = xmltodict.parse(xml_fd.read())
                        if type(xml_files_dict) is OrderedDict or type(xml_files_dict) is dict:
                            if u'manifest' in xml_files_dict.keys():
                                if type(xml_files_dict['manifest']['hal']) is OrderedDict or type(xml_files_dict['manifest']['hal']) is dict:
                                    if u'@format' in xml_files_dict['manifest']['hal'].keys():
                                        if xml_files_dict['manifest']['hal'][u'name'] not in total_hal_lst:
                                            total_hal_lst.append(xml_files_dict['manifest']['hal'][u'name'])
                                        if xml_files_dict['manifest']['hal'][u'@format'] == 'aidl' and xml_files_dict['manifest']['hal'][u'name'] not in aidl_dict.keys():
                                            if u'version' in xml_files_dict['manifest']['hal'].keys():
                                                aidl_dict[xml_files_dict['manifest']['hal'][u'name']]={u'version':str(xml_files_dict['manifest']['hal'][u'version'])}
                                            else:
                                                aidl_dict[xml_files_dict['manifest']['hal'][u'name']]={u'version':"1"}
                                        elif xml_files_dict['manifest']['hal'][u'@format'] == 'hidl' and xml_files_dict['manifest']['hal'][u'name'] not in hidl_dict.keys():
                                            hidl_dict[xml_files_dict['manifest']['hal'][u'name']]={}
                                elif type(xml_files_dict['manifest']['hal']) is list:
                                    for hal_list in xml_files_dict[u'manifest'][u'hal']:
                                        if u'@format' in hal_list.keys():
                                            if hal_list[u'name'] not in total_hal_lst:
                                                total_hal_lst.append(hal_list[u'name'])
                                            if hal_list[u'@format'] == 'aidl' and hal_list[u'name'] not in aidl_dict.keys():
                                                if u'version' in hal_list.keys():
                                                    aidl_dict[hal_list[u'name']]={u'version':str(hal_list[u'version'])}
                                                else:
                                                    aidl_dict[hal_list[u'name']]={u'verion':"1"}
                                            elif hal_list[u'@format'] == 'hidl' and hal_list[u'name'] not in hidl_dict.keys():
                                                hidl_dict[hal_list[u'name']]={}
                    except Exception as e:
                        traceback.print_exc()
                        reason = "Please verify the XML File, invalid syntax has been used."
                        Logger.logStdout.error(e)
                        UtilFunctions.print_violations_on_stdout("AIDL Plugin",file,newhidlaidllst.__name__,reason)
                    xml_fd.close()
        if len(aidl_metadata_dict) > 0 and type(aidl_metadata_dict) is list:
            for meta_dict in aidl_metadata_dict:
                if str(meta_dict[u'name']) not in total_hal_lst and str(meta_dict[u'name']).startswith("vendor."):
                    total_hal_lst.append(meta_dict[u'name'])
                    version_info=""
                    if u'versions' in meta_dict.keys():
                        if len(meta_dict[u'versions']) == 1:
                            version_info = min(meta_dict[u'versions'])
                        else:
                            version_info = str(min(meta_dict[u'versions']))+"-"+str(max(meta_dict[u'versions']))
                    aidl_dict[meta_dict[u'name']]={u'version':version_info}

        file1 = open(Constants.qiifa_aidl_db_root + "qiifa_aidl_hidl_lst.csv", "w")
        intflstnew = ["HAL_NAME, HAL_OWNER, HIDL, AIDL, AIDL_VERSION \n"]
        for hal in total_hal_lst:
            new_str=str(hal)
            if "android" in str(hal).lower():
                new_str=new_str+",google"
            else:
                new_str=new_str+",qti"
            if hal in hidl_dict.keys():
                new_str=new_str+",x"
            else:
                new_str=new_str+","
            if hal in aidl_dict.keys():
                new_str=new_str+",x"
                if len(aidl_dict[hal])>0:
                    if u'version' in aidl_dict[hal].keys():
                        new_str=new_str+",\"=\"\""+str(aidl_dict[hal][u'version'])+"\"\"\""
                else:
                     new_str=new_str+","
            else:
                new_str=new_str+",,"
            new_str= new_str+ " \n"
            intflstnew.append(new_str)
        file1.writelines(intflstnew)
        file1.close()
    except Exception as e:
        traceback.print_exc()
        reason = "Please verify the XML File, invalid syntax has been used."
        Logger.logStdout.error(e)
        UtilFunctions.print_violations_on_stdout("AIDL Plugin",fname_split,newhidlaidllst.__name__,reason)

def json_create_for_create(json_db_file_root,fcm_xml_dict, metadatadict_lst, arg_type, for_intf_name=None):
    # root,dirs,files = next(os.walk(json_db_file_root))
    json_create_dict={}
    for key in fcm_xml_dict.keys():
        if "common" not in key:
            json_fname = "qiifa_aidl_cmd_"+str(FCM_GLOBAL_DICT[((str(os.path.basename(key)).replace(".xml","").split("."))[-1])])
            json_create_dict[json_fname] = concatenate_xml_aidlmd_dict(fcm_xml_dict[key],metadatadict_lst,str(FCM_GLOBAL_DICT[((str(os.path.basename(key)).replace(".xml","").split("."))[-1])]))
            json_db_file_path = os.path.join(json_db_file_root,json_fname+".json")
            dbdict_lst=[]
            if (UtilFunctions.pathExists(json_db_file_path) == True):
                dbdict_lst = load_info_from_JSON_file(json_db_file_path)
            if arg_type == 'all':
                if len(dbdict_lst) == 0:
                    Logger.logStdout.info("Running aidl create command with option all for the 1st time for Compatibility matrix : "+str(FCM_GLOBAL_DICT[((str(os.path.basename(key)).replace(".xml","").split("."))[-1])])+" \n")
                else:
                    Logger.logStdout.info("Running aidl create command with option all for Compatibility matrix : "+str(FCM_GLOBAL_DICT[((str(os.path.basename(key)).replace(".xml","").split("."))[-1])])+" \n")
                existintfcount=0
                modintfcount=0
                newintfcount=0
                goldencmdlst=[]
                modintflst=[]
                for meta_dict in json_create_dict[json_fname]:
                    if meta_dict in dbdict_lst:
                        existintfcount += 1
                    else:
                        newintfcount+=1
                        for dbdict in dbdict_lst:
                            if meta_dict[u'name'] == dbdict[u'name']:
                                modintflst.append(str(dbdict[u'name']))
                                modintfcount += 1
                                newintfcount -= 1
                        if meta_dict[u'name'] not in modintflst:
                            goldencmdlst.append(str(meta_dict[u'name']))
                json_create_dict[json_fname] = sorted(json_create_dict[json_fname], key=lambda d: d[u'name'])
                with open(json_db_file_path,'w') as jf:
                    json.dump(json_create_dict[json_fname], jf, separators=(",", ": "), indent=4,sort_keys=True)

                Logger.logStdout.info("Completed running all option for create aidl command. \n")
                Logger.logStdout.info(str(existintfcount)+" existing interfaces are being re-added \n")
                Logger.logStdout.info(str(modintfcount)+" interfaced where modified. Below is a list of Interfaces which have been modified \n")
                if modintfcount > 0:
                    Logger.logStdout.info(modintflst)
                Logger.logStdout.info(str(newintfcount)+" interfaced where added. Below is a list of Interfaces which have been added \n")
                if newintfcount > 0:
                    Logger.logStdout.info(goldencmdlst)
            else:
                if len(dbdict_lst) == 0:
                    reason="Please run python qiifa_main.py --create aidl before running this option "+ str(arg_type)
                    if plugin_state_warning:
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA golden db generator for "+str(arg_type),json_create_for_create.__name__,reason,False)
                    else:
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA golden db generator for "+str(arg_type),json_create_for_create.__name__,reason)
                        sys.exit(1)
                else:
                    Logger.logStdout.info("Running aidl create command with option "+ str(arg_type)+" and for interface "+str(for_intf_name)+"\n")
                chk_dup_lst(dbdict_lst,json_db_file_path)
                if arg_type == Constants.AIDL_SUPPORTED_CREATE_ARGS[1]:
                    intffound,founddict = check_intf_avail_metadict(json_create_dict[json_fname],for_intf_name)
                    if not intffound:
                        reason="Interface name is not found in the aidl file path present in "+os.path.basename(json_db_file_path)
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason)
                            sys.exit(1)
                    count=0
                    intffound = False
                    for dbdict in dbdict_lst:
                        if str(dbdict[u'name']) == for_intf_name and founddict != dbdict:
                            intffound=True
                            dbdict_lst[count]=founddict
                        count+=1
                    if not intffound:
                        reason="Interface name is not found or need not be updated in the aidl file path present in "+os.path.basename(json_db_file_path)
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason)
                            sys.exit(1)
                    dbdict_lst = sorted(dbdict_lst, key=lambda d: d[u'name'])
                    with open(json_db_file_path,'w') as jf:
                        json.dump(dbdict_lst, jf, separators=(",", ": "), indent=4,sort_keys=True)
                    Logger.logStdout.info("interface "+str(for_intf_name)+" has been modified  in "+json_fname+" \n")
                elif arg_type == Constants.AIDL_SUPPORTED_CREATE_ARGS[2]:
                    intffound,founddict = check_intf_avail_metadict(json_create_dict[json_fname],for_intf_name)
                    if not intffound:
                        reason="Interface name is not found in the aidl file path present in "+os.path.basename(json_db_file_path)
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason)
                            sys.exit(1)
                    intffound=False
                    for dbdict in dbdict_lst:
                        if str(dbdict[u'name']) == for_intf_name:
                            intffound=True
                            break
                    if intffound:
                        reason="Interface information is already present, Please use aidl_one or see go/qiifa for more information"
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,for_intf_name,json_create_for_create.__name__,reason)
                            sys.exit(1)
                    dbdict_lst.append(founddict)
                    dbdict_lst = sorted(dbdict_lst, key=lambda d: d[u'name'])
                    with open(json_db_file_path,'w') as jf:
                        json.dump(dbdict_lst, jf, separators=(",", ": "), indent=4,sort_keys=True)
                    Logger.logStdout.info("interface "+str(for_intf_name)+" has been added in "+json_fname)
    if len(XML_HAL_NOTFND)>0:
        for name in XML_HAL_NOTFND:
            Logger.logStdout.warning("Please check these XML hal manifest since there is compatbility matrix entry for this hal ("+name+") but there is no aidl_interface defined")
    if len(MD_HAL_NOTFND) >0 and arg_type == "all":
        for key in MD_HAL_NOTFND.keys():
            if key.startswith("vendor."):
                Logger.logStdout.warning("Please check these Metadata hal entries since there is  no compatbility matrix entry for this hal ("+key+") but there is an aidl_interface defined")
            else:
                Logger.logInternal.warning("Please check these Metadata hal entries since there is  no compatbility matrix entry for this hal ("+key+") but there is an aidl_interface defined")

def concatenate_xml_aidlmd_dict(xml_hal_lst,metadata_lst,vintfnumber):
    global XML_HAL_NOTFND,MD_HAL_NOTFND
    json_lst = []
    hal_added_lst = []
    try:
        if type(xml_hal_lst) is list:
            for xml_hal in xml_hal_lst:
                xml_hal_found = False
                for md_hal in metadata_lst:
                    if xml_hal[u'name'] == md_hal[u'name'] and xml_hal[u'name'] not in hal_added_lst:
                        if md_hal[u'name'] in MD_HAL_NOTFND.keys():
                            del MD_HAL_NOTFND[md_hal[u'name']]
                        hal_added_lst.append(xml_hal[u'name'])
                        xml_hal_found = True
                        json_hal = md_hal.copy()
                        min_hash_ind = 1
                        max_hash_ind = 1
                        #has development needs to be updated only for new Hals not for old hals
                        if u'has_development' in json_hal.keys():
                            if not Constants.platform_version.isdigit() and vintfnumber.isdigit() and int(vintfnumber) < FCM_MAX_VAL:
                                json_hal[u'has_development'] = False
                            elif not Constants.platform_version.isdigit() and vintfnumber =="device":
                                if json_hal[u'has_development'] == True:
                                    Logger.logStdout.warning("This interface "+str(xml_hal[u'name'])+" nneds to be frozen. There should not be any development once the interface is frozen")
                            elif Constants.platform_version.isdigit():
                                if json_hal[u'has_development'] == True:
                                    Logger.logStdout.warning("This interface "+str(xml_hal[u'name'])+" nneds to be frozen. There should not be any development once the interface is frozen")
                        if u'version' in xml_hal.keys():
                            xml_version = xml_hal[u'version']
                            if xml_version.isdigit():
                                min_hash_ind = int(xml_version)
                                max_hash_ind = int(xml_version)
                            else:
                                try:
                                    version_split = xml_version.split("-")
                                    min_hash_ind = int(version_split[0])
                                    max_hash_ind = int(version_split[1])
                                except Exception as e:
                                    traceback.print_exc()
                                    reason = "Please verify the XML File(compatibility.matrix."+vintfnumber+"), invalid Version syntax has been used."
                                    Logger.logStdout.error(e)
                                    if plugin_state_warning:
                                        UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_hal[u'name'],concatenate_xml_aidlmd_dict.__name__,reason,False)
                                    else:
                                        UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_hal[u'name'],concatenate_xml_aidlmd_dict.__name__,reason)

                        else:
                            if (vintfnumber.isdigit() and int(vintfnumber) >= FCM_MAX_VAL) or vintfnumber =="device":
                                Logger.logStdout.warning("The version needs to be updated for the following hal info in the compatibility matrix(compatibility.matrix."+vintfnumber+") for "+str(xml_hal[u'name'])+"")
                            else:
                                Logger.logInternal.warning("The version needs to be updated for the following hal info in the compatibility matrix(compatibility.matrix."+vintfnumber+") for "+str(xml_hal[u'name'])+" ")
                        if u'versions' not in json_hal.keys():
                            reason = "Version is not present in AIDL Metadata JSON. Please compile again or contact qiifa-squad for more information"
                            if plugin_state_warning:
                                UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_hal[u'name'],concatenate_xml_aidlmd_dict.__name__,reason,False)
                            else:
                                UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_hal[u'name'],concatenate_xml_aidlmd_dict.__name__,reason)

                        if sys.version_info < (3,):
                            json_hal[u'versions'] = range(min_hash_ind,max_hash_ind+1)
                        else:
                            json_hal[u'versions'] = list(range(min_hash_ind,max_hash_ind+1))
                        if u'hashes' in json_hal.keys() and type(json_hal[u'hashes']) is dict:
                            json_hal_hashes = json_hal[u'hashes'].copy()
                            for hash_key in json_hal[u'hashes'].keys():
                                if int(hash_key) < min_hash_ind or int(hash_key) > max_hash_ind:
                                    del json_hal_hashes[hash_key]
                            json_hal[u'hashes'] = json_hal_hashes
                        json_lst.append(json_hal)
            if not xml_hal_found:
                if xml_hal[u'name'] not in XML_HAL_NOTFND:
                    XML_HAL_NOTFND.append(xml_hal[u'name'])
        elif type(xml_hal_lst) is OrderedDict:
            xml_hal_found = False
            for md_hal in metadata_lst:
                if xml_hal_lst[u'name'] == md_hal[u'name'] and xml_hal_lst[u'name'] not in hal_added_lst:
                    if md_hal[u'name'] in MD_HAL_NOTFND.keys():
                        del MD_HAL_NOTFND[md_hal[u'name']]
                    hal_added_lst.append(xml_hal_lst[u'name'])
                    xml_hal_found = True
                    json_hal = md_hal.copy()
                    #has development needs to be updated only for new Hals not for old hals
                    if u'has_development' in json_hal.keys():
                        if not Constants.platform_version.isdigit() and vintfnumber.isdigit() and int(vintfnumber) < FCM_MAX_VAL:
                            json_hal[u'has_development'] = False
                        elif not Constants.platform_version.isdigit() and vintfnumber =="device":
                            if json_hal[u'has_development'] == True:
                                Logger.logStdout.warning("This interface "+str(xml_hal[u'name'])+" nneds to be frozen. There should not be any development once the interface is frozen")
                        elif Constants.platform_version.isdigit():
                            if json_hal[u'has_development'] == True:
                                Logger.logStdout.warning("This interface "+str(xml_hal[u'name'])+" nneds to be frozen. There should not be any development once the interface is frozen")
                    min_hash_ind = 1
                    max_hash_ind = 1
                    if u'version' in xml_hal_lst.keys():
                        xml_version = xml_hal_lst[u'version']
                        if xml_version.isdigit():
                            min_hash_ind = int(xml_version)
                            max_hash_ind = int(xml_version)
                        else:
                            try:
                                version_split = xml_version.split("-")
                                min_hash_ind = int(version_split[0])
                                max_hash_ind = int(version_split[1])
                            except Exception as e:
                                traceback.print_exc()
                                reason = "Please verify the XML File(compatibility.matrix."+vintfnumber+"), invalid Version syntax has been used."
                                Logger.logStdout.error(e)
                                if plugin_state_warning:
                                    UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_hal[u'name'],concatenate_xml_aidlmd_dict.__name__,reason,False)
                                else:
                                    UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_hal[u'name'],concatenate_xml_aidlmd_dict.__name__,reason)
                    else:
                        if (vintfnumber.isdigit() and int(vintfnumber) >= FCM_MAX_VAL) or vintfnumber =="device":
                            Logger.logStdout.warning("The version needs to be updated for the following hal info in the compatibility matrix(compatibility.matrix."+vintfnumber+") for "+str(xml_hal_lst[u'name'])+" ")
                        else:
                            Logger.logInternal.warning("The version needs to be updated for the following hal info in the compatibility matrix(compatibility.matrix."+vintfnumber+") for "+str(xml_hal_lst[u'name'])+"")
                    if sys.version_info < (3,):
                        json_hal[u'versions'] = range(min_hash_ind,max_hash_ind+1)
                    else:
                        json_hal[u'versions'] = list(range(min_hash_ind,max_hash_ind+1))
                    if type(json_hal[u'hashes']) is dict:
                        json_hal_hashes = json_hal[u'hashes'].copy()
                        for hash_key in json_hal[u'hashes'].keys():
                            if int(hash_key) < min_hash_ind or int(hash_key) > max_hash_ind:
                                del json_hal_hashes[hash_key]
                        json_hal[u'hashes'] = json_hal_hashes
            json_lst.append(json_hal)
            if not xml_hal_found:
                if xml_hal_lst[u'name'] not in XML_HAL_NOTFND:
                    XML_HAL_NOTFND.append(xml_hal_lst[u'name'])
    except Exception as e:
        traceback.print_exc()
        reason = "Please verify the XML File, invalid syntax has been used."
        Logger.logStdout.error(e)
        UtilFunctions.print_violations_on_stdout("AIDL Plugin","XML_FILE",concatenate_xml_aidlmd_dict.__name__,reason)
        sys.exit(1)
    return json_lst

def check_xml_dup_auth_cnfm(hal,duplicate_hal):
    if hal.keys() == duplicate_hal.keys():
        for key in hal.keys():
            if key != u'interface' and hal[key] != duplicate_hal[key]:
                reason = "Please verify the XML tag "+key+", For the Same XML interface "+hal['name']+" only the instance name can be a different all the other tags should be the same"
                UtilFunctions.print_violations_on_stdout("AIDL Plugin",hal['name'],check_xml_dup_auth_cnfm.__name__,reason)
                sys.exit(1)
    else:
        reason = "Please verify the XML File, For the Same XML interface "+hal['name']+" only the instance name can be a different all the other tags should be the same"
        UtilFunctions.print_violations_on_stdout("AIDL Plugin",hal['name'],check_xml_dup_auth_cnfm.__name__,reason)
        sys.exit(1)

def read_xml_file_from_dict(FCM_MASTER_DICT,FCM_ROOT_PATH,XML_INTF_INFO=None,TARGET_LEVEL=0):
    global FCM_XML_DICT
    FCM_XML_DICT["common"]=[]
    for root,dirs,files in os.walk(FCM_ROOT_PATH):
        for xml_file in files:
            xml_file_path = os.path.join(root, xml_file)
            split_xml_fn = xml_file.split(".xml")
            split_xml_fn = split_xml_fn[0]
            xml_hal_lst_dict = {}
            if xml_file.startswith("compatibility_matrix."):
                split_name = xml_file.split('.')
                if split_name[1] in FCM_MASTER_DICT:
                    fd = open(xml_file_path)
                    try:
                        xml_files_dict = xmltodict.parse(fd.read())
                        FCM_XML_DICT[xml_file_path] = []
                        if type(xml_files_dict) is OrderedDict or type(xml_files_dict) is dict:
                            if 'compatibility-matrix' in xml_files_dict.keys():
                                if type(xml_files_dict['compatibility-matrix']['hal']) is OrderedDict or type(xml_files_dict['compatibility-matrix']['hal']) is dict:
                                    if '@format' in xml_files_dict['compatibility-matrix']['hal'].keys() and xml_files_dict['compatibility-matrix']['hal']['@format'] == 'aidl' and XML_INTF_INFO==None:
                                        if xml_files_dict['compatibility-matrix']['hal'][u'name'] not in xml_hal_lst_dict.keys():
                                            FCM_XML_DICT[xml_file_path].append(xml_files_dict['compatibility-matrix']['hal'])
                                            xml_hal_lst_dict[xml_files_dict['compatibility-matrix']['hal'][u'name']] = xml_files_dict['compatibility-matrix']['hal']
                                        else:
                                            check_xml_dup_auth_cnfm(xml_files_dict['compatibility-matrix']['hal'],xml_hal_lst_dict[xml_files_dict['compatibility-matrix']['hal'][u'name']])
                                    elif '@format' in xml_files_dict['compatibility-matrix']['hal'].keys() and xml_files_dict['compatibility-matrix']['hal']['@format'] == 'aidl' and XML_INTF_INFO==xml_files_dict['compatibility-matrix']['hal'][u'name']:
                                        FCM_XML_DICT[xml_file_path].append(xml_files_dict['compatibility-matrix']['hal'])
                                elif type(xml_files_dict['compatibility-matrix']['hal']) is list:
                                    for hal_list in xml_files_dict['compatibility-matrix']['hal']:
                                        if '@format' in hal_list.keys() and hal_list['@format'] == 'aidl' and XML_INTF_INFO==None:
                                            if hal_list[u'name'] not in xml_hal_lst_dict.keys():
                                                FCM_XML_DICT[xml_file_path].append(hal_list)
                                                xml_hal_lst_dict[hal_list[u'name']] = hal_list
                                            else:
                                                check_xml_dup_auth_cnfm(hal_list,xml_hal_lst_dict[hal_list[u'name']])
                                        elif '@format' in hal_list.keys() and hal_list['@format'] == 'aidl' and hal_list['name']==XML_INTF_INFO:
                                            FCM_XML_DICT[xml_file_path] = hal_list
                        if split_name[1].isdigit() and TARGET_LEVEL > 0 and TARGET_LEVEL > int(split_name[1]):
                            del FCM_XML_DICT[xml_file_path]
                        elif  split_name[1] == "current" and FCM_GLOBAL_DICT[split_name[1]] < TARGET_LEVEL:
                            del FCM_XML_DICT[xml_file_path]
                        elif len(FCM_XML_DICT[xml_file_path]) == 0:
                            del FCM_XML_DICT[xml_file_path]
                    except Exception as e:
                        traceback.print_exc()
                        reason = "Please verify the XML File, invalid syntax has been used."
                        Logger.logStdout.error(e)
                        UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_file,read_xml_file_from_dict.__name__,reason)
                        sys.exit(1)
                    fd.close()
            else:
                fd = open(xml_file_path)
                try:
                    xml_files_dict = xmltodict.parse(fd.read())
                    if type(xml_files_dict) is OrderedDict or type(xml_files_dict) is dict:
                        if 'manifest' in xml_files_dict.keys():
                            if type(xml_files_dict['manifest']['hal']) is OrderedDict or type(xml_files_dict['manifest']['hal']) is dict:
                                if '@format' in xml_files_dict['manifest']['hal'].keys() and xml_files_dict['manifest']['hal']['@format'] == 'aidl' and XML_INTF_INFO==None:
                                    FCM_XML_DICT["common"].append(xml_files_dict['manifest']['hal'])
                                elif '@format' in xml_files_dict['manifest']['hal'].keys() and xml_files_dict['manifest']['hal']['@format'] == 'aidl' and XML_INTF_INFO==xml_files_dict['manifest']['hal']['name']:
                                    FCM_XML_DICT["common"].append(xml_files_dict['manifest']['hal'])
                            elif type(xml_files_dict['manifest']['hal']) is list:
                                for hal_list in xml_files_dict['manifest']['hal']:
                                    if '@format' in hal_list.keys() and hal_list['@format'] == 'aidl' and XML_INTF_INFO==None:
                                        FCM_XML_DICT["common"].append(hal_list)
                                    elif '@format' in hal_list.keys() and hal_list['@format'] == 'aidl' and XML_INTF_INFO==hal_list['name']:
                                        FCM_XML_DICT["common"].append(hal_list)
                except Exception as e:
                    traceback.print_exc()
                    reason = "Please verify the XML File, invalid syntax has been used."
                    Logger.logStdout.error(e)
                    UtilFunctions.print_violations_on_stdout("AIDL Plugin",xml_file,read_xml_file_from_dict.__name__,reason)
                    sys.exit(1)
                fd.close()
    if len(FCM_XML_DICT["common"]) == 0:
        del FCM_XML_DICT["common"]

def chk_dup_lst(dictlst,path):
    '''
    Find if the list of dictionaries have any duplicates
    and stop running if any
    '''
    if len(dictlst) >0:
        compdictlst=[]
        for dict in dictlst:
            if dict not in compdictlst:
                found_dupl=False
                if len(compdictlst) > 0:
                    for comdict in compdictlst:
                        if comdict[u'name'] == dict[u'name']:
                            found_dupl=True
                if not found_dupl:
                    compdictlst.append(dict)
            else:
                reason="Duplicate interface have been found in the file present in this path "+path+". Please remove duplicates before proceeding"
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,dict[u'name'],chk_dup_lst.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,dict[u'name'],chk_dup_lst.__name__,reason)
                    sys.exit(1)

def buildfcmpath(file_path):
    global FCM_GLOBAL_DICT,FCM_MAX_KEY,DEVICES_FCM_MAX_KEY
    root,dirs,files = next(os.walk(file_path))
    max_val=0
    current_exist = False
    for xml_file in files:
        if "current" in xml_file.lower():
            current_exist = True
        if xml_file.startswith("compatibility_matrix."):
            split_name = xml_file.split('.')
            if split_name[1].isdigit():
               if int(split_name[1]) > max_val:
                   FCM_MAX_KEY = split_name[1]
                   max_val = int(split_name[1])
               if int(split_name[1]) >0 and int(split_name[1]) >= Constants.AIDL_MIN_SUPPORT_VERSION:
                   FCM_GLOBAL_DICT[split_name[1]]= int(split_name[1])
    if (current_exist):
        updatefcmcurrentval(FCM_GLOBAL_DICT)
    else:
        del FCM_GLOBAL_DICT["current"]
    removelessthanfourandver(FCM_GLOBAL_DICT,FCM_MAX_KEY)
    findminmaxfcmdict(FCM_GLOBAL_DICT)

def updatefcmcurrentval(FCM_GLOBAL_DICT):
    global FCM_MAX_KEY
    for keys in FCM_GLOBAL_DICT.keys():
        if keys != "current" and str(FCM_GLOBAL_DICT[keys]).isdigit():
            FCM_GLOBAL_DICT["current"] = max(FCM_GLOBAL_DICT[keys],FCM_GLOBAL_DICT["current"])
    FCM_GLOBAL_DICT["current"] = FCM_GLOBAL_DICT["current"]+1
    FCM_MAX_KEY="current"

def removelessthanfourandver(FCM_GLOBAL_DICT,FCM_MAX_KEY):
    try:
        allowedminver = FCM_GLOBAL_DICT[FCM_MAX_KEY] - Constants.AIDL_NO_OF_PREV_VERS_SUPP
        if (allowedminver) > Constants.AIDL_MIN_SUPPORT_VERSION:
            for i in range(allowedminver):
                if str(i) in FCM_GLOBAL_DICT.keys():
                    del FCM_GLOBAL_DICT[str(i)]
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "The QIIFA CMD or the compatibility matrix is not present. Please check or contact qiifa-squad Please check go/qiifa for more information"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aild metadata",removelessthanfourandver.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",removelessthanfourandver.__name__,reason)
def findminmaxfcmdict(fcm_dict):
    global FCM_MAX_VAL,FCM_MIN_VAL
    try:
        for key in fcm_dict:
            if not key.startswith("device"):
                FCM_MAX_VAL = max(fcm_dict[key],FCM_MAX_VAL)
                FCM_MIN_VAL = min(fcm_dict[key],FCM_MIN_VAL)
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "The QIIFA CMD or the compatibility matrix is not present. Please check or contact qiifa-squad Please check go/qiifa for more information"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aild metadata",removelessthanfourandver.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",removelessthanfourandver.__name__,reason)
def check_intf_avail_metadict(metadatadict_lst, intf_name):
    intffound=False
    founddict={}
    for metadata_dict in metadatadict_lst:
        if str(metadata_dict[u'name']) == intf_name:
            intffound=True
            founddict = metadata_dict
    return intffound,founddict

def genopdir(OPDirPath):
    '''
    Generate the path by creating directories till the end of the path is reached.
    '''
    if not (os.path.isdir(OPDirPath)):
        os.mkdir(OPDirPath)

def enforce_package_naming_convention(aidl_interface_list):
    '''
    Requirement is to enforce pacakage naming convention
    for non-AOSP HAL's.
    '''
    voilation_list = []
    try:
        for aidl_interface in aidl_interface_list:
            aidl_interface_name = aidl_interface["name"]
            aidl_interface_type = aidl_interface["types"][0]
            aidl_voilation = True
            for package in Constants.qiifa_package_convention_list:
                if aidl_interface_name.startswith(package) and aidl_interface_type.startswith(package):
                    aidl_voilation = False
                    break
            if aidl_voilation:
                voilation_list.append(aidl_interface)
        if  voilation_list:
            for interface in voilation_list:
                reason = "Interface should follow package naming convention. Prefixed with :" + str(Constants.qiifa_package_convention_list)
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,interface["name"],enforce_package_naming_convention.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,interface["name"],enforce_package_naming_convention.__name__,reason)
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "Few metadata tags are missing or not generated. Please recompile the build or please run --create to generate the tags. Please check go/qiifa for more information"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",enforce_package_naming_convention.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",enforce_package_naming_convention.__name__,reason)
def project_path_exists_in_known_list(project_list,project):
    project_found_in_list = False
    for prj in project_list:
        if(project.startswith(prj)):
            project_found_in_list = True
    return project_found_in_list

def idenitify_vendor_defined_aidl_interfaces():
    '''
    Write a logic to identify non-AOSP HAL's.
    Basis of this logic will be :

        1. Package/folder naming convention used by AOSP
        2. Generic folder structure used by AOSP
        3. One time Known interfaces list
    '''
    filtered_aidl_list = []
    ## Iterate through aidl_metadata json file
    ## Filter #1
    try:
        for aidl_metadata in aidl_metadata_dict:
            if u'types' in aidl_metadata.keys() and len(aidl_metadata["types"]) > 0:
                aidl_types = aidl_metadata["types"][0]
                if not aidl_types.startswith("vendor.qti"):
                    ## Let;s do a reverse check for vendor folder path before we skip these
                    ## We don;t expect these interfaces to be present in vendor paths.
                    violation_folder_path_list = ["vendor"]
                    project_path = module_info_dict[aidl_metadata["name"]]["path"][0]
                    folder_violation = project_path_exists_in_known_list(violation_folder_path_list,project_path)
                    if not folder_violation:
                        continue
                    else:
                        #TODO Need to discuss Action if such instance is found
                        pass
                else:
                    filtered_aidl_list.append(aidl_metadata)
            else:
                reason = "Types parameter is missing or empty. Please recompile the build or please run --create to generate the tags. Please check go/qiifa for more information"
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",idenitify_vendor_defined_aidl_interfaces.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",idenitify_vendor_defined_aidl_interfaces.__name__,reason)
        temp_filter_list=[]
        ## Filter #2
        for aidl_metadata in filtered_aidl_list:
            if aidl_metadata["name"] in aidl_skipped_lst[u'ALL'] or aidl_metadata["name"] in aidl_skipped_lst[u'VNDRAVLBLE_CHECK']:
                Logger.logInternal.info("AIDL interface "+aidl_metadata["name"]+" is part of the Skipped list")
                continue
            project_path = module_info_dict[aidl_metadata["name"]]["path"][0]
            found_in_aosp_list = project_path_exists_in_known_list(Constants.qiifa_aidl_known_aosp_project_list, project_path)
            if not found_in_aosp_list:
                temp_filter_list.append(aidl_metadata)

        ## Return list of Interfaces which are considered as non-AOSP
        return temp_filter_list
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "Few metadata tags are missing or not generated. Please recompile the build or please run --create to generate the tags. Please check go/qiifa for more information"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",idenitify_vendor_defined_aidl_interfaces.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",idenitify_vendor_defined_aidl_interfaces.__name__,reason)

def enforce_vintf_tag(aidl_interfaces):
    ## If vendor_available is set to true then make sure that stability tag is set to vintf
    voilation_list = []
    intf_name = ""
    try:
        for aidl_interface in aidl_interfaces:
            if u'vendor_available' in aidl_interface.keys():
                if aidl_interface["vendor_available"] == "true":
                    intf_name = aidl_interface["name"]
                    if u'stability' in aidl_interface.keys():
                        stability_tag = aidl_interface["stability"]
                        if stability_tag == "vintf":
                            continue
                        else:
                            voilation_list.append(aidl_interface)
                    else:
                        voilation_list.append(aidl_interface)

            else:
                reason = "Vendor Available mandatory for all QCOM interface. Please set vendor_available to true in the interface.\n \t\t\t\t\t\t Please contact qiifa-squad for more information"
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,aidl_interface["name"],enforce_vintf_tag.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,aidl_interface["name"],enforce_vintf_tag.__name__,reason)

        if  voilation_list:
            for interface in voilation_list:
                reason = "Stability : Vintf is mandatory for all vendor_available : true Aidl Interfaces"
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,interface["name"],enforce_vintf_tag.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,interface["name"],enforce_vintf_tag.__name__,reason)
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "Stability tag is not present for the AIDL interface "+intf_name+", Please run --create to generate the tags or please check go/qiifa"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",enforce_vintf_tag.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",enforce_vintf_tag.__name__,reason)

def enforce_repository_path(aidl_interfaces):
    ## Aim is to restrict all aidl_interfaces to couple of projects so that
    ## expert reviewers can ensure proper backward compatibility for these
    ## interfaces.
    voilation_list = [];
    try:
        for aidl_interface in aidl_interfaces:
            project_path = module_info_dict[aidl_interface["name"]]["path"][0]
            project_violation = True
            for allowed_path in Constants.qiifa_aidl_allowed_path:
                if(project_path.startswith(allowed_path)):
                    project_violation = False
                    break
            if project_violation:
                voilation_list.append(aidl_interface)
        if  voilation_list:
            for interface in voilation_list:
                reason = "All AIDL Interfaces should be present in folder paths :" + str(Constants.qiifa_aidl_allowed_path)
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,interface["name"],enforce_repository_path.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,interface["name"],enforce_repository_path.__name__,reason)
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "Few metadata tags are missing or not generated. Please recompile the build or please run --create to generate the tags. Please check go/qiifa for more information"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",enforce_repository_path.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_metadata",enforce_repository_path.__name__,reason)

def upd_hash_ver_metadatajson(metadata_dict,module_dict):
    '''
    Correlating the version and hash information from
    AIDL metadata JSON file and manually verifing if the frozen hashes
    are present in the corresponding AIDL interface(aidl_api) folders using the
    path from the modules info json file. The information is then updated as a
    dictionary in the hashes and the development hashes node in the
    aidl meta data dict which is compiled and saved for QIIFA CMD
    '''
    global MD_HAL_NOTFND
    intf_path = ""
    intf_name = ""
    try:
        for hal in metadata_dict:
            MD_HAL_NOTFND[hal[u'name']] = hal[u'name']
            hash_dict={}
            hash_max_val=0
            intf_name = hal["name"]
            if u'hashes' in hal.keys():
                if len(hal[u'hashes']) > 0 and type(hal[u'hashes']) == list:
                    intf_path = module_dict[hal["name"]]["path"][0]
                    intf_path = os.path.join(Constants.croot,intf_path)
                    for root,dirs,files in os.walk(intf_path):
                        for file in files:
                            if file == ".hash":
                                split_root = root.split("/")
                                hash_fd = open(os.path.join(root,file),'r')
                                hashline = hash_fd.read()
                                hashline = hashline.replace("\n","")
                                if  hashline in hal[u'hashes']:
                                    if sys.version_info < (3,):
                                        hash_dict[split_root[-1]] = unicode(hashline)
                                    else:
                                        hash_dict[split_root[-1]] = hashline
                                    hash_max_val = max(int(split_root[-1]),hash_max_val)
                                    break
                    hal[u'hashes'] = hash_dict
            else:
                reason = "Hashes is not present in AIDL Metadata JSON. Please compile again or contact qiifa-squad for more information"
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,hal["name"],upd_hash_ver_metadatajson.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,hal["name"],upd_hash_ver_metadatajson.__name__,reason)
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "Hashes is not present in AIDL Metadata JSON. Please compile again or contact qiifa-squad for more information"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,intf_name,upd_hash_ver_metadatajson.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,intf_name,upd_hash_ver_metadatajson.__name__,reason)
        sys.exit(0)
def initialize_aidl_plugin_globals():
    '''
    This function is meant to initialize global metadata
    which will be used in enforcement.
    Primarily planning to load information from :
	1. aidl_metadata.json (Contains the list of HAL;s)
    2. module_info.json (Module <-> Project mapping)
    3. AIDL Skipped List Dictionary: Getting a list of modules
       which will be skipped by AIDL
    4. Correlate the corresponding hash value and version and
       update the aidl metadata dictionary
    '''
    global aidl_metadata_dict
    global module_info_dict
    global aidl_skipped_lst
    global qiifa_aidl_chipset_dvlpmnt_enbl
    aidl_metadata_dict = load_info_from_JSON_file(Constants.aidl_metadata_file_path)
    module_info_dict   = load_info_from_JSON_file(Constants.module_info_file_path)
    aidl_skipped_lst   = load_info_from_JSON_file(Constants.aidl_skipped_intf)
    if Constants.qiifa_out_path_target_value != "qssi" and UtilFunctions.pathExists(Constants.qiifa_aidl_dvlpmnt_config_notif) == True:
        qiifa_aidl_chipset_dvlpmnt_enbl = load_info_from_JSON_file(Constants.qiifa_aidl_dvlpmnt_config_notif)
        qiifa_aidl_chipset_dvlpmnt_enbl = qiifa_aidl_chipset_dvlpmnt_enbl[u'CHIPSET_NAME']
    upd_hash_ver_metadatajson(aidl_metadata_dict,module_info_dict)

def plugin_disabled():
    if (Constants.aidl_plugin_state == "disabled"):
        return True

def check_plugin_state():
     global plugin_state_warning
     supported_values = ["disabled","enforced","warning"]
     supported = False
     if(Constants.aidl_plugin_state=="warning"):
         plugin_state_warning = True
     for value in supported_values:
        if(Constants.aidl_plugin_state == value):
             supported = True
             break
     return supported

def func_start_qiifa_aidl_checker(self,
                                  flag,
                                  qssi_path=None,
                                  target_path=None,
                                  q_file_name=None,
                                  t_file_name=None,
                                  arg_test_source=None,
                                  arg_tp_build=None,):
    '''
    Enforcements for AIDL:
       1. All package name should start from vendor.qti prefix
       2. If AIDL is defined as vendor_available then stability tag
          should be set to vintf.
    '''
    ### We intend to check AIDL interfaces which are part of commonsys-intf
    ### for qssi and vendor compatibility. All commonsys-intf projects are
    ### part of qssi SI, therefore add a check to run aidl checker while
    ### building QSSI lunch.
    if qssi_path != None and target_path != None:
        if not check_plugin_state():
            Logger.logStdout.warning("AIDL Plugin state doen't match supported values. Exiting now.")
            return
        if plugin_disabled():
            Logger.logStdout.warning("AIDL Plugin is disabled. Exiting now.")
            return
        aidl_iic_checker(flag,qssi_path,target_path,q_file_name,t_file_name)
    else:
        if not check_plugin_state():
            Logger.logStdout.warning("AIDL Plugin state doen't match supported values. Exiting now.")
            return
        if plugin_disabled():
            Logger.logStdout.error("AIDL Plugin is disabled. Exiting now.")
            return

        if Constants.qiifa_out_path_target_value == "qssi":
            initialize_aidl_plugin_globals()
            vendor_defined_interfaces = idenitify_vendor_defined_aidl_interfaces()
            enforce_package_naming_convention(vendor_defined_interfaces)
            enforce_vintf_tag(vendor_defined_interfaces)
            enforce_repository_path(vendor_defined_interfaces)
            aidl_iic_checker(flag,qssi_path,target_path,q_file_name,t_file_name)
        else:
            initialize_aidl_plugin_globals()
            aidl_iic_checker(flag,qssi_path,target_path,q_file_name,t_file_name)
        if not Variables.is_violator:
            if not Constants.platform_version.isdigit() and Constants.qiifa_out_path_target_value == "qssi":
                for directory in Constants.paths_of_xml_files:
                    if UtilFunctions.pathExists(directory):
                        newhidlaidllst(directory,FCM_MAX_VAL)
            copy_json_to_out()

def readgoldendictfromfile(dbcmd_root_path,TARGET_LEVEL=0):
    db_dict={}
    for root,dirs,files in os.walk(Constants.qiifa_aidl_db_root):
        for file in files:
            if file.startswith("qiifa_aidl_cmd_"):
                jsonfname = ((file.replace(".json","")).split("_"))[-1]
                qiifa_aidl_db_path = os.path.join(root,file)
                if TARGET_LEVEL == 0:
                    goldencmddict_lst = load_info_from_JSON_file(qiifa_aidl_db_path)
                    if len(goldencmddict_lst)<=0:
                        reason="JSON File has not been generated yet. Please run --create option before running IIC checker"
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",json_create_for_create.__name__,reason,False)
                            return
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",json_create_for_create.__name__,reason)
                            sys.exit(1)
                    chk_dup_lst(goldencmddict_lst,qiifa_aidl_db_path)
                    db_dict[qiifa_aidl_db_path] = goldencmddict_lst
                elif jsonfname == 'device' or (TARGET_LEVEL > 0 and jsonfname.isdigit() and int(jsonfname)==TARGET_LEVEL ):
                    if jsonfname.isdigit(): # make it point to the Android or the Platform version
                        Logger.logStdout.info("Reading the following file : qiifa_aidl_cmd_"+str(TARGET_LEVEL))
                    elif jsonfname == 'device':
                        Logger.logStdout.info("Reading the following file : qiifa_aidl_cmd_device")
                    goldencmddict_lst = load_info_from_JSON_file(qiifa_aidl_db_path)
                    if len(goldencmddict_lst)<=0:
                        reason="JSON File has not been generated yet. Please run --create option before running IIC checker"
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",json_create_for_create.__name__,reason,False)
                            return
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",json_create_for_create.__name__,reason)
                            sys.exit(1)
                    chk_dup_lst(goldencmddict_lst,qiifa_aidl_db_path)
                    db_dict[qiifa_aidl_db_path] = goldencmddict_lst
    return db_dict

def Cmp_Goldencmd_xmldict(goldencmd_dict, xmlconcat_dict,enforcement=True,chkcurrnt = ''):
    for xml_key in xmlconcat_dict.keys():
        for gcd_key in goldencmd_dict.keys():
            if xml_key in os.path.basename(gcd_key):
                Logger.logStdout.info("Running Comparison of Compatibility matrix and QIIFA AIDL CMD for :"+os.path.basename(gcd_key))
                sort_check = chk_cmd_dict_sorted(goldencmd_dict[gcd_key],os.path.basename(gcd_key))
                if sort_check:
                    return
                if xmlconcat_dict[xml_key] != goldencmd_dict[gcd_key]:
                    Logger.logStdout.info("Found a difference...")
                    blk_by_blk_dict_chk(xml_key,os.path.basename(gcd_key), xmlconcat_dict[xml_key],goldencmd_dict[gcd_key],enforcement)
            elif xml_key == "common" and int(FCM_VENDOR_TL) > 0:
                if str(FCM_VENDOR_TL) in os.path.basename(gcd_key) and chkcurrnt == '':
                    blk_by_blk_dict_chk(xml_key,os.path.basename(gcd_key), xmlconcat_dict[xml_key],goldencmd_dict[gcd_key],enforcement)
                elif "device" in os.path.basename(gcd_key) and chkcurrnt == '':
                    blk_by_blk_dict_chk(xml_key,os.path.basename(gcd_key), xmlconcat_dict[xml_key],goldencmd_dict[gcd_key],enforcement)
                elif chkcurrnt == 'current' and str(FCM_VENDOR_TL+1) in os.path.basename(gcd_key):
                    blk_by_blk_dict_chk(xml_key,os.path.basename(gcd_key), xmlconcat_dict[xml_key],goldencmd_dict[gcd_key],enforcement)


def blk_by_blk_dict_chk(filename,gcd_key, dict_list_1,dict_list_2,enforcement):
    global plugin_state_warning, VNDR_HAL_NOTFND,qiifa_aidl_chipset_dvlpmnt_enbl
    hal_name_dict1_no_dict2 = {}
    hal_name_dict2_no_dict1 = {}
    prefix_id = gcd_key.replace(".json","")
    prefix_id = prefix_id.split("_")
    prefix_id = prefix_id[-1]
    if Constants.target_board_platform == "anorak" and prefix_id.isdigit():
        Logger.logStdout.info("Skipping AOSP interfaces on target : "+str(Constants.target_board_platform))
        return
    if not enforcement:
        plugin_state_warning = True
    elif (Constants.aidl_plugin_state=="warning"):
        plugin_state_warning = True
    elif not Constants.platform_version.isdigit() and prefix_id.isdigit():
        plugin_state_warning = True
    elif Constants.platform_version.isdigit() and prefix_id.isdigit() and Constants.target_board_platform in qiifa_aidl_chipset_dvlpmnt_enbl:
        Logger.logStdout.info("Development Chipset found "+Constants.target_board_platform+". If this is incorrect, please contact QIIFA-squad")
        plugin_state_warning = True
    elif enforcement:
        plugin_state_warning = False
    for dict_hal_1 in dict_list_1:
        if dict_hal_1[u'name'] not in hal_name_dict1_no_dict2.keys():
            hal_name_dict1_no_dict2[dict_hal_1[u'name']] = dict_hal_1[u'name']
        if Constants.qiifa_out_path_target_value != "qssi" and prefix_id.isdigit() and dict_hal_1[u'name'].startswith("vendor."):
            del hal_name_dict1_no_dict2[dict_hal_1[u'name']]
        elif Constants.qiifa_out_path_target_value != "qssi" and not prefix_id.isdigit() and dict_hal_1[u'name'].startswith("android."):
            del hal_name_dict1_no_dict2[dict_hal_1[u'name']]
    for dict_hal_2 in dict_list_2:
        if dict_hal_2[u'name'] not in hal_name_dict2_no_dict1.keys():
            hal_name_dict2_no_dict1[dict_hal_2[u'name']] = dict_hal_2[u'name']
        if Constants.qiifa_out_path_target_value != "qssi" and prefix_id.isdigit() and dict_hal_2[u'name'].startswith("android."):
            del hal_name_dict2_no_dict1[dict_hal_2[u'name']]
        elif Constants.qiifa_out_path_target_value != "qssi" and not prefix_id.isdigit() and dict_hal_2[u'name'].startswith("vendor."):
            del hal_name_dict2_no_dict1[dict_hal_2[u'name']]
    for dict_hal_1 in dict_list_1:
        for dict_hal_2 in dict_list_2:
            if dict_hal_1[u'name'] == dict_hal_2[u'name']:
                if dict_hal_1[u'name'] in hal_name_dict1_no_dict2.keys():
                    del hal_name_dict1_no_dict2[dict_hal_1[u'name']]
                if dict_hal_2[u'name'] in hal_name_dict2_no_dict1.keys():
                    del hal_name_dict2_no_dict1[dict_hal_2[u'name']]
                if dict_hal_1[u'name'] in aidl_skipped_lst[u'ALL']:
                    Logger.logStdout.warning("AIDL Skipped INTF from Skipped List " + dict_hal_1[u'name'])
                else:
                    if Constants.qiifa_out_path_target_value == "qssi":
                        blk_by_blk_chk(dict_hal_1,dict_hal_2)
                    else:
                        #Vendor Implementation is going to be passed here
                        blk_by_blk_vndr_chk(dict_hal_1,dict_hal_2)
    if len(hal_name_dict1_no_dict2) > 0:
        for name in hal_name_dict1_no_dict2:
            if name in aidl_skipped_lst[u'ALL'] or name in Constants.qiifa_aidl_fw_intfs:
                Logger.logStdout.warning("AIDL Skipped INTF from Skipped List " + name)
            else:
                reason="Please check the AIDL Interface, QIIFA CMD does not have this information. Please run --create option before running IIC."
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,name,blk_by_blk_dict_chk.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,name,blk_by_blk_dict_chk.__name__,reason)
                return

def aidl_iic_checker(flag,
                    qssi_path=None,
                    target_path=None,
                    q_file_name=None,
                    t_file_name=None):
    if flag == "check":
        if qssi_path != None and target_path != None:
            Logger.logStdout.info("Running AIDL image to image compatibility check.....")
            run_img_to_img_checker(qssi_path, target_path, q_file_name, t_file_name)
        else:
            if (UtilFunctions.pathExists(Constants.aidl_metadata_file_path) == False):
                reason="JSON file is not found at"+Constants.aidl_metadata_file_path
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",aidl_iic_checker.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",aidl_iic_checker.__name__,reason)
                    sys.exit(1)
            json_iic_dict={}
            if Constants.qiifa_out_path_target_value == "qssi":
                Logger.logStdout.info("Running QSSI only checker.....")

            else:
                Logger.logStdout.info("Running Target only checker.....")
            for directory in Constants.paths_of_xml_files:
                if UtilFunctions.pathExists(directory):
                    if Constants.qiifa_out_path_target_value == "qssi":
                        buildfcmpath(directory)
                    else:
                        buildvndrpath(directory)
                    read_xml_file_from_dict(FCM_GLOBAL_DICT,directory)
                    if len(FCM_XML_DICT) == 0:
                        reason="No compatiblity Matrix found. This is not right. This case should not be present. Please check"
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",aidl_iic_checker.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"aidl_iic_checker",aidl_iic_checker.__name__,reason)
                            sys.exit(1)
                    for key in FCM_XML_DICT.keys():
                        if "common" not in key:
                            json_fname = "qiifa_aidl_cmd_"+str(FCM_GLOBAL_DICT[((str(os.path.basename(key)).replace(".xml","").split("."))[-1])])
                            json_iic_dict[json_fname] = concatenate_xml_aidlmd_dict(FCM_XML_DICT[key],aidl_metadata_dict,str(FCM_GLOBAL_DICT[((str(os.path.basename(key)).replace(".xml","").split("."))[-1])]))
                        elif "common" in key:
                            json_fname = "common"
                            json_iic_dict[json_fname] = concatenate_xml_aidlmd_dict(FCM_XML_DICT[key],aidl_metadata_dict,"common")
                    if len(XML_HAL_NOTFND)>0:
                        for name in XML_HAL_NOTFND:
                            Logger.logStdout.warning("Please check these XML hal manifest since there is compatbility matrix entry for this hal ("+name+") but there is no aidl_interface defined")
                    if len(MD_HAL_NOTFND) >0:
                        for key in MD_HAL_NOTFND.keys():
                            if key not in aidl_skipped_lst[u'SUBINTF']:
                                if key.startswith("vendor."):
                                    Logger.logStdout.warning("Please check these Metadata hal entries since there is  no compatbility matrix entry for this hal ("+key+") but there is an aidl_interface defined")
                                else:
                                    Logger.logInternal.warning("Please check these Metadata hal entries since there is  no compatbility matrix entry for this hal ("+key+") but there is an aidl_interface defined")
                    #Add FCM_VENDOR_TL for Vendor Route
                    goldencmd_dict = readgoldendictfromfile(Constants.qiifa_aidl_db_root,FCM_VENDOR_TL)
                    Cmp_Goldencmd_xmldict(goldencmd_dict,json_iic_dict)
                    if Constants.qiifa_out_path_target_value != "qssi" and FCM_VENDOR_TL < FCM_MAX_VAL:
                        goldencmd_dict = readgoldendictfromfile(Constants.qiifa_aidl_db_root,FCM_VENDOR_TL+1)
                        Cmp_Goldencmd_xmldict(goldencmd_dict,json_iic_dict,False,'current')

        if len(VNDR_HAL_NOTFND) > 0:
            for key in VNDR_HAL_NOTFND.keys():
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,key,aidl_iic_checker.__name__,VNDR_HAL_NOTFND[key][1],False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,key,aidl_iic_checker.__name__,VNDR_HAL_NOTFND[key][1])

    else:
        Logger.logStdout.error("Unexpected aidl checker flag!")
        sys.exit(1)

def buildvndrpath(vendorxml_path):
    global FCM_GLOBAL_DICT,FCM_MAX_KEY,FCM_VENDOR_TL
    filename = ""
    try:
        for root,dirs,files in os.walk(vendorxml_path):
            for file in files:
                if file.endswith(".xml"):
                    file_path = os.path.join(root,file)
                    xml_fd = open(file_path)
                    filename = file
                    try:
                        xml_files_dict = xmltodict.parse(xml_fd.read())
                        if len(xml_files_dict)>0:
                            if type(xml_files_dict) is OrderedDict or type(xml_files_dict) is dict:
                                for key in xml_files_dict.keys():
                                    if type(xml_files_dict[key]) is OrderedDict or type(xml_files_dict[key]) is dict:
                                        for key_xml in xml_files_dict[key].keys():
                                            if key_xml == u'@target-level':
                                                FCM_GLOBAL_DICT[str(xml_files_dict[key][u'@target-level'])]=int(xml_files_dict[key][u'@target-level'])
                                                FCM_MAX_KEY = str(xml_files_dict[key][u'@target-level'])
                                                break
                    except Exception as e:
                        traceback.print_exc()
                        reason = "Something is wrong with the xml file "+file_path
                        Logger.logStdout.error(e)
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,buildvndrpath.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,buildvndrpath.__name__,reason)
                            sys.exit(1)
        current_exist = False
        for root,dirs,files in os.walk(Constants.qiifa_aidl_db_root):
            for file in files:
                filename = file
                if file.startswith("qiifa_aidl_cmd_"):
                    fname_replace = (file.replace(".json","")).replace("qiifa_aidl_cmd_","qiifa_aidl_cmd.")
                    fnam_split = fname_replace.split(".")
                    if fnam_split[-1].isdigit() and FCM_MAX_KEY.isdigit() and int(fnam_split[-1]) >= int(FCM_MAX_KEY):
                        if int(fnam_split[-1]) > int(FCM_MAX_KEY) and not Constants.platform_version.isdigit():
                            current_exist = True
                        elif int(fnam_split[-1]) > int(FCM_MAX_KEY) and Constants.platform_version.isdigit() and Constants.vintf_mapping_pf_tl[int(FCM_MAX_KEY)] != Constants.platform_version.isdigit() and int(Constants.vintf_mapping_pf_tl[int(fnam_split[-1])]) <= int(Constants.platform_version):
                            current_exist = True
                        if int(fnam_split[-1]) == int(FCM_MAX_KEY):
                            FCM_VENDOR_TL = int(fnam_split[-1])
        if (current_exist):
            updatefcmcurrentval(FCM_GLOBAL_DICT)
        else:
            del FCM_GLOBAL_DICT["current"]
        removelessthanfourandver(FCM_GLOBAL_DICT,FCM_MAX_KEY)
        findminmaxfcmdict(FCM_GLOBAL_DICT)
    except Exception as e:
        reason = "Something is wrong with the xml file "+file_path
        traceback.print_exc()
        Logger.logStdout.error(e)
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,buildvndrpath.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,buildvndrpath.__name__,reason)
            sys.exit(1)

def run_img_to_img_checker(qssi_path,
                           target_path,
                           q_file_name,
                           t_file_name):
    '''
    Description: This defination runs image to image checker
    Type       : Internal defination
    '''
    global aidl_skipped_lst
    qssi_json_cmd_path =''
    target_json_cmd_path = ''
    qssi_json_cmd_path=(os.path.splitext(os.path.join(qssi_path,q_file_name))[0])
    target_json_cmd_path=(os.path.splitext(os.path.join(target_path,t_file_name))[0])
    qssi_lst=[]
    target_lst=[]

    if UtilFunctions.dirExists(qssi_json_cmd_path) and UtilFunctions.dirExists(target_json_cmd_path):
        try:
            qssi_mn_dict = img_load_lst_from_path(qssi_json_cmd_path)
            target_mn_dict = img_load_lst_from_path(target_json_cmd_path)
            qssi_lst = []
            target_lst=[]
            if 'qiifa_aidl_skipped_interfaces.json' in qssi_mn_dict.keys():
                aidl_skipped_lst = qssi_mn_dict['qiifa_aidl_skipped_interfaces.json']
            if len(aidl_skipped_lst) <= 0:
                reason = "Please check the AIDL Skip list. The file is not being added to the Artifact"
                if plugin_state_warning:
                    Logger.logStdout.error(reason)
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA AIDL Super IMG chk",run_img_to_img_checker.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA AIDL Super IMG chk",run_img_to_img_checker.__name__,reason)
                    return
            for target_key in target_mn_dict.keys():
                if target_key in qssi_mn_dict.keys():
                    Logger.logStdout.info("Reading from the following image: " +str(target_key) )
                    qssi_lst = qssi_mn_dict[target_key]
                    target_lst = target_mn_dict[target_key]
                    if len(qssi_lst)<= 0 or len(target_lst)<=0:
                        reason="Either target-CMD or QSSI-CMD is empty"
                        if plugin_state_warning:
                            Logger.logStdout.error(reason)
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA CMD Loading for QSSI / Target",run_img_to_img_checker.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA CMD Loading for QSSI / Target",run_img_to_img_checker.__name__,reason)
                            return
                    elif len(qssi_lst) < len(target_lst):
                        target_not_found_lst=intf_not_found_lst(qssi_lst,target_lst)
                        qssi_not_found_lst=intf_not_found_lst(target_lst,qssi_lst)
                        Logger.logStdout.error("These number of QSSI QIIFA-CMD interfaces ("+str(len(target_not_found_lst))+") are not present in the Target QIIFA-CMD. The list is below")
                        if len(target_not_found_lst)>0:
                            Logger.logStdout.error(target_not_found_lst)
                        Logger.logStdout.error("These number of Target QIIFA-CMD interfaces ("+str(len(qssi_not_found_lst))+") are not present in the QSSI QIIFA-CMD. The list is below")
                        if len(qssi_not_found_lst)>0:
                            Logger.logStdout.error(qssi_not_found_lst)
                        reason = "Target-CMD and QSSI-CMD are not identical"
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA CMD Loading for QSSI / Target",run_img_to_img_checker.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,"QIIFA CMD Loading for QSSI / Target",run_img_to_img_checker.__name__,reason)
                            sys.exit(1)
                    else:
                        for qssi_dict in qssi_lst:
                            for target_dict in target_lst:
                                if target_dict[u'name'] == qssi_dict[u'name'] and target_dict!=qssi_dict:
                                    if qssi_dict[u'name'] not in aidl_skipped_lst[u'ALL']:
                                        blk_by_blk_chk(target_dict,qssi_dict)
        except Exception as e:
            traceback.print_exc()
            Logger.logStdout.error(e)
            sys.exit(0)
    else:
        Logger.logStdout.error(qssi_json_cmd_path + " or " + target_json_cmd_path + " doesn't exist.")
        sys.exit(1)
    pass

def intf_not_found_lst(main_lst,compare_lst):
    '''Compare the list of dictionaries and if the
        name present in main list is not present in
        compare list. we collect the name of interfaces not
        found and return it as a list  '''
    found_name=False
    comapre_lst_not_found=[]
    for main_dict in main_lst:
        found_name=False
        for compare_dict in compare_lst:
            if compare_dict[u'name'] == main_dict[u'name']:
                found_name=True
        if not found_name:
            comapre_lst_not_found.append(str(main_dict[u'name']))
    return comapre_lst_not_found

def img_load_lst_from_path(path):
    import re
    img_dict = {}
    try:
        for root, dr, files in os.walk(path):
            for file in files:
                if file.endswith(".json"):
                    aidl_filename_match = re.search("aidl",file)
                    if aidl_filename_match != None:
                        tmp_filepath = os.path.join(path,file)
                        img_dict[str(os.path.basename(tmp_filepath))]=load_info_from_JSON_file(tmp_filepath)
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        Logger.logStdout.error("No File was found")
    return img_dict

def run_indi_checker(metadata_dict, goldencmddict_lst):
    """
    This check runs on the System and the Vendor Individually
    """
    for meta_dict in metadata_dict:
        if meta_dict not in goldencmddict_lst:
            name_present_check = False
            stability_flag_check = False
            type_flag_check = False
            hash_check = False
            vendavail_check = False
            if meta_dict[u'name'] in aidl_skipped_lst[u'ALL']:
                Logger.logStdout.warning("AIDL Skipped INTF from Skipped List " + meta_dict[u'name'])
                continue
            try:
                intf_skip_chk = skipintf_chk(meta_dict[u'name'])
            except Exception as e:
                traceback.print_exc()
                Logger.logStdout.warning("AIDL Skip checked needs to be validated for this intf "+meta_dict[u'name'])
                Logger.logStdout.warning(e)
                intf_skip_chk = True
            if intf_skip_chk:
                Logger.logInternal.info("ABI PRESERVED INTF " + meta_dict[u'name'])
                continue
            goldencmd_dictrlst = check_dup_intf_name(goldencmddict_lst,meta_dict[u'name'],True)
            if goldencmd_dictrlst == None:
                reason="AIDL interface doesn't exist in the coresponding QIIFA cmd. please add it to the cmd"
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],run_indi_checker.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],run_indi_checker.__name__,reason)
                continue
            #at this point, the meta entry must exist in the coresponding
            #cmd which means we can check for its validity and we know exactly
            #to which cmd entry it coresponds from the return of check_dup_intf_name
            elif not isinstance(goldencmd_dictrlst, list):
                #no duplicate for this intf. most of the cases will fall through
                #this case
                if is_intf_modified(meta_dict,goldencmd_dictrlst):
                    blk_by_blk_chk(meta_dict,goldencmd_dictrlst)
            else:
                #duplicates found, we will have to go one by one and
                #find a coresponding match
                if meta_dict not in goldencmd_dictrlst:
                    found_match = False
                    matching_hal = None
                    for golddict in goldencmd_dictrlst:
                        if meta_dict[u'name'] == golddict[u'name']:
                            found_match=True
                            matching_hal = meta_dict
                            break
                    if found_match:
                        if is_intf_modified(meta_dict,goldencmd_dictrlst):
                            blk_by_blk_chk(meta_dict,goldencmd_dictrlst)
                    else:
                        reason="AIDL interface's Mismatch error"
                        if plugin_state_warning:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],run_indi_checker.__name__,reason,False)
                        else:
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],run_indi_checker.__name__,reason)

def blk_by_blk_chk(meta_dict,goldencmd_dictrlst):
    stablility_check(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    types_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    hash_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    vndr_avble_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    has_dev_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    vrsn_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    if "frozen" in goldencmd_dictrlst.keys():
        frzn_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)

def blk_by_blk_vndr_chk(meta_dict,goldencmd_dictrlst):
    global VNDR_HAL_NOTFND
    stablility_check(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    types_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    hash_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    vndr_avble_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    has_dev_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    vrsn_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)
    if "frozen" in goldencmd_dictrlst.keys():
        frzn_cmpr_chk(meta_dict,goldencmd_dictrlst,Constants.qiifa_out_path_target_value)

def stablility_check(meta_dict,goldencmd_dictrlst,image_type):
    global VNDR_HAL_NOTFND
    try:
        if len(aidl_skipped_lst) > 0 and meta_dict[u'name'] in aidl_skipped_lst[u'STABILITY_CHECK']:
            Logger.logStdout.warning("AIDL Stability Check Skipped for "+meta_dict[u'name'])
        elif u'stability' not in meta_dict.keys() or u'stability' not in goldencmd_dictrlst.keys():
            stabilityerror = False
            if "stability" in meta_dict.keys() and "stability" not in goldencmd_dictrlst.keys():
                stabilityerror = True
                reason="AIDL interface's stability check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            elif "stability" not in meta_dict.keys() and "stability" in goldencmd_dictrlst.keys() :
                reason="AIDL interface's stability check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                stabilityerror = True
            else:
                reason="AIDL interface's stability check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
                stabilityerror = True

            if stabilityerror:
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
                else:
                    if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['stabiltiy',reason]
                return
            else:
                if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'stabiltiy':
                    del VNDR_HAL_NOTFND[meta_dict[u'name']]
        elif meta_dict[u'stability'] != goldencmd_dictrlst[u'stability']:
            reason="AIDL interface's stability check has been modified. Please run --create option before running IIC."
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['stabiltiy',reason]
            return
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        stabilityerror = False
        if "stability" in meta_dict.keys() and "stability" not in goldencmd_dictrlst.keys():
            stabilityerror = True
            reason="AIDL interface's stability check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
        elif "stability" not in meta_dict.keys() and "stability" in goldencmd_dictrlst.keys() :
            reason="AIDL interface's stability check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
            stabilityerror = True
        else:
            reason="AIDL interface's stability check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            stabilityerror = True

        if stabilityerror:
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['stabiltiy',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'stabiltiy':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]

def types_cmpr_chk(meta_dict,goldencmd_dictrlst,image_type):
    global VNDR_HAL_NOTFND
    try:
        if len(aidl_skipped_lst) > 0 and meta_dict[u'name'] in aidl_skipped_lst[u'TYPES_CHECK']:
            Logger.logStdout.warning("AIDL Types Check Skipped for "+meta_dict[u'name'])
        elif u'types' not in meta_dict.keys() or u'types' not in goldencmd_dictrlst.keys():
            typeserror = False
            if "types" in meta_dict.keys() and "types" not in goldencmd_dictrlst.keys():
                typeserror = True
                reason="AIDL interface's Types has been modified \n \t\t\t\t\t\t AIDL interface's types check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            elif "types" not in meta_dict.keys() and "types" in goldencmd_dictrlst.keys() :
                if len(goldencmd_dictrlst[u'types']) > 1:
                    reason="AIDL interface's types check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                    typeserror = True
            else:
                reason="AIDL interface's Types has been modified \n \t\t\t\t\t\t AIDL interface's types check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
                typeserror = True
            if typeserror:
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
                else:
                    if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['types',reason]
                return
            else:
                if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'types':
                    del VNDR_HAL_NOTFND[meta_dict[u'name']]
        elif meta_dict[u'types'] != goldencmd_dictrlst[u'types']:
            diffinmeta=[]
            metatypeset = set()
            goldencmdtypeset = set()
            samesets = set()
            diffinmeta = set()
            diffingoldencmd = set()
            if len(meta_dict[u'types']) > 0 or len(goldencmd_dictrlst[u'types']) > 0:
                metatypeset = set(meta_dict[u'types'])
                goldencmdtypeset = set(goldencmd_dictrlst[u'types'])
                samesets = metatypeset & goldencmdtypeset
                diffinmeta = list(samesets ^ metatypeset)
                diffingoldencmd = list(samesets ^ goldencmdtypeset)
            if len(diffinmeta) > 0:
                Logger.logStdout.error("These are the Types which are different in META")
                Logger.logStdout.error(diffinmeta)
                if len(diffingoldencmd) > 0:
                    Logger.logStdout.error("These are the Types which are different in Golden CMD")
                    Logger.logStdout.error(diffingoldencmd)
                reason="AIDL interface's Types has been modified.  please run --create option before running IIC."
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
                else:
                    if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['types',reason]
                return
            else:
                if (len(goldencmd_dictrlst[u'versions']) - len(meta_dict[u'versions'])) > 0:
                    Logger.logInternal.info("ABI INTF VERSION " + meta_dict[u'name'])
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        typeserror = False
        if "types" in meta_dict.keys() and "types" not in goldencmd_dictrlst.keys():
            typeserror = True
            reason="AIDL interface's Types has been modified \n \t\t\t\t\t\t AIDL interface's types check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
        elif "types" not in meta_dict.keys() and "types" in goldencmd_dictrlst.keys() :
            if len(goldencmd_dictrlst[u'types']) > 1:
                reason="AIDL interface's types check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                typeserror = True
        else:
            reason="AIDL interface's Types has been modified \n \t\t\t\t\t\t AIDL interface's types check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            typeserror = True
        if typeserror:
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['types',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'types':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]

def hash_cmpr_chk(meta_dict,goldencmd_dictrlst,image_type):
    global VNDR_HAL_NOTFND
    try:
        hashnotfound = False
        if len(aidl_skipped_lst) > 0 and meta_dict[u'name'] in aidl_skipped_lst[u'HASH_CHECK']:
            Logger.logStdout.warning("AIDL Hash Check Skipped for "+meta_dict[u'name'])
        elif u'hashes' not in meta_dict.keys() or u'hashes' not in goldencmd_dictrlst.keys():
            hashmetaerror = False
            if "hashes" in meta_dict.keys() and "hashes" not in goldencmd_dictrlst.keys():
                hashmetaerror = True
                reason="AIDL interface's hashes is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            elif "hashes" not in meta_dict.keys() and "hashes" in goldencmd_dictrlst.keys() :
                if len(goldencmd_dictrlst[u'hashes'].values()) > 1:
                    reason="AIDL interface's hashes is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                    hashmetaerror = True
            else:
                reason="AIDL interface's hashes is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
                hashmetaerror = True

            if meta_dict[u'name'] in aidl_skipped_lst[u'HASH_CHECK']:
                Logger.logStdout.warning("AIDL Hash Check Skipped for "+meta_dict[u'name'])
            elif hashmetaerror:
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
                else:
                    if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['hashes',reason]
                return
            else:
                if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'hashes':
                    del VNDR_HAL_NOTFND[meta_dict[u'name']]
        elif meta_dict[u'hashes'] != goldencmd_dictrlst[u'hashes']:
            diffinmeta=[]
            diffingoldcmd=[]
            metahashkeyset = set()
            metahashset = set()
            goldencmdhashkeyset = set()
            goldencmdhashset = set()
            samesets = set()
            if len(meta_dict[u'hashes']) > 0:
                if type(meta_dict[u'hashes']) is OrderedDict or type(meta_dict[u'hashes']) is dict:
                    metahashset = set(meta_dict[u'hashes'].values())
                    metahashkeyset = set(meta_dict[u'hashes'].keys())
                elif type(hash_cmpr_chk) is list:
                    metahashset = set(meta_dict[u'hashes'])
                    metahashkeyset = set(meta_dict[u'hashes'])
            if len(goldencmd_dictrlst[u'hashes']) > 0:
                if type(goldencmd_dictrlst[u'hashes']) is OrderedDict or type(goldencmd_dictrlst[u'hashes']) is dict:
                    goldencmdhashset = set(goldencmd_dictrlst[u'hashes'].values())
                    goldencmdhashkeyset = set(goldencmd_dictrlst[u'hashes'].keys())
                elif type(hash_cmpr_chk) is list:
                    goldencmdhashset = set(goldencmd_dictrlst[u'hashes'])
                    goldencmdhashkeyset = set(goldencmd_dictrlst[u'hashes'])
            samekeysets = metahashkeyset & goldencmdhashkeyset
            diffinmetakey = list(samekeysets ^ metahashkeyset)
            diffingoldcmdkey = list(samekeysets ^ goldencmdhashkeyset)
            samesets = metahashset & goldencmdhashset
            diffinmeta = list(samesets ^ metahashset)
            diffingoldcmd = list(samesets ^ goldencmdhashset)
            if image_type == "qssi":
                if len(diffinmetakey) > 0 or len(diffinmeta) > 0:
                    hashnotfound = True
            else:
                hashnotfound = True
                if len(goldencmd_dictrlst[u'hashes']) > 0:
                    for key in goldencmd_dictrlst[u'hashes'].keys():
                        if len(meta_dict[u'hashes']) > 0 and key in meta_dict[u'hashes'].keys():
                            if meta_dict[u'hashes'][key] == goldencmd_dictrlst[u'hashes'][key]:
                                hashnotfound = False
        if hashnotfound:
            if (len(diffinmeta) > 0):
                Logger.logStdout.error("These are the hashses which are different on the META Side")
                Logger.logStdout.error(diffinmeta)
            if (len(diffingoldcmd) > 0):
                Logger.logStdout.error("These are the hashses which are different on the QIIFA CMD Side")
                Logger.logStdout.error(diffingoldcmd)
            reason="AIDL interface's hashes has been modified. Please run --create option before running IIC."

            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['hashes',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'hashes':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        hashmetaerror = False
        if "hashes" in meta_dict.keys() and "hashes" not in goldencmd_dictrlst.keys():
            hashmetaerror = True
            reason="AIDL interface's hashes is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
        elif "hashes" not in meta_dict.keys() and "hashes" in goldencmd_dictrlst.keys() :
            if len(goldencmd_dictrlst[u'hashes'].values()) > 1:
                reason="AIDL interface's hashes is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                hashmetaerror = True
        else:
            reason="AIDL interface's hashes is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            hashmetaerror = True

        if meta_dict[u'name'] in aidl_skipped_lst[u'HASH_CHECK']:
            Logger.logStdout.warning("AIDL Hash Check Skipped for "+meta_dict[u'name'])
        elif hashmetaerror:
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['hashes',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'hashes':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]

def vndr_avble_cmpr_chk(meta_dict,goldencmd_dictrlst,image_type):
    global VNDR_HAL_NOTFND
    try:
        if len(aidl_skipped_lst) > 0 and meta_dict[u'name'] in aidl_skipped_lst[u'VNDRAVLBLE_CHECK']:
            Logger.logStdout.warning("AIDL Vendor Available Check Skipped for "+meta_dict[u'name'])
        elif u'vendor_available' not in meta_dict.keys() or u'vendor_available' not in goldencmd_dictrlst.keys():
            vendor_availableerror = False
            if "vendor_available" in meta_dict.keys() and "vendor_available" not in goldencmd_dictrlst.keys():
                vendor_availableerror = True
                reason="AIDL interface's Vendor Available check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            elif "vendor_available" not in meta_dict.keys() and "vendor_available" in goldencmd_dictrlst.keys() :
                reason="AIDL interface's Vendor Available check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                vendor_availableerror = True
            else:
                reason="AIDL interface's Vendor Available check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
                vendor_availableerror = True

            if meta_dict[u'name'] in aidl_skipped_lst[u'VNDRAVLBLE_CHECK']:
                Logger.logStdout.warning("AIDL Vendor Available Check Skipped for "+meta_dict[u'name'])
            elif vendor_availableerror:
                if plugin_state_warning:
                    Logger.logInternal.info("ABI VENDOR AVAILABLE TAG is not available FOR " + meta_dict[u'name'])
                else:
                    if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['vndr_avble',reason]
                return
            else:
                if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'vndr_avble':
                    del VNDR_HAL_NOTFND[meta_dict[u'name']]
        elif meta_dict[u'vendor_available'] != goldencmd_dictrlst[u'vendor_available']:
            reason="AIDL interface's vendor available flag has been modified. Please run --create option before running IIC."
            if plugin_state_warning:
                Logger.logInternal.info("ABI VENDOR AVAILABLE TAG Has been modified FOR " + meta_dict[u'name'])
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['vndr_avble',reason]
            return
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        vendor_availableerror = False
        if "vendor_available" in meta_dict.keys() and "vendor_available" not in goldencmd_dictrlst.keys():
            vendor_availableerror = True
            reason="AIDL interface's Vendor Available check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
        elif "vendor_available" not in meta_dict.keys() and "vendor_available" in goldencmd_dictrlst.keys() :
            reason="AIDL interface's Vendor Available check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
            vendor_availableerror = True
        else:
            reason="AIDL interface's Vendor Available check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            vendor_availableerror = True

        if meta_dict[u'name'] in aidl_skipped_lst[u'VNDRAVLBLE_CHECK']:
            Logger.logStdout.warning("AIDL Vendor Available Check Skipped for "+meta_dict[u'name'])
        elif vendor_availableerror:
            if plugin_state_warning:
                Logger.logInternal.info("ABI VENDOR AVAILABLE TAG is not available FOR " + meta_dict[u'name'])
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['vndr_avble',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'vndr_avble':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]

def has_dev_cmpr_chk(meta_dict,goldencmd_dictrlst,image_type):
    global VNDR_HAL_NOTFND
    try:
        if len(aidl_skipped_lst) > 0 and meta_dict[u'name'] in aidl_skipped_lst[u'HASDEVCHK']:
            Logger.logStdout.warning("AIDL Has Development Check Skipped for "+meta_dict[u'name'])
        elif u'has_development' not in meta_dict.keys() or u'has_development' not in goldencmd_dictrlst.keys():
            has_developmenterror = False
            if "has_development" in meta_dict.keys() and "has_development" not in goldencmd_dictrlst.keys():
                has_developmenterror = True
                reason="AIDL interface's has development check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            elif "has_development" not in meta_dict.keys() and "has_development" in goldencmd_dictrlst.keys() :
                reason="AIDL interface's has development check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                has_developmenterror = True
            else:
                reason="AIDL interface's has development check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
                has_developmenterror = True
            if has_developmenterror:
                if plugin_state_warning:
                    Logger.logInternal.info("ABI VERSION TAG NOT PRESENT FOR " + meta_dict[u'name'])
                else:
                    if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['has_dev',reason]
                return
            else:
                if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'has_dev':
                    del VNDR_HAL_NOTFND[meta_dict[u'name']]
        elif meta_dict[u'has_development'] != goldencmd_dictrlst[u'has_development']:
            reason="AIDL interface's has development flag has been modified. Please run --create option before running IIC."
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['has_dev',reason]
            return
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        has_developmenterror = False
        if "has_development" in meta_dict.keys() and "has_development" not in goldencmd_dictrlst.keys():
            has_developmenterror = True
            reason="AIDL interface's has development check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
        elif "has_development" not in meta_dict.keys() and "has_development" in goldencmd_dictrlst.keys() :
            reason="AIDL interface's has development check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
            has_developmenterror = True
        else:
            reason="AIDL interface's has development check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            has_developmenterror = True
        if has_developmenterror:
            if plugin_state_warning:
                Logger.logInternal.info("ABI VERSION TAG NOT PRESENT FOR " + meta_dict[u'name'])
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['has_dev',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'has_dev':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]

def vrsn_cmpr_chk(meta_dict,goldencmd_dictrlst,image_type):
    global VNDR_HAL_NOTFND
    versionnotfound = False
    try:
        if len(aidl_skipped_lst) > 0 and meta_dict[u'name'] in aidl_skipped_lst[u'VERSION_CHECK']:
            Logger.logStdout.warning("AIDL Version Check Skipped for "+meta_dict[u'name'])
        elif u'versions' not in meta_dict.keys() or u'versions' not in goldencmd_dictrlst.keys():
            versionerror = False
            if "versions" in meta_dict.keys() and "versions" not in goldencmd_dictrlst.keys():
                versionerror = True
                reason="AIDL interface's versions is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            elif "versions" not in meta_dict.keys() and "versions" in goldencmd_dictrlst.keys() :
                if len(goldencmd_dictrlst[u'versions']) > 1:
                    reason="AIDL interface's versions is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                    versionerror = True
            else:
                reason="AIDL interface's versions is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
                versionerror = True

            if versionerror:
                if plugin_state_warning:
                    Logger.logInternal.info("ABI VERSION TAG NOT PRESENT FOR " + meta_dict[u'name'])
                else:
                    if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['version',reason]
                return
            else:
                if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'version':
                    del VNDR_HAL_NOTFND[meta_dict[u'name']]
        elif meta_dict[u'versions'] != goldencmd_dictrlst[u'versions']:
            metahashset = set()
            goldencmdhashset = set()
            samesets = set()
            diffinmeta = []
            diffingoldcmd = []
            metahashset = set(meta_dict[u'versions'])
            goldencmdhashset = set(goldencmd_dictrlst[u'versions'])
            samesets = metahashset & goldencmdhashset
            diffinmeta = list(samesets ^ metahashset)
            diffingoldcmd = list(samesets ^ goldencmdhashset)
            if image_type == "qssi":
                if len(diffinmeta) > 0:
                    versionnotfound = True
            else:
                versionnotfound = True
                if len(goldencmd_dictrlst[u'versions']) > 0 and len(meta_dict[u'versions']) > 0:
                    for golden_version in goldencmd_dictrlst[u'versions']:
                        for meta_version in meta_dict[u'versions']:
                            if golden_version == meta_version:
                                versionnotfound = False
        if versionnotfound:
            if (len(diffinmeta) > 0):
                Logger.logStdout.error("These are the version which are different on the META Side")
                Logger.logStdout.error(diffinmeta)
            if (len(diffingoldcmd) > 0):
                Logger.logStdout.error("These are the version which are different on the QIIFA CMD Side")
                Logger.logStdout.error(diffingoldcmd)
            reason="AIDL interface's versions has been modified. Please run --create option before running IIC."
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['version',reason]
            return
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        versionerror = False
        if "versions" in meta_dict.keys() and "versions" not in goldencmd_dictrlst.keys():
            versionerror = True
            reason="AIDL interface's versions is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
        elif "versions" not in meta_dict.keys() and "versions" in goldencmd_dictrlst.keys() :
            if len(goldencmd_dictrlst[u'versions']) > 1:
                reason="AIDL interface's versions is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                versionerror = True
        else:
            reason="AIDL interface's versions is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            versionerror = True

        if versionerror:
            if plugin_state_warning:
                Logger.logInternal.info("ABI VERSION TAG NOT PRESENT FOR " + meta_dict[u'name'])
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['version',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'version':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]

def frzn_cmpr_chk(meta_dict,goldencmd_dictrlst,image_type):
    global VNDR_HAL_NOTFND
    try:
        if len(aidl_skipped_lst) > 0 and meta_dict[u'name'] in aidl_skipped_lst[u'FRZNCHK']:
            Logger.logStdout.warning("AIDL Frozen Check Skipped for "+meta_dict[u'name'])
        elif u'frozen' not in meta_dict.keys() or u'frozen' not in goldencmd_dictrlst.keys():
            frznerror = False
            if "frozen" in meta_dict.keys() and "frozen" not in goldencmd_dictrlst.keys():
                frznerror = True
                reason="AIDL interface's Frozen check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            elif "frozen" not in meta_dict.keys() and "frozen" in goldencmd_dictrlst.keys() :
                reason="AIDL interface's Frozen check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
                frznerror = True
            else:
                reason="AIDL interface's Frozen check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
                frznerror = True
            if frznerror:
                if plugin_state_warning:
                    Logger.logInternal.info("ABI VERSION TAG NOT PRESENT FOR " + meta_dict[u'name'])
                else:
                    if image_type == "qssi":
                            UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                    else:
                        VNDR_HAL_NOTFND[meta_dict[u'name']] = ['frozen',reason]
                return
            else:
                if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'frozen':
                    del VNDR_HAL_NOTFND[meta_dict[u'name']]
        elif meta_dict[u'frozen'] != goldencmd_dictrlst[u'frozen']:
            reason="AIDL interface's frozen flag has been modified. Please run --create option before running IIC."
            if plugin_state_warning:
                UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason,False)
            else:
                if image_type == "qssi":
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['frozen',reason]
            return
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        frznerror = False
        if "frozen" in meta_dict.keys() and "frozen" not in goldencmd_dictrlst.keys():
            frznerror = True
            reason="AIDL interface's Frozen check is present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
        elif "frozen" not in meta_dict.keys() and "frozen" in goldencmd_dictrlst.keys() :
            reason="AIDL interface's Frozen check is not present in META data but present in QIIFA CMD. Please run --create option before running IIC."
            frznerror = True
        else:
            reason="AIDL interface's Frozen check is not present in META data but not present in QIIFA CMD. Please run --create option before running IIC."
            frznerror = True
        if frznerror:
            if plugin_state_warning:
                Logger.logInternal.info("ABI VERSION TAG NOT PRESENT FOR " + meta_dict[u'name'])
            else:
                if image_type == "qssi":
                        UtilFunctions.print_violations_on_stdout(LOG_TAG,meta_dict[u'name'],blk_by_blk_chk.__name__,reason)
                else:
                    VNDR_HAL_NOTFND[meta_dict[u'name']] = ['frozen',reason]
            return
        else:
            if meta_dict[u'name'] in VNDR_HAL_NOTFND.keys() and VNDR_HAL_NOTFND[meta_dict[u'name']][0] == 'frozen':
                del VNDR_HAL_NOTFND[meta_dict[u'name']]

def chk_cmd_dict_sorted(goldencmddict_list,filename):
    filename = filename+".json"
    try:
        sorteddict_list = sorted(goldencmddict_list, key=lambda d: d[u'name'])
        listcount = len(goldencmddict_list)
        for key in range(listcount):
            if goldencmddict_list[key] != sorteddict_list[key]:
                reason = "The following QIIFA CMD is not sorted. Please check go/qiifa to get instructions for adding interface. Please contact qiifa-squad for more information."
                if plugin_state_warning:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,chk_cmd_dict_sorted.__name__,reason,False)
                else:
                    UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,chk_cmd_dict_sorted.__name__,reason)
                return True
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        reason = "Error in the QIIFA CMD check sort function. Please contact qiifa-squad for more information"
        if plugin_state_warning:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,chk_cmd_dict_sorted.__name__,reason,False)
        else:
            UtilFunctions.print_violations_on_stdout(LOG_TAG,filename,chk_cmd_dict_sorted.__name__,reason)
        return True
    return False


def is_intf_modified(meta_dict, goldencmd_dict):
    '''
    Compare the meta dictionary and the golden golden cmd dictionary
    and return true if something has changed
    '''
    if goldencmd_dict != meta_dict:
        return True
    return False

def check_dup_intf_name(goldenlist, intf_name, find_dup):
    dup_intf_lst=[]
    for goldendict in goldenlist:
        if goldendict[u'name'] == intf_name:
            if find_dup:
                dup_intf_lst.append(goldendict)
            else:
                return goldendict
    '''
    Check if Interface is present
    '''
    if len(dup_intf_lst) == 0:
        return None
    elif len(dup_intf_lst) == 1:
        return dup_intf_lst[0]
    return dup_intf_lst

def copy_json_to_out():
    try:
        if Constants.qiifa_out_path_target_value == "qssi":
            for f in os.listdir(Constants.qiifa_aidl_db_root):
                filename_number = ((f.split("_")[-1]).split("."))[0]
                if f.endswith(".json") and ((filename_number =="device" or f.startswith("qiifa_aidl_skipped_")) or ( filename_number.isdigit() and int(filename_number) > FCM_MIN_VAL and int(filename_number) <= FCM_MAX_VAL ) ):
                    src = Constants.qiifa_aidl_db_root + f
                    dest = Constants.qiifa_current_cmd_dir + "/" + f
                    Logger.logStdout.info("src : "+src)
                    Logger.logStdout.info("dest: "+dest)
                    src_lst = load_info_from_JSON_file(src)
                    try:
                        if UtilFunctions.pathExists(dest):
                            os.remove(dest)
                        with open(dest,"w") as json_file:
                            json.dump(src_lst, json_file,separators=(",", ": "), indent=4,sort_keys=True)
                    except Exception as e:
                        traceback.print_exc()
                        Logger.logStdout.error(e)
                        Logger.logStdout.error("Couldn't copy json to out.")
                elif f.endswith(".csv"):
                    src = Constants.qiifa_aidl_db_root + f
                    dest = Constants.qiifa_current_cmd_dir + "/" + f
                    Logger.logStdout.info("src : "+src)
                    Logger.logStdout.info("dest: "+dest)
                    try:
                        if UtilFunctions.pathExists(dest):
                            os.remove(dest)
                        shutil.copy(src, dest)
                    except Exception as e:
                        traceback.print_exc()
                        Logger.logStdout.error(e)
                        Logger.logStdout.error("Couldn't copy csv to out.")
        else:
            for f in os.listdir(Constants.qiifa_aidl_db_root):
                if str(FCM_VENDOR_TL) in f or "device" in f or (not Constants.platform_version.isdigit() and str(FCM_VENDOR_TL+1) in f) :
                    if (f.startswith("qiifa_aidl_cmd_") or f.startswith("qiifa_aidl_skipped_")) and f.endswith(".json"):
                        src = Constants.qiifa_aidl_db_root + f
                        dest = Constants.qiifa_current_cmd_dir + "/" + f
                        Logger.logStdout.info("src : "+src)
                        Logger.logStdout.info("dest: "+dest)
                        src_lst = load_info_from_JSON_file(src)
                        try:
                            if UtilFunctions.pathExists(dest):
                                os.remove(dest)
                            with open(dest,"w") as json_file:
                                json.dump(src_lst, json_file,separators=(",", ": "), indent=4,sort_keys=True)
                        except Exception as e:
                            traceback.print_exc()
                            Logger.logStdout.error(e)
                            Logger.logStdout.error("Couldn't copy json to out.")
    except Exception as e:
        traceback.print_exc()
        Logger.logStdout.error(e)
        Logger.logStdout.error("Couldn't copy json to out.")

def skipintf_chk(intf_name):
    string_concatenate = ''
    for words in intf_name.split('.'):
        string_concatenate += words
        if string_concatenate in Constants.AIDL_ALLWED_INTF_PREF:
            return False
        string_concatenate += '.'
    return True

class qiifa_aidl:
    def __init__(self):
         pass
    start_qiifa_aidl_checker = func_start_qiifa_aidl_checker
'''
plugin class implementation
plugin class is derived from plugin_interface class
Name of plugin class MUST be MyPlugin
'''
class MyPlugin(plugin_interface):
    def __init__(self):
        pass

    def register(self):
        return Constants.AIDL_SUPPORTED_CREATE_ARGS

    def config(self, QIIFA_type=None, libsPath=None, CMDPath=None):
        pass

    def generateGoldenCMD(self, libsPath=None, storagePath=None, create_args_lst=None):
        '''
        the assumption here is that if create_args_lst is empty, then this was called under the
        circumstance where --create arg was called with "all" option; so it should behave as if
        --create was called with "aidl" option.
        '''
        if create_args_lst is None:
            aidl_checker_main_create(self, "golden", Constants.AIDL_SUPPORTED_CREATE_ARGS[0])
        #Same behavior as above. Ignore everything in create_args_lst but the first one.
        elif create_args_lst[0] == Constants.AIDL_SUPPORTED_CREATE_ARGS[0]:
            aidl_checker_main_create(self, "golden", Constants.AIDL_SUPPORTED_CREATE_ARGS[0])
        #This is used to create a list of Interfaces currently available in the max available compatibility matrix
        elif create_args_lst[0] == Constants.AIDL_SUPPORTED_CREATE_ARGS[3]:
            aidl_checker_main_create(self, "golden", Constants.AIDL_SUPPORTED_CREATE_ARGS[3])
        #In this case create_args_lst[1] will have the particular intf name
        elif create_args_lst[0] in Constants.AIDL_SUPPORTED_CREATE_ARGS and len(create_args_lst)==2:
            aidl_checker_main_create(self, "golden", create_args_lst[0], create_args_lst[1])
        else:
            Logger.logStdout.info("Invalid --create argument options")
            Logger.logStdout.info("python qiifa_main.py -h")
            sys.exit()

    def IIC(self, **kwargs):
        if kwargs:
            kwargs.update({"flag":"check"})
            func_start_qiifa_aidl_checker(self,**kwargs)
        else:
            func_start_qiifa_aidl_checker(self,"check")
