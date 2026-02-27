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
#define LOG_TAG "GnssAidlImpl"
#include "GnssAidlImpl.h"
#include <assert.h>
#include <fcntl.h>
#include <memory.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <thread>
#include "AGnss.h"
#include "AGnssRil.h"
#include "Gnss.h"
#include "GnssBatching.h"
#include "GnssDebug.h"
#include "GnssGeofence.h"
#include "GnssMeasurementInterface.h"
#include "GnssNavigationMessageInterface.h"
#include "GnssVisibilityControl.h"
#include "air_parcel.h"
#include "airo_gps.h"
#include "simulation.h"
#include "utils/SystemClock.h"
using GnssStatusValue =
    aidl::android::hardware::gnss::IGnssCallback::GnssStatusValue;
using aidl::android::hardware::gnss::AGnss;
using aidl::android::hardware::gnss::AGnssRil;
using aidl::android::hardware::gnss::Gnss;
using aidl::android::hardware::gnss::GnssBatching;
using aidl::android::hardware::gnss::GnssDebug;
using aidl::android::hardware::gnss::GnssGeofence;
using aidl::android::hardware::gnss::GnssMeasurementInterface;
using aidl::android::hardware::gnss::GnssNavigationMessageInterface;
using aidl::android::hardware::gnss::GnssPsds;
using aidl::android::hardware::gnss::visibility_control::GnssVisibilityControl;
GnssAidlImpl *GnssAidlImpl::sInstance = new GnssAidlImpl();
GnssAidlImpl::GnssAidlImpl() : IHidlTransportInterface(HIDL2ANLD_FD) {}
int GnssAidlImpl::onSyncRequest() {
    GnssVisibilityControl::syncVisibilityControlProxy();
    // GnssPsds::syncPsds();
    return 0;
}
void GnssAidlImpl::onConnected() { LOG_D("On Connected"); }
void GnssAidlImpl::onDisconnected() { LOG_D("On Disconnected"); }
void GnssAidlImpl::onHardwareVersionReport(const HardwareVersion *version) {
    Gnss::reportName(version->version);
}
void GnssAidlImpl::onAGpsSetIdRequest(PreSetIdFlags flags) {
    AGnssRil::requestSetId((int)flags);
}
void GnssAidlImpl::onAGpsSetRefLocationRequest() { AGnssRil::requestRefLoc(); }
void GnssAidlImpl::onAGpsNiNotify() {
    // Ignore
}
void GnssAidlImpl::onAGpsDataConnectionRequest(PreAGnssType agnssType) {
    AGnss::requestDataConnection(agnssType);
}
void GnssAidlImpl::onAGpsDataConnectionRequestv2(
    const PreAGnssDataConnectionConfig *config) {
    AGnss::requestDataConnection(config->agnssType);
}
void GnssAidlImpl::onAGpsDataConnectionRelease(
    const PreAGnssDataConnectionConfig *config) {
    AGnss::releaseDataConnection(config->agnssType);
}
void GnssAidlImpl::onLocationOutput(const PreGnssLocation *location) {
    Gnss::reportLocation(location);
}
void GnssAidlImpl::onSvStatusOutput(const PreGnssSvStatus *svStatus) {
    Gnss::reportSvStatus(svStatus);
}
void GnssAidlImpl::onNmeaOutput(const NavNmea *nmea) { Gnss::reportNMEA(nmea); }
void GnssAidlImpl::onGnssMeasurement(const PreGnssData *measurement) {
    GnssMeasurementInterface::airohaGnssMeasurementCb(measurement);
}
void GnssAidlImpl::onGnssNavigationData(
    const PreGnssNavigationMessage *navigationMessage) {
    GnssNavigationMessageInterface::gnssNavigationMessageCb(navigationMessage);
}
void GnssAidlImpl::onRequestTimeAiding() { Gnss::requestTime(); }
void GnssAidlImpl::onRequestLocationAiding() { Gnss::requestLocation(); }
void GnssAidlImpl::onGnssGeofencingStatusOutput(
    const PreGeofenceData *geofenceData) {
    (void)geofenceData;
}
void GnssAidlImpl::onReportGnssCapabilities(
    const PreGnssCapabilities *capabilities) {
    (void)capabilities;
}
void GnssAidlImpl::onBatchLocationReport(
    const std::vector<PreGnssLocation> &locations) {
    (void)locations;
}
void GnssAidlImpl::onGnssDebugData(const PreDebugData *debugData) {
    GnssDebug::setDebugData(debugData);
}
void GnssAidlImpl::onNfwNotification(const PreNfwNotification &data) {
    ::aidl::android::hardware::gnss::visibility_control::
        IGnssVisibilityControlCallback::NfwNotification notification;
    notification.proxyAppPackageName = data.proxyAppPackageName;
    notification.protocolStack =
        static_cast< ::aidl::android::hardware::gnss::visibility_control::
                         IGnssVisibilityControlCallback::NfwProtocolStack>(
            data.protocolStack);
    notification.otherProtocolStackName = data.otherProtocolStackName;
    notification.requestor =
        static_cast< ::aidl::android::hardware::gnss::visibility_control::
                         IGnssVisibilityControlCallback::NfwRequestor>(
            data.requestor);
    notification.requestorId = data.requestorId;
    notification.responseType =
        static_cast< ::aidl::android::hardware::gnss::visibility_control::
                         IGnssVisibilityControlCallback::NfwResponseType>(
            data.responseType);
    notification.inEmergencyMode = data.inEmergencyMode;
    notification.isCachedLocation = data.isCachedLocation;
    GnssVisibilityControl::reportNfwNotification(notification);
}
void GnssAidlImpl::onGnssStatusReport(bool on) {
    if (on) {
        Gnss::reportStatus(GnssStatusValue::ENGINE_ON);
        Gnss::reportStatus(GnssStatusValue::SESSION_BEGIN);
    } else {
        Gnss::reportStatus(GnssStatusValue::SESSION_END);
        Gnss::reportStatus(GnssStatusValue::ENGINE_OFF);
    }
}
GnssAidlImpl *GnssAidlImpl::getInstance() { return sInstance; }