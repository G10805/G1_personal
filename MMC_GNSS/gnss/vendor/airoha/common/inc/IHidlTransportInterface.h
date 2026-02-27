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
#pragma once
#include <time.h>
#include <mutex>
#include <thread>
#include "airo_gps.h"
#include "anld_status.h"
#include "bufferable.h"
#include "hal_anld_message.h"
#include "timer_util.h"
using airoha::Bufferable;
using airoha::Tristate;
using Airoha::Gnss::NavNmea;
using Airoha::Gnss::NavTime_t;
using Airoha::Gnss::PreAGnssDataConnectionConfig;
using Airoha::Gnss::PreAGnssType;
using Airoha::Gnss::PreBatchingOptions;
using Airoha::Gnss::PreDebugData;
using Airoha::Gnss::PreGeofenceData;
using Airoha::Gnss::PreGnssAidingData;
using Airoha::Gnss::PreGnssCapabilities;
using Airoha::Gnss::PreGnssData;
using Airoha::Gnss::PreGnssGeofenceconfig;
using Airoha::Gnss::PreGnssLocation;
using Airoha::Gnss::PreGnssMeasurementOptions;
using Airoha::Gnss::PreGnssNavigationMessage;
using Airoha::Gnss::PreGnssPosisionPreference;
using Airoha::Gnss::PreGnssSvStatus;
using Airoha::Gnss::PreNfwNotification;
using Airoha::Gnss::PreNfwProtocolStack;
using Airoha::Gnss::PreSetIdFlags;
using Airoha::IPC::HardwareVersion;
using Airoha::IPC::msg_module_id_t;
using Airoha::Status::anld_status_t;
namespace airoha {
class IHidlTransportInterface : public Bufferable {
 public:
    IHidlTransportInterface(const char *abstractSocketName);
    virtual ~IHidlTransportInterface();
    bool startTransaction();
    bool stopTransaction();
    // Public function for general interface
    anld_status_t sendSetId(int type, const char *setid);
    anld_status_t sendRefLocation(int type, int mcc, int mnc, int lac, int cid);
    anld_status_t sendDataConnOpen(const char *apn);
    anld_status_t sendDataConnOpenIpType(const char *apn, int ip_type,
                                         bool network_handle_valid,
                                         uint64_t network_handle);
    anld_status_t sendDataConnFailed(void);
    anld_status_t sendDataConnClose(void);
    anld_status_t sendSetServer(int type, int port, const char *hostname);
    anld_status_t sendUpdateNetworkAvailability(int available, const char *apn);
    anld_status_t sendUpdateNetworkState(int connected, int type, int roaming);
    anld_status_t sendUpdateNetworkStateExt(int64_t networkHandle,
                                            bool isConnected,
                                            int32_t capabilities,
                                            const char *apn);
    anld_status_t sendNiRespond(int session_id, int user_response);
    anld_status_t sendNiMessage(const void *data, size_t length);
    anld_status_t sendAddGeofence(PreGnssGeofenceconfig *data);
    anld_status_t sendPauseGeofence(int32_t *geofenceId);
    anld_status_t sendResumeGeofence(int32_t *geofenceId);
    anld_status_t sendRemoveGeofence(int32_t *geofenceId);
    anld_status_t sendInjectTime(NavTime_t *navTime);
    anld_status_t sendInjectLocation(PreGnssLocation *location);
    anld_status_t sendDeleteAidingData(PreGnssAidingData *);
    anld_status_t sendPositionConfiguration(PreGnssPosisionPreference *);
    // batching
    anld_status_t sendBatchingInit();
    anld_status_t sendBatchingStart(PreBatchingOptions *);
    anld_status_t sendBatchingStop();
    anld_status_t sendBatchingFlush();
    anld_status_t sendBatchingCleanup();
    anld_status_t sendMeasurementUpdateRequest(
        const PreGnssMeasurementOptions &options);
    anld_status_t sendClearMeasurementRequest();
    anld_status_t sendGnssLocationStart();
    anld_status_t sendGnssLocationStop();
    anld_status_t sendGnssLocationStartOnly();
    anld_status_t sendGnssLocationStopOnly();
    anld_status_t sendGnssLocationInit();
    anld_status_t sendGnssLocationCleanup();
    anld_status_t sendNavigationMessageStart();
    anld_status_t sendNavigationMessageStop();
    anld_status_t sendPsdsSetCallback();
    anld_status_t closePsds();
    anld_status_t sendVisibilityControlProxyApps(const void *parcelData,
                                                 size_t length);
    anld_status_t sendStartNmea();
    anld_status_t sendStopNmea();
    anld_status_t sendStartSvStatus();
    anld_status_t sendStopSvStatus();
    anld_status_t sendExtData(const void *data, size_t length);
    void lock();
    void unlock();
    std::recursive_mutex &getLock();

