/*
 **************************************************************************************************
 * Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef QCONFIG_SERVICE_INC_QCONFIG_H_
#define QCONFIG_SERVICE_INC_QCONFIG_H_

#include <aidl/vendor/qti/hardware/qconfig/BnQConfig.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace qconfig {

using ::aidl::vendor::qti::hardware::qconfig::ChangedType;
using ::aidl::vendor::qti::hardware::qconfig::QConfigPresetIdResult;
using ::aidl::vendor::qti::hardware::qconfig::QConfigPresetParam;
using ::aidl::vendor::qti::hardware::qconfig::QConfigUserParam;
using ::aidl::vendor::qti::hardware::qconfig::Result;
using ::aidl::vendor::qti::hardware::qconfig::IQConfigListener;
using ::ndk::ScopedAStatus;


class QConfig : public BnQConfig {
 public:
    explicit QConfig(const std::string &presetPath);
    virtual ~QConfig() = default;
    ScopedAStatus addUserConfigListener(const std::vector<std::string>& in_moduleNameList,
            const std::shared_ptr<IQConfigListener>& in_cb, Result* _aidl_return) override;
    ScopedAStatus clearUserConfig(const std::string& in_moduleName, const std::string& in_key,
            Result* _aidl_return) override;
    ScopedAStatus getPresets(const std::string& in_moduleName, const std::string& in_presetId,
            std::vector<QConfigPresetParam>* out_presets, Result* _aidl_return) override;
    ScopedAStatus getUserConfigValue(const std::string& in_moduleName, const std::string& in_key,
            QConfigPresetIdResult* _aidl_return) override;
    ScopedAStatus removeUserConfigListener(const std::shared_ptr<IQConfigListener>& in_cb,
            Result* _aidl_return) override;
    ScopedAStatus setUserConfigs(const std::string& in_moduleName,
            const std::vector<QConfigUserParam>& in_configs, Result* _aidl_return) override;

 private:
    std::mutex mQConfigMutex;
    struct Module {
        std::string desc;
        /**
         * The map for user configs
         * key: user config key
         * value: user config value
         */
        std::unordered_map<std::string, std::string> userConfigMap;
        /**
         * The map for presets
         * key: preset id
         * value: parameter map: { parameter name, parameter value}
         */
        std::unordered_map<std::string,
                std::unordered_map<std::string, std::string>> presetMap;
        /**
         * The vector for the callback objects
         */
        std::vector<std::shared_ptr<IQConfigListener>> callbackVec;
    };

    /**
     * key: Module name
     * value: Module object
     */
    std::unordered_map<std::string, Module> mModuleMap;

    void loadPresetMap(const std::string &presetPath);
};

} // namespace qconfig
} // namespace hardware
} // namespace qti
} // namespace vendor
} // namespace aidl
#endif  // QCONFIG_SERVICE_INC_QCONFIG_H_
