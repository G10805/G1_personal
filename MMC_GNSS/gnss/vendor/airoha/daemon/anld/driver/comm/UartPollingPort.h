/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

/*
 * @Author: your name
 * @Date: 2020-08-13 10:29:14
 * @LastEditTime: 2020-08-14 15:06:58
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \anld\driver\comm\UartPollingPort.h
 */
#ifndef _UART_POLLING_PORT
#define _UART_POLLING_PORT
#include "Communicator.h"
#include <mutex>
using Airoha::Communicator::PollingPort;
using Airoha::Communicator::PortNum;

namespace Airoha{
namespace Communicator{

//TODO: UartPolling Object
class UartPoll: public PollingPort{
public:

    UartPoll();
    ~UartPoll();

    PortStatus portOpen(PortNum number) override;
    PortStatus portClose() override;
    PortStatus portSendPolling(const uint8_t* buffer, size_t length) override;
    PortStatus portReceivePolling(uint8_t* buffer, size_t length) override;
    //the following function for 1/2/4 butes function
    PortStatus portPutUint8(uint8_t) override;
    PortStatus portPutUint16(uint16_t) override;
    PortStatus portPutUint32(uint32_t) override;
    PortStatus portGetUint8(uint8_t &) override;
    PortStatus portGetUint16(uint16_t &) override;
    PortStatus portGetUint32(uint32_t & ) override;
    /**
     * @brief Because the port is open in polling mode, so we need a function to interrupt it
     * 
     * @return true 
     * @return false 
     */
    bool portStop(uint8_t stop) override; 
private:
    //POLLING_PORT_XXX_ENDIAN
    uint8_t endian;
    /**
     * @brief stop flag, this will be used when calling SendPolling and ReceivePolling, 
     * because these two function will block forever until receive or send enough bytes.
     * 
     */
    uint8_t stopFlag;
    int uartFd;
    std::mutex uartMutex;
};

}; // namespace Communicator
}; // namespace Airoha
#endif