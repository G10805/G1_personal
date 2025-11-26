/*
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "AutoPower"
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0


#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <utils/Log.h>
#include <pthread.h>
#include <cutils/properties.h>

#include "AutoPower.h"
#include "auto_audio_ext.h"
#include "auto_audio_ext_v2.h"

#define MAX_RETRY_TIME                  1000
#define VEHICLE_INIT_SLEEP_WAIT         1000 /* 100 ms */
#define AUDIOD_MAX_TASK_NAME_LEN        64

namespace aafap = aidl::android::frameworks::automotive::powerpolicy;
using aafap::CarPowerPolicy;
using aafap::CarPowerPolicyFilter;
using aafap::PowerComponent;
using android::frameworks::automotive::powerpolicy::hasComponent;
using ::ndk::ScopedAStatus;

constexpr PowerComponent kAudioComponent = PowerComponent::AUDIO;
const char* kPowerPolicyDaemon = "android.frameworks.automotive.powerpolicy.ICarPowerPolicyServer/default";
pthread_t    pwThread = {};

static char g_audio_framework[PROPERTY_VALUE_MAX];

AutoPower::AutoPower() {
    property_get("ro.boot.audio", g_audio_framework, NULL);
    ALOGD("AutoPower Constructor");
}

AutoPower::~AutoPower() {
}

void AutoPower::onInitFailed() {
    ALOGE("Initializing power policy client failed");
}

std::vector<PowerComponent> AutoPower::getComponentsOfInterest() {
    std::vector<PowerComponent> components{kAudioComponent};
    return components;
}

ScopedAStatus AutoPower::onPolicyChanged(const CarPowerPolicy& powerPolicy) {
	ALOGI("Power policy: onPolicyChanged");
    if (hasComponent(powerPolicy.enabledComponents, kAudioComponent)) {
        ALOGI("Power policy: Audio component is enabled");
        if (!strcmp(g_audio_framework, "audioreach")) {
            auto_audio_ext_ar_enable_hostless_all();
        } else {
            auto_audio_ext_enable_hostless_all();
        }
    } else if (hasComponent(powerPolicy.disabledComponents, kAudioComponent)) {
        ALOGI("Power policy: Audio component is disabled");
        if (!strcmp(g_audio_framework, "audioreach")) {
            auto_audio_ext_ar_disable_hostless_all();
        } else {
            auto_audio_ext_disable_hostless_all();
        }
    }
    return ScopedAStatus::ok();
}

bool AutoPower :: checkPowerPolicy(){
    ndk::SpAIBinder binder(AServiceManager_getService(kPowerPolicyDaemon));
     if (binder.get() == nullptr) {
         ALOGE("Failed to get car power policy");
         return false;
     }
     return true;
}

static void * power_policy_client_init (void *arg)
{
    std::shared_ptr<AutoPower> mAutoPowerClient;
    mAutoPowerClient = ::ndk::SharedRefBase::make<AutoPower>();

    if (arg != NULL)
	    ALOGV("dummy log for compiler");

    ABinderProcess_setThreadPoolMaxThreadCount(1);
    mAutoPowerClient->init();
    ABinderProcess_joinThreadPool();
    return NULL;
}

void AutoPower::initialize() {
    ALOGD("initialize is called");
    int retryCount = 0;
    do {
            if (checkPowerPolicy()) {
                ALOGE("Init power policy OK");
                break;
            } else {
                if (++retryCount <= MAX_RETRY_TIME) {
                    ALOGE("Sleeping for 100 ms");
                    usleep(VEHICLE_INIT_SLEEP_WAIT*1000);
                } else {
                    ALOGE("Init power policy fail");
                    break;
                }
            }
    } while (1);

    int err = pthread_create(&pwThread, nullptr, power_policy_client_init, NULL);
    if (err != 0)
	    ALOGE(" Power_policy_client_init failed");
}
