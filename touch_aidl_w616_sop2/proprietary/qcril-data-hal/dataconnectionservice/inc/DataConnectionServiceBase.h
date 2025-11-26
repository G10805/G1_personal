/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <hardware_legacy/power.h>
#include <hidl/HidlSupport.h>
#include "MessageCommon.h"

using namespace rildata;

namespace vendor::qti::hardware::data::connection {

class DataConnectionServiceBase : public android::hardware::hidl_death_recipient {
public:
    DataConnectionServiceBase() {
        wakelockTimerId = TimeKeeper::no_timer;
        wakelockAcquired = false;
    }
    virtual ~DataConnectionServiceBase() {}
    virtual bool registerService(int instanceId) = 0;
    virtual void notifyBearerAllocationUpdate(std::shared_ptr<AllocatedBearerResult_t> ) {}

protected:
    std::mutex wakelockMutex;
    TimeKeeper::timer_id wakelockTimerId;
    bool wakelockAcquired;

    static constexpr TimeKeeper::millisec BEARER_ALLOCATION_TIMEOUT = 500;
    static constexpr const char *BEARER_ALLOCATION_WAKELOCK = "qcril_bearer_allocation_result_wakelock";

    virtual void clearResponseFunctions(uint64_t cookie) = 0;
    virtual void serviceDied(uint64_t cookie,
        const android::wp<android::hidl::base::V1_0::IBase>& who) override;
    void releaseWakelockCb(void *);
    void acquireWakelock(TimeKeeper::millisec timeout);
    virtual void sendBearerAllocationResult(uint64_t ,
                                    std::shared_ptr<AllocatedBearerResult_t> ) {}
    virtual void sendAllBearerAllocationsResult(uint64_t ,
                                        std::shared_ptr<AllocatedBearerResult_t> ) {}
};

}
