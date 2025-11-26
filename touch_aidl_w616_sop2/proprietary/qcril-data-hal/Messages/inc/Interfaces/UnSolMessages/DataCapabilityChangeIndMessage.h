/**
* Copyright (c) 2021 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#ifndef DATACAPABILITYCHANGEINDMESSAGE
#define DATACAPABILITYCHANGEINDMESSAGE
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

struct DataCapability_t {
  bool CIWlanSupported;
  // add new flag below
};

/********************** Class Definitions *************************/
class DataCapabilityChangeIndMessage : public UnSolicitedMessage,
                           public add_message_id<DataCapabilityChangeIndMessage> {

private:
  DataCapability_t mDataCapability;

public:
  static constexpr const char *MESSAGE_NAME = "DataCapabilityChangeIndMessage";

  ~DataCapabilityChangeIndMessage() = default;
  DataCapabilityChangeIndMessage():
  UnSolicitedMessage(get_class_message_id())
  {
    mName = MESSAGE_NAME;
    mDataCapability = {false,};
  }
  DataCapabilityChangeIndMessage(DataCapability_t capability):
  UnSolicitedMessage(get_class_message_id()), mDataCapability(capability)
  {
    mName = MESSAGE_NAME;
  }

  void setCIWlanSupported(bool cap) {mDataCapability.CIWlanSupported=cap;}
  bool getCIWlanSupported() {return mDataCapability.CIWlanSupported;}

  std::shared_ptr<UnSolicitedMessage> clone()
  {
    return std::make_shared<DataCapabilityChangeIndMessage>(mDataCapability);
  }

  string dump()
  {
    std::stringstream ss;
    ss << MESSAGE_NAME << " CIWlanSupported=" << (int)getCIWlanSupported();
    return ss.str();
  }
};

} //namespace

#endif