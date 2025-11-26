#ifndef RILREQUESTEMBMSDEACTIVATETMGIMESSAGE
#define RILREQUESTEMBMSDEACTIVATETMGIMESSAGE

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

class RilRequestEmbmsDeactivateTmgiMessage : public SolicitedMessage<embms_deactivate_tmgi_resp_msg_v01>,
                                             public add_message_id<RilRequestEmbmsDeactivateTmgiMessage>  {
  private:
    embms_deactivate_tmgi_req_msg_v01 params;
  public:
    static constexpr const char* MESSAGE_NAME = "RilRequestEmbmsDeactivateTmgiMessage";
    RilRequestEmbmsDeactivateTmgiMessage() = delete;
    ~RilRequestEmbmsDeactivateTmgiMessage() {}

    RilRequestEmbmsDeactivateTmgiMessage(const embms_deactivate_tmgi_req_msg_v01 request)
    :SolicitedMessage<embms_deactivate_tmgi_resp_msg_v01>(get_class_message_id()), params(request){
        mName = MESSAGE_NAME;
    }

    embms_deactivate_tmgi_req_msg_v01 get_params() {
        return params;
    }
    string dump() {
        return MESSAGE_NAME;
    }
};

}//namespace

#endif
