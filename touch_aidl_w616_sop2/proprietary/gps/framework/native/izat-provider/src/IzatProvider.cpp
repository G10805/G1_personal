/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION
  Izat Provider

  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include "IzatProvider.h"

#define LOG_TAG "Izat_Provider"
#define NETWORK_PROVIDER IzatRequest::IzatProviderType::IZAT_STREAM_NETWORK
std::mutex IzatProvider::mIzatProviderLock;
IzatProvider* IzatProvider::sIzatProviderHandle = NULL;

IzatProvider::IzatProvider() : mLocationApi(NULL), mLocationCbs({}),
    mSessionId(0), mCapabilitiesMask(0), mGnssInterface(NULL),
    mIzatContext(NULL), mZaxisProvider(NULL), mZlocationResponseHandler(this),
    mNLPResponseListener(this), mEmeregency(false), mIsOdcpiSessionInProgress(false),
    mZaxisListernerAdded(false), mFlpSessionStarted(false), mIzatManager(NULL),
    mBroadcaster(NULL) {}


IzatProvider* IzatProvider::getInstance() {
    mIzatProviderLock.lock();
    if (NULL == sIzatProviderHandle) {
        sIzatProviderHandle = new IzatProvider();
    }
    mIzatProviderLock.unlock();
    return sIzatProviderHandle;
}

extern "C" IzatProvider* getIzatProvider() {
    return IzatProvider::getInstance();
}


void IzatProvider::connectToZProvider() {
    //Altitute Receiver Proxy is loaded once Z-Provider is connected
    if (NULL == mZaxisProvider && NULL != mIzatManager) {
        mZaxisProvider = new AltitudeReceiverProxy(mIzatContext);
    }

    //Register Flp callback wrapper with LocationAPI
     LOC_LOGd("Register location Cb's");
     registerLocationCbs();
    // Check if any active odcpi session, add ZlocatioResponseHandler listener
    if (mIsOdcpiSessionInProgress && mEmeregency && mZaxisProvider) {
        LOC_LOGd("Odcpi session active, add ZlocatioResponseHandler listener");
        mZaxisProvider->setAltitudeListener(&mZlocationResponseHandler, true);
        mZaxisListernerAdded = true;
    }
}

void IzatProvider::setIzatContext(void* izatManager, void* context) {
    mIzatContext = static_cast<struct s_IzatContext*>(context);
    mIzatManager = static_cast<IzatManager*>(izatManager);
}

void IzatProvider::getLocationWithZ(Location location, trackingCallback broadcaster) {
    LOC_LOGv("Received HW-FLP intermidiate fix");
    mBroadcaster = broadcaster;
    if (mEmeregency && mZaxisProvider && mZaxisListernerAdded) {
        FusedLocationReport fusedLocationReport = {};
        mZlocationResponseHandler.convertToFusedLocationReport(fusedLocationReport,
                location);
        mZaxisProvider->getAltitudeFromLocation(fusedLocationReport, mEmeregency);
    } else {
        broadCastLocationWithZ(location);
    }
}

void IzatProvider::broadCastLocationWithZ(Location location) {
    LOC_LOGd("BroadCastLocation with Z");
    if (mBroadcaster) {
        mBroadcaster(location);
    }
    if (mEmeregency) {
        odcpiInject(location);
    }
}

void IzatProvider::registerIzatOdcpiCallback() {
    if (mIzatManager) {
        mGnssInterface = mIzatManager->getGnssInterface();
        if (mGnssInterface) {
            LOC_LOGd("Registering ODCPI callback...");
            odcpiRequestCallback cb = [this](const OdcpiRequestInfo& odcpiRequest) {
                odcpiRequestCb(odcpiRequest);
            };
            mGnssInterface->odcpiInit(cb, OdcpiPrioritytype::ODCPI_HANDLER_PRIORITY_HIGH);
        } else {
            LOC_LOGe("Failed to registering ODCPI callback...");
        }
    }
}

// Add Z-axis listener non-emergency and emergency case as well
// But reporting the location with Z is Activated for only Emergency request
// Below odcpiRequestCb would handle
//   CASE 1: For non-emergency request, where NLP fix is triggerd as part tracking session ODCPI
//   CASE 2: For emergency request, start FLP session. But regardless, we start NLP here,
//           because for handling ODCPI, modem is not going to send us another ODCPI request
//           as part of the regular FLP call flow
void IzatProvider::odcpiRequestCb(const OdcpiRequestInfo& odcpiRequest) {
    ENTRY_LOG();
    LOC_LOGd("Odcpi request in start:0 stop:1: %d", odcpiRequest.type);
    if (ODCPI_REQUEST_TYPE_START == odcpiRequest.type) {
        mEmeregency = odcpiRequest.isEmergencyMode;
        if (mZaxisProvider) {
            if (mEmeregency && !mZaxisListernerAdded) {
                LOC_LOGd("ZlocatioResponseHandler Listener added");
                mZaxisProvider->setAltitudeListener(&mZlocationResponseHandler, true);
                mZaxisListernerAdded = true;
            }
            // Start FLP session in E-911 case
            if (mEmeregency && !mFlpSessionStarted) {
                LOC_LOGd("Start FLP session");
                mFlpSessionStarted = true;
                startFLPSession();
            }
        }
        // Check if there is existing NLP session on-going
        if (mIsOdcpiSessionInProgress) {
            LOC_LOGd("Odcpi Session started Already");
            return;
        }
        // Add NLP fix request and listener
        IzatRequest* request = mIzatManager->createIzatReqest(odcpiRequest, NETWORK_PROVIDER);
        request->setProvider(NETWORK_PROVIDER);
        mIzatManager->addRequest(request);
        mIzatManager->subscribeListener(&mNLPResponseListener);
        mIsOdcpiSessionInProgress = true;
    } else if (ODCPI_REQUEST_TYPE_STOP == odcpiRequest.type) {
        if (mZaxisProvider) {
            if (mEmeregency && mZaxisListernerAdded) {
                LOC_LOGd("ZlocatioResponseHandler Listener removed");
                mZaxisProvider->setAltitudeListener(&mZlocationResponseHandler, false);
                mZaxisListernerAdded = false;
            }
            // stop active FLP session
            if (mEmeregency && mFlpSessionStarted) {
                LOC_LOGd("Stop FLP session");
                stopFLPSession();
            }
        }
        //Remove NLP fix request and listener
        IzatRequest* request = mIzatManager->createIzatReqest(odcpiRequest, NETWORK_PROVIDER);
        request->setProvider(NETWORK_PROVIDER);
        mIzatManager->removeRequest(request);
        mIzatManager->unsubscribeListener(&mNLPResponseListener);
        mIsOdcpiSessionInProgress = false;
    }
}

