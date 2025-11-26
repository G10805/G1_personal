/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013, 2016-2020, 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifndef LBS_ADAPTER_H
#define LBS_ADAPTER_H

#include <WiperData.h>
#include <stdbool.h>
#include <string.h>
#include <loc_pla.h>
#include <LBSAdapterBase.h>
#include <LocContext.h>

#ifdef __cplusplus
extern "C"
{
#endif

using namespace loc_core;
using namespace lbs_core;
using namespace izat_manager;

enum zpp_fix_type {
    ZPP_FIX_WWAN,
    ZPP_FIX_BEST_AVAILABLE
};

struct WiperSsrInform : public LocMsg {
    inline WiperSsrInform() : LocMsg() {}
    virtual void proc() const;
};

struct WiperApDataRequest : public LocMsg {
    const WifiApDataRequest mRequest;
    inline WiperApDataRequest(const WifiApDataRequest &request) :
        LocMsg(), mRequest(request) {}
    virtual void proc() const;
};

struct ZppFixMsg : public LocMsg {
    inline ZppFixMsg (LocGpsLocation &gpsLocation, GpsLocationExtended &locationExtended,
    LocPosTechMask posTechMask) :
    LocMsg(), mPosTechMask(posTechMask), mGpsLocation(gpsLocation), mGpsLocationExtended(locationExtended) {}
    virtual void proc() const;
private:
    LocPosTechMask mPosTechMask;
    LocGpsLocation mGpsLocation;
    GpsLocationExtended mGpsLocationExtended;
};

struct WwanFixMsg : public LocMsg {
    WwanFixMsg(LocGpsLocation *gpsLocation);
    virtual void proc() const;
private:
    LocGpsLocation mGpsLocation;
};

struct TimeZoneInfoRequest : public LocMsg {
    inline TimeZoneInfoRequest() : LocMsg() {}
    virtual void proc() const;
};

class LBSAdapter : public LBSAdapterBase {
    static LBSAdapter* mMe;
protected:
    inline LBSAdapter(const LOC_API_ADAPTER_EVENT_MASK_T mask) :
        LBSAdapterBase(mask,
                LocContext::getLocContext(LocContext::mLocationHalName),
                true),
        mFdclCallbacks(),
        mFdclClientData(nullptr) {}
    inline virtual ~LBSAdapter() {}
public:
    static LBSAdapter* get(const LOC_API_ADAPTER_EVENT_MASK_T mask);

    inline virtual bool requestWifiApData(const WifiApDataRequest &request) {
        sendMsg(new WiperApDataRequest(request));
        return true;
    }

    inline virtual void handleEngineUpEvent() {
        sendMsg(new WiperSsrInform());
    }

    virtual bool requestTimeZoneInfo();

    int cinfoInject(int cid, int lac, int mnc, int mcc, bool roaming);
    int oosInform();
    int niSuplInit(const char* supl_init, int length);
    int wifiEnabledStatusInject(int status);
    int wifiAttachmentStatusInject(WifiSupplicantInfo &wifiSupplicantInfo);
    int injectWifiApInfo(WifiApInfo &wifiApInfo);
    int injectWifiPosition(const Location* location);
    int setWifiWaitTimeoutValue(int timeout);
    int timeInfoInject(long curTimeMillis, int rawOffset, int dstOffset);

    FdclCallbacks mFdclCallbacks;
    void* mFdclClientData;
    void registerFdclCommand(FdclCallbacks&, void* fdclClientData);
    void unregisterFdclCommand();
    void injectFdclDataCommand(FdclData& fdclData);
    void requestFdclDataCommand(int32_t expireInDays);
    bool requestFdclDataEvent();
    bool reportFdclStatusEvent(bool success, const char* errorString, uint32_t errorLength);
    bool reportFdclInfoEvent(FdclInfo& fdclInfo, uint8_t status, UlpLocation& location);

    CellCSCallbacks mCellCSCallbacks;
    void* mCellCSClientData;
    void registerCellCSCommand(CellCSCallbacks&, void* cellCSClientData);
    void unregisterCellCSCommand();
    void requestCellCSDataCommand();
    bool requestCellCSDataEvent();
    bool reportCellCSInfoEvent(CellCSInfo& cellCSInfo, uint8_t status);

    // Zpp related
    int getZppFixRequest(enum zpp_fix_type type);
    bool reportWwanZppFix(LocGpsLocation &zppLoc);
    bool reportZppBestAvailableFix(LocGpsLocation &zppLoc,
            GpsLocationExtended &location_extended, LocPosTechMask tech_mask);

    int enableMask(LOC_API_ADAPTER_EVENT_MASK_T mask);

    // Sap Map related
    void sendMapData(const string maString);
};

#ifdef __cplusplus
}
#endif

#endif /* LBS_ADAPTER_H */
