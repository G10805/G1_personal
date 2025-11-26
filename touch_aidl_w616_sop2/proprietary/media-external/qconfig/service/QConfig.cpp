/*
 **************************************************************************************************
 * Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

// #undef LOG_NDEBUG
// #define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "QConfig"

#include "QConfig.h"
#include <android-base/file.h>
#include <inttypes.h>
#include <cutils/properties.h>
#include <json/reader.h>
#include <json/json.h>
#include <utils/Log.h>
#include <fstream>
#include <iostream>

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace qconfig {

#define QCONFIG_ENABLE_LOG_PROP "vendor.qconfig.log"
static int32_t gLogLevel;

#define QLOG_CHECK() {                                          \
    gLogLevel = property_get_int32(QCONFIG_ENABLE_LOG_PROP, 0); \
}

/*
 * adb shell setprop vendor.qconfig.log 1 -> enable QLOGI
 * adb shell setprop vendor.qconfig.log 2 -> enable QLOGI/QLOGD/QLOGV
 * QLOGE is always enabled
*/

#define QLOGV(format, args...) ALOGD_IF((gLogLevel > 1), format, ##args)
#define QLOGD(format, args...) ALOGD_IF((gLogLevel > 1), format, ##args)
#define QLOGI(format, args...) ALOGI_IF((gLogLevel > 0), format, ##args)
#define QLOGE(format, args...) ALOGE(format, ##args)

#define TAG_MOD_DESC "descriptor"
#define TAG_MOD_PRESETS "presets"
#define TAG_PRESET_ID "id"
#define TAG_PRESET_CONFIGS "configs"
#define INTERFACE_EQUAL(l, r) ((static_cast<AIBinder*>((l)->asBinder().get()) == static_cast<AIBinder*>((r)->asBinder().get())) ? true : false) \


QConfig::QConfig(const std::string &presetPath) {
    QLOG_CHECK();
    QLOGI("%s", __func__);
    loadPresetMap(presetPath);
}

ScopedAStatus QConfig::setUserConfigs(const std::string& in_moduleName,
        const std::vector<QConfigUserParam>& in_configs, Result* _aidl_return) {
    QLOG_CHECK();
    if (!_aidl_return) {
        QLOGE("%s: invalid input", __func__);
        return ScopedAStatus::ok();
    }
    std::lock_guard<std::mutex> lock(mQConfigMutex);
    QLOGV("%s: module: %s", __func__, in_moduleName.c_str());

    auto moduleIterator = mModuleMap.find(in_moduleName);
    if (moduleIterator == mModuleMap.end()) {
        struct QConfig::Module module;
        QLOGV("create new module: %s", in_moduleName.c_str());
        for (auto const &item : in_configs) {
            QLOGV("add new key: %s, value: %s",
                    item.key.c_str(), item.value.c_str());
            module.userConfigMap.insert({item.key, item.value});
        }
        mModuleMap.insert({in_moduleName, module});
        *_aidl_return = Result::SUCCESS;
        return ScopedAStatus::ok();
    }
    std::vector<QConfigUserParam> addedConfigs;
    std::vector<QConfigUserParam> changedConfigs;

    for (auto const &item : in_configs) {
        auto configIterator = moduleIterator->second.userConfigMap.find(item.key);
        if (configIterator == moduleIterator->second.userConfigMap.end()) {
            QLOGV("add new key: %s, value: %s",
                    item.key.c_str(), item.value.c_str());
            moduleIterator->second.userConfigMap.insert({item.key, item.value});
            addedConfigs.push_back(item);
        } else {
            if (configIterator->second != item.value) {
                QLOGV("update key: %s, value: %s -> %s",
                        item.key.c_str(),
                        configIterator->second.c_str(),
                        item.value.c_str());
                configIterator->second = item.value;
                changedConfigs.push_back(item);
            }
        }
    }
    if (changedConfigs.size() > 0 || addedConfigs.size() > 0) {
        for (auto const& cb : moduleIterator->second.callbackVec) {
            if (changedConfigs.size() > 0) {
                QLOGV("notify client for %s(modify configs)",
                        in_moduleName.c_str());
                cb->onConfigChanged(in_moduleName, ChangedType::MODIFIED, changedConfigs);
            }
            if (addedConfigs.size() > 0) {
                QLOGV("notify client for %s(add configs)",
                        in_moduleName.c_str());
                cb->onConfigChanged(in_moduleName, ChangedType::ADDED, addedConfigs);
            }
        }
    }
    *_aidl_return = Result::SUCCESS;

    return ScopedAStatus::ok();
}

