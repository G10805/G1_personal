/******************************************************************************
#  Copyright (c) 2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "qti_radio_config_aidl_service_utils.h"
namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradioconfig {
namespace utils {

Msim_Preference convert_msim_preference(aidlqtiradioconfig::MsimPreference pref) {
    switch (pref) {
        case aidlqtiradioconfig::MsimPreference::DSDA:
          return MSIM_PREFERENCE_DSDA;
        case aidlqtiradioconfig::MsimPreference::DSDS:
          return MSIM_PREFERENCE_DSDS;
        default:
          return MSIM_PREFERENCE_INVALID;
    }
}

}  // namespace utils
}  // namespace qtiradioconfig
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
