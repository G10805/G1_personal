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

package vendor.qti.hardware.vpp;
@VintfStability
interface IVpp {
  vendor.qti.hardware.vpp.VppError closeSession();
  vendor.qti.hardware.vpp.VppError drain();
  vendor.qti.hardware.vpp.VppError flush(in vendor.qti.hardware.vpp.VppPort ePort);
  vendor.qti.hardware.vpp.VppRequirements getRequirements(in vendor.qti.hardware.vpp.VppPortParam stInputParam, in vendor.qti.hardware.vpp.VppPortParam stOutputParam);
  vendor.qti.hardware.vpp.VppError initSession(in int u32Flags, in vendor.qti.hardware.vpp.IVppCallbacks cb);
  vendor.qti.hardware.vpp.VppError openSession();
  vendor.qti.hardware.vpp.VppError queueBuf(in vendor.qti.hardware.vpp.VppPort ePort, in vendor.qti.hardware.vpp.VppBuffer stBuf);
  vendor.qti.hardware.vpp.VppError reconfigure(in vendor.qti.hardware.vpp.VppPortParam stInputParam, in vendor.qti.hardware.vpp.VppPortParam stOutputParam);
  vendor.qti.hardware.vpp.VppError setCtrlAie(in vendor.qti.hardware.vpp.VppCtrlAie stCtrlAie);
  vendor.qti.hardware.vpp.VppError setCtrlAis(in vendor.qti.hardware.vpp.VppCtrlAis stCtrlAis);
  vendor.qti.hardware.vpp.VppError setCtrlCade(in vendor.qti.hardware.vpp.VppCtrlCade stCtrlCade);
  vendor.qti.hardware.vpp.VppError setCtrlCnr(in vendor.qti.hardware.vpp.VppCtrlCnr stCtrlCnr);
  vendor.qti.hardware.vpp.VppError setCtrlDi(in vendor.qti.hardware.vpp.VppCtrlDi stCtrlDi);
  vendor.qti.hardware.vpp.VppError setCtrlEar(in vendor.qti.hardware.vpp.VppCtrlEar stCtrlEar);
  vendor.qti.hardware.vpp.VppError setCtrlFrc(in vendor.qti.hardware.vpp.VppCtrlFrc stCtrlFrc);
  vendor.qti.hardware.vpp.VppError setCtrlMeas(in vendor.qti.hardware.vpp.VppCtrlMeas stCtrlMeas);
  vendor.qti.hardware.vpp.VppError setCtrlQbr(in vendor.qti.hardware.vpp.VppCtrlQbr stCtrlQbr);
  vendor.qti.hardware.vpp.VppError setCtrlSplitScreen(in vendor.qti.hardware.vpp.VppCtrlSplitScreen stCtrlSplitScreen);
  vendor.qti.hardware.vpp.VppError setCtrlTnr(in vendor.qti.hardware.vpp.VppCtrlTnr stCtrlTnr);
  vendor.qti.hardware.vpp.VppError setCtrlVppMode(in vendor.qti.hardware.vpp.VppMode eMode);
  vendor.qti.hardware.vpp.VppError setParameter(in vendor.qti.hardware.vpp.VppPort ePort, in vendor.qti.hardware.vpp.VppPortParam stParam);
  vendor.qti.hardware.vpp.VppError setVidPropCodec(in vendor.qti.hardware.vpp.VppCodecType eCodec);
  vendor.qti.hardware.vpp.VppError setVidPropNonRealTime(in int bNonRealtime);
  vendor.qti.hardware.vpp.VppError setVidPropOperatingRate(in int u32OperatingRate);
  vendor.qti.hardware.vpp.VppError setVidPropPriority(in vendor.qti.hardware.vpp.VppPriority ePriority);
  void termSession();
}
