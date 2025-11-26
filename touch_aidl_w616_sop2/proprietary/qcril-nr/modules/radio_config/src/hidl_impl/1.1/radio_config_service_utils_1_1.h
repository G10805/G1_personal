/******************************************************************************
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef __RADIO_CONFIG_SERVICE_UTILS_1_1_H_
#define __RADIO_CONFIG_SERVICE_UTILS_1_1_H_

#include <android/hardware/radio/config/1.1/IRadioConfig.h>
#include <android/hardware/radio/config/1.1/types.h>
#include "MessageCommon.h"
#include "interfaces/nas/nas_types.h"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace utils {

config::V1_1::PhoneCapability convertPhoneCapabilityToHal(
    const qcril::interfaces::RIL_PhoneCapability& ril_phoneCap);
void convertErrorToHidl(rildata::ResponseError_t rilError, ::android::hardware::radio::V1_0::RadioError &hidlError);

}  // namespace utils
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // __RADIO_CONFIG_SERVICE_UTILS_1_1_H_
