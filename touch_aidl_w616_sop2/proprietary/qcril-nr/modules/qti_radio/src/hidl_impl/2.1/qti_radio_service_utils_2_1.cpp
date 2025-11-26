/******************************************************************************
  Copyright (c) 2017, 2020-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "qti_radio_service_utils_2_1.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace utils {

V2_1::BearerStatus convert_five_g_bearer_status_2_1(five_g_bearer_status bearer_status) {
  switch (bearer_status) {
    case FIVE_G_BEARER_STATUS_NOT_ALLOCATED:
      return V2_1::BearerStatus::NOT_ALLOCATED;
    case FIVE_G_BEARER_STATUS_ALLOCATED:
      return V2_1::BearerStatus::ALLOCATED;
    case FIVE_G_BEARER_STATUS_MMW_ALLOCATED:
      return V2_1::BearerStatus::MMW_ALLOCATED;
    default:
      return V2_1::BearerStatus::INVALID;
  }
}

V2_1::PlmnInfoListStatus convert_five_g_plmn_list_info_status(
    five_g_plmn_info_list_status plmn_list_status) {
  switch (plmn_list_status) {
    case FIVE_G_PLMN_LIST_UNAVAILABLE:
      return V2_1::PlmnInfoListStatus::UNAVAILABLE;
    case FIVE_G_PLMN_LIST_AVAILABLE:
      return V2_1::PlmnInfoListStatus::AVAILABLE;
    default:
      return V2_1::PlmnInfoListStatus::INVALID;
  }
}

V2_1::UpperLayerIndStatus convert_five_g_upper_layer_ind_status(
    five_g_upper_layer_ind_status upli_status) {
  switch (upli_status) {
    case FIVE_G_UPPER_LAYER_IND_UNAVAILABLE:
      return V2_1::UpperLayerIndStatus::UNAVAILABLE;
    case FIVE_G_UPPER_LAYER_IND_AVAILABLE:
      return V2_1::UpperLayerIndStatus::AVAILABLE;
    default:
      return V2_1::UpperLayerIndStatus::INVALID;
  }
}

V2_1::UpperLayerIndInfo convert_five_g_upper_layer_ind_info(
    five_g_upper_layer_ind_info& five_g_upli_info) {
  V2_1::UpperLayerIndInfo upli_info;

  upli_info.plmnInfoList = convert_five_g_plmn_list_info_status(five_g_upli_info.plmn_list_status);
  upli_info.upplerLayerInd =
      convert_five_g_upper_layer_ind_status(five_g_upli_info.upper_layer_ind_info_status);

  return upli_info;
}

void initialize(V2_1::UpperLayerIndInfo& upli_info) {
  upli_info.plmnInfoList = V2_1::PlmnInfoListStatus::INVALID;
  upli_info.upplerLayerInd = V2_1::UpperLayerIndStatus::INVALID;
}

V2_1::ConfigType convert_five_g_config_info(five_g_config_type config) {
  switch (config) {
    case FIVE_G_CONFIG_TYPE_NSA:
      return V2_1::ConfigType::NSA_CONFIGURATION;
    case FIVE_G_CONFIG_TYPE_SA:
      return V2_1::ConfigType::SA_CONFIGURATION;
    default:
      return V2_1::ConfigType::INVALID;
  }
}

}  // namespace utils
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
