/* auto-generated source file */

#pragma once

#include <aidl/vendor/qti/hardware/bluetooth/handsfree_audio/IBluetoothHandsfreeAudio.h>

using aidl::vendor::qti::hardware::bluetooth::handsfree_audio::IBluetoothHandsfreeAudio;

/**
 * Create BluetoothHandsfreeAudio AIDL instance for RPC
 *
 *   instance_name: BluetoothHandsfreeAudio HAL instance name (e.g. "default")
 *
 *   return: BluetoothHandsfreeAudio AIDL instance for RPC
 */
std::shared_ptr<IBluetoothHandsfreeAudio> create_bluetooth_handsfree_audio_rpc(const char* instance_name);
