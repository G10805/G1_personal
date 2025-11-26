/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "hidl_impl/2.0/qti_radio_service_2_0.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_0 {
namespace implementation {

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_0::IQtiRadio>::getVersion() {
  static HalServiceImplVersion version(2, 0);
  return version;
}

static void __attribute__((constructor)) registerQtiRadioImpl_2_0();
void registerQtiRadioImpl_2_0() {
  QCRIL_LOG_INFO("Calling registerQtiRadioImpl_2_0");
  getHalServiceImplFactory<QtiRadioServiceBase>().registerImpl<QtiRadioImpl<V2_0::IQtiRadio>>();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
