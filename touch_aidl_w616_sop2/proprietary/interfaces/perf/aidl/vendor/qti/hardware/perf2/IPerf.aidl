/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

// FIXME: license file, or use the -l option to generate the files with the header.

package vendor.qti.hardware.perf2;

import vendor.qti.hardware.perf2.IPerfCallback;

// Interface inherits from vendor.qti.hardware.perf@2.2::IPerf but AIDL does not support interface inheritance.
@VintfStability
interface IPerf {
    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfAsyncRequest(in int cmd, in String userDataStr, in int[] params);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfCallbackDeregister(in IPerfCallback callback, in int clientId);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfCallbackRegister(in IPerfCallback callback, in int clientId);

    oneway void perfEvent(in int eventId, in String userDataStr, in int[] reserved);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfGetFeedback(in int featureId, in String userDataStr, in int[] reserved);

    // Adding return type to method instead of out param String ret since there is only one return value.
    String perfGetProp(in String propName, in String defaultVal);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfHint(in int hint, in String userDataStr, in int userData1, in int userData2,
        in int[] reserved);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfHintAcqRel(in int pl_handle, in int hint, in String pkg_name, in int duration,
        in int hint_type, in int[] reserved);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfHintRenew(in int pl_handle, in int hint, in String pkg_name, in int duration,
        in int hint_type, in int[] reserved);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfLockAcqAndRelease(in int pl_handle, in int duration, in int[] boostsList,
        in int[] reserved);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfLockAcquire(in int pl_handle, in int duration, in int[] boostsList,
        in int[] reserved);

    void perfLockCmd(in int cmd, in int reserved);

    void perfLockRelease(in int pl_handle, in int[] reserved);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfProfile(in int pl_handle, in int profile, in int reserved);

    // Adding return type to method instead of out param int ret since there is only one return value.
    int perfSetProp(in String propName, in String value);

    // Adding return type to method instead of out param String ret since there is only one return value.
    String perfSyncRequest(in int cmd, in String userDataStr, in int[] params);
}
