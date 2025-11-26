/******************************************************************************
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef QMI_RIL_UTF

#include "hidl_impl/1.2/radio_config_service_1_2.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_2 {
namespace implementation {

// Note:
// The V1_2 contains only IRadioConfigResponse.hal and IRadioConfigIndication.hal
// Use V1_1::IRadioConfig

template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_1::IRadioConfig>::getVersion() {
  static HalServiceImplVersion version(1, 2);
  return version;
}

static void __attribute__((constructor)) registerRadioConfigImpl_1_2();
void registerRadioConfigImpl_1_2() {
  QCRIL_LOG_INFO("Calling registerRadioConfigImpl_1_2");
  getHalServiceImplFactory<RadioConfigServiceBase>()
      .registerImpl<RadioConfigServiceImpl<V1_1::IRadioConfig>>();
}

}  // namespace implementation
}  // namespace V1_2
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif
