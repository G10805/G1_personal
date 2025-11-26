/******************************************************************************
* File: VtsBtHfAudio.cpp
*
* Copyright (c) 2024 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
* Not a Contribution.
******************************************************************************//*
* Copyright (C) 2019 The Android Open Source Project
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

#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/BnBluetoothHandsfreeAudio.h>
#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/IBluetoothHandsfreeAudio.h>
#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/HandsfreeAudioParameter.h>
#include <android/binder_ibinder.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>
#include <utils/Log.h>
#include <vector>

using ::aidl::vendor::qti::hardware::bluetooth::handsfree_audio::BnBluetoothHandsfreeAudio;
using ::aidl::vendor::qti::hardware::bluetooth::handsfree_audio::IBluetoothHandsfreeAudio;
using ::aidl::vendor::qti::hardware::bluetooth::handsfree_audio::HandsfreeAudioParameter;

static constexpr char kAidlHalServiceName[] =
    "vendor.qti.hardware.bluetooth.handsfree_audio.IBluetoothHandsfreeAudio/default";
static constexpr char kHelpMsg[]=
    "VtsBtHfAudio usage\n"
    "Start BT HF audio\n"
    "  command1: VtsBtHfAudio\n"
    "  command2: VtsBtHfAudio start\n"
    "Stop BT HF audio\n"
    "  command1: VtsBtHfAudio stop\n";
static constexpr char kStartCommand[] = "start";
static constexpr char kStopCommand[] = "stop";

std::shared_ptr<IBluetoothHandsfreeAudio> aidlHfAudio;

static void start_aidl(void)
{
    ABinderProcess_startThreadPool();

    ::ndk::SpAIBinder binder(AServiceManager_waitForService(kAidlHalServiceName));
    aidlHfAudio = IBluetoothHandsfreeAudio::fromBinder(binder);
    if (aidlHfAudio != nullptr) {
      ALOGI("Using the AIDL interface");
      ::ndk::ScopedAIBinder_DeathRecipient aidl_death_recipient =
          ::ndk::ScopedAIBinder_DeathRecipient(AIBinder_DeathRecipient_new([](void* cookie) {
            ALOGE("Bluetooth Handsfree audio service died. Calling exit(0);");
            exit(0);
          }));

      auto death_link =
          AIBinder_linkToDeath(aidlHfAudio->asBinder().get(), aidl_death_recipient.get(),
                               nullptr);

      if (death_link != STATUS_OK)
      {
          ALOGE("Unable to set the death recipient for the Bluetooth Handsfree audio HAL");
          abort();
      }
    }
}

static void send_start_audio(void)
{
    std::shared_ptr<HandsfreeAudioParameter> param = std::make_shared<HandsfreeAudioParameter>();
    param->samplingRate = 16000;
    param->volume = 15;
    aidlHfAudio->start(*param);
}

static void send_stop_audio(void)
{
    aidlHfAudio->stop();
}

static void output_help(void)
{
    ALOGE("Invalid input");
    fprintf(stderr, "%s\n", kHelpMsg);
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        output_help();
        return 0;
    }

    start_aidl();
    ALOGD("start_aidl done");

    if (argc == 1 || strcmp(kStartCommand, argv[1]) == 0)
    {
        send_start_audio();
        ALOGD("send_start_audio done");
    }
    else if (strcmp(kStopCommand, argv[1]) == 0)
    {
        send_stop_audio();
        ALOGD("send_stop_audio done");
    }
    else
    {
        output_help();
    }

    return 0;
}
