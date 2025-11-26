/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "qti_radio_service_utils_2_0.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_0::EnableStatus convert_five_g_status(five_g_status status) {
  switch (status) {
    case FIVE_G_STATUS_DISABLED:
      return V2_0::EnableStatus::DISABLED;
    case FIVE_G_STATUS_ENABLED:
      return V2_0::EnableStatus::ENABLED;
    default:
      return V2_0::EnableStatus::INVALID;
  }
}

V2_0::DcParam convert_five_g_endc_dcnr(five_g_endc_dcnr& endc_dcnr_info) {
  V2_0::DcParam dc_param;

  switch (endc_dcnr_info.endc_available) {
    case 0:
      dc_param.endc = V2_0::EndcAvailableType::ENDC_UNAVAILABLE;
      break;
    case 1:
      dc_param.endc = V2_0::EndcAvailableType::ENDC_AVAILABLE;
      break;
    default:
      dc_param.endc = V2_0::EndcAvailableType::INVALID;
      break;
  }

  switch (endc_dcnr_info.restrict_dcnr) {
    case 0:
      dc_param.dcnr = V2_0::RestrictDcnrType::DCNR_UNRESTRICTED;
      break;
    case 1:
      dc_param.dcnr = V2_0::RestrictDcnrType::DCNR_RESTRICTED;
      break;
    default:
      dc_param.dcnr = V2_0::RestrictDcnrType::INVALID;
      break;
  }

  return dc_param;
}

void initialize(V2_0::DcParam& dc_param) {
  dc_param.endc = V2_0::EndcAvailableType::INVALID;
  dc_param.dcnr = V2_0::RestrictDcnrType::INVALID;
}

V2_0::SignalStrength convert_five_g_signal_strength(five_g_signal_strength& signal_strength) {
  V2_0::SignalStrength ss;
  ss.rsrp = signal_strength.rsrp;
  ss.snr = signal_strength.snr;
  return ss;
}

void initialize(V2_0::SignalStrength& ss) {
  ss.rsrp = std::numeric_limits<int32_t>::min();
  ss.snr = std::numeric_limits<int32_t>::min();
}

V2_0::BearerStatus convert_five_g_bearer_status(five_g_bearer_status bearer_status) {
  switch (bearer_status) {
    case FIVE_G_BEARER_STATUS_NOT_ALLOCATED:
      return V2_0::BearerStatus::NOT_ALLOCATED;
    case FIVE_G_BEARER_STATUS_ALLOCATED:
    case FIVE_G_BEARER_STATUS_MMW_ALLOCATED:
      return V2_0::BearerStatus::ALLOCATED;
    default:
      return V2_0::BearerStatus::INVALID;
  }
}

}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
