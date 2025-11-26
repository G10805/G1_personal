/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef _QTI_RADIO_SERVICE_UTILS_2_1_H_
#define _QTI_RADIO_SERVICE_UTILS_2_1_H_

#include <vendor/qti/hardware/radio/qtiradio/2.1/types.h>

#include <telephony/ril.h>
#include "interfaces/nas/nas_types.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_1::BearerStatus convert_five_g_bearer_status_2_1(five_g_bearer_status bearer_status);
V2_1::PlmnInfoListStatus convert_five_g_plmn_list_info_status(
    five_g_plmn_info_list_status plmn_list_status);
V2_1::UpperLayerIndStatus convert_five_g_upper_layer_ind_status(
    five_g_upper_layer_ind_status upli_status);
V2_1::UpperLayerIndInfo convert_five_g_upper_layer_ind_info(
    five_g_upper_layer_ind_info& five_g_upli_info);
void initialize(V2_1::UpperLayerIndInfo& upli_info);
V2_1::ConfigType convert_five_g_config_info(five_g_config_type config);

}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif  // _QTI_RADIO_SERVICE_UTILS_2_1_H_
