/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once

#include "ams_core_ioctl.h"

/** Synchronization source is internal. */
#define AMS_TDM_SYNC_SRC_INTERNAL (1)

/** Enable clock. */
int32_t ams_endpoint_clk_enable(const ams_core_endpoint_t *endpoint);

/** Disable clock. */
int32_t ams_endpoint_clk_disable(const ams_core_endpoint_t *endpoint);

/** Configure and start AFE HW port. */
int32_t ams_endpoint_start_endpoint(const ams_core_endpoint_t *endpoint);

/** Stop AFE port. */
int32_t ams_endpoint_stop_endpoint(const ams_core_endpoint_t *endpoint);

int32_t ams_endpoint_clk_attr_set(int32_t port, uint16_t attr);

int32_t ams_endpoint_clk_attr_get(int32_t port, uint16_t *attr);

int32_t ams_endpoint_clk_attr_clear(int32_t port);

int32_t ams_endpoint_hw_intf_enable(uint32_t intf_id);

int32_t ams_endpoint_hw_intf_disable(uint32_t intf_id);