/******************************************************************************
#  Copyright (c) 2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------*/

#include "ril_utf_service_manager.h"

[[clang::no_destroy]] static android::sp<android::IServiceManager> srvMgr;
android::sp<android::IServiceManager> getServiceManager()
{
    if(srvMgr == nullptr) {
       srvMgr = new android::ServiceManager();
    }
    return srvMgr;
}
