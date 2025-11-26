/******************************************************************************
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef QMI_RIL_UTF

#include "HalServiceImplFactory.h"
#include "hidl_impl/1.0/radio_config_service_1_0.h"

#undef TAG
#define TAG "RILQ"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_0 {
namespace implementation {

template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_0::IRadioConfig>::getVersion() {
  static HalServiceImplVersion version(1, 0);
  return version;
}

static void __attribute__((constructor)) registerRadioConfigImpl_1_0();
void registerRadioConfigImpl_1_0() {
  QCRIL_LOG_INFO("Calling registerRadioConfigImpl_1_0");
  getHalServiceImplFactory<RadioConfigServiceBase>()
      .registerImpl<RadioConfigServiceImpl<V1_0::IRadioConfig>>();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif
