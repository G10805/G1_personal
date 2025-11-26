/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <someip_client.h>
#include <someip_common_def.h>

#include "routing_manager_def.h"

using qti::hal::rpc::Someip;
using qti::hal::rpc::SomeipCallback;
using qti::hal::rpc::SomeipClient;
using qti::hal::rpc::SomeipContext;

int main()
{
    SomeipCallback callback;
    Someip::setup(callback);

    SomeipContext context(ROUTING_MANAGER_SERVICE_ID, ROUTING_MANAGER_INSTANCE_ID);
    SomeipClient client(ROUTING_MANAGER_APP_NAME, context);

    client.start();

    Someip::destroy();

    return 0;
}
