/******************************************************************************
  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#ifndef QMI_RIL_UTF

#include "hidl_impl/1.1/radio_config_service_1_1.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_1 {
namespace implementation {

template <>
const HalServiceImplVersion& RadioConfigServiceImpl<V1_1::IRadioConfig>::getVersion() {
  static HalServiceImplVersion version(1, 1);
  return version;
}

static void __attribute__((constructor)) registerRadioConfigImpl_1_1();
void registerRadioConfigImpl_1_1() {
  QCRIL_LOG_INFO("Calling registerRadioConfigImpl_1_1");
  getHalServiceImplFactory<RadioConfigServiceBase>()
      .registerImpl<RadioConfigServiceImpl<V1_1::IRadioConfig>>();
}

}  // namespace implementation
}  // namespace V1_1
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif
