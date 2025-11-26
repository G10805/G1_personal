/*
 **************************************************************************************************
 * Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef QCONFIG_CLIENT_INC_QCONFIGCLIENT_H_
#define QCONFIG_CLIENT_INC_QCONFIGCLIENT_H_

#include <utils/RefBase.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace qconfig {

class AidlQConfigCallback;

class QConfigClient {
 public:
    class ConfigCallback {
     public:
        enum TYPE : int32_t {
            ADDED,
            MODIFIED,
            REMOVED
        };
        using config_map = std::unordered_map<std::string, std::string>;
        virtual ~ConfigCallback() = default;
        virtual void onConfigChanged(
                const std::string& moduleName,
                TYPE type,
                const config_map& configs) = 0;
    };
    /* If isSingleton is true, QConfigClient will create only one
       IQConfig instance. If it's false, each public function will
       create the IQConfig instance.
    */
    explicit QConfigClient(bool isSingleton);

    ~QConfigClient();

    bool setUserConfigs(const std::string& moduleName,
            const std::unordered_map<std::string, std::string>& configs);

    bool clearUserConfig(const std::string& moduleName,
            const std::string& key);

    bool getUserConfigValue(const std::string& moduleName,
            const std::string& key,
            std::string* value);

    bool getPresets(const std::string& moduleName,
            const std::string& presetId,
            std::unordered_map<std::string, std::string>* presets);

    bool addUserConfigListener(const std::vector<std::string>& moduleNameList,
            const std::shared_ptr<ConfigCallback>& configCb);

    bool removeUserConfigListener(const std::shared_ptr<ConfigCallback>& configCb);

 private:
    bool mIsSingleton;
    std::vector<std::shared_ptr<AidlQConfigCallback>> mAidlCallbackList;
    std::mutex mCbListMutex;
    bool clearCallbackList();

};

} // namespace qconfig
} // namespace hardware
} // namespace qti
} // namespace vendor
} // namespace aidl
#endif  // QCONFIG_CLIENT_INC_QCONFIGCLIENT_H_