void IzatProvider::startFLPSession() {
    if (mSessionId > 0) {
        LOC_LOGd("Already FLPSession started with session id: %d", mSessionId);
    }
    TrackingOptions options;
    memset(&options, 0, sizeof(LocationOptions));
    options.size = sizeof(LocationOptions);
    options.minInterval = (uint32_t)(1000);
    options.mode = GNSS_SUPL_MODE_STANDALONE;
    if (mCapabilitiesMask & LOCATION_CAPABILITIES_GNSS_MSB_BIT) {
        options.mode = GNSS_SUPL_MODE_MSB;
    }
    options.powerMode = GNSS_POWER_MODE_INVALID;
    if (mLocationApi) {
        mSessionId = mLocationApi->startTracking(options);
        LOC_LOGd("FLPSession started with session id: %d", mSessionId);
    }
}

void IzatProvider::stopFLPSession() {
    if (mSessionId > 0 && mLocationApi) {
        mLocationApi->stopTracking(mSessionId);
        LOC_LOGd("FLPSession stopped with session id: %d", mSessionId);
        mFlpSessionStarted = false;
        mSessionId = 0;
    }
}

void IzatProvider::onResponseCb(LocationError locErrCode, uint32_t sessionId) {
    LOC_LOGd("ResponseCb: LocationError: %d, SessionId: %d", locErrCode, sessionId);
    if (LOCATION_ERROR_SUCCESS != locErrCode) {
        mFlpSessionStarted = false;
    }
}

void IzatProvider::onCapabilitiesCb(LocationCapabilitiesMask capabilitiesMask) {
    LOC_LOGd("capabilitiesMask: 0x%x", capabilitiesMask);
    mCapabilitiesMask = capabilitiesMask;
}

void IzatProvider::registerLocationCbs() {
    if (NULL == mLocationApi) {
        mLocationCbs.size = sizeof(mLocationCbs);
        mLocationCbs.trackingCb = nullptr;
        mLocationCbs.batchingCb = nullptr;
        mLocationCbs.geofenceBreachCb = nullptr;
        mLocationCbs.geofenceStatusCb = nullptr;
        mLocationCbs.gnssNiCb = nullptr;
        mLocationCbs.gnssSvCb = nullptr;
        mLocationCbs.gnssNmeaCb =  nullptr;
        mLocationCbs.gnssMeasurementsCb = nullptr;
        mLocationCbs.batchingStatusCb = nullptr;

        // mandatory callback
        mLocationCbs.capabilitiesCb = [this](LocationCapabilitiesMask capabilitiesMask) {
            onCapabilitiesCb(capabilitiesMask);
        };

        // mandatory callback
        mLocationCbs.responseCb = [this](LocationError err, uint32_t id_t) {
            onResponseCb(err, id_t);
        };

        // mandatory callback
        mLocationCbs.collectiveResponseCb = [this](size_t count, LocationError* errs,
                uint32_t* ids) {
            onCollectiveResponseCb(count, errs, ids);
        };

        mLocationCbs.trackingCb = [this](const Location& locInfo) {
            onTrackingCb(locInfo);
        };
        mLocationApi = LocationAPI::createInstance(mLocationCbs);
        if (mLocationApi == NULL) {
            LOC_LOGd("failed to create LocationAPI instance");
        }
    }
}

void IzatProvider::odcpiInject(Location location) {
    if (mGnssInterface) {
        LOC_LOGd("Inject Fused location");
        mGnssInterface->odcpiInject(location);
    }
}

void IzatProvider::injectNLPFix(double latitude, double longitude, float accuracy) {
    if (mGnssInterface) {
        LOC_LOGd("Inject NLP fix");
        mGnssInterface->injectLocation(latitude, longitude, accuracy);
    }
}

// NLP fix listener
void NLPResponseListener::onLocationChanged(const IzatLocation *location,
        const IzatLocationStatus status) {
    if (IZAT_LOCATION_STATUS_FINAL == status) {
        mIzatProvider->injectNLPFix(location->mLatitude, location->mLongitude,
            location->mHorizontalAccuracy);
    }
}
