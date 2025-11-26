/******************************************************************************
#  Copyright (c) 2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <signal.h>

#include <hardware_legacy/power.h>
#include <hidl/HidlSupport.h>
#include "MessageCommon.h"
#include "QtiMutex.h"

using namespace rildata;

namespace vendor::qti::hardware::data::iwlan {

class IWlanServiceBase : public android::hardware::hidl_death_recipient {
public:
    virtual ~IWlanServiceBase() {}

    virtual void onDataRegistrationStateChange() {}
    virtual void onDataCallListChange(std::vector<DataCallResult_t> ) {}
    virtual void onQualifiedNetworksChange(std::vector<QualifiedNetwork_t> ) {}
    virtual void onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t , int32_t , Message::Callback::Status ) {}
    virtual void onDeactivateDataCallIWlanResponseIndMessage(ResponseError_t , int32_t , Message::Callback::Status ) {}
    virtual void onUnthrottleApn(std::string apn) {}
    virtual void acknowledgeRequest(int32_t ) {}
    virtual bool registerService(int instanceId) = 0;

protected:
    static std::mutex wakelockMutex;
    static uint8_t wakelockRefCount;
    static timer_t wakelockTimerId;
    static std::string iwlanSvcWakelock;

    qtimutex::QtiSharedMutex mCallbackLock;
    static void acquireWakeLock();
    static void releaseWakeLock();
    static void wakeLockTimeoutHdlr(union sigval sval);
    static void resetWakeLock();
    virtual void clearResponseFunctions() = 0;
    virtual void serviceDied(uint64_t cookie,
        const android::wp<android::hidl::base::V1_0::IBase>& who) override;
};

}
