/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_UTILS_2_0_H_
#define _QTI_RADIO_SERVICE_UTILS_2_0_H_

#include <android/hardware/radio/1.0/types.h>
#include <vendor/qti/hardware/radio/qtiradio/2.0/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.0/IQtiRadioIndication.h>
#include <vendor/qti/hardware/radio/qtiradio/2.0/IQtiRadioResponse.h>

#include <telephony/ril.h>
#include "interfaces/nas/nas_types.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_0::EnableStatus convert_five_g_status(five_g_status status);
V2_0::DcParam convert_five_g_endc_dcnr(five_g_endc_dcnr& endc_dcnr_info);
void initialize(V2_0::DcParam& dc_param);
V2_0::SignalStrength convert_five_g_signal_strength(five_g_signal_strength& signal_strength);
void initialize(V2_0::SignalStrength& ss);
V2_0::BearerStatus convert_five_g_bearer_status(five_g_bearer_status bearer_status);

}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_UTILS_2_0_H_
