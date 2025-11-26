/*
* Copyright (c) 2021 Qualcomm Technologies, Inc.
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

package vendor.qti.gnss;
@Backing(type="int") @VintfStability
enum LocAidlSubscriptionDataItemId {
  INVALID_DATA_ITEM_ID = -1,
  AIRPLANEMODE_DATA_ITEM_ID = 0,
  ENH_DATA_ITEM_ID = 1,
  GPSSTATE_DATA_ITEM_ID = 2,
  NLPSTATUS_DATA_ITEM_ID = 3,
  WIFIHARDWARESTATE_DATA_ITEM_ID = 4,
  NETWORKINFO_DATA_ITEM_ID = 5,
  RILVERSION_DATA_ITEM_ID = 6,
  RILSERVICEINFO_DATA_ITEM_ID = 7,
  RILCELLINFO_DATA_ITEM_ID = 8,
  SERVICESTATUS_DATA_ITEM_ID = 9,
  MODEL_DATA_ITEM_ID = 10,
  MANUFACTURER_DATA_ITEM_ID = 11,
  VOICECALL_DATA_ITEM = 12,
  ASSISTED_GPS_DATA_ITEM_ID = 13,
  SCREEN_STATE_DATA_ITEM_ID = 14,
  POWER_CONNECTED_STATE_DATA_ITEM_ID = 15,
  TIMEZONE_CHANGE_DATA_ITEM_ID = 16,
  TIME_CHANGE_DATA_ITEM_ID = 17,
  WIFI_SUPPLICANT_STATUS_DATA_ITEM_ID = 18,
  SHUTDOWN_STATE_DATA_ITEM_ID = 19,
  TAC_DATA_ITEM_ID = 20,
  MCCMNC_DATA_ITEM_ID = 21,
  BTLE_SCAN_DATA_ITEM_ID = 22,
  BT_SCAN_DATA_ITEM_ID = 23,
  BATTERY_LEVEL_DATA_ITEM_ID = 26,
  MAX_DATA_ITEM_ID = 27,
}
