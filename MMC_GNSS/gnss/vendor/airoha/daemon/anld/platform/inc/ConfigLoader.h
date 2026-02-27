/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

#ifndef _CONFIG_LOADER_H
#define _CONFIG_LOADER_H
#include <map>
#include <string>
#include "tinyxml2.h"
#include <vector>
#include <sstream>
using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
namespace Airoha {
struct FirmwareDownloadSetting {
    enum TriggerMethod {
        TRIGGER_OR,
        TRIGGER_AND,
    };
    TriggerMethod triggerMethod = TRIGGER_OR;
    bool chipFirmwareChange = false;
    bool backupFileChange = true;
    bool download = true;
    int retryTimes = 0;
};
struct UartConfigNode {
    enum FlowControl {
        NONE_FLOW_CONTROL,
        SOFTWARE_FLOW_CONTROL,
        HARDWARE_FLOW_CONTROL,
        SOFTWARE_FLOW_CONTROL_EXT,
    };
    char nodeName[64] = "/dev/ttyUSB0";
    FlowControl fc = SOFTWARE_FLOW_CONTROL;
};
struct LogConfigNode {
    bool enable = false;
    int maxFileNum = 10;
    int maxFileSize = 5 * 1024 * 1024;
};
struct AdvanceSettingList {
    int suspendImplementType;
    FirmwareDownloadSetting firmwareDownload;
    std::string toString();
};
struct InitLocationConfiguration {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    bool enable = false;
};
struct NmeaFilterConfiguration {
    std::vector<std::string> nmeaTalkers;
    bool enable = false;
    bool contains(const char *nmea) const;
};
struct DataIoProperty {
    int ioPortType = 0;
    int ioPortIndex = 0;
};
#define KERNEL_NODE_NAME_MAX_LENGTH 128
struct KernelNode {
    bool enable;
    char powerNodeName[KERNEL_NODE_NAME_MAX_LENGTH];
    char powerOpenCmd[10];
    char powerCloseCmd[10];
    char wakeupNodeName[KERNEL_NODE_NAME_MAX_LENGTH];
    char wakeupCmd[10];
    char interruptNodeName[KERNEL_NODE_NAME_MAX_LENGTH];
};
static_assert(sizeof(DataIoProperty) == 8, "Wrong sizeof(DataIoProperty)");
static_assert(offsetof(DataIoProperty, ioPortType) == 0,
              "offsetof(DataIoProperty, ioPortType)");
static_assert(offsetof(DataIoProperty, ioPortIndex) == 4,
              "offsetof(DataIoProperty, ioPortType)");
#if 0
class ConfigLoader{
public:

struct ConfigList{
    ConfigList();
    bool outputDebugLog;
    bool outputBinary;
    bool epoEnable;
    std::string epoVendorID;
    std::string epoProjectName;
    std::string epoDeviceID;
    std::string dataPath;
    int debugPort;
    int powerGPSPort;
    bool rawMeasurementSupport;
    bool uartLogRecord;
    bool autoLockSleep;
    bool autoDownloadEPO;
    bool supportHighFixRate;
    int wakeDuration;
    bool wakeupBeforeSendData;
    bool traceLogRecord;
    std::string logLevel;
    std::string toString();
};
    int loadConfigFile(std::string);
    int loadConfigFile(const char *filename);
    std::string getSetting(std::string group, std::string name);

    void printAllSetting();
    const ConfigList* getConfigList() const;
    const AdvanceSettingList& getAdvanceConfig() const;
    bool isValid() const;

