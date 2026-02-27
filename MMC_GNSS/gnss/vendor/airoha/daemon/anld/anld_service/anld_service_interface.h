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
#ifndef ANLD_SERVICE_INTERFACE_H
#define ANLD_SERVICE_INTERFACE_H
#include <airoha_std.h>
#include <stdint.h>
#include <unistd.h>
#include <string>
#include "airo_gps.h"
#include "airoha_proprietary_message.h"
#include "anld_status.h"
using Airoha::Status::anld_status_t;
namespace Airoha {
enum AnldServiceMessage {
    // 3335=>Service
    GNSS_DATA_INPUT = 1,
    GNSS_CHIP_DATA_DOWNLOAD_FIRMWARE_SUCCESS = 2,
    GNSS_HOST_WAKEUP_GPIO24 = 3,
    // service => 3335
    GNSS_DATA_OUTPUT = 4,
    GNSS_CHIP_POWER_ON = 5,
    GNSS_CHIP_POWER_OFF = 6,
    GNSS_CHIP_RTC_WAKEUP = 7,
    GNSS_CHIP_WAKE_UP_GPIO26 = 8,
    /**
     * @brief we assume that GNSS_CHIP_DATA_CONNECTION_OPEN and
     * GNSS_CHIP_DATA_CONNECTION_CLOSE will be finish immediately, so no rsponse
     * use
     *
     */
    GNSS_CHIP_DATA_CONNECTION_OPEN = 9,
    GNSS_CHIP_DATA_CONNECTION_CLOSE = 10,
    HOST_DRIVER_WAKE_LOCK = 11,
    HOST_DRIVER_WAKE_UNLOCK = 12,
    HOST_POWER_SUSPEND_CALLBACK = 13,
    HOST_POWER_RESUME_CALLBACK = 14,
    // linux => service
    HOST_DRIVER_RECEIVED_WAKEUP_MESSAGE = 15,
    GNSS_CHIP_PREPARE_DATA_CONNECTION = 16,
    GNSS_CHIP_SUSPEND_DATA_CONNECTION = 17,
    ANLD_MESSAGE_NMEA_CALLBACK = 18,
    ANLD_MESSAGE_SEND_DATA = 19,
    FACTORY_MESSAGE = 20,
    // platform->service
    GNSS_EPO_FILE_LOCK = 24,
    // service->platform
    GNSS_EPO_FILE_LOCK_SUCCESS = 25,
    // platform->service
    GNSS_EPO_FILE_UNLOCK = 26,
    // service->platform
    GNSS_EPO_FILE_UNLOCK_SUCCESS = 27,
    /**
     * If platform download epo successfully, it can send this msg to anld
     * service. Then the service will dump all epo file msg Please Notice that
     * GPS+GLONASS (Q)EPO (e.g QG_R, EPO_GR_3_1) will be check twice
     */
    GNSS_EPO_FILE_CHECK_STATUS_REQ = 28,
    GNSS_EPO_FILE_CHECK_STATUS_RSP = 29,
    GNSS_CHIP_DATA_TRIGGER_DOWNLOAD_FIRMWARE = 30,
    GNSS_EPO_FILE_EXPIRE = 31,
    GNSS_EPO_FILE_UPDATE_FINISH = 32,
    GNSS_RELAY_PROPRIETARY_MESSAGE = 33,
    GNSS_CHIP_RESET = 34,
    ANLD_SERVICE_MSG_REQUEST_LOCATION_AIDING = 35,
    ANLD_SERVICE_MSG_INJECT_LOCATION_BY_PLATFORM = 36,
    ANLD_SERVICE_PREPARE_SESSION = 37,
    ANLD_SERVICE_ASSERT_MESSAGE_REPORT = 38,
    ANLD_SERVICE_MSG_DELETE_AIDING_DATA = 5000,
    ANLD_SERVICE_MSG_NETWORK_CONNECTED = 5001,
};
class IDataConnection;
class IAnldServiceHandle {
 public:
    // virtual ~IAnldServiceHandle(){
    //     if(dataConn){
    //         delete(dataConn);
    //     }
    // }
    virtual ~IAnldServiceHandle() {}
    virtual int onServiceMessage(AnldServiceMessage message, void *data,
                                 size_t len) = 0;
 private:
    // IDataConnection *dataConn;
};
struct ANLCustomerConfig {
    bool outputDebugLog;
    bool outputBinary;
    const char* epoVendorID;
    const char* epoProjectName;
    const char* epoDeviceID;
    const char* dataPath;
    bool rawMeasurementSupport;
    bool autoLockSleep;
    bool autoDLEpo;
    bool supportHighFixRate;
    int wakeDuration;
    bool wakeupBeforeSendData;
    int suspendImplementVersion;
};
int anldServiceInit();
int anldServiceDeinit();
int anldServiceSendStopSignal();
int anldRun();
int anldServiceRegisterHandler(IAnldServiceHandle *handle);
int anldSendMessage2Service(AnldServiceMessage message, const void *data,
                            size_t len);
int anldReportCriticalMessage(const char *message);
int anldReportFormatCriticalMessage(const char *message, ...);
/**
 * @brief set config to anld
 * This function should be call before anldServiceInit
 * @param config
 * @return int
 */
anld_status_t anldServiceSetConfig(const ANLCustomerConfig &config);
// class IDataConnection{
// };
}  // namespace Airoha
enum FactoryMessageID {
    FACTORY_SET_POWER_POWER_OFF,
    FACTORY_SET_POWER_POWER_ON,
    FACTORY_SET_POWER_DSP_ON,
    FACTORY_SET_SILENT_MODE,
    FACTORY_SET_NORMAL_MODE,
    FACTORY_STATUS_UPDATE,
};
enum FactoryStatus {
    FACTORY_STATUS_POWER_OFF,
    FACTORY_STATUS_POWER_ON,
    FACTORY_STATUS_DSP_ON,
};
struct FactoryMessage {
    FactoryMessageID msgid;
    FactoryStatus status;
    uint8_t factoryExtra[300];
    std::string toString();
};
enum AnldServiceEPOType {
    ANLD_QEPO_GPS,
    ANLD_QEPO_GR,
    ANLD_QEPO_GALILEO,
    ANLD_QEPO_BEIDOU,
    ANLD_EPO_3DAY_GPS,
    ANLD_EPO_3DAY_GR,
    ANLD_EPO_3DAY_GALILEO,
    ANLD_EPO_3DAY_BEIDOU,
};
enum AnldEpoDownloadRsp {
    ANLD_EPO_FILE_READY = 0,
    ANLD_EPO_FILE_DOWNLOAD_FAILED,
};
struct AnldEpoInfo {
    uint8_t constellation;
    uint8_t epoFileType;
};
struct AnldReadableMessage {
    char message[512];
};
struct EpoCheckStatusRspPayload {
    const static int kMaxFileNum = 12;
    int fileNum;
    struct {
        char filename[50];
        airoha::epo::EpoFileFilter filter;
        bool valid;
    } fileinfo[kMaxFileNum];
};

struct LocationDebug {
    enum ValidFlag {
        FLAG_FIX = 1 << 0,
    };
    double latitude;
    double longitude;
    double altitude;
    double clockDrift;
    float hacc;
    float vacc;
    uint32_t flags;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t second;
    uint8_t reserved1;
    uint16_t msecond;
    uint16_t reserved2;
};
static_assert(sizeof(LocationDebug) == 56, "Debug Location Size Error");
struct PropMessageRelayPayload {
    airoha::proprietary::ProprietaryMessageType type;
    const void *data;
    size_t length;
};
/**
 *
 * Porting Function for ANLD-Core.
 *
 */
namespace portable {
enum PowerStatus {
    POWER_STATUS_UNKNOWN,
    POWER_STATUS_POWER_OFF,
    POWER_STATUS_POWER_ON
};
PowerStatus getChipPowerStatus();
void powerOnPower();
void powerOffPower();
}  // namespace portable
#endif