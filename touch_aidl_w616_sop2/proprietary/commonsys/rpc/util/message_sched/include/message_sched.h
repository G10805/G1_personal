/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#pragma once

#include <stdio.h>
#include <unistd.h>

typedef void (* MessageCallback)(void *mv);

typedef struct
{
    MessageCallback messageHandler;
} MessageSchedRegisterInfo;

bool MessageSchedOpen(MessageSchedRegisterInfo *registerInfo);

bool MessageSchedSend(void *mv);

void MessageSchedClose();
