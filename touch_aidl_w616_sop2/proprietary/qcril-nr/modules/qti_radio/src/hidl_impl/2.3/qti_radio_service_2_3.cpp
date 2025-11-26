/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "hidl_impl/2.3/qti_radio_service_2_3.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_3 {
namespace implementation {

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_3::IQtiRadio>::getVersion() {
  static HalServiceImplVersion version(2, 3);
  return version;
}

static void __attribute__((constructor)) registerQtiRadioImpl_2_3();
void registerQtiRadioImpl_2_3() {
  QCRIL_LOG_INFO("Calling registerQtiRadioImpl_2_3");
  getHalServiceImplFactory<QtiRadioServiceBase>().registerImpl<QtiRadioImpl<V2_3::IQtiRadio>>();
}

property_id_type getPropertyId(std::string prop) {
  static std::unordered_map<std::string, property_id_type> propertyMap {
    {"persist.vendor.radio.poweron_opt", PERSIST_VENDOR_RADIO_POWERON_OPT},
    {"persist.vendor.radio.apm_sim_not_pwdn", PERSIST_VENDOR_RADIO_APM_SIM_NOT_PWDN},
    {"persist.vendor.radio.enableadvancedscan", PERSIST_VENDOR_RADIO_ENABLEADVANCEDSCAN},
  };

  auto searchResult = propertyMap.find(prop);
  if (searchResult != propertyMap.end()) {
    return searchResult->second;
  }
  return PROPERTY_ID_MAX;
}

}  // namespace implementation
}  // namespace V2_3
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
