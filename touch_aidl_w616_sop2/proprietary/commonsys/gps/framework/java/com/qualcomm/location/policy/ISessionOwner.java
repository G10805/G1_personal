/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.policy;

import com.qualcomm.location.policy.SessionRequest;
import com.qualcomm.location.policy.Policy;

public interface ISessionOwner {

    public void handle(SessionRequest request);
    public boolean isPolicyApplicable(Policy.PolicyName policyName, SessionRequest request);
}