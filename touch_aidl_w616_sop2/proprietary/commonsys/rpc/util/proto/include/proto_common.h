/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#pragma once

#include <vector>

bool ProtoMessageSerialize(void* msg, std::vector<uint8_t>& payload);

bool ProtoMessageParse(const uint8_t* data, size_t length, void* msg);