ScopedAStatus QConfig::clearUserConfig(const std::string& in_moduleName, const std::string& in_key,
        Result* _aidl_return) {
    QLOG_CHECK();
    if (!_aidl_return) {
        QLOGE("%s: invalid input", __func__);
        return ScopedAStatus::ok();
    }
    std::lock_guard<std::mutex> lock(mQConfigMutex);
    QLOGV("%s: module: %s, key: %s", __func__, in_moduleName.c_str(), in_key.c_str());
    auto moduleIterator = mModuleMap.find(in_moduleName);
    if (moduleIterator == mModuleMap.end()) {
        QLOGE("failed to find module %s", in_moduleName.c_str());
        *_aidl_return = Result::INVALID_ARGUMENT;
        return ScopedAStatus::ok();
    }
    auto configIterator = moduleIterator->second.userConfigMap.find(in_key);
    if (configIterator == moduleIterator->second.userConfigMap.end()) {
        QLOGE("failed to find key %s", in_key.c_str());
        *_aidl_return = Result::INVALID_ARGUMENT;
        return ScopedAStatus::ok();
    }

    moduleIterator->second.userConfigMap.erase(in_key);

    for (auto const& cb : moduleIterator->second.callbackVec) {
        QLOGV("notify client for %s(remove configs)",
                in_moduleName.c_str());
        cb->onConfigChanged(in_moduleName, ChangedType::REMOVED, {{in_key, ""}});
    }
    *_aidl_return = Result::SUCCESS;

    return ScopedAStatus::ok();
}

ScopedAStatus QConfig::getUserConfigValue(const std::string& in_moduleName, const std::string& in_key,
        QConfigPresetIdResult* _aidl_return) {
    QLOG_CHECK();
    if (!_aidl_return) {
        QLOGE("%s: invalid input", __func__);
        return ScopedAStatus::ok();
    }
    std::lock_guard<std::mutex> lock(mQConfigMutex);
    QLOGV("%s: module: %s, key: %s", __func__, in_moduleName.c_str(), in_key.c_str());
    auto moduleIterator = mModuleMap.find(in_moduleName);
    if (moduleIterator == mModuleMap.end()) {
        QLOGI("failed to find module %s", in_moduleName.c_str());
        _aidl_return->result = Result::INVALID_ARGUMENT;
        _aidl_return->value = "";
        return ScopedAStatus::ok();
    }
    auto configIterator = moduleIterator->second.userConfigMap.find(in_key);
    if (configIterator == moduleIterator->second.userConfigMap.end()) {
        QLOGI("failed to find config %s", in_key.c_str());
        _aidl_return->result = Result::INVALID_ARGUMENT;
        _aidl_return->value = "";
        return ScopedAStatus::ok();
    }
    QLOGV("get config value : %s for key: %s",
            configIterator->second.c_str(), in_key.c_str());
    _aidl_return->result = Result::SUCCESS;
    _aidl_return->value = configIterator->second;
    return ScopedAStatus::ok();
}

ScopedAStatus QConfig::addUserConfigListener(const std::vector<std::string>& in_moduleNameList,
        const std::shared_ptr<IQConfigListener>& in_cb, Result* _aidl_return) {
    QLOG_CHECK();
    if (!_aidl_return) {
        QLOGE("%s: invalid input", __func__);
        return ScopedAStatus::ok();
    }
    std::lock_guard<std::mutex> lock(mQConfigMutex);
    QLOGV("%s", __func__);
    if (in_cb == nullptr) {
        QLOGE("config listener is null");
        *_aidl_return = Result::INVALID_ARGUMENT;
        return ScopedAStatus::ok();
    }

    bool isAdded = false;
    for (const auto& module : in_moduleNameList) {
        auto moduleIterator = mModuleMap.find(module);
        QLOGV("register listener for %s", module.c_str());
        if (moduleIterator == mModuleMap.end()) {
            QLOGE("no such module: %s", module.c_str());
            continue;
        } else {
            bool isDup = false;
            for (auto const& l : moduleIterator->second.callbackVec) {
                if (INTERFACE_EQUAL(l, in_cb)) {
                    isDup = true;
                    QLOGV("callback is duplicated");
                    break;
                }
            }
            if (!isDup) {
                QLOGD("Add callback for module: %s", module.c_str());
                moduleIterator->second.callbackVec.push_back(in_cb);
                isAdded = true;
            }
        }
    }
    *_aidl_return = isAdded ? Result::SUCCESS : Result::ERROR;
    return ScopedAStatus::ok();
}

