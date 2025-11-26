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

package vendor.qti.automotive.qcarcam2;
@VintfStability
interface IQcarCameraStream {
  vendor.qti.automotive.qcarcam2.QCarCamError configureStream(in vendor.qti.automotive.qcarcam2.QCarCamParamType Param, in vendor.qti.automotive.qcarcam2.QCarCamStreamConfigData data);
  vendor.qti.automotive.qcarcam2.QCarCamError getFrame(in long timeout, in int flags, out vendor.qti.automotive.qcarcam2.QCarCamFrameInfo Info);
  vendor.qti.automotive.qcarcam2.QCarCamError getStreamConfig(in vendor.qti.automotive.qcarcam2.QCarCamParamType Param, in vendor.qti.automotive.qcarcam2.QCarCamStreamConfigData data, out vendor.qti.automotive.qcarcam2.QCarCamStreamConfigData datatype);
  vendor.qti.automotive.qcarcam2.QCarCamError pauseStream();
  vendor.qti.automotive.qcarcam2.QCarCamError releaseFrame(in int id, in int bufferIdx);
  vendor.qti.automotive.qcarcam2.QCarCamError resumeStream();
  vendor.qti.automotive.qcarcam2.QCarCamError setStreamBuffers(in android.hardware.common.NativeHandle hndl, in vendor.qti.automotive.qcarcam2.QcarcamBuffersInfo info);
  vendor.qti.automotive.qcarcam2.QCarCamError startStream(in vendor.qti.automotive.qcarcam2.IQcarCameraStreamCB streamObj);
  vendor.qti.automotive.qcarcam2.QCarCamError stopStream();
  vendor.qti.automotive.qcarcam2.QCarCamError reserveStream();
  vendor.qti.automotive.qcarcam2.QCarCamError releaseStream();
  vendor.qti.automotive.qcarcam2.QCarCamError submitRequest(in vendor.qti.automotive.qcarcam2.QCarCamRequest Info);
}
