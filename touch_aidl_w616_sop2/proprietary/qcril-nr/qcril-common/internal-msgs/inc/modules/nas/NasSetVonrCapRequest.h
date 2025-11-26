/******************************************************************************
#  Copyright (c) 2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "network_access_service_v01.h"
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class NasSetVonrCapRequest : public SolicitedMessage<RIL_Errno>,
    public add_message_id<NasSetVonrCapRequest>
{
  private:
    std::optional<bool> mVonrEnabled;

  public:
    using cb_t = GenericCallback<RIL_Errno>::cb_t;
    static constexpr const char *MESSAGE_NAME = "NasSetVonrCapRequest";

    NasSetVonrCapRequest() = delete;

    ~NasSetVonrCapRequest() {};

    inline NasSetVonrCapRequest(std::optional<bool> enabled, cb_t callbackfn)
        : SolicitedMessage<RIL_Errno>(get_class_message_id()) {
      mName = MESSAGE_NAME;
      mVonrEnabled = enabled;
      GenericCallback<RIL_Errno> cb(callbackfn);
      setCallback(&cb);
    }

    bool getVonrEnabled() {
        return *mVonrEnabled;
    }

    string dump() {
      return mName + "{setVonrEnabled=" +
          (mVonrEnabled ? (*mVonrEnabled ? "true" : "false") : "<invalid>")+ "}";
    }
};
