/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <aidl/vendor/qti/gnss/BnLocAidlIzatConfig.h>
#include <string>
#include "LocAidlUtils.h"

namespace aidl {
namespace vendor {
namespace qti {
namespace gnss {
namespace implementation {

using ::aidl::vendor::qti::gnss::ILocAidlIzatConfig;
using ::aidl::vendor::qti::gnss::ILocAidlIzatConfigCallback;

struct LocAidlIzatConfig : public BnLocAidlIzatConfig {

    LocAidlIzatConfig();
    ~LocAidlIzatConfig();

    // Methods from ILocAidlIzatConfig follow.
    ::ndk::ScopedAStatus init(const std::shared_ptr<ILocAidlIzatConfigCallback>& callback,
            bool* _aidl_return) override;
    ::ndk::ScopedAStatus readConfig(bool* _aidl_return) override;

    // Methods from ::android::hidl::base::IBase follow.

private:
    std::shared_ptr<ILocAidlIzatConfigCallback> mCallbackIface = nullptr;
    static std::shared_ptr<LocAidlDeathRecipient> mDeathRecipient;

    bool readIzatConf(std::string& izatConfContent);
};


}  // namespace implementation
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
}  // namespace aidl
