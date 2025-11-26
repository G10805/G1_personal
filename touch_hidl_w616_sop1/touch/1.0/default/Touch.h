// FIXME: your file license if you have one

#pragma once
#include <sstream>
#include <map>
#include <utils/List.h>
#include <thread>
#include <fcntl.h>    
#include <linux/input.h>
#include <unistd.h>
#include <log/log.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <utility>
//#include <bits/stdc++.h>

#include "Utils.h"
#include <vendor/visteon/hardware/interfaces/touch/1.0/ITouch.h>
#include <vendor/visteon/hardware/interfaces/touch/1.0/ITouchCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "TouchCallback.h"
namespace vendor::visteon::hardware::interfaces::touch::implementation {

using vendor::visteon::hardware::interfaces::touch::V1_0::ITouchCallback;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using namespace std;

struct Touch : public V1_0::ITouch {
    public:
    // Methods from ::vendor::visteon::hardware::interfaces::touch::V1_0::ITouch follow.
    Return<void> registerCallback(const sp<::vendor::visteon::hardware::interfaces::touch::V1_0::ITouchCallback>& obj, int32_t clientID) override;
    Touch();
    ~Touch();
    static sp<Touch> getInstance();
    void handleClientDeath();  
    //void touch_start();  

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    
    static sp<Touch> sInstance;    
    std::map<uint64_t, sp<ITouchCallback>> mCallbacks;
    std::map<uint64_t, sp<ITouchCallback>>::iterator iter;
    std::vector<std::unordered_map<std::string, std::string>> devices;
    std::vector<std::thread> display_thread;
    std::thread display0_thread; 
    std::thread display1_thread;

    const std::string driverPath = "/dev/input/";
    const std::string DISPLAY_0 = "/dev/input/event0";
    const std::string DISPLAY_1 = "/dev/input/event1";
    const std::string device_file = "/proc/bus/input/devices";

    void Display_0(std::string path, int id);
    void Display_1(std::string path, int id);
    void notifyClients(int id);
    
    std::string get_device_name(int fd);
    //void eraseClient();

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" ITouch* HIDL_FETCH_ITouch(const char* name);

}  // namespace vendor::visteon::hardware::interfaces::touch::implementation

