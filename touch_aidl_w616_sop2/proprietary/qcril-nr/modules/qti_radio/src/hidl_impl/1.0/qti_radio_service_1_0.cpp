/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "HalServiceImplFactory.h"
#include "hidl_impl/1.0/qti_radio_service_1_0.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {
namespace implementation {

template <>
const HalServiceImplVersion& QtiRadioImpl<V1_0::IQtiRadio>::getVersion() {
  static HalServiceImplVersion version(1, 0);
  return version;
}

static void __attribute__((constructor)) registerQtiRadioImpl_1_0();
void registerQtiRadioImpl_1_0() {
  QCRIL_LOG_INFO("Calling registerQtiRadioImpl_1_0");
  getHalServiceImplFactory<QtiRadioServiceBase>().registerImpl<QtiRadioImpl<V1_0::IQtiRadio>>();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
