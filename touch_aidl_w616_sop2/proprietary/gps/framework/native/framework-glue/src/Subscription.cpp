/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION
  loc service module

  Copyright  (c) 2015-2017, 2020-2023 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/


#define LOG_TAG "Subscription"
#define LOG_NDEBUG 0
#include <loc_pla.h>
#include <log_util.h>
#include "Subscription.h"
#include "IzatDefines.h"

Subscription* Subscription::mSubscriptionObj = NULL;
#ifdef USE_QCMAP
LocNetIfaceBase* Subscription::mLocNetIfaceObj = NULL;
#endif
IDataItemObserver* Subscription::mObserverObj = NULL;
std::unordered_set<DataItemId> Subscription::mCachedDi = {};
std::unordered_map<SubscriptionCallbackIface*, uint64_t> Subscription::sSubscriptionCbMap = {};

using namespace std;

// Subscription class implementation
Subscription* Subscription::getSubscriptionObj()
{
    int result = 0;

    ENTRY_LOG();
    do {
          // already initialized
          BREAK_IF_NON_ZERO(0, mSubscriptionObj);

          mSubscriptionObj = new (std::nothrow) Subscription();
          BREAK_IF_ZERO(1, mSubscriptionObj);
#ifdef USE_QCMAP
          int use_qcmap_manager = 1;
          static loc_param_s_type gps_conf_param_table[] = {
              {"USE_QCMAP_MANAGER", &use_qcmap_manager, NULL, 'n'}
          };
          UTIL_READ_CONF(LOC_PATH_GPS_CONF, gps_conf_param_table);
          LOC_LOGd("USE_QCMAP_MANAGER %d", use_qcmap_manager);
          if (use_qcmap_manager != 0) {
              mLocNetIfaceObj = LocNetIfaceBase::getLocNetIfaceImpl();
              BREAK_IF_ZERO(2, mLocNetIfaceObj);
              mLocNetIfaceObj->registerDataItemNotifyCallback(
                    Subscription::locNetIfaceCallback, NULL);
          }
#endif
          result = 0;
    } while(0);

    EXIT_LOG_WITH_ERROR("%d", result);
    return mSubscriptionObj;
}

void Subscription::destroyInstance()
{
    ENTRY_LOG();

    delete mSubscriptionObj;
    mSubscriptionObj = NULL;
    EXIT_LOG_WITH_ERROR("%d", 0);
}

//IDataItemSubscription overrides
void Subscription::subscribe(const std::unordered_set<DataItemId> & l, IDataItemObserver * observerObj)
{
    // Assign the observer object if required
    if ((Subscription::mObserverObj == NULL) && (observerObj)) {
        Subscription::mObserverObj = observerObj;
    }
    std::unordered_set<DataItemId> dataItemSet;
    for (auto it = l.begin(); it != l.end(); ++it) {
        if ((*it != POWER_CONNECTED_STATE_DATA_ITEM_ID) && (*it != NETWORKINFO_DATA_ITEM_ID)) {
            dataItemSet.insert(*it);
        }
    }

    if (dataItemSet.empty()) {
        LOC_LOGe("No dataItem subscribed");
        return;
    }

#if defined(USE_QCMAP)
    if (mLocNetIfaceObj) {
        mLocNetIfaceObj->subscribe(dataItemSet);
    }
#else

    uint64_t dataItemSetMask = 0;
    for (auto item: dataItemSet)  {
        dataItemSetMask |= (1 << item);
    }

    if (dataItemSetMask != 0) {
        for (auto subscriber : sSubscriptionCbMap) {
            uint64_t outMask = 0;
            std::unordered_set<DataItemId> subDataItemSet = {};
            outMask = getDataItemSetPerMask(dataItemSet, subDataItemSet, subscriber.second);
            if (!subDataItemSet.empty()) {
                (subscriber.first)->updateSubscribe(subDataItemSet, true);
                if (!mCachedDi.empty()) {
                    for (auto di: subDataItemSet) {
                        mCachedDi.erase(di);
                    }
                }
                dataItemSetMask &= ~(outMask);
            }
        }
    }

    // cache the data items
    if (dataItemSetMask > 0) {
        std::unordered_set<DataItemId> dataItemsToCache = {};
        getDataItemSetPerMask(dataItemSet, dataItemsToCache, dataItemSetMask);
        mCachedDi.insert(dataItemsToCache.begin(), dataItemsToCache.end());
    }
#endif
}

uint64_t Subscription::getDataItemSetPerMask(const std::unordered_set<DataItemId>& in,
        std::unordered_set<DataItemId>& out, uint64_t dataItemIdMask) {
    uint64_t outDataItemIdMask = 0;
    for (auto item : in) {
        if (dataItemIdMask & (1 << item)) {
            out.insert(item);
            outDataItemIdMask |= (1 << item);
        }
    }
    return outDataItemIdMask;
}

