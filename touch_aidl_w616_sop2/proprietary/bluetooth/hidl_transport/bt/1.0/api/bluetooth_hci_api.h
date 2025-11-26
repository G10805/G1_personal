/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <android/hardware/bluetooth/1.0/IBluetoothHci.h>

using android::hardware::bluetooth::V1_0::IBluetoothHci;

/**
 * Create Bluetooth HAL instance
 *
 *   name: Bluetooth HAL instance's name (e.g. "default")
 *
 *   return: Bluetooth HAL instance
 */
IBluetoothHci* CreateBluetoothHci(const char* name);
