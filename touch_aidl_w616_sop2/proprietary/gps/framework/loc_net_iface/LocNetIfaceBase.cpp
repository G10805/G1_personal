/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  Copyright (c) 2017, 2020 The Linux Foundation. All rights reserved
=============================================================================*/
#define LOG_TAG "LocSvc_LocNetIfaceBase"

#include <LocNetIfaceBase.h>
#include <loc_pla.h>
#include <log_util.h>
#include <loc_cfg.h>

#include <LocNetIface.h>

/* Data Item notify callback
 * Only one instance of LocNetIfaceBase can register this callback.
 * No support for multiple clients */
LocNetStatusChangeCb LocNetIfaceBase::sNotifyCb = NULL;
void* LocNetIfaceBase::sNotifyCbUserDataPtr = NULL;

// LocNetIface implementation object
LocNetIfaceBase* LocNetIfaceBase::mLocNetIfaceImpl = nullptr;

LocNetIfaceBase* LocNetIfaceBase::getLocNetIfaceImpl() {
    if (nullptr == mLocNetIfaceImpl) {
        mLocNetIfaceImpl = new LocNetIface();
    }
    return mLocNetIfaceImpl;
}

void LocNetIfaceBase::registerWwanCallStatusCallback(LocWwanCallStatusCb wwanCallStatusCb) {

    ENTRY_LOG();

    mWwanCallStatusCb = wwanCallStatusCb;
}

void LocNetIfaceBase::registerDataItemNotifyCallback(
        LocNetStatusChangeCb callback, void* userDataPtr) {

    ENTRY_LOG();

    if (LocNetIfaceBase::sNotifyCb != NULL) {
        LOC_LOGE("Notify cb already registered !");
        return;
    }

    LocNetIfaceBase::sNotifyCb = callback;
    LocNetIfaceBase::sNotifyCbUserDataPtr = userDataPtr;
}

bool LocNetIfaceBase::updateSubscribedItemSet(
        const std::unordered_set<DataItemId>& itemSet, bool addOrDelete){

    ENTRY_LOG();
    bool anyUpdatesToSet = false;

    /* Scroll through specified item unordered_set */
    std::unordered_set<DataItemId>::const_iterator it = itemSet.begin();
    for (; it != itemSet.end(); it++) {

        DataItemId itemId = *it;

        bool itemInSubscribedList = isItemSubscribed(itemId);

        /* Request to add */
        if (addOrDelete == true && !itemInSubscribedList) {

            mSubscribedItemList.push_back(itemId);
            anyUpdatesToSet = true;

        } else if (addOrDelete == false && itemInSubscribedList) {
            /* Request to delete */
            mSubscribedItemList.erase(
                    std::remove(
                            mSubscribedItemList.begin(),
                            mSubscribedItemList.end(), itemId),
                            mSubscribedItemList.end());
            anyUpdatesToSet = true;
        }
    }

    return anyUpdatesToSet;
}

// LocNetIfaceTimer implementation
void LocNetIfaceTimer::start()
{
    LOC_LOGd("start timer %s, waiting %" PRId64 " ms...", mName.c_str(), mWaitTimeInMs);

    mStarted = true;
    LocTimer::start((unsigned int) mWaitTimeInMs, false);
}

void LocNetIfaceTimer::stop()
{
    if (!mStarted) {
        LOC_LOGd("time %s not started yet.", mName.c_str());
        return;
    }

    LOC_LOGd("stop timer %s", mName.c_str());
    LocTimer::stop();
    mStarted = false;
}

void LocNetIfaceTimer::timeOutCallback() {
    LOC_LOGd("timer %s timeout", mName.c_str());
    mStarted = false;
    mRunable();
}
