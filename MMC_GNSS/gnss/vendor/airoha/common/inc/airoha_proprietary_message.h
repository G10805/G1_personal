/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
#ifndef AIROHA_PROPRIETARY_MESSAGE_H
#define AIROHA_PROPRIETARY_MESSAGE_H
#include <stdint.h>
namespace airoha {
namespace proprietary {
// Reference to Airoha IoT SDK for GNSS Developers Guide
enum ProprietaryMessageType {
    PROP_MSG_TIME_PVT = 0x07D7,
    PROP_MSG_TIME_INFO = 0x07DE,
};
struct PropTimeInfo {
    uint32_t tow;
    uint16_t wn;
    uint16_t tdop;
    uint16_t leap_second;
    uint16_t reserved;
    int32_t clockBias;   // scale: 1e-8
    int32_t clockDrift;  // scale 1e-12
};
static_assert(sizeof(PropTimeInfo) == 20, "Prop Time Info Size not Match");
struct PropPvt {
    int32_t latitude;   // scale: 1e-7
    int32_t longitude;  // scale: 1e-7
    int32_t hMsl;       // scale: 1e-3
    int32_t mslCorr;    // scale: 1e-3
    uint32_t gSpeed;    // scale: 1e-3
    uint32_t course;    // scale: 1e-5
    uint16_t hdop;      // scale: 1e-2
    uint16_t vdop;      // scale: 1e-2
    uint16_t pdop;      // scale: 1e-2
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint16_t millSecond;
    uint8_t second;
    uint8_t usedSvNum;
    uint16_t dgpsStationId;
    uint16_t dgpsAge;  // scale: 1e-1  unit: sec
    uint8_t fixQuality;
    uint8_t fixMode;
    uint8_t reserved;
};
static_assert(sizeof(PropPvt) == 0x30, "Prop PVT size error");
}  // namespace proprietary
}  // namespace airoha
#endif