/*
 * Copyright (C) 2021 The Android Open Source Project
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
#define LOG_TAG "GnssNavigationMessageAidl"
#include "GnssNavigationMessageInterface.h"
#include <aidl/android/hardware/gnss/BnGnss.h>
#include <log/log.h>
#include "GnssAidlImpl.h"
namespace aidl::android::hardware::gnss {


using GnssNavigationMessage = IGnssNavigationMessageCallback::GnssNavigationMessage;
using GnssNavigationMessageType = GnssNavigationMessage::GnssNavigationMessageType;

std::shared_ptr<IGnssNavigationMessageCallback> GnssNavigationMessageInterface::sCallback = nullptr;

GnssNavigationMessageInterface::GnssNavigationMessageInterface() : mMinIntervalMillis(1000) {}

GnssNavigationMessageInterface::~GnssNavigationMessageInterface() {

}

ndk::ScopedAStatus GnssNavigationMessageInterface::setCallback(
        const std::shared_ptr<IGnssNavigationMessageCallback>& callback) {
    ALOGD("setCallback navigation message");
    GnssAidlImpl::getInstance()->lock();
    sCallback = callback;
    GnssAidlImpl::getInstance()->unlock();
    start();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus GnssNavigationMessageInterface::close() {
    ALOGD("close navigation message");
    GnssAidlImpl::getInstance()->lock();
    sCallback = nullptr;
    GnssAidlImpl::getInstance()->unlock();
    return ndk::ScopedAStatus::ok();
}

void GnssNavigationMessageInterface::start() {
    ALOGD("start Navigation Message");
    GnssAidlImpl::getInstance()->sendNavigationMessageStart();
}
void GnssNavigationMessageInterface::stop() {
    ALOGD("stop Navigation Message");
    GnssAidlImpl::getInstance()->sendNavigationMessageStop();
}
void GnssNavigationMessageInterface::gnssNavigationMessageCb(
    const PreGnssNavigationMessage* message) {
    GnssAidlImpl::getInstance()->lock();
    auto navigationCb(sCallback);
    GnssAidlImpl::getInstance()->unlock();
    if (navigationCb == nullptr) {
        ALOGE(
            "%s: GnssNavigation Message Callback Interface configured "
            "incorrectly",
            __func__);
        return;
    }
    if (message == nullptr) {
        ALOGE("%s, received invalid GnssNavigationMessage from GNSS HAL",
              __func__);
        return;
    }
    IGnssNavigationMessageCallback::GnssNavigationMessage navigationMsg;
    navigationMsg.svid = message->svid;
    navigationMsg.type =
        static_cast<IGnssNavigationMessageCallback::GnssNavigationMessage::
                        GnssNavigationMessageType>(message->type);
    navigationMsg.status = message->status;
    navigationMsg.messageId = message->message_id;
    navigationMsg.submessageId = message->submessage_id;
    navigationMsg.data = std::vector<uint8_t>(
        message->data, message->data + message->data_length);
    auto ret = navigationCb->gnssNavigationMessageCb(navigationMsg);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
}
}  // namespace aidl::android::hardware::gnss
