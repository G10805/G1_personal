/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

// FIXME: license file, or use the -l option to generate the files with the header.
///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.

package vendor.qti.hardware.perf2;
@VintfStability
interface IPerf {
  int perfAsyncRequest(in int cmd, in String userDataStr, in int[] params);
  int perfCallbackDeregister(in vendor.qti.hardware.perf2.IPerfCallback callback, in int clientId);
  int perfCallbackRegister(in vendor.qti.hardware.perf2.IPerfCallback callback, in int clientId);
  oneway void perfEvent(in int eventId, in String userDataStr, in int[] reserved);
  int perfGetFeedback(in int featureId, in String userDataStr, in int[] reserved);
  String perfGetProp(in String propName, in String defaultVal);
  int perfHint(in int hint, in String userDataStr, in int userData1, in int userData2, in int[] reserved);
  int perfHintAcqRel(in int pl_handle, in int hint, in String pkg_name, in int duration, in int hint_type, in int[] reserved);
  int perfHintRenew(in int pl_handle, in int hint, in String pkg_name, in int duration, in int hint_type, in int[] reserved);
  int perfLockAcqAndRelease(in int pl_handle, in int duration, in int[] boostsList, in int[] reserved);
  int perfLockAcquire(in int pl_handle, in int duration, in int[] boostsList, in int[] reserved);
  void perfLockCmd(in int cmd, in int reserved);
  void perfLockRelease(in int pl_handle, in int[] reserved);
  int perfProfile(in int pl_handle, in int profile, in int reserved);
  int perfSetProp(in String propName, in String value);
  String perfSyncRequest(in int cmd, in String userDataStr, in int[] params);
}
