/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */
/*
 * Copyright 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "aidl.android.hardware.bluetooth.service-qti.default"

#include <aidl/android/hardware/bluetooth/BnBluetoothHci.h>
#include <aidl/android/hardware/bluetooth/IBluetoothHci.h>
#ifdef ENABLE_BT_HF_AUDIO_RPC
#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/BnBluetoothHandsfreeAudio.h>
#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/IBluetoothHandsfreeAudio.h>
#endif
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <cutils/properties.h>
#include <utils/Log.h>

#include <iostream>
#include <chrono>
#include <thread>

#include "BluetoothHci.h"
#include "BluetoothHciApi.h"
#ifdef ENABLE_BT_HF_AUDIO_RPC
#include "bluetooth_handsfree_audio_api.h"
#endif
using ::aidl::android::hardware::bluetooth::implementation::BluetoothHci;
using ::aidl::android::hardware::bluetooth::BnBluetoothHci;
#ifdef ENABLE_BT_HF_AUDIO_RPC
using ::aidl::vendor::qti::hardware::bluetooth::handsfree_audio::BnBluetoothHandsfreeAudio;
using ::aidl::vendor::qti::hardware::bluetooth::handsfree_audio::IBluetoothHandsfreeAudio;
#endif

static std::shared_ptr<BnBluetoothHci> CreateBluetoothHci(const char* name) {
  char value[PROPERTY_VALUE_MAX] = { '\0' };
  property_get("persist.vendor.bluetooth.hal_mode", value, "");

#ifdef ENABLE_BT_HAL_RPC
  if (!strcmp(value, "rpc")) {
    return std::static_pointer_cast<BnBluetoothHci>(create_bluetooth_hci_rpc(name));
  }
#endif

  return ndk::SharedRefBase::make<BluetoothHci>();
}

#ifdef ENABLE_BT_HF_AUDIO_RPC
static std::shared_ptr<BnBluetoothHandsfreeAudio> CreateBluetoothHandsfreeAudio(const char* name) {
  char value[PROPERTY_VALUE_MAX] = { '\0' };
  property_get("persist.vendor.bluetooth_handsfree_audio.hal_mode", value, "");

  if (!strcmp(value, "rpc")) {
    /* Add delay to avoid two vsomeip applications are connecting to routing socket
     * at the same time which results into failure in starting vsomeip
     */
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::static_pointer_cast<BnBluetoothHandsfreeAudio>(create_bluetooth_handsfree_audio_rpc(name));
  }

  return nullptr;
}
#endif

int main(int /* argc */, char** /* argv */) {
  ALOGI("Bluetooth HAL starting");
  if (!ABinderProcess_setThreadPoolMaxThreadCount(1)) {
    ALOGI("Failed to set thread pool max thread count");
    return 1;
  }

  std::shared_ptr<BnBluetoothHci> service = CreateBluetoothHci("default");
  if (service == nullptr) {
    ALOGE("Fail to create BluetoothHci");
    return 1;
  }
  std::string instance = std::string() + BluetoothHci::descriptor + "/default";
  auto result =
      AServiceManager_addService(service->asBinder().get(), instance.c_str());

#ifdef ENABLE_BT_HF_AUDIO_RPC
  std::shared_ptr<BnBluetoothHandsfreeAudio> service_hfaudio = CreateBluetoothHandsfreeAudio("default");
  if (service_hfaudio != nullptr) {
    ALOGI("Bluetooth handsfree audio RPC starting");
    std::string instance_hfaudio = std::string() + IBluetoothHandsfreeAudio::descriptor + "/default";
    result = result ||
      AServiceManager_addService(service_hfaudio->asBinder().get(), instance_hfaudio.c_str());
  }
#endif

  if (result == STATUS_OK) {
    ABinderProcess_joinThreadPool();
  } else {
    ALOGE("Could not register as a service!");
  }
  return 0;
}
