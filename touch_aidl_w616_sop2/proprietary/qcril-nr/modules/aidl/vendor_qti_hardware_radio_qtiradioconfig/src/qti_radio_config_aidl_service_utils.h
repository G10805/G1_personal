/******************************************************************************
#  Copyright (c) 2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef _QTI_RADIO_CONFIG_AIDL_SERVICE_UTILS_H_
#define _QTI_RADIO_CONFIG_AIDL_SERVICE_UTILS_H_
#include <telephony/ril.h>
#include <aidl/vendor/qti/hardware/radio/qtiradioconfig/MsimPreference.h>

namespace aidlqtiradioconfig {
using namespace aidl::vendor::qti::hardware::radio::qtiradioconfig;
}

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradioconfig {
namespace utils {

Msim_Preference convert_msim_preference(aidlqtiradioconfig::MsimPreference pref);

}  // namespace utils
}  // namespace qtiradioconfig
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
#endif //_QTI_RADIO_CONFIG_AIDL_SERVICE_UTILS_H_