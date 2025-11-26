/******************************************************************************
#  Copyright (c) 2022 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include "framework/legacy.h"
#include "QtiMutex.h"
#include "binder/IBinder.h"
#include "BnQtiRadioConfigDefault.h"
#include "IQtiRadioConfigContext.h"
#include <memory>
#include <aidl/vendor/qti/hardware/radio/qtiradioconfig/MsimPreference.h>
#include "qti_radio_config_aidl_service_utils.h"

namespace aidlqtiradioconfigimpl {
using namespace aidl::vendor::qti::hardware::radio::qtiradioconfig::implementation;
}

namespace aidlqtiradioconfig {
using namespace aidl::vendor::qti::hardware::radio::qtiradioconfig;
}

class IQtiRadioConfigImpl : public aidlqtiradioconfigimpl::BnQtiRadioConfigDefault {
private:
  qcril_instance_id_e_type mInstanceId;
  qtimutex::QtiSharedMutex mCallbackLock;
  AIBinder_DeathRecipient* mDeathRecipient{nullptr};
  std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigIndication> mIndicationCb;
  std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse> mResponseCb;

protected:
  std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse> getResponseCallback();
  std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigIndication> getIndicationCallback();
  void clearCallbacks_nolock();
  void clearCallbacks();

  void sendResponseForGetSecureModeStatus(int32_t in_serial,
    RIL_Errno errorCode, bool status);
  void sendResponseForSetMsimPreference(int32_t in_serial,
    RIL_Errno errorCode);

public:
  IQtiRadioConfigImpl() = delete;
  IQtiRadioConfigImpl(qcril_instance_id_e_type instance);
  ~IQtiRadioConfigImpl();

  android::status_t registerService();

  void deathNotifier(void* cookie);

  std::shared_ptr<IQtiRadioConfigContext> getContext(uint32_t serial);

  qcril_instance_id_e_type getInstanceId();

  void setResponseFunctions_nolock(
      const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse>& in_radioConfigResponse,
      const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigIndication>& in_radioConfigIndication);

// Config Request APIs
  ::ndk::ScopedAStatus setCallbacks(
      const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigResponse>& in_radioConfigResponse,
      const std::shared_ptr<aidlqtiradioconfig::IQtiRadioConfigIndication>& in_radioConfigIndication) override;
  ::ndk::ScopedAStatus getSecureModeStatus(int32_t in_serial) override;
  ::ndk::ScopedAStatus setMsimPreference(int32_t in_serial,
      aidlqtiradioconfig::MsimPreference pref) override;

};

