/*===========================================================================

  Copyright (c) 2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#ifndef QDPADAPTER
#define QDPADAPTER

#include "MessageCommon.h"
#include "ProfileParams.h"

qdp::ProfileParams convertDataProfileInfoToProfileParams(const rildata::DataProfileInfo_t& dataProfileInfo);
rildata::DataProfileInfo_t convertProfileParamsToDataProfileInfo(const qdp::ProfileParams& dataProfileInfo);
rildata::ApnTypes_t getApnTypesForName(std::string apn);
int getProfileCountForApnType(rildata::ApnTypes_t apnTypes);
int convertInstanceIdToSubId(qcril_instance_id_e_type instanceId);
bool isNullProfile(uint16_t profileId);

#endif