 protected:
    // ==== Function For callback start ===
    virtual int onSyncRequest() = 0;
    virtual void onConnected() = 0;
    virtual void onDisconnected() = 0;
    virtual void onHardwareVersionReport(const HardwareVersion *version) = 0;
    virtual void onAGpsSetIdRequest(PreSetIdFlags flags) = 0;
    virtual void onAGpsSetRefLocationRequest() = 0;
    virtual void onAGpsNiNotify() = 0;
    virtual void onAGpsDataConnectionRequest(PreAGnssType agnssType) = 0;
    virtual void onAGpsDataConnectionRequestv2(
        const PreAGnssDataConnectionConfig *config) = 0;
    virtual void onAGpsDataConnectionRelease(
        const PreAGnssDataConnectionConfig *config) = 0;
    virtual void onLocationOutput(const PreGnssLocation *location) = 0;
    virtual void onSvStatusOutput(const PreGnssSvStatus *svStatus) = 0;
    virtual void onNmeaOutput(const NavNmea *nmea) = 0;
    virtual void onGnssMeasurement(const PreGnssData *measurement) = 0;
    virtual void onGnssNavigationData(
        const PreGnssNavigationMessage *navigationMessage) = 0;
    virtual void onRequestTimeAiding() = 0;
    virtual void onRequestLocationAiding() = 0;
    virtual void onGnssGeofencingStatusOutput(
        const PreGeofenceData *geofenceData) = 0;
    virtual void onReportGnssCapabilities(
        const PreGnssCapabilities *capabilities) = 0;
    virtual void onBatchLocationReport(
        const std::vector<PreGnssLocation> &locations) = 0;
    virtual void onGnssDebugData(const PreDebugData *debugData) = 0;
    virtual void onNfwNotification(const PreNfwNotification &notification) = 0;
    virtual void onGnssStatusReport(bool on) = 0;
    // ==== Function For callback end ===
 private:
    const static int kMaxBufferSize = (32 * 1024);
    enum MessageType : uint32_t {
        MESSAGE_TYPE_UNKNOWN,
        MESSAGE_TYPE_START = 100,
        MESSAGE_TYPE_RECONNECT,
        MESSAGE_TYPE_EXIT = 5555,
    };
    struct Message {
        MessageType type = MESSAGE_TYPE_UNKNOWN;
        uint8_t payload[32];
    };
    /**
     * @brief Send internal message.
     *
     * @param type Message Type
     * @param data UNUSED
     * @param length UNUSED
     */
    void sendIlmMessage(MessageType type, const void *data, size_t length);
    static void sTransactionThreadCallback(void *argv);
    static void sReconnectTimerCallback(union sigval val);
    void syncRequestInternal();
    std::string mSocketName;
    int mEpollFd = -1;
    int mSocketFd = -1;
    int mSocketCtrl[2];
    Tristate mRequestPositionInit = Tristate::UNKNOWN;
    Tristate mRequestGnssStart = Tristate::UNKNOWN;
    Tristate mRequestMeasurement = Tristate::UNKNOWN;
    Tristate mRequestNmea = Tristate::UNKNOWN;
    Tristate mRequestSvStatus = Tristate::UNKNOWN;
    Tristate mPsdsEnable = Tristate::UNKNOWN;
    PreGnssPosisionPreference mRequestGnssPreference;
    PreGnssMeasurementOptions mRequestMeasurementOptions;
    std::thread mThread;
    timer_t mTimer = nullptr;
    std::recursive_mutex mMutex;
    int loop();
    int startup();
    int teardown();
    bool addFd(int fd);
    bool delFd(int fd);
    void startReconnectTimer(int timeoutMs);
    void stopReconnectTimer();
    int handleIlmMessage(const Message *msg);
    int connectServer();
    // void handleSocketMessage(const void *data, size_t length);
    void parserIpcData();
    void handleModuleMessage(msg_module_id_t moduleId, uint32_t messageId,
                             const uint8_t *data, size_t length);
    void handleModuleMessageSystem(msg_module_id_t moduleId, uint32_t messageId,
                                   const uint8_t *data, size_t length);
    void handleModuleMessageNavigation(msg_module_id_t moduleId,
                                       uint32_t messageId, const uint8_t *data,
                                       size_t length);
    void handleModuleMessagePower(msg_module_id_t moduleId, uint32_t messageId,
                                  const uint8_t *data, size_t length);
    void handleModuleMessageAGps(msg_module_id_t moduleId, uint32_t messageId,
                                 const uint8_t *data, size_t length);
    anld_status_t transmit(msg_module_id_t mod, uint32_t message_id,
                           const void *userdata, size_t len);
    anld_status_t registerClient();
};
}  // namespace airoha
template <typename Enum>
using air_bitfield = typename std::underlying_type<Enum>::type;
