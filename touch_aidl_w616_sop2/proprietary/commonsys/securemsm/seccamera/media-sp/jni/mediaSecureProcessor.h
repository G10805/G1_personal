/*
 * Copyright (c) 2019, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ANDROID_MEDIA_MEDIASECUREPROCESSOR_H_
#define _ANDROID_MEDIA_MEDIASECUREPROCESSOR_H_

#include "jni.h"
#define UNUSED(x) ((void)(x))
#include <aidl/vendor/qti/hardware/secureprocessor/config/ConfigTag.h>
#include <aidl/vendor/qti/hardware/secureprocessor/device/BnSecureProcessor.h>
#include <android/binder_manager.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <vendor/qti/hardware/secureprocessor/config/1.0/types.h>
#include <vendor/qti/hardware/secureprocessor/device/1.0/ISecureProcessor.h>

using ISecureProcessorHidl =
    vendor::qti::hardware::secureprocessor::device::V1_0::ISecureProcessor;
using ISecureProcessorAidl =
    ::aidl::vendor::qti::hardware::secureprocessor::device::ISecureProcessor;
using ErrorCodeHidl =
    vendor::qti::hardware::secureprocessor::common::V1_0::ErrorCode;
using ErrorCodeAidl =
    ::aidl::vendor::qti::hardware::secureprocessor::common::ErrorCode;
using ConfigTagHidl =
    vendor::qti::hardware::secureprocessor::config::V1_0::ConfigTag;
using ConfigTagAidl =
    ::aidl::vendor::qti::hardware::secureprocessor::config::ConfigTag;
using sessionIDout =
    ::aidl::vendor::qti::hardware::secureprocessor::device::sessionIDout;
using android::hardware::hidl_array;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_vec;
using android::hidl::base::V1_0::IBase;

namespace android {
struct JMediaSecureProcessor : public RefBase {
public:
  JMediaSecureProcessor(JNIEnv *env, jobject thiz, const char *service);
  static sp<ISecureProcessorHidl> GetSecureProcessor(JNIEnv *env, jobject obj);

  static int mediaSecureProcessor_createSession_AIDL();
  static uint32_t mediaSecureProcessor_createSession_HIDL(JNIEnv *env,
                                                          jobject thiz);

  static jobject mediaSecureProcessor_getConfig_AIDL(JNIEnv *env,
                                                     jint jsessionId,
                                                     jobject jSessionConfig);
  static jobject mediaSecureProcessor_getConfig_HIDL(JNIEnv *env, jobject thiz,
                                                     jint jsessionId,
                                                     jobject jSessionConfig);

  static jint mediaSecureProcessor_setConfig_AIDL(JNIEnv *env, jint jsessionId,
                                                  jobject jSessionConfig);
  static jint mediaSecureProcessor_setConfig_HIDL(JNIEnv *env, jobject thiz,
                                                  jint jsessionId,
                                                  jobject jSessionConfig);

  static jint mediaSecureProcessor_startSession_AIDL(jint jsessionId);
  static jint mediaSecureProcessor_startSession_HIDL(JNIEnv *env, jobject thiz,
                                                     jint jsessionId);

  static jobject mediaSecureProcessor_processImage_AIDL(JNIEnv *env,
                                                        jint jsessionId,
                                                        jobject jhwBuffer,
                                                        jobject jImageConfig);
  static jobject mediaSecureProcessor_processImage_HIDL(JNIEnv *env,
                                                        jobject thiz,
                                                        jint jsessionId,
                                                        jobject jhwBuffer,
                                                        jobject jImageConfig);

  static jint mediaSecureProcessor_stopSession_AIDL(jint jsessionId);
  static jint mediaSecureProcessor_stopSession_HIDL(JNIEnv *env, jobject thiz,
                                                    jint jsessionId);

  static jint mediaSecureProcessor_deleteSession_AIDL(jint jsessionId);
  static jint mediaSecureProcessor_deleteSession_HIDL(JNIEnv *env, jobject thiz,
                                                      jint jsessionId);

protected:
  virtual ~JMediaSecureProcessor();

private:
  jclass mClass;
  jweak mObject;
  sp<ISecureProcessorHidl> secureProcessorHidl = nullptr;

  struct serviceDeathRecipient : public hidl_death_recipient {
    virtual void serviceDied(uint64_t cookie __unused,
                             const android::wp<IBase> &who __unused) {
      ALOGE("%s : secureprocessor service died", __func__);
      delete this;
    }
  };

  sp<serviceDeathRecipient> death_recepient = NULL;
  void MakeSecureProcessorAidl(const std::string instance);
  sp<ISecureProcessorHidl> MakeSecureProcessorHidl(const char *service);
  DISALLOW_EVIL_CONSTRUCTORS(JMediaSecureProcessor);
};

} // namespace android

#endif // _ANDROID_MEDIA_MEDIASECUREPROCESSOR_H_
