#ifndef RILREQUESTEMBMSSENDINTTMGILISTMESSAGE
#define RILREQUESTEMBMSSENDINTTMGILISTMESSAGE

/*===========================================================================

  Copyright (c) 2017, 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include "framework/Message.h"
#include "framework/SolicitedMessage.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include "DataCommon.h"
#include "qmi_embms_v01.h"

namespace rildata {

class RilRequestEmbmsSendIntTmgiListMessage : public SolicitedMessage<embms_get_interested_tmgi_list_resp_ack_msg_v01>,
                                              public add_message_id<RilRequestEmbmsSendIntTmgiListMessage> {
  private:
    embms_get_interested_tmgi_list_resp_msg_v01 params;
  public:
    static constexpr const char* MESSAGE_NAME = "RilRequestEmbmsSendIntTmgiListMessage";
    RilRequestEmbmsSendIntTmgiListMessage() = delete;

    RilRequestEmbmsSendIntTmgiListMessage(const embms_get_interested_tmgi_list_resp_msg_v01 request)
    :SolicitedMessage<embms_get_interested_tmgi_list_resp_ack_msg_v01>(get_class_message_id()),params(request){
        mName = MESSAGE_NAME;
    }
    ~RilRequestEmbmsSendIntTmgiListMessage() {}

    embms_get_interested_tmgi_list_resp_msg_v01 get_params() {
        return params;
    }
    string dump() {
        return MESSAGE_NAME;
    }
};

}//namespace

#endif
