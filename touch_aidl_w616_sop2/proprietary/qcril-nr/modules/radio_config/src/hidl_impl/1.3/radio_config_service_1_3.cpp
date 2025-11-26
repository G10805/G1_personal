/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef QMI_RIL_UTF

#include "hidl_impl/1.3/radio_config_service_1_3.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_3 {
namespace implementation {

template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_3::IRadioConfig>::getVersion() {
  static HalServiceImplVersion version(1, 3);
  return version;
}

static void __attribute__((constructor)) registerRadioConfigImpl_1_3();
void registerRadioConfigImpl_1_3() {
  QCRIL_LOG_INFO("Calling registerRadioConfigImpl_1_3");
  getHalServiceImplFactory<RadioConfigServiceBase>()
      .registerImpl<RadioConfigServiceImpl<V1_3::IRadioConfig>>();
}

}  // namespace implementation
}  // namespace V1_3
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif
