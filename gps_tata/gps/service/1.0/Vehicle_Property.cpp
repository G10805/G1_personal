/*
 * Vehicle_Property.cpp
 * Author: jramakr1@visteon.com
 */

#include "Vehicle_Property.h"
#include <mutex>
#include <fstream>
#include <cmath>
#include <iomanip>
//#include <string>
//using namespace std;
extern "C" {
#include "vehicle_info.h"
}

#define VPTAG "GPS_Vehicle_Property"

std::mutex iLock;

enum gearStatus {
    UNKNOWN = 0,
    PARk = 1,
    DRIVE = 2,
    NEUTRAL = 3,
    REVERSE = 4,
};

float v_speed = 0.0f;
char v_gstatus = '\0';

static inline void *zalloc(size_t size)
{
    return calloc(1, size);
}


UART_INFO *uart_info = NULL;

int open_uart()
{
    uart_info = (UART_INFO *)zalloc(sizeof(UART_INFO));
    char uart_device[] = "/dev/ttyS1";
    if(V_SUCCESS != vehicle_set_config(uart_info)) {
        ALOGE("set config failed");
        return V_ERROR;
    }
    if (V_SUCCESS != open_uart_device(&uart_info->fd, uart_device)) {
        ALOGE("open uart device failed");
        if (uart_info->fd != -1)
            close(uart_info->fd);
        free(uart_info);
        uart_info = NULL;

        return V_ERROR;
    }
    int flags = fcntl(uart_info->fd, F_SETFL, 0); /* reset file asscee mode, ext:O_NONBLOCK */
    if (flags == -1) {
        ALOGE("fcntl() failed invalid flags=%d reason=[%s]%d", flags, strerror(errno), errno);
        if (uart_info->fd != -1)
            close(uart_info->fd);
        free(uart_info);
        uart_info = NULL;

        return V_ERROR;
    }
    if(uart_can_set_param(uart_info) != 0) {
        ALOGE("set param failed");
        if (uart_info->fd != -1)
            close(uart_info->fd);
        free(uart_info);
        uart_info = NULL;

        return V_ERROR;
    }
    ALOGI("using uart device : %s, fd : %d",uart_device, uart_info->fd);
    return V_SUCCESS;
}

std::string checkSum(char* nmea)
{
    std::string nmeaString (nmea);
    uint32_t j = 0;
    int32_t checksum = 0;
    std::stringstream nmea_stream;

    for (j = nmeaString.find("\""); j < nmeaString.length(); j++) {
        if (nmeaString.at(j) == '*') {
            break;
        }
        checksum = checksum ^ nmeaString.at(j);
    }

    //making string without characters followed by *, to append calculated checksum at end
    nmeaString = nmeaString.substr(0, j+1);
    nmea_stream << std::uppercase << std::setfill('0') <<  std::setw(2) << std::right << std::hex << checksum;
    nmeaString += nmea_stream.str();
    return nmeaString;
}

void mtk_write_thread()
{
    while (1) {
        char cmd[100];
        sprintf(cmd, ">>CAN2=\"%03.2f,%1c*\"", v_speed, v_gstatus);
        std::string command = checkSum(cmd);
        send_data(uart_info->fd, command.c_str(), command.length());
        usleep(20000);
    }
}

Vehicle_Property *Vehicle_Property::instance = nullptr;

Vehicle_Property *Vehicle_Property::getInstance()
{
    std::lock_guard<std::mutex> guard(iLock);
    if (instance == nullptr) {
        instance = new Vehicle_Property();
    }
    return instance;
}

Vehicle_Property::Vehicle_Property() {
    pVhal = IVehicle::getService();
    mVhalHidlCallbackListener = new VhalHidlCallbackListener(this);
    subscribeToVHal(mVhalHidlCallbackListener);
    mRunning = true;
    if (V_SUCCESS != open_uart()) {
        ALOGE("Failed to perform open_uart()");
    } else
        mThread = std::thread(mtk_write_thread);
}

Vehicle_Property::~Vehicle_Property() {
    mRunning = false;
    close(g_device_fd);
    g_device_fd = -1;
    if (mThread.joinable()) {
        mThread.join();
    }
    unsubscribeToVHal(mVhalHidlCallbackListener);
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
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
    SubscribeOptions *optionsPtr = optionsData;
    for (auto propId = propIdList.begin(); propId != propIdList.end(); propId++) {
        optionsPtr->flags  = SubscribeFlags::EVENTS_FROM_CAR;
        optionsPtr->propId = *propId;
        //if (optionsPtr->propId == 291504647) {
        //    optionsPtr->sampleRate = 10.0f;
        //}
        optionsPtr++;
    }
    
    hidl_vec < SubscribeOptions > options;
    options.setToExternal(optionsData, propIdList.size());
    StatusCode status = pVhal->subscribe(listener, options);
    if (status != StatusCode::OK) {
        ALOGW("%s: VHAL subscription failed with status: %d.", VPTAG, status);
        return false;
    } else if (status == StatusCode::OK) {
#if GNSS_VMSG
        ALOGW("%s: VHAL subscription successed with status: %d.", VPTAG,
                status);
#endif //GNSS_VMSG
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
Return<void> Vehicle_Property::onPropertyEvent(const hidl_vec <VehiclePropValue> & values) {
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
        case (int32_t) VehicleProperty::AMT1__CurGearTcu_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] >= 2 && propValue.value.int32Values[0] <= 7) {
                v_gstatus = 'D';
		//string str = "Drive"
		ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 1 /*Neutral*/) {
                v_gstatus = 'N';
		//string str = "Neutral"
		ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 8 /*Reverse*/) {
                v_gstatus = 'R';
		//string str = "Reverse"
                
		ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else {
                //Nothing to do
            }
            break;
         case (int32_t) VehicleProperty::AMT1__CurGearTcuStatus_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] != 0 /*If Value is not plausible*/) {
                v_gstatus = '\0';
		
            }
            break;
        case (int32_t) VehicleProperty::DCT2__GearShftLvrPosDct_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] == 0 /*Park*/) {
                v_gstatus = 'P';
		//string str = "Park"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 5 /*Drive*/) {
                v_gstatus = 'D';
		//string str = "Drive"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 6 /*Neutral*/) {
                v_gstatus = 'N';
		//string str = "Neutral"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 7 /*Reverse*/) {
                v_gstatus = 'R';
		//string str = "Reverse"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            }
            break;
        case (int32_t) VehicleProperty::DCT2__GearShftLvrPosDctStatus_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] != 0 /*Value is not plausible*/) {
                v_gstatus = '\0';
		//string str = "Invalid"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            }
            break;
        case (int32_t) VehicleProperty::AT2__GearShftLvrPos_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] == 0 /*Park*/) {
                v_gstatus = 'P';
		//string str = "Park"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 5 /*Drive*/) {
                v_gstatus = 'D';
		//string str = "Drive"
               ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 6 /*Neutral*/) {
                v_gstatus = 'N';
		//string str = "Neutral"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            } else if (propValue.value.int32Values[0] == 7 /*Reverse*/) {
                v_gstatus = 'R';
		//string str = "Reverse"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            }
            break;
        case (int32_t) VehicleProperty::AT2__GearShftLvrPosStatus_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] != 0 /*Value is not plausible*/) {
                v_gstatus = '\0';
		//string str = "Invalid"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            }
            break;
        case (int32_t) VehicleProperty::EMS5__CurrGearEms_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            //if (propValue.value.int32Values[0] == (int32_t)VehicleProperty::EMS5__CurrGearEms_RX_V) {
                if (propValue.value.int32Values[0] >= 2 && propValue.value.int32Values[0] <= 7) {
                    v_gstatus = 'D';
		    //string str = "Drive"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
                } else if (propValue.value.int32Values[0] == 1 /*Neutral*/) {
                    v_gstatus = 'N';
		    //string str = "Neutral"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
                } else if (propValue.value.int32Values[0] == 8 /*Reverse*/) {
                    v_gstatus = 'R';
		    //string str = "Reverse"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
                } else {
                   //Nothing to do
                }
            //}
            break;
        case (int32_t) VehicleProperty::EMS5__CurrGearEmsStatus_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] != 0 /*If Value is not plausible*/) {
                v_gstatus = '\0';
		//string str = "Invalid"
                ALOGI("[%s] propId=%#x values %c", __func__, propValue.prop,v_gstatus);
            }
            break;
        case (int32_t) VehicleProperty::IPC1__VehSpdIC_RX_V:
            if (propValue.value.floatValues.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.floatValues.size(), 1);
                return;
            }
            v_speed = propValue.value.floatValues[0]; //mps
            //Convert ms to kmph
            v_speed = v_speed * 3.6; //kmph
            ALOGI("[%s] propId=%#x kmphSpeed %f", __func__, propValue.prop,v_speed);
            break;
        case (int32_t) VehicleProperty::IPC1__VehSpdICStatus_RX_V:
            if (propValue.value.int32Values.size() < 1) {
                ALOGW("[%s] propId=%#x values size(%u), less than expected(%d), returning.",
                        __func__, propValue.prop,(unsigned int) propValue.value.int32Values.size(), 1);
                return;
            }
            if (propValue.value.int32Values[0] != 0 /*If Value is not plausible*/) {
                v_speed = -1;
		//string str = "Invalid"
                ALOGI("[%s] propId=%#x Speed %f", __func__, propValue.prop,v_speed);
            }
            break;
        default:
            ;
#if GNSS_VMSG
            ALOGE("%s: [%s] Falls to default", VPTAG, __FUNCTION__);
#endif //GNSS_VMSG
    }
}
