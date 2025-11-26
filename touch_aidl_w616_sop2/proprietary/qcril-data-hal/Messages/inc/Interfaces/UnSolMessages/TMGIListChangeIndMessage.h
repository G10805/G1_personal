/**
* Copyright (c) 2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#ifndef TMGILISTCHANGEINDMESSAGE
#define TMGILISTCHANGEINDMESSAGE
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "qmi_embms_v01.h"

namespace rildata {

/********************** Class Definitions *************************/
class TMGIListChangeIndMessage: public UnSolicitedMessage,
                           public add_message_id<TMGIListChangeIndMessage> {

private:
    embms_unsol_active_tmgi_ind_msg_v01 mList;
public:
    static constexpr const char *MESSAGE_NAME = "TMGIListChangeIndMessage";

   ~TMGIListChangeIndMessage() = default;
    TMGIListChangeIndMessage(embms_unsol_active_tmgi_ind_msg_v01 list):
    UnSolicitedMessage(get_class_message_id()), mList(list)
    {
      mName = MESSAGE_NAME;
    }
    std::shared_ptr<UnSolicitedMessage> clone()
    {
      return std::make_shared<TMGIListChangeIndMessage>(mList);
    }
    embms_unsol_active_tmgi_ind_msg_v01 getParam() {
        return mList;
    }
    string dump()
    {
      return MESSAGE_NAME;
    }

};

} //namespace

#endif