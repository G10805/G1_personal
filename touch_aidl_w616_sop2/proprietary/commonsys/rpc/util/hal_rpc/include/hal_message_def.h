/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <cstdint>

typedef uint16_t    HalMsg;

#define HAL_REQ_BASE                ((HalMsg) (0x0000))

#define HAL_CFM_BASE                ((HalMsg) (0xA000))

#define HAL_IND_BASE                ((HalMsg) (0x8000))

#define HAL_UPSTREAM_REQ_BASE       ((HalMsg) (0x1000))

#define HAL_DOWNSTREAM_CFM_BASE     ((HalMsg) (0xB000))
