/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <aidl/android/hardware/bluetooth/IBluetoothHci.h>

using aidl::android::hardware::bluetooth::IBluetoothHci;


/**
 * Create Bluetooth AIDL instance for QCSR BlueCore (e.g. CSR8311)
 *
 *   name: Bluetooth AIDL instance's name (e.g. "default")
 *
 *   return: Bluetooth AIDL instance for BlueCore
 */
std::shared_ptr<IBluetoothHci> create_bluetooth_hci_bluecore(const char* name);

/**
 * Create Bluetooth AIDL instance for RPC
 *
 *   name: Bluetooth AIDL instance's name (e.g. "default")
 *
 *   return: Bluetooth AIDL instance for RPC
 */
std::shared_ptr<IBluetoothHci> create_bluetooth_hci_rpc(const char* name);
