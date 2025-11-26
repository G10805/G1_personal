/*
 * Copyright (c) 2021-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_AIDL_WiFiDBProvider"
#define LOG_NDEBUG 0

#include "LocAidlWiFiDBProvider.h"
#include <log_util.h>
#include <gps_extended_c.h>
#include <LocationAPI.h>
#include <izat_remote_api.h>
#include <IzatRemoteApi.h>
#include <IzatTypes.h>
#include <string>
#include "izat_wifi_db_provider.h"
#include "IzatWiFiDBProvider.h"
#include "LocAidlUtils.h"

using namespace izat_remote_api;
using namespace izat_manager;

namespace aidl {
namespace vendor {
namespace qti {
namespace gnss {
namespace implementation {

using ::aidl::vendor::qti::gnss::LocAidlWifiDBListStatus;

// ======== WiFiDBProviderWrapper ========
class WiFiDBProviderWrapper : public WiFiDBUploader {
    const wifiDBProvObsLocDataUpdateCb mApObsListUpdateCb;
    const wifiDBProvServiceRequestCb mServiceReqCb;
    const wifiDBProvEnvNotifyCb mEnvNotifyCb;
    const void *mClientData;
public:
    inline WiFiDBProviderWrapper(wifiDBProvObsLocDataUpdateCb apObsListUpdateCb,
                                wifiDBProvServiceRequestCb serviceReqCb,
                                wifiDBProvEnvNotifyCb envNotifyCb,
                                void *clientData) :
            WiFiDBUploader(),
            mApObsListUpdateCb(apObsListUpdateCb),
            mServiceReqCb(serviceReqCb),
            mEnvNotifyCb(envNotifyCb),
            mClientData(clientData) {
        registerSelf();
    }

    inline virtual void onAPObsLocDataAvailable(std::vector<APObsData>* ap_obsdata_ptr,
            const int apListStatus) override {
        if (nullptr != ap_obsdata_ptr) {
            bool completeList = true;
            std::vector<APObsData>& ap_obsdata_info = *ap_obsdata_ptr;

            APObsData_s **ap_obsdatalist_arr = new APObsData_s *[ap_obsdata_info.size()];
            if (ap_obsdatalist_arr!=NULL) {
                for (size_t ii = 0; ii < ap_obsdata_info.size(); ++ii) {
                    ap_obsdatalist_arr[ii] = new APObsData_s;
                    if (ap_obsdatalist_arr[ii]!=NULL) {
                        ap_obsdatalist_arr[ii]->scanTimestamp_ms =
                                ap_obsdata_info[ii].scanTimestamp_ms;
                        ap_obsdatalist_arr[ii]->bUlpLocValid =
                                ap_obsdata_info[ii].bUlpLocValid;
                        if (ap_obsdata_info[ii].bUlpLocValid) {
                            // if bUlpLocValid is true only, then fill up ulplocation
                            ap_obsdatalist_arr[ii]->gpsLoc.size =
                                    ap_obsdata_info[ii].gpsLoc.size;
                            ap_obsdatalist_arr[ii]->gpsLoc.position_source =
                                    ap_obsdata_info[ii].gpsLoc.position_source;
                            ap_obsdatalist_arr[ii]->gpsLoc.tech_mask =
                                    ap_obsdata_info[ii].gpsLoc.tech_mask;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.flags =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.flags;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.latitude =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.latitude;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.longitude =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.longitude;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.altitude =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.altitude;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.speed =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.speed;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.bearing =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.bearing;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.accuracy =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.accuracy;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.vertUncertainity =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.vertUncertainity;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.timestamp =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.timestamp;
                        }
                        ap_obsdatalist_arr[ii]->cellInfo.cell_type =
                                ap_obsdata_info[ii].cellInfo.cellType;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id1 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID1;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id2 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID2;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id3 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID3;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id4 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID4;

                        ap_obsdatalist_arr[ii]->ap_scan_info_size =
                                ap_obsdata_info[ii].ap_scan_info.size();
                        size_t scan_list_len = (size_t) ap_obsdatalist_arr[ii]->ap_scan_info_size;

                        for (size_t jj = 0; jj < scan_list_len; jj++) {
                            APScanInfo_s ap_scan_info;
                            ap_scan_info.mac_R48b =
                                    ap_obsdata_info[ii].ap_scan_info[jj].mac_R48b;
                            ap_scan_info.rssi =
                                    ap_obsdata_info[ii].ap_scan_info[jj].rssi;
                            ap_scan_info.age_usec =
                                    ap_obsdata_info[ii].ap_scan_info[jj].age_usec;
                            ap_scan_info.channel_id =
                                    ap_obsdata_info[ii].ap_scan_info[jj].channel_id;
                            memcpy(ap_scan_info.ssid, ap_obsdata_info[ii].ap_scan_info[jj].ssid,
                                    std::min(sizeof(ap_scan_info.ssid),
                                    sizeof(ap_obsdata_info[ii].ap_scan_info[jj].ssid)));
                            ap_scan_info.ssid_valid_byte_count =
                                    ap_obsdata_info[ii].ap_scan_info[jj].ssid_valid_byte_count;

                            ap_obsdatalist_arr[ii]->ap_scan_info.push_back(ap_scan_info);
                        }
                    } else {
                        completeList=false;
                        break;
                    }
                }
                if (true == completeList) {
                    mApObsListUpdateCb(ap_obsdatalist_arr, ap_obsdata_info.size(), apListStatus,
                            (void *)mClientData);
                }
                for (size_t ii = 0; ii < ap_obsdata_info.size(); ++ii) {
                    if (ap_obsdatalist_arr[ii]!=NULL) {
                        delete ap_obsdatalist_arr[ii];
                    }
                }
                delete[] ap_obsdatalist_arr;
            }
        }
    }
    inline virtual void serviceRequest() override {
        mServiceReqCb((void *)mClientData);
    }

    inline virtual void notifyCallbackEnv() override {
        if (nullptr != mEnvNotifyCb) {
            mEnvNotifyCb((void *)mClientData);
        }
    }
};

LocAidlWiFiDBProvider::LocAidlWiFiDBProvider() :
        mIzatWiFiDBProviderApiClient(nullptr), mCallbackIface(nullptr) {

    ENTRY_LOG();
}

LocAidlWiFiDBProvider::~LocAidlWiFiDBProvider() {

    ENTRY_LOG();

    if (mIzatWiFiDBProviderApiClient!=nullptr) {
        delete (WiFiDBProviderWrapper*)mIzatWiFiDBProviderApiClient;
        mIzatWiFiDBProviderApiClient = nullptr;
    }
}

// Methods from ILocAidlWiFiDBProvider follow.
static std::shared_ptr<ILocAidlWiFiDBProviderCallback> sCallback = nullptr;
std::shared_ptr<LocAidlDeathRecipient> LocAidlWiFiDBProvider::mDeathRecipient = nullptr;

void WifiDBProviderServiceDied(void* cookie) {
    LOC_LOGe("ILocAidlWiFiDBProviderCallback died.");
    sCallback = nullptr;
}

::ndk::ScopedAStatus LocAidlWiFiDBProvider::init(
        const std::shared_ptr<ILocAidlWiFiDBProviderCallback>& callback, bool* _aidl_return) {

    FROM_AIDL_CLIENT();

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = make_shared<LocAidlDeathRecipient>(WifiDBProviderServiceDied);
    }
    if (callback != nullptr) {
        mDeathRecipient->registerToPeer(callback->asBinder().get(), this);
    } else {
        LOC_LOGe("callback is nullptr!");
    }
    mCallbackIface = callback;

    *_aidl_return = true;
    return ndk::ScopedAStatus::ok();
}

static void DBProviderApListUpdateCb(APObsData_s *ap_obs_data[], size_t ap_obs_data_size,
        int apListStatus, void * /*clientData*/ ) {
    std::vector<LocAidlApObsData> apObsDataList;
    int32_t apListSize = 0;

    LOC_LOGE("Native hidl ApList Update: AP list size: %d apListStatus: %u",
            (int) ap_obs_data_size, apListStatus);

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }

    for (size_t ii = 0; ii <(size_t) ap_obs_data_size; ii++) {
         LocAidlApObsData info;
         info.scanTimestamp_ms = ap_obs_data[ii]->scanTimestamp_ms;
         info.bUlpLocValid = ap_obs_data[ii]->bUlpLocValid;
         if (info.bUlpLocValid) {
            // if bUlpLocValid is true only, then fill up ulplocation
            info.gpsLoc.position_source = ap_obs_data[ii]->gpsLoc.position_source;
            info.gpsLoc.tech_mask = ap_obs_data[ii]->gpsLoc.tech_mask;
            info.gpsLoc.gpsLocation.locationFlagsMask = ap_obs_data[ii]->gpsLoc.gpsLocation.flags;
            info.gpsLoc.gpsLocation.latitude = ap_obs_data[ii]->gpsLoc.gpsLocation.latitude;
            info.gpsLoc.gpsLocation.longitude = ap_obs_data[ii]->gpsLoc.gpsLocation.longitude;
            info.gpsLoc.gpsLocation.altitude = ap_obs_data[ii]->gpsLoc.gpsLocation.altitude;
            info.gpsLoc.gpsLocation.speed = ap_obs_data[ii]->gpsLoc.gpsLocation.speed;
            info.gpsLoc.gpsLocation.bearing = ap_obs_data[ii]->gpsLoc.gpsLocation.bearing;
            info.gpsLoc.gpsLocation.accuracy = ap_obs_data[ii]->gpsLoc.gpsLocation.accuracy;
            info.gpsLoc.gpsLocation.verticalAccuracy =
                    ap_obs_data[ii]->gpsLoc.gpsLocation.vertUncertainity;
            info.gpsLoc.gpsLocation.timestamp = ap_obs_data[ii]->gpsLoc.gpsLocation.timestamp;
            info.gpsLoc.gpsLocation.locationTechnologyMask = ap_obs_data[ii]->gpsLoc.tech_mask;
         }
         info.cellInfo.cell_type = ap_obs_data[ii]->cellInfo.cell_type;
         info.cellInfo.cell_id1 = ap_obs_data[ii]->cellInfo.cell_id1;
         info.cellInfo.cell_id2 = ap_obs_data[ii]->cellInfo.cell_id2;
         info.cellInfo.cell_id3 = ap_obs_data[ii]->cellInfo.cell_id3;
         info.cellInfo.cell_id4 = ap_obs_data[ii]->cellInfo.cell_id4;

         std::vector<LocAidlApScanData> apScanDataList;
         info.ap_scan_info_size = (uint32_t) ap_obs_data[ii]->ap_scan_info_size;
         for (size_t jj = 0; jj <(size_t) info.ap_scan_info_size; jj++) {
            LocAidlApScanData scanData;
            scanData.mac_R48b = ap_obs_data[ii]->ap_scan_info[jj].mac_R48b;
            scanData.rssi = ap_obs_data[ii]->ap_scan_info[jj].rssi;
            scanData.age_usec = ap_obs_data[ii]->ap_scan_info[jj].age_usec;

            scanData.channel_id = ap_obs_data[ii]->ap_scan_info[jj].channel_id;
            scanData.ssid = std::string(ap_obs_data[ii]->ap_scan_info[jj].ssid);
            scanData.ssid_valid_byte_count =
                    ap_obs_data[ii]->ap_scan_info[jj].ssid_valid_byte_count;
            apScanDataList.push_back(scanData);
         }
         info.ap_scan_info = apScanDataList;

         apObsDataList.push_back(info);
    }
    apListSize = (int32_t) ap_obs_data_size;

    sCallback->apObsLocDataUpdateCallback(apObsDataList, apListSize,
            (LocAidlWifiDBListStatus)apListStatus);
}

