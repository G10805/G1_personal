
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

package android.hardware.automotive.vehicle;

@VintfStability
@Backing(type="int")
enum VMS_ALERT {
      BATTERRY_DRAIN = 14,
      WIRELESS_CHARGING = 13,
      CHECK_SUNROOF = 12,
      GPF_VMS_Trigger = 11,
      DPF2_VMS_Trigger = 10,
      DPF1_VMS_Trigger = 9,
      DOOR_OPEN = 8,
      LOW_FUEL = 7,
      TPMS_AIR_LEAKAGE = 6,
      TPMS_LOW_PRESSURE = 5,
      HAND_BREAK_ENGAGED = 4,
      LOW_BREAK_FLUID = 3,
      HIGH_ENG_TEMP = 2,
      ENG_OIL_PRESSURE_LOW = 1,
      NO_ALERT = 0
}	 
