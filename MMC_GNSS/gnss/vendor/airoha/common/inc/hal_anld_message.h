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
 
#ifndef HAL_ANLD_MESSAGE_H
#define HAL_ANLD_MESSAGE_H
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <functional>
#include <string>
#include "airoha_std.h"
#include "anld_status.h"
using namespace Airoha::Status;
using airoha::Tristate;
#define APP2ANLD_ANDROID_SOCKET "app_anld"
#define HIDL2ANLD_FD "/data/airoha_hidl_anld"
#define HIDL_ANLD_MSG_ALIGN_BIT 3
namespace Airoha{
namespace IPC{



typedef enum Anld_Module_ID : uint8_t {
    ANLD_MSG_MODULE_SYSTEM = 0,
    ANLD_MSG_MODULE_POWER = 1,
    ANLD_MSG_MODULE_EPO = 2,
    ANLD_MSG_MODULE_FIRMWARE = 3,
    ANLD_MSG_MODULE_SUPL = 4,
    ANLD_MSG_MODULE_TIMER = 5,
    ANLD_MSG_MODULE_NAVIGATION = 6,
    ANLD_MSG_MODULE_AGPS = 7,
    ANLD_MSG_MODULE_GEOFENCE = 8,
    ANLD_MSG_MODULE_APP = 9,
    ANLD_MSG_MODULE_DBGDATA = 10,
}msg_module_id_t;

#pragma pack(push,1)
typedef struct ANLD_MESSAGE_HDR{
    msg_module_id_t module_id;
    uint32_t msg_id;
    size_t payload_size;
} __attribute__((aligned(1))) anld_msg_hdr_t;
typedef struct ANLD_MESSAGE{
    anld_msg_hdr_t header;
    const void *userdata;
}__attribute__ ((aligned (1))) anld_message_t;
#pragma pack(pop)
typedef struct ANLD_MESSAGE_HDRv2 {
    uint8_t pre1;
    uint8_t pre2;
    uint32_t hash;
    msg_module_id_t module_id;
    uint32_t msg_id;
    uint32_t payload_size;
    uint32_t padding_size;
    uint8_t data[0];
} anld_msg_hdr_v2_t;
// Set data to 8_bit align
static_assert(offsetof(anld_msg_hdr_v2_t, data) == 24,
              "offsetof(anld_msg_hdr_v2_t, data) == 24 error");
static_assert(sizeof(anld_msg_hdr_v2_t) == 24,
              "sizeof(anld_msg_hdr_v2_t) == 24 error");
static_assert(sizeof(anld_msg_hdr_v2_t) % (1 << HIDL_ANLD_MSG_ALIGN_BIT) == 0,
              "sizeof(anld_msg_hdr_v2_t) align error");
#define INTERGRITY_HASH 0x5a5a6611
#define PREAMBLE_WORD_W1   0x55
#define PREAMBLE_WORD_W2   0x55
#define END_WORD_W1        0xAA
#define END_WORD_W2        0xAA
#define END_WORD_LENGTH      2
#define PREAMBLE_WORD_LENGTH 2
#define LENGTH_FIELD_LENGTH  (sizeof(size_t))
#define HIDL_ANLD_PACKET_CONTROL_SIZE (sizeof(size_t) + 4)

// --------- MSG BETWEEN GPS HIDL and ANLD --------
//  Preamble(2 bytes) | LENGTH(sizeof(size_t)) | DATA(n bytes) | ENDWORD (2 bytes)
//
// ---------
//
/* definition of direction */
#define DIRECTION_ANLD2HIDL         0x01
#define DIRECTION_HIDL2ANLD         0x02
/* definition of MODULE */


/* end of definition MODULE */

/*definition of message id */
/* ====== SYSTEM ======== */
typedef enum ANLD_SYSTEM_MESSAGE {
    ANLD_SYS_REGISTER,
    ANLD_SYS_DEREGISTER,
    ANLD_SYS_RESPONSE,
    ANLD_SYS_SYNC_REQUEST,
    ANLD_SYS_REQUEST_HIDL_SYNC,
    ANLD_SYS_HARDWARE_VERSION_REPORT,
} anld_system_message_id_t;
struct HardwareVersion {
    char version[128];
    char reserved[24];
};
#define ANLD_CLIENT_INVALID 0x0
#define ANLD_CLIENT_HIDL 0x53

#pragma pack(push,1)
#define ANLD_RESP_EXTRA_DATA_LEN 128
typedef struct ANLD_RESPONSE{
    msg_module_id_t mod_to_resp;
    uint32_t message_id_to_resp;
    anld_status_t status;
    uint8_t extra_data[ANLD_RESP_EXTRA_DATA_LEN];
}anld_response_t;

/** SYSTEM **/

/**
 * @brief anld system message payload
 * moduleid: client module id
 * name : client name , Optional
 * utc : utc time in second
 */
#define CLIENT_NAME_SIZE 6
typedef struct ANLD_SYS_MESSAGE_PAYLOAD{
    uint8_t client_moduleid;
    char name[CLIENT_NAME_SIZE];
    time_t utc;
}anld_sys_payload_t;
struct HidlRequestFlag {
    enum RequestFlag {
        GNSS_INIT = 1 << 0,
        GNSS_START = 1 << 1,
        MEASUREMENT_INIT = 1 << 2,
    };
    uint32_t flags;
};
#pragma pack(pop)
/* ====== POWER =========*/
typedef enum ANLD_POWER_MESSAGE {
    ANLD_POWER_MSG_POWER_ON,
    ANLD_POWER_MSG_HIDL_REQUEST_POWER_ON,
    ANLD_POWER_MSG_POWER_STATUS_POWER_ON,
    ANLD_POWER_MSG_POWER_OFF,
    ANLD_POWER_MSG_HIDL_REQUEST_POWER_OFF,
    ANLD_POWER_MSG_POWER_STATUS_POWER_OFF,
    ANLD_POWER_MSG_POWER_STATUS_DSP_ON,
    ANLD_POWER_MSG_POWER_STATUS_DSP_OFF,
    /**
     * @brief Use to request GPS DSP on, for Gnss::Start
     * 
     */
    ANLD_POWER_MSG_HIDL_REQUEST_DSP_ON,
    /**
     * @brief Use to request GPS DSP off, for Gnss::Stop
     * NOTICE: this means engine on and session off,
     * If you send this message when engine off, this message will cause chip raise its power to ENGING ON
     * If you want to Power off chip, just send ANLD_POWER_MSG_HIDL_REQUEST_POWER_OFF
     * 
     */
    ANLD_POWER_MSG_HIDL_REQUEST_DSP_OFF,
    ANLD_POWER_MSG_ACQUIRE_WAKEUP_LOCK,
    ANLD_POWER_MSG_RELEASE_WAKEUP_LOCK,

    

    
}anld_power_message_id_t;
/* ====== EPO =========*/
typedef enum ANLD_EPO_MESSAGE {
    ANLD_EPO_MSG_QUERY_FILE_STATE,
    ANLD_EPO_MSG_DOWNLOAD_TO_CHIP,
    ANLD_EPO_MSG_SEND_DIRECT_DATA,
    ANLD_EPO_MSG_REQUEST_EPO,         // This message is from anld to HIDL
    ANLD_EPO_MSG_AUTO_DOWNLOAD_TRIGGER,   // This message is send by HIDL to let anld autodownload epo to chip
}anld_epo_message_id_t;

struct EPOExtraData {
    size_t size;
    anld_epo_message_id_t msg_id;
    uint8_t *data;
    size_t data_len;
}__attribute__ ((aligned (1)));
#define QEPO_GPS_BIT                 (1 << 0)
#define QEPO_GR_BIT                  (1 << 1)
#define QEPO_GA_BIT                  (1 << 2)
#define QEPO_GB_BIT                  (1 << 3)
#define EPO_GPS_3_1_BIT              (1 << 4)
#define EPO_GPS_3_2_BIT              (1 << 5)
#define EPO_GPS_3_3_BIT              (1 << 6)
#define EPO_GR_3_1_BIT               (1 << 7)
#define EPO_GR_3_2_BIT               (1 << 8)
#define EPO_GR_3_3_BIT               (1 << 9)

struct EPOExtraQueryFile {
    uint32_t fileExistBit; // Show if file exists
    uint32_t fileValidBit; // The file can be used currently
    uint32_t fileFutureBit; // The file need to be use future
};


/* ====== FIRMWARE =========*/
typedef enum ANLD_FW_MESSAGE {
    ANLD_FW_MSG_CHECK_FIRMWARE,
    ANLD_FW_MSG_TRIGGER_DOWNLOAD,
    ANLD_FW_MSG_DL_FROM_INTERNET,        // later may be use
}anld_firmware_message_id_t;
/* ==== NAvigation==== */
typedef enum ANLD_Nav_MESSAGE {
    ANLD_NAV_NMEA_OUTPUT,      // struct NmeaInfo
    ANLD_NAV_LOCATION_OUTPUT,  // Pre SvInfo
    ANLD_NAV_SV_STATUS_OUTPUT,
    ANLD_NAV_PAIR_OUTPUT,         // const char*
    ANLD_NAV_INFORMATION_OUTPUT,  // NavInfo_t
    ANLD_NAV_REQUEST_TIME,
    ANLD_NAV_REQUEST_LOCATION,
    ANLD_NAV_INJECT_TIME,      // NavTime_t
    ANLD_NAV_INJECT_LOCATION,  // NavLocation_t
    ANLD_NAV_DELETE_AIDING_DATA,
    ANLD_RAW_MEASUREMENT_GNSSDATA,
    ANLD_NAV_GFESTATUS_OUTPUT,      // Pre GFE INFO
    ANLD_NAV_GFESCMDSTATUS_OUTPUT,  // Pre GFE INFO
    ANLD_NAVIGATION_MESSAGE_DATA,
    ANLD_NAVIGATION_REPORT_CAPABILITIES,  // PreGnssCapabilities
    // batching
    ANLD_NAV_BATCHING_LOCATION_OUTPUT,  // location_t
    ANLD_NAV_BATCHING_INIT,
    ANLD_NAV_BATCHING_START,  // PreBatchingOptions
    ANLD_NAV_BATCHING_STOP,
    ANLD_NAV_BATCHING_CLEANUP,
    ANLD_NAV_BATCHING_FLUSH,
    ANLD_NAV_GNSS_PREFERENCE,
    ANLD_NAV_GNSS_MEAS_START,
    ANLD_NAV_GNSS_MEAS_STOP,
    ANLD_NAV_GNSS_POSITION_START,
    ANLD_NAV_GNSS_POSITION_STOP,
    ANLD_NAV_GNSS_POSITION_INIT,
    ANLD_NAV_GNSS_POSITION_CLEANUP,
    ANLD_NAV_GNSS_NAVIGATION_START,
    ANLD_NAV_GNSS_NAVIGATION_STOP,
    ANLD_NAV_GNSS_DEBUG_DATA,
    ANLD_NAV_VISIBILITY_CONTROL_NOTIFY,
    ANLD_NAV_VISIBILITY_ENABLE_PROXY_APPS,
    ANLD_NAV_GNSS_START_NMEA,
    ANLD_NAV_GNSS_STOP_NMEA,
    ANLD_NAV_GNSS_START_SV_STATUS,
    ANLD_NAV_GNSS_STOP_SV_STATUS,
    ANLD_NAV_GNSS_START_ONLY_POSITION,
    ANLD_NAV_GNSS_STOP_ONLY_POSITION,
    ANLD_NAV_GNSS_PSDS_SET_CALLBACK,
    // HalRequestSyncPacket
    ANLD_NAV_GNSS_REQUEST_SYNC,
    ANLD_NAV_GNSS_SEND_EXTDATA,
} anld_nav_message_id_t;
typedef struct RequestExtra{

}request_extra_t;
typedef struct BatchingExtra{

}batching_extra_t;
/* ==== AGPS==== */
typedef enum ANLD_AGPS_MESSAGE {
    AGPS_ANLD_SET_ID_REQ,         //
    AGPS_ANLD_REF_LOC_REQ,        //
    AGPS_ANLD_DATA_CONN_REQ,      //
    AGPS_ANLD_DATA_CONN_REQ2,     //
    AGPS_ANLD_DATA_CONN_RELEASE,  //
    AGPS_ANLD_NI_NOTIFY,          //
    AGPS_ANLD_NI_NOTIFY2,         //
    ANLD_AGPS_SEND_SETID = 100,
    ANLD_AGPS_SEND_REFLOC,
    ANLD_AGPS_SEND_DATACONOPEN,
    ANLD_AGPS_SEND_DATA_CONN_OPEN_IP_TYPE,
    ANLD_AGPS_SEND_DATACONCLOSE,
    ANLD_AGPS_SEND_DATACONFAIL,
    ANLD_AGPS_SEND_SETSERVER,
    ANLD_AGPS_SEND_UPDATANETSTATES,
    ANLD_AGPS_SEND_UPDATANETSTATESAVAILABILITY,
    ANLD_AGPS_SEND_NIRESPOND,
    ANLD_AGPS_SEND_CLEANUP,
    ANLD_AGPS_SEND_CONFIGURATION_SET_SUPL_ES,
    ANLD_AGPS_SEND_CONFIGURATION_SET_SUPL_MODE,
    ANLD_AGPS_SEND_CONFIGURATION_SET_SUPL_VERSION,
    ANLD_AGPS_SEND_CONFIGURATION_SET_LPP_PROFILE,
    ANLD_AGPS_SEND_CONFIGURATION_SET_GNSS_POS_PROTOCOL_SELECT,
    ANLD_AGPS_SEND_CONFIGURATION_SET_GPS_LOCK,
    ANLD_AGPS_SEND_CONFIGURATION_SET_EMERGENCY_SUPL_PDN,
    ANLD_AGPS_SEND_CONFIGURATION_SET_SATELLITE_BLACKLIST,
    ANLD_AGPS_SEND_UPDATANETSTATES_EXT,
    HIDL_ANLD_SUPL_NI_MESSAGE,
} anld_agps_message_id_t;
/* ==== GEOFENCING==== */

typedef enum ANLD_GEOFENCE_MESSAGE {
    HAL2ANLD_GEOFENCE_ADD,       // 
    HAL2ANLD_GEOFENCE_PAUSE,
    HAL2ANLD_GEOFENCE_RESUME,
    HAL2ANLD_GEOFENCE_REMOVE,
}anld_geofencing_message_id_t;

/* APP */



/** ====== ANLD STATUS ==== */


#define ANLD_AGPS_MAX_BUFF_SIZE (64 * 1024)


typedef void(*decodeIPCCallback)(const uint8_t *data, size_t data_len); 
typedef void(*decodeModuleMsgCallback)(msg_module_id_t mid, uint32_t msgid, const uint8_t *data, size_t data_len); 
/**
 * @brief 
 * 
 * @param msgBuf 
 * @param len 
 * @param lastOffset 
 * @return anld_status_t 
 */
anld_status_t decode_anld_hidl_message(const uint8_t *msgBuf, size_t len, decodeIPCCallback, size_t *comsumeLength);
/**
 * @brief 
 * encode user defined message to packet that send to ANLD or HIDL \n
 * 
 * @param buffer  The buffer to store de data
 * @param buffer_len buffer length
 * @param userData user define data
 * @param userDataLen user define data len
 * @return anld_status_t return status
 */
anld_status_t encode_anld_hidl_message(uint8_t *buffer,size_t buffer_len, uint8_t *userData, size_t userDataLen, size_t *encodeLen);
anld_status_t encode_anld_hidl_message_v2(uint8_t *buffer, size_t bufferLen,
                                          anld_msg_hdr_v2_t *hdr,
                                          const void *userData,
                                          size_t *encodeLen);
anld_status_t decode_anld_hidl_message_v2(const uint8_t *buffer,
                                          size_t bufferLen,
                                          decodeModuleMsgCallback callback,
                                          size_t *comsume);
anld_status_t decode_anld_hidl_message_v2a(
    const uint8_t *buffer, size_t bufferLen,
    std::function<void(msg_module_id_t, uint32_t, const uint8_t *, size_t)>
        callback,
    size_t *comsume);
/**
 * @brief decode anld message payload to module id and msg id
 * 
 * @param msgBuf data need to be decode
 * @param len    data len
 * @param result decode result, needRelease will be set to true if the userdata pointer need to be released 
 * @return anld_status_t 
 */
anld_status_t decode_message_payload(const uint8_t *msgBuf, size_t len, decodeModuleMsgCallback callback);


/**
 * @brief encode data
 * 
 * @param msgBuf 
 * @param len 
 * @param source  encode source , if needRelease is true, then the function will call free(userData) when finish encode 
 * @return anld_status_t 
 */
anld_status_t encode_message_payload(uint8_t *msgBuf, size_t len, anld_message_t *source, size_t *total_len);
struct HalRequestSyncPacket {
    Tristate isLocationInit = Tristate::UNKNOWN;
    Tristate isLocationStart = Tristate::UNKNOWN;
    Tristate isMeasurementInit = Tristate::UNKNOWN;
    Tristate isNmeaStart = Tristate::UNKNOWN;
    Tristate isSvStatusStart = Tristate::UNKNOWN;
};
} // namespace IPC
} // namespace Airoha
#define LOG_LEVEL_DEBUG   0
#define LOG_LEVEL_INFO    1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR   3
typedef uint8_t debug_level_t;
extern void print_ipc_log(debug_level_t level,const char* format,...);
#endif
