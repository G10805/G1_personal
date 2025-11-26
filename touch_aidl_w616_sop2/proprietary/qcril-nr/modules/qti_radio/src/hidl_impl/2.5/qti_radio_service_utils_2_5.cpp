/******************************************************************************
  Copyright (c) 2017,2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "qti_radio_service_utils_2_5.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_5::NrConfig convert_nr_disable_mode(RIL_NR_DISABLE_MODE mode) {
  switch (mode) {
    case NR_DISABLE_MODE_NONE:
      return V2_5::NrConfig::NR_CONFIG_COMBINED_SA_NSA;
    case NR_DISABLE_MODE_SA:
      return V2_5::NrConfig::NR_CONFIG_NSA;
    case NR_DISABLE_MODE_NSA:
      return V2_5::NrConfig::NR_CONFIG_SA;
    default:
      return V2_5::NrConfig::NR_CONFIG_INVALID;
  }
}

RIL_NR_DISABLE_MODE convert_nr_config(V2_5::NrConfig config) {
  switch (config) {
    case V2_5::NrConfig::NR_CONFIG_COMBINED_SA_NSA:
      return NR_DISABLE_MODE_NONE;
    case V2_5::NrConfig::NR_CONFIG_NSA:
      return NR_DISABLE_MODE_SA;
    case V2_5::NrConfig::NR_CONFIG_SA:
      return NR_DISABLE_MODE_NSA;
    default:
      return NR_DISABLE_MODE_NONE;
  }
}

}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
