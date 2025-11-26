/* auto-generated source file */

#pragma once

#include <aidl/android/hardware/bluetooth/IBluetoothHci.h>

using aidl::android::hardware::bluetooth::IBluetoothHci;

/**
 * Create BluetoothHci AIDL instance for RPC
 *
 *   instance_name: BluetoothHci HAL instance name (e.g. "default")
 *
 *   return: BluetoothHci AIDL instance for RPC
 */
std::shared_ptr<IBluetoothHci> create_bluetooth_hci_rpc(const char* instance_name);
