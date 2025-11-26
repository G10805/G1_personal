#ifndef RILREQUESTEMBMSCONTENTDESCUPDATEMESSAGE
#define RILREQUESTEMBMSCONTENTDESCUPDATEMESSAGE

/*===========================================================================

  Copyright (c) 2017, 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include "framework/Message.h"
#include "DataCommon.h"
#include "framework/message_translator.h"
#include "framework/SolicitedMessage.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include "qmi_embms_v01.h"

namespace rildata {

class RilRequestEmbmsContentDescUpdateMessage : public SolicitedMessage<embms_update_content_desc_resp_msg_v01>,
                                                public add_message_id<RilRequestEmbmsContentDescUpdateMessage> {
  private:
    embms_update_content_desc_req_msg_v01 params;
  public:
    static constexpr const char* MESSAGE_NAME = "RilRequestEmbmsContentDescUpdateMessage";
    RilRequestEmbmsContentDescUpdateMessage() = delete;

    RilRequestEmbmsContentDescUpdateMessage(const embms_update_content_desc_req_msg_v01 request)
    :SolicitedMessage<embms_update_content_desc_resp_msg_v01>(get_class_message_id()),params(request){
        mName = MESSAGE_NAME;
    }
    ~RilRequestEmbmsContentDescUpdateMessage() {}

    embms_update_content_desc_req_msg_v01 get_params() {
        return params;
    }
    string dump() {
        return MESSAGE_NAME;
    }
};

}//namespace

#endif
