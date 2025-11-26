/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <string>
#include <vector>

#include "hal_message_def.h"
#include "hal_status.h"
#include "someip_util.h"

using std::string;
using std::vector;
using ::qti::hal::rpc::SomeipMessage;

typedef uint16_t HalMsgT;

#define HAL_MSG_UNKNOWN     ((HalMsgT) 0x0)
#define HAL_MSG_REQ         ((HalMsgT) 0x1)
#define HAL_MSG_CFM         ((HalMsgT) 0x2)
#define HAL_MSG_EVT         ((HalMsgT) 0x3)

#define MAX_HAL_MSG         0xFFFF

#define HAL_DEFAULT_INSTANCE        "default"
#define HAL_DEFAULT_INSTANCE_ID     0
#define HAL_MAX_INSTANCE_ID         ((uint16_t) 0xFFFF)
#define INVALID_HAL_INSTANCE_ID     ((uint16_t) 0xFFFF)

void* CreateMessage(const std::shared_ptr<SomeipMessage>& msg);

void DeleteMessage(void* buf);

bool SerializePayload(uint16_t service_id, uint16_t instance_id,
    const vector<uint8_t>& payload, vector<uint8_t>& data);

bool ParsePayload(uint8_t* data, size_t length, uint16_t& service_id,
    uint16_t& instance_id, uint8_t* &payload, size_t& size);