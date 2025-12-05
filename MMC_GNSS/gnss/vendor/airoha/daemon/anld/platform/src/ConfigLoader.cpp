/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
#define LOG_TAG "CONFIGLOADER"
#include "ConfigLoader.h"
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <string>
#include "simulation.h"
#include "tinyxml2.h"
#define XML_CONFIG_VERSION "1.0.0"
using Airoha::AdvanceSettingList;
using Airoha::Configuration;
using Airoha::FirmwareDownloadSetting;
using Airoha::LogConfigNode;
using Airoha::NmeaFilterConfiguration;
using Airoha::UartConfigNode;
using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using tinyxml2::XMLError;
#define COMMON_SETTING_NAME "CommonSetting"
#define ADVANCE_SETTING_NAME "AdvanceSetting"
/* COMMON CONFIG */
#define CONFIG_TRUE_STRING "true"
#define CONFIG_FALSE_STRING "false"
#define CONFIG_OUTPUT_DEBUG_LOG_KEY "output_debug_log"
#define CONFIG_OUTPUT_BINARY_LOG_KEY "output_binary_log"
#define CONFIG_EPO_VENDOR_ID_KEY "epo_vendor_id"
#define CONFIG_EPO_PROJECT_NAME_KEY "epo_project_name"
#define CONFIG_EPO_DEVICE_ID_KEY   "epo_device_id"
#define CONFIG_EPO_FILE_PATH_KEY   "epo_file_path"
#define CONFIG_RAW_MEASUREMENT_SUPPORT "raw_measurement_support"
#define CONFIG_UART_LOG_REPORT        "uart_log_record"
#define CONFIG_TRACE_LOG_ENABLE       "trace_log_record"
#define CONFIG_AUTO_LOCK_SLEEP        "auto_lock_sleep"
#define CONFIG_AUTO_DOWNLOAD_EPO       "auto_download_epo"
#define CONFIG_SUPPORT_HIGH_FIX_RATE       "support_high_fix_rate"
#define CONFIG_NMEA_FILTER       "nmea_filter"
#define CONFIG_DEBUG_PORT       "debug_port"
#define CONFIG_POWERGPS_PORT    "powergps_port"
#define CONFIG_WAKE_DURATION       "wake_duration"
#define CONFIG_WAKEUP_BEFORE_SEND "need_wakeup_before_send_data"
#define CONFIG_LOG_LEVEL          "log_level"
#define CONFIG_DATA_PATH "data_path"
#define CONFIG_UART_NODE_NAME "uart_node_name"
#define CONFIG_UART_FLOWCONTROL "uart_flowcontrol"
/* ADVANCE CONFIG */
#define CONFIG_SUSPEND_IMPLEMENT_VERSION "suspend_implement"
#define CONFIG_FIRMWARE_DOWNLOAD "firmware_download"
#define CONFIG_FIRMWARE_DOWNLOAD_TRIGGER_METHOD "trigger_method"
#define CONFIG_FIRMWARE_DOWNLOAD_CHIP_FIRMWARE_CHANGE "chip_firmware_change"
#define CONFIG_FIRMWARE_DOWNLOAD_BACKUP_FILE_CHANGE "backup_file_change"
#define CONFIG_FIRMWARE_DOWNLOAD_RETRY_TIMES        "retry_times"
#define CONFIG_RF_PATH_LOSS_CP       "rf_pass_loss_cp"
#define CONFIG_RF_PATH_LOSS_AP       "rf_pass_loss_ap"
#define CONFIG_DUMP_SATELLITE_INFO     "dump_satellite_info"
#define CONFIG_SUPPORT_QC_PLATFORM     "support_qc_platform"
#define CONFIG_MAX_UART_LOG_NUM "max_uart_log_num"
#define CONFIG_MAX_UART_LOG_SINGLE_SIZE "max_uart_log_single_size"
#define CONFIG_MAX_TRACE_LOG_NUM "max_trace_log_num"
#define CONFIG_MAX_TRACE_LOG_SINGLE_SIZE "max_trace_log_single_size"
#define CONFIG_INITIAL_LOCATION_NODE "init_location"
#define CONFIG_INITIAL_LOCATION_LATITUDE "latitude"
#define CONFIG_INITIAL_LOCATION_LONGITUDE "longitude"
#define CONFIG_INITIAL_LOCATION_ALTITUDE "altitude"
#define CONFIG_CHIP_DATA_IO_TYPE "data_io"
#define CONFIG_AUTO_SET_IO "auto_set_io"
#define CONFIG_EVENT_LOG_NODE "event_log"
#define CONFIG_LTE_FILTER "lte_filter"
#define CONFIG_VDR "vdr_enable"
#define CONFIG_HAS_AGNSS "has_agnss"
#define CONFIG_SUPPORT_M_PLATFORM "support_m_platform"
#define CONFIG_QUERY_CUSTOMER_VERSION "query_customer_version"
#define CONFIG_COMPATIBLE_WITH_CUSTOMER_ALGORITHM \
    "compatible_customer_algorithm"