 private:
    XMLDocument xmlLoader;
    void printSettingByNama(const char * catagory, const char *name);
    const char *getSettingTextByName(const char * catagory, const char *name);
    void toConfigList();
    void toConfigListValue(const char * catagory, const char *key, std::string &valueToBeSet);
    void toConfigListValue(const char * catagory, const char *key, bool &valueToBeSet);
    void toConfigListValue(const char * catagory, const char *key, int &valueToBeSet);
    bool toBool(const char *str, bool defaultValue);
    void parserFirmwareDownloadConfig();
    ConfigList configList;
    AdvanceSettingList mAdvanceSetting;
    bool mValid;
};
#endif
class Configuration {
 public:
    enum UartLogRecordConfig {
        UART_LOG_RECORD_DISABLED,
        UART_LOG_RECORD_PARTIAL,
        UART_LOG_RECORD_FULL,
    };
    static Configuration *INSTANCE;
    bool loadConfigureFile(const char *filename);
    bool outputDebugLog();
    bool outputBinary();
    bool epoEnable();
    const char *epoVendorId();
    const char *epoProjectId();
    const char *epoDeviceId();
    const char *dataPath();
    int debugPort();
    int powerGPSPort();
    bool rawMeasurementSupport();
    bool supportHighFixRate();
    UartLogRecordConfig uartLogSupport();
    bool autoLockSleep();
    bool autoDownloadEPO();
    int wakeDuration();
    bool wakeupBeforeSendData();
    bool traceLogRecord();
    int suspendImplement();
    const char *logLevel();
    const FirmwareDownloadSetting &firmwareDownloadSetting();
    float rfPathLossValueCp();
    float rfPathLossValueAp();
    bool dumpSatelliteInfo();
    bool supportQcPlatform();
    const UartConfigNode &uartConfig();
    const LogConfigNode &uartLogConfig();
    const LogConfigNode &traceLogConfig();
    const LogConfigNode &eventLogConfig();
    const InitLocationConfiguration &initLocation();
    const NmeaFilterConfiguration &nmeaFilter();
    const DataIoProperty &ioProperty();
    const KernelNode &kernelNode();
    bool autoSetIo();
    bool vdrEnable();
    bool lteFilter();
    bool hasAGnss();
    const char *customerSuffix();
    bool supportMPlatform();
    bool queryCustomerVersion();
    bool compatibleWithCustomerAlgorithm();
    bool dumpEpochData();
    bool dumpNmeaWhenInvalidLocation();

 private:
    Configuration();
    Configuration(const Configuration &) = delete;
    Configuration operator=(const Configuration &) = delete;
    bool initConfig(XMLElement *);
    bool getValueByGroupAndName(XMLElement *ele, const char *catagory,
                                const char *name, char *dst, size_t cap);
    bool getValueByGroupAndName(XMLElement *ele, const char *catagory,
                                const char *name, bool *dst);
    bool getValueByGroupAndName(XMLElement *ele, const char *catagory,
                                const char *name, int *dst);
    bool getValueByGroupAndName(XMLElement *ele, const char *catagory,
                                const char *name, float *dst);
    void parserFirmwareDownloadConfig(XMLElement *ele);
    void parserUartSetting(XMLElement *ele);
    void parserInitLocationConfig(XMLElement *ele);
    void parserNmeaFilter(XMLElement *ele);
    void parserChipIoProperty(XMLElement *ele);
    void parserUartLogRecord(XMLElement *ele);
    void parserExtraLog(XMLElement *eleRoot, const char *catagory,
                        const char *name, LogConfigNode *node);
    std::map<std::string, std::string> getValueMap(XMLElement *node);
    void parserKernelNode(XMLElement *eleRoot, const char *catagory);
    bool toBool(const char *value, bool defaultValue);
    void dump();
    // Config List
    // DO NOT USE CLASS HERE such as "std::string"
    bool mOutputDebugLog = true;
    bool mOutputBinary = true;
    bool mEpoEnable = false;
    char mEpoVendorID[64] = "xxx";
    char mEpoProjectName[256] = "xxx";
    char mEpoDeviceID[256] = "xxx";
    char mDataPath[256] = "/data/vendor/airoha/";
    int mDebugPort = -1;
    int mPowerGPSPort = -1;
    bool mRawMeasurementSupport = true;
    UartLogRecordConfig mUartLogRecord = UART_LOG_RECORD_DISABLED;
    bool mAutoLockSleep = false;
    bool mAutoDownloadEPO = false;
    bool mSupportHighFixRate = false;
    int mWakeDuration = 1000;
    bool mWakeupBeforeSendData = true;
    bool mTraceLogRecord = false;
    char mLogLevel[10] = "debug";
    int mSuspendImplement = 0;
    float mRfPathLossValueAp = 0;
    float mRfPathLossValueCp = 0;
    bool mDumpSatelliteInfo = false;
    bool mSupportQcPlatform = false;
    bool mAutoSetIo = false;
    FirmwareDownloadSetting mFirmwareDownloadSetting;
    UartConfigNode mUartConfig;
    LogConfigNode mUartLogConfig;
    LogConfigNode mTraceLogConfig;
    LogConfigNode mEventLogConfig;
    InitLocationConfiguration mInitLocationConfig;
    NmeaFilterConfiguration mNmeaFilterConfig;
    DataIoProperty mIoProperty;
    KernelNode mKernelNode;
    bool mVdrEnable = false;
    bool mLteFilter = false;
    bool mHasAGnss = false;
    char mCustomerSuffix[32] = {0};
    bool mSupportMPlatform = false;
    bool mQueryCustomerVersion = false;
    bool mCompatibleCustomerAlgorithm = false;
    bool mDumpEpochData = false;
    bool mDumpNmeaWhenLocationInvalid = false;
};
}  // namespace Airoha
#endif