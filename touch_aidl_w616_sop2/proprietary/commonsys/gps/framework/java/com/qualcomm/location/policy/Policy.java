/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.policy;

import java.util.List;
import java.util.ArrayList;

import android.util.Log;

public abstract class Policy {

    private static final String TAG = "Policy";

    static List<Policy> sPolicyList;

    static {
        sPolicyList = new ArrayList<Policy>();
    }

    protected PolicyName mPolicyName;

    public Policy(PolicyName name) {

        mPolicyName = name;
        sPolicyList.add(this);
    }

    public static boolean validateAllPolicies(SessionRequest request) {

        for (Policy p : sPolicyList) {
            if (!validatePolicy(p, request)) {
                return false;
            }
        }

        return true;
    }
    public static boolean validatePolicy(Policy policy, SessionRequest request) {

        boolean result = true;

        // Validate background throttling policy even if marked not applicable by session owner,
        //  since background session should always be either throttled (change tbf), or stopped.
        if (request.mIdentity.owner.isPolicyApplicable(policy.mPolicyName, request) ||
                policy.mPolicyName == PolicyName.POLICY_NAME_BACKGROUND_THROTTLING) {
            result = policy.isSessionAllowed(request);
            Log.v(TAG, "Policy [" + policy.mPolicyName + "] check - " + result);
        } else {
            Log.v(TAG, "Policy [" + policy.mPolicyName + "] skipped");
        }

        return result;
    }

    public static enum PolicyName {

        POLICY_NAME_USER_PROFILE_CHANGE,
        POLICY_NAME_LOCATION_SETTING_CHANGE,
        POLICY_NAME_BACKGROUND_THROTTLING,
        POLICY_NAME_BACKGROUND_DENIED,
        POLICY_NAME_POWER_BLAME_REPORTING,
        POLICY_NAME_PERMISSION_CHANGE,
        POLICY_NAME_POWER_SAVE_MODE,
        POLICY_NAME_SCREEN_INACTIVE,
        POLICY_NAME_APPOPS_CHANGE
    }

    public abstract boolean isSessionAllowed(SessionRequest request);
}
