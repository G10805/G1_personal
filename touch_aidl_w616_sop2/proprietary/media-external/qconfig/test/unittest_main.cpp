/*
  **************************************************************************************************
  * Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
  **************************************************************************************************
*/
#include <gtest/gtest.h>
#include "QConfig.h"
#include "QConfigClient.h"
#include <android-base/file.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <json/reader.h>
#include <json/json.h>
#include <utils/Log.h>
#include <fstream>
#include <iostream>
#include <thread>

using namespace ::aidl::vendor::qti::hardware::qconfig;
using ::aidl::vendor::qti::hardware::qconfig::ChangedType;
using ::aidl::vendor::qti::hardware::qconfig::QConfigPresetIdResult;
using ::aidl::vendor::qti::hardware::qconfig::QConfigPresetParam;
using ::aidl::vendor::qti::hardware::qconfig::QConfigUserParam;
using ::aidl::vendor::qti::hardware::qconfig::Result;

static const std::string TEST_PRESET_PATH = "/vendor/etc/qconfigpresets.json";

class QConfigTestEnvironment : public ::testing::Environment {
 public:
    virtual void SetUp() {
        std::cout << "QConfigTestEnvironment Setup" << std::endl;
        std::cout << "test json file: " << TEST_PRESET_PATH.c_str() << std::endl;

        const std::string instance = std::string() + IQConfig::descriptor + "/default";
        auto qconfigBinder = ndk::SpAIBinder(AServiceManager_checkService(instance.c_str()));
        if (qconfigBinder.get() == nullptr) {
            std::cout << "Unable to get interface" << std::endl;
                return;
        }
        std::shared_ptr<IQConfig> config = IQConfig::fromBinder(qconfigBinder);
        if (config != nullptr) {
            std::cout << "service existed" <<std::endl;
        } else {
            std::thread testService([] {
                ABinderProcess_setThreadPoolMaxThreadCount(10);
                std::shared_ptr<QConfig> qConfigSvc = ndk::SharedRefBase::make<QConfig>(TEST_PRESET_PATH);
                if (qConfigSvc == nullptr) {
                    std::cout << "qConfigSvc is null" << std::endl;
                    return;
                }
                const std::string instance = std::string() + QConfig::descriptor + "/default";
                std::cout << "Starting QConfig service..." << std::endl;
                int status = AServiceManager_addService(qConfigSvc->asBinder().get(), instance.c_str());
                if (status) {
                    std::cout << "failed to register qconfig service: 0x%x" << status << std::endl;
                }
                ABinderProcess_joinThreadPool();
            });
            testService.detach();
        }
    }
};

class QConfigClientTest : public ::testing::Test {};

TEST_F(QConfigClientTest, TestUserConfigs) {
    std::shared_ptr<QConfigClient> client =
            std::make_shared<QConfigClient>(false);
    ASSERT_TRUE(client != nullptr);

    std::vector<std::string> moduleList =
            {"QModName_0", "QModName_1", "QModName_2"};
    std::unordered_map<std::string, std::string> userConfigs =
            {{"10001", "1"},  {"10002", "3"}, {"10003", "5"}, {"10004", "7"}};
    for (const auto& mod : moduleList) {
        ASSERT_TRUE(client->setUserConfigs(mod, userConfigs));
    }
    for (const auto& mod : moduleList) {
        for (const auto& config : userConfigs) {
            std::string value;
            ASSERT_TRUE(client->getUserConfigValue(mod, config.first, &value));
            ASSERT_TRUE(config.second == value);
        }
    }

    std::string changedKey = "10003";
    std::string changedValue = "6";
    ASSERT_TRUE(client->setUserConfigs(moduleList[0],
            {{changedKey, changedValue}}));
    std::string queriedValue;
    ASSERT_TRUE(client->getUserConfigValue(moduleList[0],
            changedKey, &queriedValue));
    ASSERT_TRUE(changedValue == queriedValue);

    ASSERT_FALSE(client->getUserConfigValue("NoMod", changedKey, &queriedValue));
    ASSERT_FALSE(client->getUserConfigValue(moduleList[0], "NoKey", &queriedValue));

    for (const auto& mod : moduleList) {
        for (const auto& config : userConfigs) {
            ASSERT_TRUE(client->clearUserConfig(mod, config.first));
            ASSERT_FALSE(client->getUserConfigValue(
                    mod, config.first, &queriedValue));
        }
    }
    ASSERT_FALSE(client->clearUserConfig(moduleList[0], "10001"));
}

TEST_F(QConfigClientTest, TestPreset) {
    std::string jsonString;
    ASSERT_TRUE(android::base::ReadFileToString(TEST_PRESET_PATH, &jsonString));
    Json::Value allPresets;
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errorMessage;
    ASSERT_TRUE(reader->parse(&*jsonString.begin(), &*jsonString.end(), &allPresets, &errorMessage));
    std::shared_ptr<QConfigClient> client =
            std::make_shared<QConfigClient>(false);
    ASSERT_TRUE(client != nullptr);
    std::unordered_map<std::string, std::string> clientPresets;

    for (auto const &moduleName : allPresets.getMemberNames()) {
        for (auto const &preset : allPresets[moduleName]["presets"]) {
            clientPresets.clear();
            ASSERT_TRUE(client->getPresets(moduleName,
                    preset["id"].asString(), &clientPresets));
            for (const auto& outPreset : clientPresets) {
                ASSERT_EQ(preset["configs"][outPreset.first].asString(),
                        outPreset.second);
            }
        }
    }

    clientPresets.clear();
    ASSERT_FALSE(client->getPresets("NoMod", "1", &clientPresets));
    clientPresets.clear();
    ASSERT_FALSE(client->getPresets("QModKey_VppFilter",
            "No_Key", &clientPresets));
}

