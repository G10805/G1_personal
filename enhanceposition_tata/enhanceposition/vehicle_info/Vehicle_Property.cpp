/*
 * Vehicle_Property.cpp
 * Author: jramakr1@visteon.com
 */

#include "Vehicle_Property.h"
#include <mutex>
#include <fstream>
#include <thread>
#include <cmath>
#include <iomanip>
#include <cutils/properties.h>
#include <time.h>


extern "C" {
#include "vehicle_info.h"
}

//#define VPTAG "GPS_Vehicle_Property"

std::mutex iLock;

Vehicle_Property* Vehicle_Property::instance = nullptr;

Vehicle_Property* gInstance = nullptr;

float v_speed = 0.0f;
int32_t v_gstatus = 0;

std::thread testThread;

int32_t logLevel = L_VERBOSE;

double lastGearLog=0.0;
double lastSpeedLog=0.0;
bool   resetS2RFlag = true;
typedef enum
{
    V_GEAR_P = 0,
    V_GEAR_D = 1,
    V_GEAR_S = 2,
    V_GEAR_L = 3,
    V_GEAR_N = 4,
    V_GEAR_R = 6
}V_GEAR;


// from android samples
/* return current time in milliseconds */
static double now_ms(void) {

    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;

}

//Defining vehicle_info.h interfaces
int32_t vehicle_set_log_level(void* handle, int32_t level)
{
    UNUSED(handle);
    logLevel = level;
    return V_SUCCESS;
}

int32_t vehicle_init(void** handle)
{
  //ALOGD(VPTAG, "Vehicle_init");
    UNUSED(handle);
    gInstance = Vehicle_Property::getInstance();
    return V_SUCCESS;
}

int32_t vehicle_deinit(void** handle)
{
    //ALOGD(VPTAG, "Vehicle_deinit");
    /*UNUSED(handle);
    if (gInstance != nullptr) {
        delete gInstance;
        gInstance = nullptr;
    }*/
    return V_SUCCESS;
}

int32_t vehicle_start(void* handle)
{
    //ALOGD(VPTAG, "Vehicle_start");
    UNUSED(handle);
    //API will be unused as subscribing for VHAL events as part of init()
    return V_SUCCESS;
}

int32_t vehicle_stop(void* handle)
{
    //ALOGD(VPTAG, "Vehicle_stop");
    UNUSED(handle);
    //API will be unused as unsubscribing for VHAL events as part of init()
    return V_SUCCESS;
}

int32_t vehicle_update(void* handle)
{
    //ALOGD(VPTAG, "Vehicle_update");
    UNUSED(handle);
    //Not required this API
    return V_SUCCESS;
}

int32_t vehicle_get_vehicle_speed(void* handle, float* vehicle_speed,
    struct timespec* timestamp)
{
    UNUSED(handle);
    *vehicle_speed = v_speed;
    clock_gettime(CLOCK_BOOTTIME, timestamp);
      double time = now_ms();
    if ((time - lastSpeedLog) > 2000.0){
          lastSpeedLog = time;
       ALOGD("Vehicle_speed : %f", v_speed);
    }
    return V_SUCCESS;
}

int32_t vehicle_get_wheel_speed(void* handle, float* l_front_speed,
    float* r_front_speed, float* l_rear_speed, float* r_rear_speed,
    struct timespec* timestamp)
{
    UNUSED(handle);
    UNUSED(l_front_speed);
    UNUSED(r_front_speed);
    UNUSED(l_rear_speed);
    UNUSED(r_rear_speed);
    UNUSED(timestamp);

    return V_NOTSUPPORT;
}

int32_t vehicle_get_gear(void* handle, int32_t* gear, struct timespec* timestamp)
{
    UNUSED(handle);
    *gear = v_gstatus;
    //ALOGD("Vehicle_GEAR : %d", v_gstatus);
    clock_gettime(CLOCK_BOOTTIME, timestamp);
     double time = now_ms();
    if ((time - lastGearLog) > 2000.0){
          lastGearLog = time;
       ALOGD("Vehicle_GEAR : %d", v_gstatus);
    }
    return V_SUCCESS;
}

