/*
 **************************************************************************************************
 * Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include "QConfigClient.h"
#include <cutils/properties.h>
#include <aidl/vendor/qti/hardware/qconfig/BnQConfigListener.h>
#include <aidl/vendor/qti/hardware/qconfig/IQConfig.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <utils/Log.h>
#include <inttypes.h>
#include <thread>
#include <memory>

#undef LOG_TAG
#define LOG_TAG "QConfigClient"
// #undef LOG_NDEBUG
// #define LOG_NDEBUG 0

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace qconfig {

using ::ndk::ScopedAStatus;
using ::aidl::vendor::qti::hardware::qconfig::ChangedType;
using ::aidl::vendor::qti::hardware::qconfig::QConfigPresetIdResult;
using ::aidl::vendor::qti::hardware::qconfig::QConfigPresetParam;
using ::aidl::vendor::qti::hardware::qconfig::QConfigUserParam;
using ::aidl::vendor::qti::hardware::qconfig::Result;

//  adb shell setprop vendor.qconfigclient.log true -> enable client debug log
#define LOG_PROP "vendor.qconfigclient.log"
static bool gLogEnable = false;
#define QCLOG(format, args...) ALOGD_IF(gLogEnable, format, ##args)


static std::shared_ptr<IQConfig> sInstance = nullptr;
static std::mutex sMutex;

static void GetInterface(std::shared_ptr<IQConfig> *interface, bool isSingleton) {
    auto funcGetConfig = [](std::shared_ptr<IQConfig> *config) {
        uint32_t count = 0;
        while (*config == nullptr && count < 100) {
            const std::string instance = std::string() + IQConfig::descriptor + "/default";
            auto qconfigBinder = ndk::SpAIBinder(AServiceManager_checkService(instance.c_str()));
            if (qconfigBinder.get() == nullptr) {
                QCLOG("sleep to get interface:%s", instance.c_str());
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                count++;
            } else {
                QCLOG("successful to get interface:%s", instance.c_str());
                *config = IQConfig::fromBinder(qconfigBinder);
            }
        }
    };

    if (isSingleton) {
        std::lock_guard<std::mutex> lock(sMutex);
        QCLOG("Get Singleton interface");
        if (sInstance == nullptr) {
            funcGetConfig(&sInstance);
        }
        *interface = sInstance;
    } else {
        QCLOG("Get interface");
        funcGetConfig(interface);
    }
}

#define INST_CHECK()                        \
    std::shared_ptr<IQConfig> interface = nullptr;       \
    GetInterface(&interface, mIsSingleton); \
    if (interface == nullptr) {             \
        ALOGE("interface is invalid");      \
        return false;                       \
    }

class AidlQConfigCallback : public BnQConfigListener {
 public:
    explicit AidlQConfigCallback(
            const std::shared_ptr<QConfigClient::ConfigCallback>& cb)
        : mCallback(cb) {}
    virtual ~AidlQConfigCallback() {
        QCLOG("%s", __func__);
    }
    ScopedAStatus onConfigChanged(const std::string& in_moduleName,
            ChangedType in_type, const std::vector<QConfigUserParam>& in_configs) override {
        if (mCallback != nullptr) {
            std::unordered_map<std::string, std::string> updateConfigs;
            std::string updateName = in_moduleName;
            QCLOG("onConfigChanged module: %s type: %u",
                    in_moduleName.c_str(), in_type);
            for (auto const& config : in_configs) {
                QCLOG("onConfigChanged: key: %s, value: %s",
                        config.key.c_str(), config.value.c_str());
                updateConfigs.insert({config.key, config.value});
            }
            mCallback->onConfigChanged(updateName,
                    static_cast<QConfigClient::ConfigCallback::TYPE>(in_type),
                    updateConfigs);
        }
        return ScopedAStatus::ok();
    }

    std::shared_ptr<QConfigClient::ConfigCallback> callback() {
        return mCallback;
    }

 private:
    std::shared_ptr<QConfigClient::ConfigCallback> mCallback;

};

QConfigClient::QConfigClient(bool isSingleton)
    : mIsSingleton(isSingleton) {
    ALOGI("QConfigClient: interface is singleton:%u", isSingleton);
    gLogEnable = property_get_bool(LOG_PROP, false);
}

QConfigClient::~QConfigClient() {
    ALOGI("~QConfigClient");
    clearCallbackList();
}

bool QConfigClient::setUserConfigs(const std::string& moduleName,
        const std::unordered_map<std::string, std::string>& configs) {
    QCLOG("setUserConfigs: module name: %s", moduleName.c_str());
    INST_CHECK();
    std::vector<QConfigUserParam> aidl_configs(configs.size());
    uint32_t index = 0;
    for (auto const& item : configs) {
        QCLOG("setUserConfigs: key %s, value: %s",
                item.first.c_str(), item.second.c_str());
        aidl_configs[index].key = item.first;
        aidl_configs[index].value = item.second;
        index++;
    }
    Result result = Result::NOT_SUPPORTED;
    interface->setUserConfigs(moduleName, aidl_configs, &result);
    return result == Result::SUCCESS ? true : false;
}

bool QConfigClient::clearUserConfig(const std::string& moduleName,
        const std::string& key) {
    QCLOG("clearUserConfig: module name: %s, userid: %s",
            moduleName.c_str(), key.c_str());
    INST_CHECK();
    Result result = Result::NOT_SUPPORTED;
    interface->clearUserConfig(moduleName, key, &result);
    return result == Result::SUCCESS ? true : false;
}

bool QConfigClient::getUserConfigValue(const std::string& moduleName,
        const std::string& key,
        std::string* value) {
    QCLOG("getUserConfigValue: module name: %s, key: %s",
            moduleName.c_str(), key.c_str());
    if (value == nullptr) {
        return false;
    }
    INST_CHECK();
    QConfigPresetIdResult presetIdResult = {Result::NOT_SUPPORTED, ""};
    interface->getUserConfigValue(moduleName, key, &presetIdResult);
    if (presetIdResult.result == Result::SUCCESS) {
        *value = presetIdResult.value;
    } else {
        *value = "";
    }
    return presetIdResult.result == Result::SUCCESS ? true : false;
}

bool QConfigClient::addUserConfigListener(
        const std::vector<std::string>& moduleNameList,
        const std::shared_ptr<ConfigCallback>& configCb) {
    QCLOG("addUserConfigListener");
    if (moduleNameList.size() == 0 || configCb == nullptr) {
        ALOGE("invalid parameters for addUserConfigListener");
        return false;
    }
    INST_CHECK();
    std::shared_ptr<AidlQConfigCallback> cb = ndk::SharedRefBase::make<AidlQConfigCallback>(configCb);
    std::lock_guard<std::mutex> lock(mCbListMutex);
    mAidlCallbackList.push_back(cb);
    Result result = Result::NOT_SUPPORTED;
    interface->addUserConfigListener(moduleNameList, cb, &result);
    return result == Result::SUCCESS ? true : false;
}

bool QConfigClient::removeUserConfigListener(
        const std::shared_ptr<ConfigCallback>& configCb) {
    QCLOG("removeUserConfigListener");
    INST_CHECK();
    std::lock_guard<std::mutex> lock(mCbListMutex);
    auto cbIterator = mAidlCallbackList.begin();
    while (cbIterator != mAidlCallbackList.end()) {
        if ((*cbIterator)->callback().get() == configCb.get()) {
            Result result = Result::NOT_SUPPORTED;
            interface->removeUserConfigListener(*cbIterator, &result);
            bool ret = result == Result::SUCCESS ? true : false;
            QCLOG("remove config listener: %s", ret ? "successfully" : "failed");
            mAidlCallbackList.erase(cbIterator);
            return ret;
        } else {
            ++cbIterator;
        }
    }
    QCLOG("failed to find config listener");
    return false;
}

bool QConfigClient::getPresets(const std::string& moduleName,
        const std::string& presetId,
        std::unordered_map<std::string, std::string>* presets) {
    QCLOG("getPresets: module name: %s, presetId: %s",
            moduleName.c_str(), presetId.c_str());
    if (presets == nullptr) {
        return false;
    }
    INST_CHECK();
    bool ret = false;
    std::vector<QConfigPresetParam> outParams;
    Result result = Result::NOT_SUPPORTED;
    interface->getPresets(moduleName, presetId, &outParams, &result);
    ret = result == Result::SUCCESS;
    if (ret) {
        for (auto const& param : outParams) {
            QCLOG("getPresets: name: %s, value: %s",
                    param.name.c_str(), param.value.c_str());
            presets->insert({param.name, param.value});
        }
    }
    return ret;
}

bool QConfigClient::clearCallbackList() {
    INST_CHECK();
    Result result = Result::NOT_SUPPORTED;
    std::lock_guard<std::mutex> lock(mCbListMutex);
    for (auto const &cb : mAidlCallbackList) {
        interface->removeUserConfigListener(cb, &result);
    }
    mAidlCallbackList.clear();
    return true;
}

} // namespace qconfig
} // namespace hardware
} // namespace qti
} // namespace vendor
} // namespace aidl
