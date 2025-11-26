/**
* Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#ifndef DDSSWITCHRECOMMENDINTERNALMESSAGE
#define DDSSWITCHRECOMMENDINTERNALMESSAGE
#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"

#include "MessageCommon.h"

namespace rildata {

enum class DdsSwitchType_t : int8_t {
  Permanent = 0,
  Temporary = 1,
};

enum class DdsSwitchLevel_t : int8_t {
  None   = 0,
  Revoke = 1,
  Low    = 2,
  High   = 3,
};

class DdsSwitchRecommendInternalMessage : public SolicitedSyncMessage<int>,
                             public add_message_id<DdsSwitchRecommendInternalMessage> {
private:
  SubscriptionId_t mSubId;
  DdsSwitchType_t mSwitchType;
  DdsSwitchLevel_t mLevel;

public:
  static constexpr const char *MESSAGE_NAME = "DdsSwitchRecommendInternalMessage";
  ~DdsSwitchRecommendInternalMessage() = default;

  inline DdsSwitchRecommendInternalMessage(GenericCallback<int> *callback)
        : SolicitedSyncMessage<int>(get_class_message_id())
  {
    mName = MESSAGE_NAME;
    std::ignore = callback;
  }

  inline  void setParams(SubscriptionId_t subId, DdsSwitchType_t switchType, DdsSwitchLevel_t level)
  {
    mSubId = subId;
    mSwitchType = switchType;
    mLevel = level;
  }

  inline SubscriptionId_t getSubId() {
    return mSubId;
  }

  inline DdsSwitchType_t getSwitchType() {
    return mSwitchType;
  }

  inline DdsSwitchLevel_t getLevel() {
    return mLevel;
  }

  inline string dump()
  {
    std::stringstream ss;
    ss << mName << " subId=" << (int)mSubId;
    ss << " switchtype=" << (int)mSwitchType;
    ss << " level=" << (int)mLevel;
    return ss.str();
  }
};

} //namespace

#endif