int32_t vehicle_get_steering_angle(void* handle, float* angle,
    struct timespec* timestamp) {
    UNUSED(handle);
    UNUSED(angle);
    UNUSED(timestamp);
    return V_NOTSUPPORT;
}

Vehicle_Property* Vehicle_Property::getInstance()
{
    std::lock_guard<std::mutex> guard(iLock);
    if (instance == nullptr) {
        instance = new Vehicle_Property();
    }
    return instance;
}

/*void mockSpeed()
{
    int a[3] = { 20, 30, 40 };
    int i = 0;
    while (1) {
        if (v_gstatus == 6) {
            v_speed = -5;
        }
        else {
            v_speed = a[i];
        }
        if (i == 2) {
            i = 0;
        }
        else {
            i++;
        }
        sleep(3);
    }
}*/


Vehicle_Property::Vehicle_Property() {
    pVhal = IVehicle::getService();
    mVhalHidlCallbackListener = new VhalHidlCallbackListener(this);
    subscribeToVHal(mVhalHidlCallbackListener);
    //testThread = std::thread(mockSpeed);
}

Vehicle_Property::~Vehicle_Property() {
    unsubscribeToVHal(mVhalHidlCallbackListener);
    //testThread.join();
}

bool Vehicle_Property::subscribeToVHal(sp<IVehicleCallback> listener) {
    if (pVhal == nullptr || listener == nullptr) {
        return false;
    }
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::AMT1__CurGearTcu_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::AMT1__CurGearTcuStatus_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::DCT2__GearShftLvrPosDct_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::DCT2__GearShftLvrPosDctStatus_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::AT2__GearShftLvrPos_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::AT2__GearShftLvrPosStatus_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::EMS5__CurrGearEms_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::EMS5__CurrGearEmsStatus_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::IPC1__VehSpdIC_RX_V));
    propIdList.push_back(static_cast<int32_t>(VehicleProperty::IPC1__VehSpdICStatus_RX_V));

    SubscribeOptions optionsData[propIdList.size()];
    SubscribeOptions* optionsPtr = optionsData;
    for (auto propId = propIdList.begin(); propId != propIdList.end(); propId++) {
        optionsPtr->flags = SubscribeFlags::EVENTS_FROM_CAR;
        optionsPtr->propId = *propId;
        optionsPtr++;
    }

    hidl_vec < SubscribeOptions > options;
    options.setToExternal(optionsData, propIdList.size());
    StatusCode status = pVhal->subscribe(listener, options);
    if (status != StatusCode::OK) {
        //ALOGW("%s: VHAL subscription failed with status: %d.", VPTAG, status);
        return false;
    }
    else if (status == StatusCode::OK) {
        //ALOGW("%s: Subscribed to VHAL properties with status: %d.", VPTAG, status);
    }
    return true;
}

bool Vehicle_Property::unsubscribeToVHal(sp<IVehicleCallback> listener) {
    for (auto propId = propIdList.begin(); propId != propIdList.end(); propId++) {
        StatusCode status = pVhal->unsubscribe(listener, *propId);
        if (status != StatusCode::OK) {
            ALOGW("VHAL unsubscribe for propId=%x failed with code %d.", *propId, status);
        }
    }
    return true;
}

// Methods from ::android::hardware::automotive::vehicle::V2_0::IVehicleCallback follow.
Return<void> Vehicle_Property::onPropertyEvent(const hidl_vec <VehiclePropValue>& values) {
    for (unsigned int index = 0; index < (unsigned int)values.size(); index++) {
        VehiclePropValue parameter = values[index];
#if GNSS_VMSG
        ALOGV("[%s] propId=%#x", __func__, parameter.prop);
#endif //GNSS_VMSG
        processVhalProperty(parameter);
    }
    return Return<void>();
}

