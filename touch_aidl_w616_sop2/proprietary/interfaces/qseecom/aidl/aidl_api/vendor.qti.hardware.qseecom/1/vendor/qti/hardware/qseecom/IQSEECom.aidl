/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
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

package vendor.qti.hardware.qseecom;
@VintfStability
interface IQSEECom {
  int appLoadQuery(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in String appName);
  void getAppInfo(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, out int[] status, out vendor.qti.hardware.qseecom.IQSEECom.AppInfo info);
  int receiveRequest(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in vendor.qti.hardware.qseecom.IQSEECom.Buffer buf);
  void registerListner(in int listnerId, in int sharedBufferSize, in int flags, in vendor.qti.hardware.qseecom.IQSEEComCallback cbToken, out int[] status, out vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, out android.hardware.common.NativeHandle sharedBuffer);
  int scaleBusBandwidth(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in int mode);
  int sendCommand(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in vendor.qti.hardware.qseecom.IQSEECom.Buffer send, in vendor.qti.hardware.qseecom.IQSEECom.Buffer rsp);
  int sendModifiedCommand(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in vendor.qti.hardware.qseecom.IQSEECom.Buffer send, in vendor.qti.hardware.qseecom.IQSEECom.Buffer rsp, in vendor.qti.hardware.qseecom.IQSEECom.ModifiedBufferInfo bufferInfo, in boolean is64);
  int sendModifiedResponse(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in vendor.qti.hardware.qseecom.IQSEECom.Buffer send, in vendor.qti.hardware.qseecom.IQSEECom.ModifiedBufferInfo bufferInfo, in boolean is64);
  int sendResponse(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in vendor.qti.hardware.qseecom.IQSEECom.Buffer send);
  int setBandwidth(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, in boolean high);
  int shutdownApp(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle);
  void startApp(in String path, in String name, in int sharedBufferSize, in vendor.qti.hardware.qseecom.IQSEEComCallback cbToken, out int[] status, out vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, out android.hardware.common.NativeHandle sharedBuffer);
  void startAppV2(in String name, in android.hardware.common.Ashmem trustlet, in int sharedBufferSize, in vendor.qti.hardware.qseecom.IQSEEComCallback cbToken, out int[] status, out vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle, out android.hardware.common.NativeHandle sharedBuffer);
  int unregisterListner(in vendor.qti.hardware.qseecom.IQSEECom.AppHandle appHandle);
  @VintfStability
  parcelable AppHandle {
    long vendorLibHandle;
  }
  @VintfStability
  parcelable AppInfo {
    boolean is64;
    int requiredSGBufferSize;
    byte[64] reserved;
  }
  @VintfStability
  parcelable Buffer {
    int offset;
    int length;
  }
  @VintfStability
  parcelable ModifiedBuffer {
    boolean validFd;
    int offset;
  }
  @VintfStability
  parcelable ModifiedBufferInfo {
    vendor.qti.hardware.qseecom.IQSEECom.ModifiedBuffer[4] data;
    android.hardware.common.NativeHandle ionFd;
  }
}