ScopedAStatus QConfig::removeUserConfigListener(const std::shared_ptr<IQConfigListener>& in_cb,
        Result* _aidl_return) {
    QLOG_CHECK();
    if (!_aidl_return) {
        QLOGE("%s: invalid input", __func__);
        return ScopedAStatus::ok();
    }
    std::lock_guard<std::mutex> lock(mQConfigMutex);
    QLOGV("%s", __func__);
    bool ret = false;
    for (auto& it : mModuleMap) {
        auto cbIterator = it.second.callbackVec.begin();
        while (cbIterator != it.second.callbackVec.end()) {
            if ((in_cb != nullptr) && INTERFACE_EQUAL(in_cb, *cbIterator)) {
                cbIterator = it.second.callbackVec.erase(cbIterator);
                ret = true;
            } else {
                ++cbIterator;
            }
        }
    }
    if (ret) {
        QLOGV("removed the cb");
        *_aidl_return = Result::SUCCESS;
    } else {
        QLOGE("no listener was found");
        *_aidl_return = Result::INVALID_ARGUMENT;
    }
    return ScopedAStatus::ok();
}

ScopedAStatus QConfig::getPresets(const std::string& in_moduleName, const std::string& in_presetId,
        std::vector<QConfigPresetParam>* out_presets, Result* _aidl_return) {
    QLOG_CHECK();
    if (!out_presets || !_aidl_return) {
        QLOGE("%s: invalid input", __func__);
        return ScopedAStatus::ok();
    }
    std::lock_guard<std::mutex> lock(mQConfigMutex);
    QLOGV("%s: module: %s, preset id: %s",
            __func__, in_moduleName.c_str(), in_presetId.c_str());
    auto moduleIterator = mModuleMap.find(in_moduleName);
    if (moduleIterator == mModuleMap.end()) {
        QLOGE("failed to find module %s", in_moduleName.c_str());
        *_aidl_return = Result::INVALID_ARGUMENT;
        return ScopedAStatus::ok();
    }
    auto presetIterator = moduleIterator->second.presetMap.find(in_presetId);

    if (presetIterator == moduleIterator->second.presetMap.end()) {
        QLOGE("failed to find presetId %s", in_presetId.c_str());
        *_aidl_return = Result::INVALID_ARGUMENT;
        return ScopedAStatus::ok();
    }

    out_presets->resize(presetIterator->second.size());
    uint32_t index = 0;
    for (auto const &preset : presetIterator->second) {
        QLOGV("get preset name: %s, value:%s",
                preset.first.c_str(), preset.second.c_str());
        (*out_presets)[index].name = preset.first;
        (*out_presets)[index].value = preset.second;
        index++;
    }
    *_aidl_return = Result::SUCCESS;
    return ScopedAStatus::ok();
}

void QConfig::loadPresetMap(const std::string &presetPath) {
    std::lock_guard<std::mutex> lock(mQConfigMutex);
    QLOGV("%s from %s", __func__, presetPath.c_str());
    std::string jsonString;
    if (!android::base::ReadFileToString(presetPath, &jsonString)) {
        QLOGE("Error reading file: %s", presetPath.c_str());
        return;
    }

    Json::Value allPresets;
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errorMessage;

    if (!reader->parse(&*jsonString.begin(), &*jsonString.end(), &allPresets, &errorMessage)) {
        QLOGE("Error to parser preset file: %s: %s", presetPath.c_str(), errorMessage.c_str());
        return;
    }

    struct QConfig::Module module;
    std::unordered_map<std::string, std::string> paramConfigs;
    for (auto const &moduleName : allPresets.getMemberNames()) {
        module.desc = allPresets[moduleName][TAG_MOD_DESC].asString();
        QLOGV("create new module: %s, descriptor: %s",
                moduleName.c_str(), module.desc.c_str());
        module.presetMap.clear();
        for (auto const &preset : allPresets[moduleName][TAG_MOD_PRESETS]) {
            std::string presetId = preset[TAG_PRESET_ID].asString();
            QLOGV("-preset id: %s", presetId.c_str());
            paramConfigs.clear();
            for (auto const &name : preset[TAG_PRESET_CONFIGS].getMemberNames()) {
                std::string value = preset[TAG_PRESET_CONFIGS][name].asString();
                paramConfigs.insert({name, value});
                QLOGV("--parameter key: %s, value: %s",
                        name.c_str(), value.c_str());
            }
            module.presetMap.insert({presetId, paramConfigs});
        }
        mModuleMap.insert({moduleName, module});
    }
}

} // namespace qconfig
} // namespace hardware
} // namespace qti
} // namespace vendor
} // namespace aidl