#define CONFIG_CUSTOMER_SUFFIX "customer_suffix"
#define CONFIG_DUMP_EPOCH_DATA "dump_epoch_data"
#define CONFIG_DUMP_NMEA_WHEN_LOCATION_INVALID "dump_nmea_when_location_invalid"
//#define CONFIG_CLOSE_UART_IDLE     "close_uart_when_idle"
#if 0
int ConfigLoader::loadConfigFile(std::string filename){
    return loadConfigFile(filename.c_str());;
}
int ConfigLoader::loadConfigFile(const char* filename){
    LOG_D("config file path:%s", filename);
    XMLError ret = xmlLoader.LoadFile(filename);
    if(ret != XMLError::XML_SUCCESS){
        LOG_E("load config file error,%d",ret);
        mValid = false;
        return -1;
    }
    toConfigList();
    mValid = true;
    return 0;
}
bool ConfigLoader::isValid() const {
    return mValid;
}
std::string ConfigLoader::getSetting(std::string group, std::string name){
    (void)group;
    (void)name;
    return "";
}
void ConfigLoader::printAllSetting(){
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_OUTPUT_DEBUG_LOG_KEY);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_OUTPUT_BINARY_LOG_KEY);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_EPO_VENDOR_ID_KEY);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_EPO_PROJECT_NAME_KEY);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_EPO_DEVICE_ID_KEY);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_EPO_FILE_PATH_KEY);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_POWERGPS_PORT);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_DEBUG_PORT);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_RAW_MEASUREMENT_SUPPORT);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_UART_LOG_REPORT);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_AUTO_LOCK_SLEEP);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_AUTO_DOWNLOAD_EPO);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_WAKEUP_BEFORE_SEND);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_TRACE_LOG_ENABLE);
    printSettingByNama(COMMON_SETTING_NAME, CONFIG_DATA_PATH);
    return;
}
void ConfigLoader::printSettingByNama(const char * catagory, const char *name){
    LOG_D("Setting %s:%s",name,getSettingTextByName(catagory,name));
    return;
}
const char *ConfigLoader::getSettingTextByName(const char * catagory, const char *name){
    XMLElement* commonSetting = xmlLoader.FirstChildElement(catagory);
    if(!commonSetting){
        LOG_E("Failed to get common setting");
        return "";
    }
                             
    XMLElement* commonChildEle = commonSetting->FirstChildElement(name);
    if(!commonChildEle){
        LOG_E("Failed to get child setting");
        return "";
    }    
    return commonChildEle->GetText();
}
void ConfigLoader::toConfigList(){
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_OUTPUT_DEBUG_LOG_KEY, configList.outputDebugLog);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_OUTPUT_BINARY_LOG_KEY, configList.outputBinary);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_EPO_VENDOR_ID_KEY, configList.epoVendorID);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_EPO_PROJECT_NAME_KEY, configList.epoProjectName);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_EPO_DEVICE_ID_KEY,configList.epoDeviceID);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_DEBUG_PORT,configList.debugPort);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_POWERGPS_PORT, configList.powerGPSPort);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_RAW_MEASUREMENT_SUPPORT,configList.rawMeasurementSupport);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_UART_LOG_REPORT,configList.uartLogRecord);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_AUTO_LOCK_SLEEP,configList.autoLockSleep);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_AUTO_DOWNLOAD_EPO, configList.autoDownloadEPO);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_WAKE_DURATION, configList.wakeDuration);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_WAKEUP_BEFORE_SEND, configList.wakeupBeforeSendData);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_TRACE_LOG_ENABLE,
                      configList.traceLogRecord);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_LOG_LEVEL,
        configList.logLevel);
    toConfigListValue(
        ADVANCE_SETTING_NAME,
        CONFIG_SUSPEND_IMPLEMENT_VERSION,
        mAdvanceSetting.suspendImplementType
    );
    toConfigListValue(ADVANCE_SETTING_NAME, CONFIG_SUPPORT_HIGH_FIX_RATE,configList.supportHighFixRate);
    toConfigListValue(COMMON_SETTING_NAME, CONFIG_DATA_PATH, configList.dataPath);
    parserFirmwareDownloadConfig();
    LOG_D("%s", configList.toString().c_str());
    LOG_D("%s", mAdvanceSetting.toString().c_str());
    return;
}
void ConfigLoader::toConfigListValue(const char * catagory, const char *key, std::string &valueToBeSet){
    const char *configString;
    configString = getSettingTextByName(catagory, key);
    valueToBeSet = configString;
}
void ConfigLoader::toConfigListValue(const char * catagory, const char *key, bool &valueToBeSet){
    const char *configString;
    configString = getSettingTextByName(catagory, key);
    if(strcmp(configString,CONFIG_TRUE_STRING) == 0){
        valueToBeSet = true;
    }else{
        valueToBeSet = false;
    }
}
void ConfigLoader::toConfigListValue(const char * catagory, const char *key, int &valueToBeSet){
    const char *configString;
    configString = getSettingTextByName(catagory, key);
    valueToBeSet = atoi(configString);
}
bool ConfigLoader::toBool(const char *str, bool defaultValue) {
    if (!str) {
        return defaultValue;
    }
    const char *boolTrue[] = {"TRUE", "true"};
    const char *boolFalse[] = {"FALSE", "false"};
    for (size_t i = 0; i < sizeof(boolTrue) / sizeof(const char *); i++) {
        if (strcmp(str, boolTrue[i]) == 0) {
            return true;
        }
    }
    for (size_t i = 0; i < sizeof(boolFalse) / sizeof(const char *); i++) {
        if (strcmp(str, boolFalse[i]) == 0) {
            return false;
        }
    }
    LOG_W("toBool: %s not a key in true or false", str);
    return defaultValue;
}
void ConfigLoader::parserFirmwareDownloadConfig() {
    LOG_D("parser firmware download config");
    XMLElement *advSetting = xmlLoader.FirstChildElement(ADVANCE_SETTING_NAME);
    if (!advSetting) {
        LOG_E("Failed to get adv setting");
        return;
    }
    XMLElement *firmwareCfg =
        advSetting->FirstChildElement(CONFIG_FIRMWARE_DOWNLOAD);
    if (!firmwareCfg) {
        LOG_E("Failed to get fw dl setting");
        return;
    }
    const char *downloadSwitch = firmwareCfg->GetText();
    if (!downloadSwitch) {
        LOG_E("fail to get download switch setting");
        return;
    }
    mAdvanceSetting.firmwareDownload.download = toBool(downloadSwitch, true);
    const char *triggerMethod =
        firmwareCfg->Attribute(CONFIG_FIRMWARE_DOWNLOAD_TRIGGER_METHOD);
    if (!triggerMethod) {
        return;
    }
    if (strcmp(triggerMethod, "or") == 0 || strcmp(triggerMethod, "OR") == 0) {
        mAdvanceSetting.firmwareDownload.triggerMethod =
            FirmwareDownloadSetting::TRIGGER_OR;
    } else if (strcmp(triggerMethod, "and") == 0 ||
               strcmp(triggerMethod, "AND") == 0) {
        mAdvanceSetting.firmwareDownload.triggerMethod =
            FirmwareDownloadSetting::TRIGGER_AND;
    } else {
        LOG_E("trigger method invalid, use default value");
        return;
    }
    mAdvanceSetting.firmwareDownload.chipFirmwareChange =
        firmwareCfg->BoolAttribute(
            CONFIG_FIRMWARE_DOWNLOAD_CHIP_FIRMWARE_CHANGE, false);
    mAdvanceSetting.firmwareDownload.backupFileChange =
        firmwareCfg->BoolAttribute(CONFIG_FIRMWARE_DOWNLOAD_BACKUP_FILE_CHANGE,
                                   true);
    LOG_D("parser firmware download config done");
    return;
}
std::string ConfigLoader::ConfigList::toString(){
    char temp[1024] = {0};
    snprintf(temp,1023,"debuglog[%d],binary[%d],vendorid[%s],projName[%s],DevId[%s],epofilePath[%s], \
                rawmeasSupport[%d],uartLog[%d],trace[%d], wakeDuration[%d], debugport[%d], powerGPSPort[%d]", 
        outputDebugLog,
        outputBinary,
        epoVendorID.c_str(),
        epoProjectName.c_str(),
        epoVendorID.c_str(),
        dataPath.c_str(),
        rawMeasurementSupport,
        uartLogRecord,
        traceLogRecord,
        wakeDuration,
        debugPort,
        powerGPSPort
            );
    return temp;
}
ConfigLoader::ConfigList::ConfigList(){
    outputDebugLog = false;
    outputBinary = false;
    epoVendorID = "";
    epoProjectName = "";
    epoDeviceID = "";
    debugPort = -1;
    powerGPSPort = -1;
    rawMeasurementSupport = false;
    uartLogRecord = false;
    wakeDuration = 0;
    traceLogRecord = false;
}
const ConfigLoader::ConfigList* ConfigLoader::getConfigList() const{
    return &configList;
}
const AdvanceSettingList & ConfigLoader::getAdvanceConfig() const {
    return mAdvanceSetting;
}
std::string AdvanceSettingList::toString() {
    std::string temp;
    char t[128];
    snprintf(t, sizeof(t), "AdvanceSetting: suspend_impl=%d",
             (int)suspendImplementType);
    temp += t;
    snprintf(
        t, sizeof(t),
        " firmware setting: switch=%d, trigger=%d, chip_firmware_check=%d, backup_file_check=%d",
        firmwareDownload.download,
        (int)firmwareDownload.triggerMethod,
        firmwareDownload.chipFirmwareChange, firmwareDownload.backupFileChange);
    temp += t;
    return temp;
}
#endif
Configuration *Configuration::INSTANCE = new Configuration();
bool Configuration::loadConfigureFile(const char *filename) {
    LOG_D("load configuration: %s", filename);
    XMLDocument xmlLoader;
    tinyxml2::XMLError ret = xmlLoader.LoadFile(filename);
    if (ret != XMLError::XML_SUCCESS) {
        LOG_E("load config file error,%d, use default config", ret);
        return false;
    }
    XMLElement *root = xmlLoader.RootElement();
    const char *version = root->Attribute("version");
    if (version == nullptr) {
        LOG_E("no version field in config file!!");
        return false;
    }
    if (strncmp(version, XML_CONFIG_VERSION, strlen(XML_CONFIG_VERSION)) != 0) {
        LOG_E("xml version not match %s vs %s", version, XML_CONFIG_VERSION);
        return false;
    }
    bool initRet = initConfig(root);
    if (!initRet) {
        LOG_E("XML Configuration Init ERROR!!!!");
        return false;
    }
    LOG_D("###################################");
    LOG_D("##                               ##");
    LOG_D("##  CONFIGURATION INIT SUCCESS   ##");
    LOG_D("##                               ##");
    LOG_D("###################################");
    return true;
}
Configuration::Configuration() { memset(&mKernelNode, 0, sizeof(mKernelNode)); }
bool Configuration::initConfig(XMLElement *root) {
    getValueByGroupAndName(root, COMMON_SETTING_NAME,
                           CONFIG_OUTPUT_DEBUG_LOG_KEY, &mOutputDebugLog);
    getValueByGroupAndName(root, COMMON_SETTING_NAME,
                           CONFIG_OUTPUT_BINARY_LOG_KEY, &mOutputBinary);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_EPO_VENDOR_ID_KEY,
                           mEpoVendorID, sizeof(mEpoVendorID));
    getValueByGroupAndName(root, COMMON_SETTING_NAME,
                           CONFIG_EPO_PROJECT_NAME_KEY, mEpoProjectName,
                           sizeof(mEpoProjectName));
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_EPO_DEVICE_ID_KEY,
                           mEpoDeviceID, sizeof(mEpoDeviceID));
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_DEBUG_PORT,
                           &mDebugPort);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_POWERGPS_PORT,
                           &mPowerGPSPort);
    getValueByGroupAndName(root, COMMON_SETTING_NAME,
                           CONFIG_RAW_MEASUREMENT_SUPPORT,
                           &mRawMeasurementSupport);
    // getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_UART_LOG_REPORT,
    //                        &mUartLogRecord);
    parserUartLogRecord(root);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_AUTO_LOCK_SLEEP,
                           &mAutoLockSleep);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_AUTO_DOWNLOAD_EPO,
                           &mAutoDownloadEPO);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_WAKE_DURATION,
                           &mWakeDuration);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_WAKEUP_BEFORE_SEND,
                           &mWakeupBeforeSendData);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_TRACE_LOG_ENABLE,
                           &mTraceLogRecord);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_LOG_LEVEL,
                           mLogLevel, sizeof(mLogLevel));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,
                           CONFIG_SUSPEND_IMPLEMENT_VERSION,
                           &mSuspendImplement);
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,
                           CONFIG_SUPPORT_HIGH_FIX_RATE, &mSupportHighFixRate);
    getValueByGroupAndName(root, COMMON_SETTING_NAME, CONFIG_DATA_PATH,
                           mDataPath, sizeof(mDataPath));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_RF_PATH_LOSS_AP, &mRfPathLossValueAp);
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_RF_PATH_LOSS_CP, &mRfPathLossValueCp);
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,CONFIG_DUMP_SATELLITE_INFO,
                           &mDumpSatelliteInfo);
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,
                           CONFIG_SUPPORT_QC_PLATFORM, &mSupportQcPlatform);
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_MAX_UART_LOG_NUM,
                           &(mUartLogConfig.maxFileNum));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_MAX_UART_LOG_SINGLE_SIZE,
                           &(mUartLogConfig.maxFileSize));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_MAX_TRACE_LOG_NUM,
                           &(mTraceLogConfig.maxFileNum));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_MAX_TRACE_LOG_SINGLE_SIZE,
                           &(mTraceLogConfig.maxFileSize));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_AUTO_SET_IO,
                           &(mAutoSetIo));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_LTE_FILTER,
                           &(mLteFilter));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_VDR,
                           &(mVdrEnable));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_HAS_AGNSS,
                           &(mHasAGnss));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_CUSTOMER_SUFFIX,
                           mCustomerSuffix, sizeof(mCustomerSuffix));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,
                           CONFIG_SUPPORT_M_PLATFORM, &(mSupportMPlatform));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,
                           CONFIG_QUERY_CUSTOMER_VERSION,
                           &(mQueryCustomerVersion));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,
                           CONFIG_COMPATIBLE_WITH_CUSTOMER_ALGORITHM,
                           &(mCompatibleCustomerAlgorithm));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME, CONFIG_DUMP_EPOCH_DATA,
                           &(mDumpEpochData));
    getValueByGroupAndName(root, ADVANCE_SETTING_NAME,
                           CONFIG_DUMP_NMEA_WHEN_LOCATION_INVALID,
                           &(mDumpNmeaWhenLocationInvalid));
    parserFirmwareDownloadConfig(root);
    parserUartSetting(root);
    parserInitLocationConfig(root);
    parserChipIoProperty(root);
    parserExtraLog(root, ADVANCE_SETTING_NAME, CONFIG_EVENT_LOG_NODE,
                   &(mEventLogConfig));
    parserKernelNode(root, ADVANCE_SETTING_NAME);
    parserNmeaFilter(root);
    dump();
    return true;
}
bool Configuration::outputDebugLog() { return mOutputDebugLog; }
bool Configuration::outputBinary() { return mOutputBinary; }
bool Configuration::epoEnable() { return mEpoEnable; }
const char *Configuration::epoVendorId() { return mEpoVendorID; }
const char *Configuration::epoProjectId() { return mEpoProjectName; }
const char *Configuration::epoDeviceId() { return mEpoDeviceID; }
int Configuration::debugPort() { return mDebugPort; }
int Configuration::powerGPSPort() { return mPowerGPSPort; }
bool Configuration::rawMeasurementSupport() { return mRawMeasurementSupport; }
Configuration::UartLogRecordConfig Configuration::uartLogSupport() {
    return mUartLogRecord;
}
bool Configuration::autoLockSleep() { return mAutoLockSleep; }
bool Configuration::autoDownloadEPO() { return mAutoDownloadEPO; }
bool Configuration::supportHighFixRate() { return mSupportHighFixRate; }
int Configuration::wakeDuration() { return mWakeDuration; }
bool Configuration::wakeupBeforeSendData() { return mWakeDuration; }
bool Configuration::traceLogRecord() { return mTraceLogRecord; }
int Configuration::suspendImplement() { return mSuspendImplement; }
const char *Configuration::logLevel() { return mLogLevel; }
const char *Configuration::dataPath() { return mDataPath; }
float Configuration::rfPathLossValueAp() { return mRfPathLossValueAp; }
float Configuration::rfPathLossValueCp() { return mRfPathLossValueCp; }
bool Configuration::dumpSatelliteInfo() { return mDumpSatelliteInfo; }
bool Configuration::supportQcPlatform() { return mSupportQcPlatform; }
bool Configuration::autoSetIo() { return mAutoSetIo; }
bool Configuration::vdrEnable() { return mVdrEnable; }
bool Configuration::lteFilter() { return mLteFilter; }
bool Configuration::hasAGnss() { return mHasAGnss; }
const char *Configuration::customerSuffix() { return mCustomerSuffix; }
bool Configuration::supportMPlatform() { return mSupportMPlatform; }
bool Configuration::queryCustomerVersion() { return mQueryCustomerVersion; }
bool Configuration::compatibleWithCustomerAlgorithm() {
    return mCompatibleCustomerAlgorithm;
}
bool Configuration::dumpEpochData() { return mDumpEpochData; }
bool Configuration::dumpNmeaWhenInvalidLocation() {
    return mDumpNmeaWhenLocationInvalid;
}
const UartConfigNode &Configuration::uartConfig() { return mUartConfig; }
const LogConfigNode &Configuration::uartLogConfig() { return mUartLogConfig; }
const LogConfigNode &Configuration::traceLogConfig() { return mTraceLogConfig; }
const LogConfigNode &Configuration::eventLogConfig() { return mEventLogConfig; }
const Airoha::InitLocationConfiguration &Configuration::initLocation() {
    return mInitLocationConfig;
}
const Airoha::NmeaFilterConfiguration &Configuration::nmeaFilter() {
    return mNmeaFilterConfig;
}
const Airoha::DataIoProperty &Configuration::ioProperty() {
    return mIoProperty;
}
const Airoha::KernelNode &Configuration::kernelNode() { return mKernelNode; }
const FirmwareDownloadSetting &Configuration::firmwareDownloadSetting() {
    return mFirmwareDownloadSetting;
}
bool Configuration::getValueByGroupAndName(XMLElement *ele, const char *catagory,
                                               const char *name, char *dst, size_t cap) {
    XMLElement *commonSetting = ele->FirstChildElement(catagory);
    if (!commonSetting) {
        LOG_E("Failed to get common setting: %s, %s", catagory, name);
        return false;
    }
    XMLElement *commonChildEle = commonSetting->FirstChildElement(name);
    if (!commonChildEle) {
        LOG_E("Failed to get child setting  %s, %s", catagory, name);
        return false;
    }
    strncpy(dst, commonChildEle->GetText(), cap - 1);
    return true;
}
bool Configuration::getValueByGroupAndName(XMLElement *ele, const char *catagory,
                            const char *name, bool *dst) {
    XMLElement *commonSetting = ele->FirstChildElement(catagory);
    if (!commonSetting) {
        LOG_E("Failed to get common setting: %s, %s", catagory, name);
        return false;
    }
    XMLElement *commonChildEle = commonSetting->FirstChildElement(name);
    if (!commonChildEle) {
        LOG_E("Failed to get child setting  %s, %s", catagory, name);
        return false;
    }
    *dst = toBool(commonChildEle->GetText(), false);
    return true;
}
bool Configuration::getValueByGroupAndName(XMLElement *ele, const char *catagory,
                            const char *name, int *dst) {
    XMLElement *commonSetting = ele->FirstChildElement(catagory);
    if (!commonSetting) {
        LOG_E("Failed to get common setting: %s, %s", catagory, name);
        return false;
    }
    XMLElement *commonChildEle = commonSetting->FirstChildElement(name);
    if (!commonChildEle) {
        LOG_E("Failed to get child setting  %s, %s", catagory, name);
        return false;
    }
    *dst = atoi(commonChildEle->GetText());
    return true;
}
bool Configuration::getValueByGroupAndName(XMLElement *ele, const char *catagory,
                                const char *name, float *dst) {
    XMLElement *commonSetting = ele->FirstChildElement(catagory);
    if (!commonSetting) {
        LOG_E("Failed to get common setting: %s, %s", catagory, name);
        return false;
    }
    XMLElement *commonChildEle = commonSetting->FirstChildElement(name);
    if (!commonChildEle) {
        LOG_E("Failed to get child setting  %s, %s", catagory, name);
        return false;
    }
    *dst = atof(commonChildEle->GetText());
    return true;
}
bool Configuration::toBool(const char *value, bool defaultValue) {
    if (!value) {
        return defaultValue;
    }
    const char *boolTrue[] = {"TRUE", "true"};
    const char *boolFalse[] = {"FALSE", "false"};
    for (size_t i = 0; i < sizeof(boolTrue) / sizeof(const char *); i++) {
        if (strcmp(value, boolTrue[i]) == 0) {
            return true;
        }
    }
    for (size_t i = 0; i < sizeof(boolFalse) / sizeof(const char *); i++) {
        if (strcmp(value, boolFalse[i]) == 0) {
            return false;
        }
    }
    LOG_W("toBool: %s not a key in true or false", value);
    return defaultValue;
}
void Configuration::parserFirmwareDownloadConfig(XMLElement *ele) {
    LOG_D("parser firmware download config");
    XMLElement *advSetting = ele->FirstChildElement(ADVANCE_SETTING_NAME);
    if (!advSetting) {
        LOG_E("Failed to get adv setting");
        return;
    }
    XMLElement *firmwareCfg =
        advSetting->FirstChildElement(CONFIG_FIRMWARE_DOWNLOAD);
    if (!firmwareCfg) {
        LOG_E("Failed to get fw dl setting");
        return;
    }
    const char *downloadSwitch = firmwareCfg->GetText();
    if (!downloadSwitch) {
        LOG_E("fail to get download switch setting");
        return;
    }
    mFirmwareDownloadSetting.download = toBool(downloadSwitch, true);
    const char *triggerMethod =
        firmwareCfg->Attribute(CONFIG_FIRMWARE_DOWNLOAD_TRIGGER_METHOD);
    if (!triggerMethod) {
        return;
    }
    if (strcmp(triggerMethod, "or") == 0 || strcmp(triggerMethod, "OR") == 0) {
        mFirmwareDownloadSetting.triggerMethod =
            FirmwareDownloadSetting::TRIGGER_OR;
    } else if (strcmp(triggerMethod, "and") == 0 ||
               strcmp(triggerMethod, "AND") == 0) {
        mFirmwareDownloadSetting.triggerMethod =
            FirmwareDownloadSetting::TRIGGER_AND;
    } else {
        LOG_E("trigger method invalid, use default value");
        return;
    }
    mFirmwareDownloadSetting.chipFirmwareChange = firmwareCfg->BoolAttribute(
        CONFIG_FIRMWARE_DOWNLOAD_CHIP_FIRMWARE_CHANGE, false);
    mFirmwareDownloadSetting.backupFileChange = firmwareCfg->BoolAttribute(
        CONFIG_FIRMWARE_DOWNLOAD_BACKUP_FILE_CHANGE, true);
    mFirmwareDownloadSetting.retryTimes =
        firmwareCfg->IntAttribute(CONFIG_FIRMWARE_DOWNLOAD_RETRY_TIMES, 0);
    LOG_D("parser firmware download config done");
    return;
}
void Configuration::parserUartSetting(XMLElement *ele) {
    XMLElement *commonSetting = ele->FirstChildElement(COMMON_SETTING_NAME);
    if (!commonSetting) {
        LOG_E("Failed to get common setting");
        return;
    }
    XMLElement *uartCfg =
        commonSetting->FirstChildElement("uart");
    if (!uartCfg) {
        LOG_E("Failed to get uart node");
        return;
    }
    XMLElement *nameNode = uartCfg->FirstChildElement("name");
    if (!nameNode) {
        LOG_E("Failed to get name item");
        return;
    }
    strncpy(mUartConfig.nodeName, nameNode->GetText(),
            sizeof(mUartConfig.nodeName));
    XMLElement *flowcontrolNode = uartCfg->FirstChildElement("flow_control");
    if (!flowcontrolNode) {
        LOG_E("Failed to get flowcontrol node");
        return;
    }
    if (strncmp(flowcontrolNode->GetText(), "none", strlen("none")) == 0) {
        mUartConfig.fc = UartConfigNode::NONE_FLOW_CONTROL;
    } else if (strcmp(flowcontrolNode->GetText(), "software") == 0) {
        mUartConfig.fc = UartConfigNode::SOFTWARE_FLOW_CONTROL;
    } else if (strcmp(flowcontrolNode->GetText(), "hardware") == 0) {
        mUartConfig.fc = UartConfigNode::HARDWARE_FLOW_CONTROL;
    } else if (strcmp(flowcontrolNode->GetText(), "software_ext") == 0) {
        mUartConfig.fc = UartConfigNode::SOFTWARE_FLOW_CONTROL_EXT;
    } else {
        LOG_E("Flow control value incorrect: %s", flowcontrolNode->GetText());
    }
    LOG_D("parser uart config finish.");
    return;
}
void Configuration::parserInitLocationConfig(XMLElement *ele) {
    LOG_D("parser init location config");
    XMLElement *advSetting = ele->FirstChildElement(ADVANCE_SETTING_NAME);
    if (!advSetting) {
        LOG_E("Failed to get adv setting");
        return;
    }
    XMLElement *initLocCfg =
        advSetting->FirstChildElement(CONFIG_INITIAL_LOCATION_NODE);
    if (!initLocCfg) {
        LOG_E("Failed to get initLocCfg node");
        return;
    }
    XMLElement *latitude =
        initLocCfg->FirstChildElement(CONFIG_INITIAL_LOCATION_LATITUDE);
    XMLElement *longitude =
        initLocCfg->FirstChildElement(CONFIG_INITIAL_LOCATION_LONGITUDE);
    XMLElement *altitude =
        initLocCfg->FirstChildElement(CONFIG_INITIAL_LOCATION_ALTITUDE);
    if (!latitude || !longitude || !altitude) {
        LOG_E("Failed to get location field: %p|%p|%p", latitude, longitude,
              altitude);
        return;
    }
    mInitLocationConfig.enable = initLocCfg->BoolAttribute("enable", false);
    mInitLocationConfig.latitude = latitude->DoubleText(0.0);
    mInitLocationConfig.longitude = longitude->DoubleText(0.0);
    mInitLocationConfig.altitude = altitude->DoubleText(0.0);
    LOG_D("parser initial location config finish.");
}
void Configuration::parserNmeaFilter(XMLElement *ele) {
    LOG_D("parser nmea filter config");
    XMLElement *advSetting = ele->FirstChildElement(ADVANCE_SETTING_NAME);
    if (!advSetting) {
        LOG_E("Failed to get adv setting");
        return;
    }
    XMLElement *nmeaFilterCfg =
        advSetting->FirstChildElement(CONFIG_NMEA_FILTER);
    if (!nmeaFilterCfg) {
        LOG_E("Failed to get nmeaFilter node");
        return;
    }
    
    mNmeaFilterConfig.enable = nmeaFilterCfg->BoolAttribute("enable", false);
    std::string cfgStr = nmeaFilterCfg->GetText();
    std::stringstream ss(cfgStr);
    std::string token;
    while (std::getline(ss, token, '|')) {
        if(!token.empty()) {
            mNmeaFilterConfig.nmeaTalkers.push_back(token);
        }
    }
    LOG_D("parser nmea filter config finish.");

    #if 1
    LOG_D("NMEA Filter Config: enable:%d, nmeaTalkers:", mNmeaFilterConfig.enable);
    for (auto &token : mNmeaFilterConfig.nmeaTalkers) {
        LOG_D("%s", token.c_str());
    }
    #endif
}
void Configuration::parserChipIoProperty(XMLElement *ele) {
    LOG_D("parser chip io");
    XMLElement *advSetting = ele->FirstChildElement(ADVANCE_SETTING_NAME);
    if (!advSetting) {
        LOG_E("Failed to get adv setting");
        return;
    }
    XMLElement *ioConfig =
        advSetting->FirstChildElement(CONFIG_CHIP_DATA_IO_TYPE);
    if (!ioConfig) {
        LOG_E("Failed to get io property node");
        return;
    }
    XMLElement *portType = ioConfig->FirstChildElement("port_type");
    XMLElement *portIndex = ioConfig->FirstChildElement("port_index");
    if (!portType || !portIndex) {
        LOG_E("Failed to get io field: %p|%p", portType, portIndex);
        return;
    }
    this->mIoProperty.ioPortType = portType->IntText();
    this->mIoProperty.ioPortIndex = portIndex->IntText();
}
void Configuration::parserUartLogRecord(XMLElement *ele) {
    LOG_D("parser uart record");
    XMLElement *setting = ele->FirstChildElement(COMMON_SETTING_NAME);
    if (!setting) {
        LOG_E("Failed to get COMMON_SETTING_NAME");
        return;
    }
    XMLElement *uConfig = setting->FirstChildElement(CONFIG_UART_LOG_REPORT);
    if (!uConfig) {
        return;
    }
    const char *cfgValue = uConfig->GetText();
    LOG_D("uart_record=%s", cfgValue);
    if (strcmp(cfgValue, "partial") == 0) {
        this->mUartLogRecord = UART_LOG_RECORD_PARTIAL;
    } else if (strcmp(cfgValue, "true") == 0) {
        this->mUartLogRecord = UART_LOG_RECORD_FULL;
    } else if (strcmp(cfgValue, "false") == 0) {
        this->mUartLogRecord = UART_LOG_RECORD_DISABLED;
    } else {
        this->mUartLogRecord = UART_LOG_RECORD_DISABLED;
    }
    LOG_D("uart record: %d", this->mUartLogRecord);
}
void Configuration::parserExtraLog(XMLElement *eleRoot, const char *catagory,
                                   const char *name, LogConfigNode *node) {
    XMLElement *setting = eleRoot->FirstChildElement(catagory);
    if (!setting) {
        LOG_E("Failed to get catagory: %s->%s", catagory, name);
        return;
    }
    XMLElement *nodeE = setting->FirstChildElement(name);
    if (!nodeE) {
        LOG_E("Failed to get name: %s->%s", catagory, name);
        return;
    }
    bool enable = nodeE->BoolAttribute("enable");
    int fileNum = nodeE->IntAttribute("num");
    int size = nodeE->IntAttribute("size");
    LOG_D("extra log: %s->%s, enable=%d, filenum=%d, size=%d", catagory, name,
          enable, fileNum, size);
    node->enable = enable;
    node->maxFileNum = fileNum;
    node->maxFileSize = size;
    return;
}
std::map<std::string, std::string> Configuration::getValueMap(
    XMLElement *node) {
    if (node == nullptr) {
        return std::map<std::string, std::string>();
    }
    std::map<std::string, std::string> map;
    XMLElement *first = node->FirstChildElement();
    for (; first != nullptr; first = first->NextSiblingElement()) {
        LOG_D("Get Value Map: [%s]", first->Name());
        if (strncmp(first->Name(), "value", strlen("value")) != 0) {
            LOG_E("VS: [%s] vs [%s] [%d]", first->Name(), "value",
                  strncmp(first->Name(), "value", strlen("value")));
        }
        const char *type = first->Attribute("type");
        const char *value = first->GetText();
        // LOG_D("Get Value Map: [%s]=[%s]", type, value);
        if (type && value) {
            map[type] = value;
        }
    }
    return map;
}
void Configuration::parserKernelNode(XMLElement *root, const char *catagory) {
    LOG_D("parser kernel node...");
    XMLElement *setting = root->FirstChildElement(catagory);
    mKernelNode.enable = false;
    if (!setting) {
        LOG_E("%s Failed to get catagory: %s", __FUNCTION__, catagory);
        return;
    }
    XMLElement *kernelNode = setting->FirstChildElement("kernel_node");
    if (!kernelNode) {
        LOG_E("Failed to get Kernel Node, return to default");
        return;
    }
    XMLElement *power = kernelNode->FirstChildElement("power");
    if (!power) {
        LOG_E("Failed to get power Node, return to default");
    }
    std::map<std::string, std::string> valueMap = getValueMap(power);
    if (valueMap["node_name"] == "" || valueMap["open"] == "" ||
        valueMap["close"] == "") {
        LOG_E("Failed to get power Node Value(Get=%zu), return to default",
              valueMap.size());
        return;
    }
    strncpy(mKernelNode.powerNodeName, valueMap["node_name"].c_str(),
            sizeof(mKernelNode.powerNodeName));
    strncpy(mKernelNode.powerOpenCmd, valueMap["open"].c_str(),
            sizeof(mKernelNode.powerOpenCmd));
    strncpy(mKernelNode.powerCloseCmd, valueMap["close"].c_str(),
            sizeof(mKernelNode.powerCloseCmd));
    mKernelNode.enable = true;
}
void Configuration::dump() {
    LOG_D("=======  Config Dump ======");
    LOG_D("data path: %s", Configuration::INSTANCE->dataPath());
    LOG_D("rf path loss(CP)=%f", Configuration::INSTANCE->rfPathLossValueCp());
    LOG_D("rf path loss(AP)=%f", Configuration::INSTANCE->rfPathLossValueAp());
    LOG_D("Uart Config: name=[%s], flowcontrol=[%d]",
          Configuration::INSTANCE->uartConfig().nodeName,
          (int)Configuration::INSTANCE->uartConfig().fc);
    LOG_D("support qc platform=%d", Configuration::INSTANCE->supportQcPlatform());
    LOG_D("uart log config: max_num=%d, max_size=%d",
          Configuration::INSTANCE->uartLogConfig().maxFileNum,
          Configuration::INSTANCE->uartLogConfig().maxFileSize);
    LOG_D("trace log config: max_num=%d, max_size=%d",
          Configuration::INSTANCE->traceLogConfig().maxFileNum,
          Configuration::INSTANCE->traceLogConfig().maxFileSize);
    LOG_D("init loc: enable=%d, lat=%f, lng=%f, alt=%f",
          Configuration::INSTANCE->initLocation().enable,
          Configuration::INSTANCE->initLocation().latitude,
          Configuration::INSTANCE->initLocation().longitude,
          Configuration::INSTANCE->initLocation().altitude);
    LOG_D("io property: port_type: %d, port_index: %d , auto_set_io=%d",
          Configuration::INSTANCE->ioProperty().ioPortType,
          Configuration::INSTANCE->ioProperty().ioPortIndex,
          Configuration::INSTANCE->autoSetIo());
    LOG_D("=======  Config Dump Finish ======");
}
bool NmeaFilterConfiguration::contains(const char *nmea) const {
    std::string nmeaString = nmea;
    if (std::find(nmeaTalkers.begin(), nmeaTalkers.end(), nmeaString) !=
        nmeaTalkers.end()) {
        return true;
    }
    return false;
}
