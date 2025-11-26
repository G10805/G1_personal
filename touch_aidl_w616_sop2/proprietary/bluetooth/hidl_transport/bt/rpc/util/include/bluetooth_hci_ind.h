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

void *BtHciMsgCreateInitCompleteInd(uint32_t status);

void *BtHciMsgCreateAclDataInd(const vector<uint8_t>& data);

void *BtHciMsgCreateEventInd(const vector<uint8_t>& event);

void *BtHciMsgCreateIsoDataInd(const vector<uint8_t>& data);

void *BtHciMsgCreateScoDataInd(const vector<uint8_t>& data);

bool BtHciMsgParseInd(void **msg, uint16_t type, uint8_t *data, size_t length);

void BtHciMsgFreeInd(uint16_t type, void *msg);