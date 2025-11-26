/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

package android.hardware.automotive.vehicle;
@Backing(type="int") @VintfStability
enum VehiclePowerStateReport {
  RESERVED = 0x0,
  BOOT_COMPLETE = 0x1,
  DEEP_SLEEP_ENTRY = 0x2,
  DEEP_SLEEP_EXIT = 0x3,
  SHUTDOWN_POSTPONE = 0x4,
  SHUTDOWN_START = 0x5,
  HOST_POWER_LEVEL_SYSTEM_IDEL = 0x6,
  FULL_RUN = 0x7,
  HOST_POWER_LEVEL_INIT = 0x8,
  HOST_POWER_LEVEL_CORE = 0x9,
  HOST_POWER_LEVEL_ZYGOTE = 0xA,
  USER_OFF = 0xB,
  HOST_POWER_LEVEL_SHUT_DOWN_COMPLETE = 0xC,
  HOST_DISPLAY_OFF_AUDIO_OFF = 0xD,
  SHUTDOWN_PREPARE = 0xE,
  USER_OFF_TEMP_ON = 0X0F,
}
