/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <vector>

#include "bluetooth_hci_common.h"

using std::vector;

void *BtHciMsgCreateCloseReq();

void *BtHciMsgCreateInitializeReq();

void *BtHciMsgCreateAclDataReq(const vector<uint8_t>& data);

void *BtHciMsgCreateCommandReq(const vector<uint8_t>& packet);

void *BtHciMsgCreateIsoDataReq(const vector<uint8_t>& data);

void *BtHciMsgCreateScoDataReq(const vector<uint8_t>& data);

bool BtHciMsgParseReq(void **msg, uint16_t type, uint8_t *data, size_t length);

void BtHciMsgFreeReq(uint16_t type, void *msg);