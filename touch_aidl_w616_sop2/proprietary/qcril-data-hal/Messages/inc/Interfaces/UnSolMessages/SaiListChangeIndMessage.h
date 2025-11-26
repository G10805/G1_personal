/**
* Copyright (c) 2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#ifndef SAILISTCHANGEINDMESSAGE
#define SAILISTCHANGEINDMESSAGE
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qmi_embms_v01.h"

namespace rildata {

/********************** Class Definitions *************************/
class SaiListChangeIndMessage: public UnSolicitedMessage,
                           public add_message_id<SaiListChangeIndMessage> {

private:
    embms_unsol_sai_ind_msg_v01 mList;
public:
    static constexpr const char *MESSAGE_NAME = "SaiListChangeIndMessage";

   ~SaiListChangeIndMessage() = default;
    SaiListChangeIndMessage(embms_unsol_sai_ind_msg_v01 list):
    UnSolicitedMessage(get_class_message_id()), mList(list)
    {
      mName = MESSAGE_NAME;
    }
    std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::make_shared<SaiListChangeIndMessage>(mList);
    }
    embms_unsol_sai_ind_msg_v01 getParam() {
        return mList;
    }
    string dump()
    {
      return MESSAGE_NAME;
    }

};

} //namespace

#endif