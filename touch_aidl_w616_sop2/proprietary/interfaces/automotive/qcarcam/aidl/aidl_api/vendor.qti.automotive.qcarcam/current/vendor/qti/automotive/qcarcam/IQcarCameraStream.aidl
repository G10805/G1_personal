/*
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
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

package vendor.qti.automotive.qcarcam;
@VintfStability
interface IQcarCameraStream {
  vendor.qti.automotive.qcarcam.QcarcamError configureStream_1_1(in vendor.qti.automotive.qcarcam.QcarcamStreamParam Param, in vendor.qti.automotive.qcarcam.QcarcamStreamConfigData data);
  vendor.qti.automotive.qcarcam.QcarcamError getFrame_1_1(in long timeout, in int flags, out vendor.qti.automotive.qcarcam.QcarcamFrameInfov2 Info);
  vendor.qti.automotive.qcarcam.QcarcamError getStreamConfig_1_1(in vendor.qti.automotive.qcarcam.QcarcamStreamParam Param, out vendor.qti.automotive.qcarcam.QcarcamStreamConfigData data);
  vendor.qti.automotive.qcarcam.QcarcamError pauseStream();
  vendor.qti.automotive.qcarcam.QcarcamError releaseFrame_1_1(in int id, in int idx);
  vendor.qti.automotive.qcarcam.QcarcamError resumeStream();
  vendor.qti.automotive.qcarcam.QcarcamError setStreamBuffers_1_1(in android.hardware.common.NativeHandle hndl, in vendor.qti.automotive.qcarcam.QcarcamBuffersInfoList info);
  vendor.qti.automotive.qcarcam.QcarcamError startStream_1_1(in vendor.qti.automotive.qcarcam.IQcarCameraStreamCB streamObj);
  vendor.qti.automotive.qcarcam.QcarcamError stopStream_1_1();
  vendor.qti.automotive.qcarcam.QcarcamError configureStream(in vendor.qti.automotive.qcarcam.QcarcamStreamParam Param, in vendor.qti.automotive.qcarcam.QcarcamStreamConfigData data);
  vendor.qti.automotive.qcarcam.QcarcamError getFrame(in long timeout, in int flags, out vendor.qti.automotive.qcarcam.QcarcamFrameInfov2 Info);
  vendor.qti.automotive.qcarcam.QcarcamError getStreamConfig(in vendor.qti.automotive.qcarcam.QcarcamStreamParam Param, in vendor.qti.automotive.qcarcam.QcarcamStreamConfigData in_data, out vendor.qti.automotive.qcarcam.QcarcamStreamConfigData data);
  vendor.qti.automotive.qcarcam.QcarcamError releaseFrame(in int id, in int idx);
  vendor.qti.automotive.qcarcam.QcarcamError setStreamBuffers(in android.hardware.common.NativeHandle hndl, in vendor.qti.automotive.qcarcam.QcarcamBuffersInfoList info);
  vendor.qti.automotive.qcarcam.QcarcamError startStream(in vendor.qti.automotive.qcarcam.IQcarCameraStreamCB streamObj);
  vendor.qti.automotive.qcarcam.QcarcamError stopStream();
}
