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
#include "anld_service_interface.h"
#include <stdarg.h>
#include <string>
#include "driver/gpio/GPIOControl.h"
std::string FactoryMessage::toString() {
    char format[] = "FactoryMessage{msgid:%d,status:%d}";
    char tmp[50] = { 0 };
    snprintf(tmp, 50, format, msgid, status);
    return std::string(tmp,50);
}
namespace portable {
PowerStatus getChipPowerStatus() {
    using Airoha::GPIO::DriverPowerStatus;
    Airoha::GPIO::DriverPowerStatus driverStatus =
        Airoha::GPIO::portGetPowerStatus();
    PowerStatus coreStatus = PowerStatus::POWER_STATUS_UNKNOWN;
    switch (driverStatus) {
        case DriverPowerStatus::POWER_STATUS_UNKNOWN: {
            coreStatus = PowerStatus::POWER_STATUS_UNKNOWN;
            break;
        }
        case DriverPowerStatus::POWER_STATUS_POWER_OFF: {
            coreStatus = PowerStatus::POWER_STATUS_POWER_OFF;
            break;
        }
        case DriverPowerStatus::POWER_STATUS_POWER_ON: {
            coreStatus = PowerStatus::POWER_STATUS_POWER_ON;
            break;
        }
    }
    return coreStatus;
}
void powerOnPower() { Airoha::GPIO::portPowerOnChip(); }
void powerOffPower() { Airoha::GPIO::portPowerOffChip(); }
}  // namespace portable
int Airoha::anldReportCriticalMessage(const char *message) {
    AnldReadableMessage msg;
    strncpy(msg.message, message, sizeof(msg.message));
    msg.message[sizeof(msg.message) - 1] = 0;
    anldSendMessage2Service(
        Airoha::AnldServiceMessage::ANLD_SERVICE_ASSERT_MESSAGE_REPORT, &msg,
        sizeof(msg));
    return 0;
}
int Airoha::anldReportFormatCriticalMessage(const char *format, ...) {
    AnldReadableMessage msg;
    va_list ap;
    va_start(ap, format);
    memset(msg.message, 0, sizeof(msg.message));
    vsnprintf(msg.message, sizeof(msg.message), format, ap);
    va_end(ap);
    anldSendMessage2Service(
        Airoha::AnldServiceMessage::ANLD_SERVICE_ASSERT_MESSAGE_REPORT, &msg,
        sizeof(msg));
    return 0;
}