static void DBProviderServiceRequestCb(void* /*client data*/) {

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    sCallback->serviceRequestCallback();
}

static void DBProviderEnvNotifyCb(void* /*client data*/) {

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    sCallback->attachVmOnCallback();
}

::ndk::ScopedAStatus LocAidlWiFiDBProvider::registerWiFiDBProvider(
        const std::shared_ptr<ILocAidlWiFiDBProviderCallback>& callback) {
    FROM_AIDL_CLIENT();

    sCallback = callback;

    void *clientData = NULL;
    // Clean up previous wrapper
    if (mIzatWiFiDBProviderApiClient != nullptr) {
        delete (WiFiDBProviderWrapper*)mIzatWiFiDBProviderApiClient;
        mIzatWiFiDBProviderApiClient = nullptr;
    }

    mIzatWiFiDBProviderApiClient = new WiFiDBProviderWrapper(DBProviderApListUpdateCb,
                                                            DBProviderServiceRequestCb,
                                                            DBProviderEnvNotifyCb, clientData);
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->unregisterFromPeer(mCallbackIface->asBinder().get(), this);
    }
    mCallbackIface = callback;
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->registerToPeer(mCallbackIface->asBinder().get(), this);
    } else {
        LOC_LOGe("callback is nullptr!");
    }
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlWiFiDBProvider::unregisterWiFiDBProvider() {
    if (mIzatWiFiDBProviderApiClient != nullptr) {
        delete (WiFiDBProviderWrapper*)mIzatWiFiDBProviderApiClient;
        mIzatWiFiDBProviderApiClient = nullptr;
    }
    sCallback = nullptr;
    return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus LocAidlWiFiDBProvider::sendAPObsLocDataRequest() {
    FROM_AIDL_CLIENT();

    if (mIzatWiFiDBProviderApiClient == nullptr) {
        LOC_LOGE("WiFiDBProvider API Client null !");
        return ndk::ScopedAStatus::ok();
    }
   ((WiFiDBProviderWrapper *)mIzatWiFiDBProviderApiClient)->sendAPOBSLocDataReq();

    return ndk::ScopedAStatus::ok();
}


// Methods from ::android::hidl::base::IBase follow.

//ILocAidlWiFiDBProvider* HIDL_FETCH_ILocAidlWiFiDBProvider(const char* /* name */) {
//    return new LocAidlWiFiDBProvider();
//}

}  // namespace implementation
}  // namespace aidl
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
