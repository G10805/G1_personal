/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <base/bind.h>

#include "message_loop_thread.h"
#include "message_sched.h"

static MessageLoopThread rpcThread("rpcThread");
static MessageCallback messageHandler;

bool MessageSchedOpen(MessageSchedRegisterInfo *registerInfo)
{
    LOG(INFO) << __func__;
    rpcThread.StartUp();
    if (!rpcThread.IsRunning()) {
        LOG(FATAL) << __func__ << ": unable to start btu message loop thread.";
    }
    if (!rpcThread.EnableRealTimeScheduling()) {
        LOG(FATAL) << __func__ << ": unable to enable real time scheduling";
    }
    messageHandler = registerInfo->messageHandler;
    return true;
}

bool MessageSchedSend(void *mv)
{
    if (!rpcThread.DoInThread(FROM_HERE, base::Bind(messageHandler, mv))) {
        LOG(ERROR) << __func__ << ": failed from " << FROM_HERE.ToString();
        return false;
    }
    return true;
}

void MessageSchedClose()
{
    LOG(INFO) << __func__;
    rpcThread.ShutDown();
}
