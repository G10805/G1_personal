// FIXME: your file license if you have one

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <log/log.h>
#include "Touch.h"

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

    /*
        USED TO DECODE
        I: Bus=0000 Vendor=0000 Product=0000 Version=0000
        N: Name="himax-touchscreen"
        P: Phys=
        S: Sysfs=/devices/virtual/input/input0
        U: Uniq=
        H: Handlers=event0
        B: PROP=2
        B: EV=b
        B: KEY=10 0 0 0 400 0 2000000 40000800 4000000000 0
        B: ABS=665800000000000
    */
    std::vector<std::unordered_map<std::string, std::string>> retrieveEventIdForEachDisplay(const std::string& path);

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" ITouch* HIDL_FETCH_ITouch(const char* name);

}  // namespace vendor::visteon::hardware::interfaces::touch::implementation
