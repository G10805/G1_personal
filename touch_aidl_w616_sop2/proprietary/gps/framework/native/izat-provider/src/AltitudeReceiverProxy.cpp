/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION
  Altitude Receiver Proxy

  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#define LOG_NDEBUG 0
#define LOG_TAG "IzatSvc_zaxis"

#include "AltitudeReceiverProxy.h"
#include<log_util.h>
#include<functional>
#include <thread>
#include <unistd.h>
#include <inttypes.h>

#define TIMEOUT_IN_MS           (5000)
#define RECOVER_TIME_IN_MS      (60000)
#define NUM_THRES_TO_PAUSE_REQ  (10)

namespace izat_zaxis {

class AltitudeReqTimer : public LocTimer {
    function<void()> mTimeOutFunc;
public:
    inline AltitudeReqTimer(function<void()> timeoutFunc) :
            mTimeOutFunc(timeoutFunc) {
        LOC_LOGd("AltitudeReqTimer ctor");
    }

    inline ~AltitudeReqTimer() {
        LOC_LOGd("~AltitudeReqTimer");
    }
    virtual void timeOutCallback() {
        mTimeOutFunc();
    }
};

AltitudeReceiverProxy::AltitudeReceiverProxy(const struct s_IzatContext* izatContext) :
    LocationProvider(izatContext),
    mIPCTag("ALT-RECV-PROXY"),
    mHasAltStatusForPreviousFix(true),
    mIsPaused(false),
    mContinuousFailedFixesNum(1),
    mRecoverTimerPtr(nullptr) {
    mIzatContext->mIPCMessagingProxyObj->registerResponseObj(mIPCTag.c_str(), this);
}

//This function handles IPC message from NLP_API
void AltitudeReceiverProxy::handleMsg(InPostcard * const in_card) {
    int result = -1;

    do {
        BREAK_IF_ZERO(1, in_card);
        const char* resp = NULL;
        const char* from = NULL;
        BREAK_IF_NON_ZERO(2, in_card->getString("FROM", &from));

        LOC_LOGD("FROM: %s", from);
        if (0 == strcmp(from, "ALT-PROVIDER")) {
            FusedLocationReport location;
            unsigned long long elapsedRealTimeInMs;
            unsigned long long utcTimeStampInMs;
            in_card->getUInt64("ELAPSED_REAL_TIME-MS", elapsedRealTimeInMs);
            location.mElapsedRealTimeInNanoSecs = (int64_t)elapsedRealTimeInMs * 1000000;
            in_card->getUInt64("TIMESTAMP-MS", utcTimeStampInMs);
            location.mUtcTimestampInMsec = (int64_t) utcTimeStampInMs;
            in_card->getDouble("LATITUDE", location.mLatitude);
            in_card->getDouble("LONGITUDE", location.mLongitude);
            in_card->getBool("IS_ALTITUDE_VALID", location.mHasAltitudeWrtEllipsoid);
            in_card->getFloat("ALTITUDE_ACCURACY", location.mVertUnc);
            in_card->getDouble("ALTITUDE", location.mAltitudeWrtEllipsoid);
            mIzatContext->mMsgTask->sendMsg(
                    new (nothrow) reportAltitudeMsg(*this, location));
        }
    } while (0);
}

//Send Altitude Lookup request to NLP_API
void AltitudeReceiverProxy::sendAltitudeLookUpRequest(const FusedLocationReport& location,
        bool isEmergency) {
    OutPostcard* card = OutPostcard::createInstance();
    if (nullptr != card) {
        card->init();
        card->addString("TO", "ALT-PROVIDER");
        card->addString("FROM", mIPCTag.c_str());
        card->addString("RESP", "ALT");
        card->addString("INFO", "ALT");
        card->addBool("IS_EMERGENCY", isEmergency);
        card->addDouble("LATITUDE", location.mLatitude);
        card->addDouble("LONGITUDE", location.mLongitude);
        card->addUInt64("TIMESTAMP-MS", (uint64_t)location.mUtcTimestampInMsec);
        card->addUInt64("ELAPSED_REAL_TIME-MS",
                (uint64_t)location.mElapsedRealTimeInNanoSecs/1000000);
        card->finalize();
        //Send post card
        mIzatContext->mIPCMessagingProxyObj->sendMsg(card, "ALT-PROVIDER");
    }
}

//Public APIs for altitude listener
void AltitudeReceiverProxy::getAltitudeFromLocation(const FusedLocationReport& location,
        bool isEmergency) {
    string locStr;
    location.stringify(locStr);
    LOC_LOGd("getAltitudeFromLocation, %s", locStr.c_str());
    shared_ptr<AltitudeReqTimer> reqTimer = make_shared<AltitudeReqTimer>(
            [=]() {
                mIzatContext->mMsgTask->sendMsg(
                        new (nothrow) altitudeReqTimerExpireMsg(*this, location));
            });
    mIzatContext->mMsgTask->sendMsg(
            new (nothrow) altitudeReqMsg(*this, location, isEmergency, reqTimer));
}

void AltitudeReceiverProxy::setAltitudeListener(const ILocationResponse* listener, bool enable) {
    if (enable) {
        subscribe(listener);
    } else {
        unsubscribe(listener);
    }
    mIzatContext->mMsgTask->sendMsg(new (nothrow) setListenerMsg(*this, enable));
}

void AltitudeReceiverProxy::reportAltitude(const FusedLocationReport& location, bool status) {
    string locStr;
    location.stringify(locStr);
    LOC_LOGd("reportAltitude:broadcastLocation to client: %s, altitude: %f,"
            "elapsedRealTime: %" PRIi64,
            locStr.c_str(), location.mAltitudeWrtEllipsoid, location.mElapsedRealTimeInNanoSecs);
    //send location with Z to client
    broadcastLocation(&location);

    if (!status && (mHasAltStatusForPreviousFix == status)) {
        mContinuousFailedFixesNum++;
    } else {
        mContinuousFailedFixesNum = 1;
    }

    mHasAltStatusForPreviousFix = status;
    if (mContinuousFailedFixesNum >= NUM_THRES_TO_PAUSE_REQ) {
        mIsPaused = true;
        mContinuousFailedFixesNum = 1;
        if (mRecoverTimerPtr == nullptr) {
            mRecoverTimerPtr = make_shared<AltitudeReqTimer>(
                [&]() {
                    mIzatContext->mMsgTask->sendMsg(
                            new (nothrow) recoverAltitudeReqMsg(*this));
                });
            mRecoverTimerPtr->start(RECOVER_TIME_IN_MS, false);
        }
    }
}

//Message proc functions
void AltitudeReceiverProxy::altitudeReqMsg::proc() const {
    LOC_LOGd("altitudeReqMsg::proc()");
    if (mAltReceiver.mIsPaused) {
        //Report all locations in queue
        for (auto iter = mAltReceiver.mLocationQueue.begin();
                iter != mAltReceiver.mLocationQueue.end(); iter++) {
            mAltReceiver.broadcastLocation(&((*iter).first));
        }
        mAltReceiver.mLocationQueue.clear();

        string locStr;
        mLocation.stringify(locStr);
        LOC_LOGd("Direct return: broadcastLocation to client: %s, alt: %f,"
                "elapsedRealTime: %" PRIi64,
                locStr.c_str(), mLocation.mAltitudeWrtEllipsoid,
                mLocation.mElapsedRealTimeInNanoSecs);
        mAltReceiver.broadcastLocation(&mLocation);

    } else {
        mAltReceiver.sendAltitudeLookUpRequest(mLocation, mIsEmergency);
        mTimerPtr->start(TIMEOUT_IN_MS, false);
        mAltReceiver.mLocationQueue.push_back(make_pair(mLocation, mTimerPtr));
    }
}

void AltitudeReceiverProxy::reportAltitudeMsg::proc() const {
    LOC_LOGd("reportAltitudeMsg::proc()");
    for (auto iter = mAltReceiver.mLocationQueue.begin();
            iter != mAltReceiver.mLocationQueue.end();) {
        FusedLocationReport item = (*iter).first;
        auto nextIter = std::next(iter, 1);
        if (item.mElapsedRealTimeInNanoSecs > mLocation.mElapsedRealTimeInNanoSecs) {
            break;
        } else if (item.mElapsedRealTimeInNanoSecs < mLocation.mElapsedRealTimeInNanoSecs &&
                nextIter != mAltReceiver.mLocationQueue.end() &&
                (*nextIter).first.mElapsedRealTimeInNanoSecs <=
                mLocation.mElapsedRealTimeInNanoSecs) {
            mAltReceiver.reportAltitude(item, false);
            iter = mAltReceiver.mLocationQueue.erase(iter);
        } else if (nextIter == mAltReceiver.mLocationQueue.end() ||
                (item.mElapsedRealTimeInNanoSecs <= mLocation.mElapsedRealTimeInNanoSecs &&
                (*nextIter).first.mElapsedRealTimeInNanoSecs >
                mLocation.mElapsedRealTimeInNanoSecs)) {
            if (mLocation.mHasAltitudeWrtEllipsoid) {
                item.mHasAltitudeWrtEllipsoid = true;
                item.mAltitudeWrtEllipsoid = mLocation.mAltitudeWrtEllipsoid;
            }
            mAltReceiver.reportAltitude(item, mLocation.mHasAltitudeWrtEllipsoid);
            iter = mAltReceiver.mLocationQueue.erase(iter);
            break;
        }
    }
}


void AltitudeReceiverProxy::altitudeReqTimerExpireMsg::proc() const {
    LOC_LOGd("altitudeReqTimerExpireMsg::proc()");
    if (mAltReceiver.mLocationQueue.size() > 0) {
        FusedLocationReport headItem = mAltReceiver.mLocationQueue.front().first;
        if (headItem.mElapsedRealTimeInNanoSecs == mLocation.mElapsedRealTimeInNanoSecs) {
            mAltReceiver.reportAltitude(mLocation, false);
            mAltReceiver.mLocationQueue.pop_front();
        }
    }
}

void AltitudeReceiverProxy::recoverAltitudeReqMsg::proc() const {
    LOC_LOGd("recoverAltitudeReqMsg::proc()");
    mAltReceiver.mIsPaused = false;
    mAltReceiver.mContinuousFailedFixesNum = 1;
    mAltReceiver.mHasAltStatusForPreviousFix = true;
    if (mAltReceiver.mRecoverTimerPtr != nullptr) {
        mAltReceiver.mRecoverTimerPtr = nullptr;
    }
}

void AltitudeReceiverProxy::setListenerMsg::proc() const {
    LOC_LOGd("setListenerMsg::proc()");
    if (!mIsEnabled) {
        mAltReceiver.mLocationQueue.clear();
    }
}

}
