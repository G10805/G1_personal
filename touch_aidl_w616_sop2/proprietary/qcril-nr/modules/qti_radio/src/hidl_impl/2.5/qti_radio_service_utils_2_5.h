/******************************************************************************
  Copyright (c) 2017,2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_UTILS_2_5_H_
#define _QTI_RADIO_SERVICE_UTILS_2_5_H_

#include <vendor/qti/hardware/radio/qtiradio/2.5/types.h>

#include <telephony/ril.h>
#include "interfaces/nas/nas_types.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_5::NrConfig convert_nr_disable_mode(RIL_NR_DISABLE_MODE mode);
RIL_NR_DISABLE_MODE convert_nr_config(V2_5::NrConfig config);

}  // namespace utils
}  // namespace qtiradio
}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
#endif  // _QTI_RADIO_SERVICE_UTILS_2_5_H_
