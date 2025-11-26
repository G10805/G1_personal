/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "qti_radio_service_utils_2_2.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_2::NrIconType convert_five_g_icon_type(five_g_icon_type iconType) {
  switch (iconType) {
    case FIVE_G_ICON_TYPE_NONE:
      return V2_2::NrIconType::TYPE_NONE;
    case FIVE_G_ICON_TYPE_BASIC:
      return V2_2::NrIconType::TYPE_5G_BASIC;
    case FIVE_G_ICON_TYPE_UWB:
      return V2_2::NrIconType::TYPE_5G_UWB;
    default:
      return V2_2::NrIconType::INVALID;
  }
}

}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
