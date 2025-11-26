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
enum TGSLEVER {
      SignalInvalid = 15,
      MiddlePosition = 14,
      DefaultValue = 13,
      MminusForwardDrivePosition = 12,
      Tipminus = 11,
      Tipplus = 10,
      Park = 9,
      Reverse = 8,
      Neutral = 7,
      Drive = 6,
      Reserved_5 = 5,
      Reserved_4 = 4,
      Reserved_3 = 3,
      Reserved_2 = 2,
      Reserved_1 = 1,
      Reserved_0 = 0,
}
