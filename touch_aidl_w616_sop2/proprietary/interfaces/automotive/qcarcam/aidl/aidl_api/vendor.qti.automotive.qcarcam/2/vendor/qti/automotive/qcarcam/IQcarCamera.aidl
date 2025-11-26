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
interface IQcarCamera {
  vendor.qti.automotive.qcarcam.QcarcamError getInputStreamList_1_1(out vendor.qti.automotive.qcarcam.QcarcamInputInfov2[] inputs);
  vendor.qti.automotive.qcarcam.QcarcamError closeStream_1_1(in vendor.qti.automotive.qcarcam.IQcarCameraStream camStream);
  vendor.qti.automotive.qcarcam.IQcarCameraStream openStream_1_1(in vendor.qti.automotive.qcarcam.QcarcamInputDesc Desc);
  vendor.qti.automotive.qcarcam.QcarcamError getInputStreamList(out vendor.qti.automotive.qcarcam.QcarcamInputInfov2[] inputs);
  vendor.qti.automotive.qcarcam.QcarcamError closeStream(in vendor.qti.automotive.qcarcam.IQcarCameraStream camStream);
  vendor.qti.automotive.qcarcam.IQcarCameraStream openStream(in vendor.qti.automotive.qcarcam.QcarcamInputDesc Desc);
}
