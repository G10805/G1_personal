/******************************************************************************
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#include "HalServiceImplFactory.h"
#include "hidl_impl/1.0/qti_audio_service_1_0.h"

#undef TAG
#define TAG "RILQ"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace am {
namespace V1_0 {
namespace implementation {

template <>
const HalServiceImplVersion& QtiAudioImpl<V1_0::IQcRilAudio>::getVersion() {
  static HalServiceImplVersion version(1, 0);
  return version;
}

static void __attribute__((constructor)) registerQtiAudioImpl_1_0();
void registerQtiAudioImpl_1_0() {
  QCRIL_LOG_INFO("Calling registerQtiAudioImpl_1_0");
  getHalServiceImplFactory<QtiAudioServiceBase>().registerImpl<QtiAudioImpl<V1_0::IQcRilAudio>>();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace am
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