using TYPE = QConfigClient::ConfigCallback::TYPE;
class MockCallback : public QConfigClient::ConfigCallback {
 public:
    MockCallback() {
        std::cout<< "MockCallback" << std::endl;
    }
    virtual ~MockCallback() {
        std::cout<< "~MockCallback" << std::endl;
    }
    void onConfigChanged(const std::string& moduleName,
            TYPE type,
            const std::unordered_map<std::string, std::string>&
                    configs) override {
        mIsCalled = true;
        mIsVerified = false;
        bool isModuleFound = false;
        for (auto const& module : mModules) {
            if (module == moduleName) {
                isModuleFound = true;
                break;
            }
        }
        if (!isModuleFound) {
            mMsg = "Callback:Wrong module name";
            return;
        }

        auto it = mConfigs.find(type);
        if (it == mConfigs.end()) {
            mMsg = "Callback:Wrong changed type";
            return;
        }

        if (type != TYPE::REMOVED && configs.size() != it->second.size()) {
            mMsg = "Callback:Wrong config size";
            return;
        }

        for (auto const& config : configs) {
            if (it->second.find(config.first) == it->second.end()) {
                mMsg = "Callback:Wrong config key";
                return;
            }
            if (type != TYPE::REMOVED
                    && it->second[config.first] != config.second) {
                mMsg = "Callback:Wrong config value";
                return;
            }
        }
        mIsVerified = true;
        mMsg = "verified";
    }
    std::atomic<bool> mIsCalled = false;
    std::atomic<bool> mIsVerified = false;
    std::string mMsg;
    std::vector<std::string> mModules;
    std::unordered_map<TYPE,
            std::unordered_map<std::string, std::string>> mConfigs;
};

TEST_F(QConfigClientTest, TestListener) {
    std::shared_ptr<QConfigClient> client =
            std::make_shared<QConfigClient>(false);
    ASSERT_TRUE(client != nullptr);

    std::vector<std::shared_ptr<MockCallback>> listenerList =
            {nullptr, nullptr, nullptr};
    for (auto &listener : listenerList) {
        listener = std::make_shared<MockCallback>();
        ASSERT_TRUE(listener != nullptr);
    }

    std::vector<std::string> moduleList =
            {"QModName_0", "QModName_1", "QModName_2", "QModName_3"};
    std::unordered_map<std::string, std::string> userConfigs =
            {{"10001", "1"}, {"10002", "3"}, {"10003", "5"}};
    for (const auto& mod : moduleList) {
        ASSERT_TRUE(client->setUserConfigs(mod, userConfigs));
    }

    for (auto &listener : listenerList) {
        listener->mModules = {"QModName_1", "QModName_2", "QModName_3"};
        listener->mConfigs = {
                {TYPE::ADDED, {{"10004", "1"}, {"10005", "2"}}},
                {TYPE::MODIFIED, {{"10001", "7"}, {"10002", "5"},
                        {"10003", "3"}}},
                {TYPE::REMOVED, {{"10001", "1"},  {"10002", "1"},
                        {"10003", "1"}, {"10004", "1"}, {"10005", "1"}}}
        };
        ASSERT_TRUE(client->addUserConfigListener(
                listener->mModules, listener));
    }

#define CLEAR_LISENERS {                  \
    for (auto &listener : listenerList) { \
        listener->mIsCalled = false;      \
    }                                     \
}

#define VERIFY_LISENERS {                                    \
    for (auto &listener : listenerList) {                    \
        ASSERT_TRUE(listener->mIsCalled);                    \
        if (!listener->mIsVerified)                          \
            std::cout<< listener->mMsg.c_str() << std::endl; \
        ASSERT_TRUE(listener->mIsVerified);                  \
    }                                                        \
}

    CLEAR_LISENERS
    ASSERT_TRUE(client->setUserConfigs(moduleList[1],
            {{"10004", "1"}, {"10005", "2"}}));
    VERIFY_LISENERS

    CLEAR_LISENERS
    ASSERT_TRUE(client->clearUserConfig(moduleList[1], "10004"));
    VERIFY_LISENERS

    CLEAR_LISENERS
    ASSERT_TRUE(client->clearUserConfig(moduleList[1], "10005"));
    VERIFY_LISENERS

    CLEAR_LISENERS
    ASSERT_TRUE(client->setUserConfigs(moduleList[2],
            {{"10001", "7"}, {"10002", "5"}, {"10003", "3"}}));
    VERIFY_LISENERS

    for (const auto& mod : moduleList) {
        for (const auto& config : userConfigs) {
            CLEAR_LISENERS
            ASSERT_TRUE(client->clearUserConfig(mod, config.first));
            if (mod == "QModName_0") {
                for (auto &listener : listenerList)
                    ASSERT_FALSE(listener->mIsCalled);
            } else {
                VERIFY_LISENERS
            }
        }
    }
    for (const auto &cb : listenerList) {
        ASSERT_TRUE(client->removeUserConfigListener(cb));
    }
    CLEAR_LISENERS
    ASSERT_TRUE(client->setUserConfigs(moduleList[2], {{"10001", "7"}}));
    for (auto &listener : listenerList)
        ASSERT_FALSE(listener->mIsCalled);

    ASSERT_TRUE(client->clearUserConfig(moduleList[2], "10001"));
    for (auto &listener : listenerList)
        ASSERT_FALSE(listener->mIsCalled);

    listenerList.clear();
}

int main(int argc, char **argv) {
    ::testing::AddGlobalTestEnvironment(new QConfigTestEnvironment);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
