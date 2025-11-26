/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_UTILS_2_2_H_
#define _QTI_RADIO_SERVICE_UTILS_2_2_H_

#include <vendor/qti/hardware/radio/qtiradio/2.2/types.h>

#include <telephony/ril.h>
#include "interfaces/nas/nas_types.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_2::NrIconType convert_five_g_icon_type(five_g_icon_type iconType);

}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_UTILS_2_2_H_
