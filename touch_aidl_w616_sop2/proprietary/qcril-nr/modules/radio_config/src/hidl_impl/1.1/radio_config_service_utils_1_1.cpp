/******************************************************************************
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef QMI_RIL_UTF

#include "radio_config_service_utils_1_1.h"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace utils {

config::V1_1::PhoneCapability convertPhoneCapabilityToHal(
    const qcril::interfaces::RIL_PhoneCapability& ril_phoneCap) {
  config::V1_1::PhoneCapability hal_phoneCap{};

  hal_phoneCap.maxActiveData = ril_phoneCap.maxActiveData;
  hal_phoneCap.maxActiveInternetData = ril_phoneCap.maxActiveInternetData;
  hal_phoneCap.isInternetLingeringSupported = ril_phoneCap.isInternetLingeringSupported;

  hal_phoneCap.logicalModemList.resize(ril_phoneCap.logicalModemList.size());

  for (int i = 0; i < ril_phoneCap.logicalModemList.size(); i++) {
    hal_phoneCap.logicalModemList[i].modemId = ril_phoneCap.logicalModemList[i].modemId;
  }

  return hal_phoneCap;
}

void convertErrorToHidl(rildata::ResponseError_t rilError, ::android::hardware::radio::V1_0::RadioError &hidlError) {
    using namespace ::android::hardware::radio::V1_0;
    switch (rilError) {
        case rildata::ResponseError_t::NO_ERROR:
            hidlError = RadioError::NONE;
            break;
        case rildata::ResponseError_t::NOT_AVAILABLE:
            hidlError = RadioError::RADIO_NOT_AVAILABLE;
            break;
        case rildata::ResponseError_t::INTERNAL_ERROR:
            hidlError = RadioError::INTERNAL_ERR;
            break;
        case rildata::ResponseError_t::INVALID_ARGUMENT:
            hidlError = RadioError::INVALID_ARGUMENTS;
            break;
        default:
            hidlError = RadioError::INTERNAL_ERR;
            break;
    }
}

}  // namespace utils
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif
