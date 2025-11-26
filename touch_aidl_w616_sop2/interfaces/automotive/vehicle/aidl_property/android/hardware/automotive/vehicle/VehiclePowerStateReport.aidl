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
enum VehiclePowerStateReport {
      RESERVED=0x0,
      BOOT_COMPLETE=0x1,
      DEEP_SLEEP_ENTRY=0x2,
      DEEP_SLEEP_EXIT=0x3,
      SHUTDOWN_POSTPONE=0x4,
      SHUTDOWN_START=0x5,
      HOST_POWER_LEVEL_SYSTEM_IDEL=0x6,
      FULL_RUN=0x7,
      HOST_POWER_LEVEL_INIT=0x8,
      HOST_POWER_LEVEL_CORE=0x9,
      HOST_POWER_LEVEL_ZYGOTE=0xA,
      USER_OFF=0xB,
      HOST_POWER_LEVEL_SHUT_DOWN_COMPLETE=0xC,
      HOST_DISPLAY_OFF_AUDIO_OFF=0xD,
      SHUTDOWN_PREPARE=0xE,
      USER_OFF_TEMP_ON = 0X0F

}