void Vehicle_Property::processVhalProperty(VehiclePropValue propValue) {
    switch (propValue.prop) {
    case (int32_t)VehicleProperty::AMT1__CurGearTcu_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] >= 2 && propValue.value.int32Values[0] <= 7) {
            v_gstatus = V_GEAR_D;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 1 /*Neutral*/) {
            v_gstatus = V_GEAR_N;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 8 /*Reverse*/) {
            v_gstatus = V_GEAR_R;
            v_speed = -v_speed;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else {
            //Nothing to do
        }
        break;
    case (int32_t)VehicleProperty::AMT1__CurGearTcuStatus_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] != 0 /*If Value is not plausible*/) {
            v_gstatus = V_GEAR_P;

        }
        break;
    case (int32_t)VehicleProperty::DCT2__GearShftLvrPosDct_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] == 0 /*Park*/) {
            v_gstatus = V_GEAR_P;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 5 /*Drive*/) {
            v_gstatus = V_GEAR_D;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 6 /*Neutral*/) {
            v_gstatus = V_GEAR_N;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 7 /*Reverse*/) {
            v_gstatus = V_GEAR_R;
            v_speed = -v_speed;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        break;
    case (int32_t)VehicleProperty::DCT2__GearShftLvrPosDctStatus_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] != 0 /*Value is not plausible*/) {
            //Invalid - so default
            v_gstatus = V_GEAR_P;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        break;
    case (int32_t)VehicleProperty::AT2__GearShftLvrPos_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] == 0 /*Park*/) {
            v_gstatus = V_GEAR_P;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 5 /*Drive*/) {
            v_gstatus = V_GEAR_D;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 6 /*Neutral*/) {
            v_gstatus = V_GEAR_N;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 7 /*Reverse*/) {
            v_gstatus = V_GEAR_R;
            v_speed = -v_speed;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        break;
    case (int32_t)VehicleProperty::AT2__GearShftLvrPosStatus_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] != 0 /*Value is not plausible*/) {
            v_gstatus = V_GEAR_P;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        break;
    case (int32_t)VehicleProperty::EMS5__CurrGearEms_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] >= 2 && propValue.value.int32Values[0] <= 7) {
            v_gstatus = V_GEAR_D;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 1 /*Neutral*/) {
            v_gstatus = V_GEAR_N;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else if (propValue.value.int32Values[0] == 8 /*Reverse*/) {
            v_gstatus = V_GEAR_R;
          //  v_speed = -v_speed; //VISTEON CHANGE
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        else {
            //Nothing to do
        }
          char value[PROPERTY_VALUE_MAX];
           property_get("vendor.pm.suspend_entry",value,"0");
          if(atoi(value) == 2){
             resetS2RFlag = true;
          }else{
             resetS2RFlag = false;
          }
        break;
    case (int32_t)VehicleProperty::EMS5__CurrGearEmsStatus_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] != 0 /*If Value is not plausible*/) {
            v_gstatus = V_GEAR_P;
            ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop, v_gstatus);
        }
        break;
    case (int32_t)VehicleProperty::IPC1__VehSpdIC_RX_V:
        if (propValue.value.floatValues.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.floatValues.size(), 1);
            return;
        }
        //v_speed = propValue.value.floatValues[0]; //mps
        //VISTEON MODIFIED
        v_speed = propValue.value.floatValues[0]; //mps
         if (v_gstatus == V_GEAR_R){
           v_speed = -1 * v_speed;
        }
         
        if (resetS2RFlag && (v_speed > 0.0 || v_speed < 0.0 )  ){
           property_set("vendor.pm.suspend_entry","0");
           resetS2RFlag = false;
        }
        //ALOGI("[%s] propId=%#x kmphSpeed %f", __func__, propValue.prop, v_speed);
        break;
    case (int32_t)VehicleProperty::IPC1__VehSpdICStatus_RX_V:
        if (propValue.value.int32Values.size() < 1) {
            ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                __func__, propValue.prop, (unsigned int)propValue.value.int32Values.size(), 1);
            return;
        }
        if (propValue.value.int32Values[0] != 0 /*If Value is not plausible*/) {
            v_speed = -1;
            ALOGI("[%s] propId=%#x Speed %f", __func__, propValue.prop, v_speed);
        }
        break;
    //default:
        //ALOGE("%s: [%s] Falls to default", VPTAG, __FUNCTION__);
    }
}
