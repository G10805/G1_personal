/*
 * Vehicle_Property.h
 * Author: jramakr1@visteon.com
 */

#ifndef __Vehicle_Property_Provider_H__
#define __Vehicle_Property_Provider_H__

#include <android/hardware/automotive/vehicle/2.2/IVehicle.h>
#include <android/hardware/automotive/vehicle/2.2/types.h>

#include <log/log.h>
#include <thread>
#include <string>

using android::hardware::automotive::vehicle::V2_2::IVehicle;
using android::hardware::automotive::vehicle::V2_2::VehicleProperty;
using android::hardware::automotive::vehicle::V2_2::VehiclePowerStateReport;

using android::hardware::automotive::vehicle::V2_0::IVehicleCallback;
using android::hardware::automotive::vehicle::V2_0::VehiclePropertyStatus;
using android::hardware::automotive::vehicle::V2_0::VehiclePropValue;
using android::hardware::automotive::vehicle::V2_0::StatusCode;
using android::hardware::automotive::vehicle::V2_0::SubscribeFlags;
using android::hardware::automotive::vehicle::V2_0::SubscribeOptions;
using android::hardware::automotive::vehicle::V2_0::VehiclePropConfig;

using android::sp;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;


class Vehicle_Property
{
    sp<IVehicle> pVhal = nullptr;
    std::vector<int32_t> propIdList;
    static Vehicle_Property *instance;
    Vehicle_Property();
    bool subscribeToVHal(sp<IVehicleCallback> listener);
    bool unsubscribeToVHal(sp<IVehicleCallback> listener);
    void processVhalProperty(VehiclePropValue propValue);
    Return<void> onPropertyEvent(const hidl_vec <VehiclePropValue> & values);

    class VhalHidlCallbackListener :  public IVehicleCallback {
        Vehicle_Property *mSdpInterface;
        public:
        VhalHidlCallbackListener(Vehicle_Property *SdpPtr) : mSdpInterface(SdpPtr) {
            ALOGW("[%s] VhalHidlCallbackListener Vhal Listener has been created ", __func__);
        }
        Return<void> onPropertyEvent(const hidl_vec<VehiclePropValue> & values) {
#ifdef GNSS_VMSG
            ALOGW("[%s] VhalHidlCallbackListener :: onPropertyEvent Vhal Listener has been called ", __func__);
#endif //GNSS_VMSG
            return mSdpInterface->onPropertyEvent(values);
        }
        Return<void> onPropertySet(const VehiclePropValue & /*value*/){
            return Return<void>();
        }
        Return<void> onPropertySetError(StatusCode /*errorCode*/, int32_t /*propId*/, int32_t /* areaId */) {
            return Return<void>();
        }
        ~VhalHidlCallbackListener( ){
            ALOGW("[%s] ~ VhalHidlCallbackListener looks like vhal crashed ", __func__);
        }
    };

    sp<VhalHidlCallbackListener> mVhalHidlCallbackListener;

    std::thread mThread;
    bool mRunning = false;
    int g_device_fd = -1;
    public:
    ~Vehicle_Property();
    static Vehicle_Property *getInstance();
};
#endif
