/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "hidl_impl/2.4/qti_radio_service_2_4.h"
#include "HalServiceImplFactory.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_4 {
namespace implementation {

template <>
const HalServiceImplVersion& QtiRadioImpl<V2_4::IQtiRadio>::getVersion() {
  static HalServiceImplVersion version(2, 4);
  return version;
}

static void __attribute__((constructor)) registerQtiRadioImpl_2_4();
void registerQtiRadioImpl_2_4() {
  QCRIL_LOG_INFO("Calling registerQtiRadioImpl_2_4");
  getHalServiceImplFactory<QtiRadioServiceBase>().registerImpl<QtiRadioImpl<V2_4::IQtiRadio>>();
}

}  // namespace implementation
}  // namespace V2_4
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
