/******************************************************************************
#  Copyright (c) 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "hidl_impl/1.0/qti_oem_hook_service_1_0.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {
namespace implementation {

static void __attribute__((constructor)) registerOemhookImpl_1_0();

void registerOemhookImpl_1_0() {
  QCRIL_LOG_INFO("Calling registerOemhookImpl_1_0");
  getHalServiceImplFactory<OemHookServiceBase>()
      .registerImpl<OemHookServiceImpl<V1_0::IQtiOemHook>>();
}

template <>
const HalServiceImplVersion& OemHookServiceImpl<V1_0::IQtiOemHook>::getVersion() {
  static HalServiceImplVersion version(1, 0);
  return version;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace qcrilhook
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
