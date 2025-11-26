/******************************************************************************
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#pragma once

#ifndef __RADIO_CONFIG_SERVICE_UTILS_1_0_H_
#define __RADIO_CONFIG_SERVICE_UTILS_1_0_H_

#include "interfaces/uim/qcril_uim_types.h"
#include <android/hardware/radio/config/1.0/types.h>


namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace utils {

void convertUimSlotStatusToHal(RIL_UIM_SlotStatus &ril_slot_status ,V1_0::SimSlotStatus &slot_status);
char * radio_config_bin_to_hexstring (const uint8_t * data_ptr,
                uint16_t        data_len);

}  // namespace utils
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // __RADIO_CONFIG_SERVICE_UTILS_1_1_H_
