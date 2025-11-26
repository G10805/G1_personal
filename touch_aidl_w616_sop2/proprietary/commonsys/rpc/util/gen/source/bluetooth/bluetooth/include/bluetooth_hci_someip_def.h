/* auto-generated source file */

#pragma once

#include "bluetooth_hci_message_def.h"

#include "someip_common_def.h"

#define BLUETOOTH_HCI_EVENTGROUP_ID_BASE    (HAL_EVENTGROUP_ID_BASE)

/*******************************************************************************
 * BluetoothHci common definition
 *******************************************************************************/

#define BLUETOOTH_HCI_EVENTGROUP_ID         ((uint16_t) (BLUETOOTH_HCI_EVENTGROUP_ID_BASE + 0x0000))

#define BLUETOOTH_HCI_EVENT_ID_NUMBER       BLUETOOTH_HCI_IND_COUNT

/*******************************************************************************
 * BluetoothHci client definition
 *******************************************************************************/

#define BLUETOOTH_HCI_CLIENT_APP_NAME       "bluetooth_hci_client"

/*******************************************************************************
 * BluetoothHci server definition
 *******************************************************************************/

#define BLUETOOTH_HCI_SERVER_APP_NAME       "bluetooth_hci_service"