void Subscription::updateSubscription(const std::unordered_set<DataItemId> & /*l*/, IDataItemObserver * /*observerObj*/)
{
}

void Subscription::requestData(const std::unordered_set<DataItemId> & dataItemSet,
                               IDataItemObserver * observerObj)
{
    // Assign the observer object if required
    if ((Subscription::mObserverObj == NULL) && (observerObj)) {
        Subscription::mObserverObj = observerObj;
    }

#if defined(USE_QCMAP)
    if (mLocNetIfaceObj) {
        mLocNetIfaceObj->requestData(dataItemSet);
    }
#else
    for (auto item : sSubscriptionCbMap) {
        std::unordered_set<DataItemId> subDataItemSet = {};
        getDataItemSetPerMask(dataItemSet, subDataItemSet, item.second);
        if (!subDataItemSet.empty()) {
            (item.first)->requestData(subDataItemSet);
        }
    }
#endif
}

void Subscription::unsubscribe(const std::unordered_set<DataItemId> & dataItemList,
                               IDataItemObserver * observerObj)
{
     // Assign the observer object if required
    if ((Subscription::mObserverObj == NULL) && (observerObj)) {
        Subscription::mObserverObj = observerObj;
    }

#if defined(USE_QCMAP)
    if (mLocNetIfaceObj) {
        mLocNetIfaceObj->unsubscribe(dataItemList);
    }
#else
    for (auto item : sSubscriptionCbMap) {
        std::unordered_set<DataItemId> subDataItemSet = {};
        getDataItemSetPerMask(dataItemList, subDataItemSet, item.second);
        if (!subDataItemSet.empty()) {
            (item.first)->updateSubscribe(subDataItemSet, false);
        }
    }
#endif
}

void Subscription::unsubscribeAll(IDataItemObserver * observerObj)
{
    // Assign the observer object if required
    if ((Subscription::mObserverObj == NULL) && (observerObj)) {
        Subscription::mObserverObj = observerObj;
    }

#if defined(USE_QCMAP)
    if (mLocNetIfaceObj) {
       mLocNetIfaceObj->unsubscribeAll();
    }
#else
    if (sSubscriptionCbMap.empty()) {
        LOC_LOGE("sSubscriptionCbMap NULL !");
        return;
    }
    for (auto item : sSubscriptionCbMap) {
        item.first->unsubscribeAll();
    }
#endif
}

void Subscription::setSubscriptionCallback(SubscriptionCallbackIface* cb, uint64_t dataItemIdMask) {

    ENTRY_LOG();
    // erase the Subscription callback when dataItem Mask is 0
    if (dataItemIdMask == 0) {
        auto itr = sSubscriptionCbMap.find(cb);
        if (sSubscriptionCbMap.end() != itr) {
           sSubscriptionCbMap.erase(itr);
        }
        return;
    } else {
        sSubscriptionCbMap[cb] = dataItemIdMask;
    }

    LOC_LOGd("Client subscription mask [%p] = 0x%" PRIx64 "", cb, sSubscriptionCbMap[cb]);
    if ((NULL != Subscription::mObserverObj) && !mCachedDi.empty()) {
        // Subscribe request came before we received SubscriptionCallbackIface
        // object. Subscribe to these data items and request for data item value.
        LOC_LOGD("Subscribing to items in cache..");
        mSubscriptionObj->subscribe(mCachedDi, Subscription::mObserverObj);
        mSubscriptionObj->requestData(mCachedDi, Subscription::mObserverObj);
    }
}

SubscriptionCallbackIface* Subscription::getSubscriptionCallback(DataItemId id) {
    SubscriptionCallbackIface* cb = NULL;
    for (auto item : sSubscriptionCbMap) {
        if ((item.second) & (1 << id)) {
            cb = item.first;
            break;
        }
    }
    return cb;
}

#ifdef USE_QCMAP
void Subscription::locNetIfaceCallback(void* userDataPtr,
        const std::unordered_set<IDataItemCore*>& itemSet) {
    LOC_LOGV("Subscription::locNetIfaceCallback");
    if (Subscription::mObserverObj == NULL) {
        LOC_LOGE("NULL observer object");
        return;
    }
    Subscription::mObserverObj->notify(itemSet);
}
#endif

extern "C" void notifyToSubscriberExt(std::unordered_set<IDataItemCore*> itemSet){
    if (NULL != Subscription::mObserverObj) {
        LOC_LOGd("updating opt-in");
        Subscription::mObserverObj->notify(itemSet);
    } else {
        LOC_LOGd("Subscription::mObserverObj is null!!!");
    }
}
