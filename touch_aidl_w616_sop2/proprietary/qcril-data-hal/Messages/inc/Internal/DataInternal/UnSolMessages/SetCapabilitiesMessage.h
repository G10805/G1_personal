/******************************************************************************
#  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef SETCAPABILITIESMESSAGE
#define SETCAPABILITIESMESSAGE

#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

using namespace std;

namespace rildata {

class SetCapabilitiesMessage : public UnSolicitedMessage,
                      public add_message_id<SetCapabilitiesMessage>
{
 private:
  bool dsdSystemStatusV2;
  bool dsdUiInfoV2;
  bool wdsPdnThrottleV2;
  bool wdsNswo;

 public:
  static constexpr const char *MESSAGE_NAME = "SetCapabilities";

  inline SetCapabilitiesMessage() : UnSolicitedMessage(get_class_message_id())
  {
    mName = MESSAGE_NAME;
  }
  ~SetCapabilitiesMessage() {}

  void setDsdSystemStatusV2(bool enabled) { dsdSystemStatusV2 = enabled; }
  void setDsdUiInfoV2(bool enabled) { dsdUiInfoV2 = enabled; }
  void setWdsPdnThrottleV2(bool enabled) { wdsPdnThrottleV2 = enabled; }
  void setWdsNswo(bool enabled) { wdsNswo = enabled; }

  bool getDsdSystemStatusV2() { return dsdSystemStatusV2; }
  bool getDsdUiInfoV2() { return dsdUiInfoV2; }
  bool getWdsPdnThrottleV2() { return wdsPdnThrottleV2; }
  bool getWdsNswo() { return wdsNswo; }

  std::shared_ptr<UnSolicitedMessage> clone() {
    auto msg = std::make_shared<SetCapabilitiesMessage>();
    if (msg != nullptr) {
      msg->setDsdSystemStatusV2(getDsdSystemStatusV2());
      msg->setDsdUiInfoV2(getDsdUiInfoV2());
      msg->setWdsPdnThrottleV2(getWdsPdnThrottleV2());
      msg->setWdsNswo(getWdsNswo());
    }
    return msg;
  }

  string dump() {
    std::stringstream ss;
    ss << mName << boolalpha << "{ dsdSystemStatusV2=" << dsdSystemStatusV2
      << ", dsdUiInfoV2=" << dsdUiInfoV2
      << ", wdsPdnThrottleV2=" << wdsPdnThrottleV2
      << ", wdsNswo=" << wdsNswo << " }";
    return ss.str();
  }
};

}

#endif
