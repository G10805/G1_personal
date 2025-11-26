/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t SomeipServiceId;

typedef uint16_t SomeipInstanceId;

#define SOMEIP_INSTANCE_ID_BASE             ((SomeipInstanceId) 0x06A3)

#define HAL_EVENTGROUP_ID_BASE              ((uint16_t) 0x6666)

/*******************************************************************************
 * Service ID base definitions
 *******************************************************************************/
#define SOMEIP_SERVICE_ID_BASE              ((SomeipServiceId) 0x7700)
#define ROUTING_MANAGER_SERVICE_ID_BASE     ((SomeipServiceId) (SOMEIP_SERVICE_ID_BASE + 0x0000))
#define BLUETOOTH_SERVICE_ID_BASE           ((SomeipServiceId) (SOMEIP_SERVICE_ID_BASE + 0x0100))
#define WIFI_SERVICE_ID_BASE                ((SomeipServiceId) (SOMEIP_SERVICE_ID_BASE + 0x0200))

/*******************************************************************************
 * Routing manager service definitions
 *******************************************************************************/
#define ROUTING_MANAGER_SERVICE_ID          ((SomeipServiceId) (ROUTING_MANAGER_SERVICE_ID_BASE + 0x0000))

/*******************************************************************************
 * Bluetooth service definitions
 *******************************************************************************/
#define BLUETOOTH_HCI_SERVICE_ID              ((SomeipServiceId) (BLUETOOTH_SERVICE_ID_BASE + 0x0000))
#define BLUETOOTH_HANDSFREE_AUDIO_SERVICE_ID  ((SomeipServiceId) (BLUETOOTH_SERVICE_ID_BASE + 0x0001))

/*******************************************************************************
 * Wifi service definitions
 *******************************************************************************/
#define WIFI_HAL_SERVICE_ID                 ((SomeipServiceId) (WIFI_SERVICE_ID_BASE + 0x0001))
#define WIFI_COND_SERVICE_ID                ((SomeipServiceId) (WIFI_SERVICE_ID_BASE + 0x0002))
#define WIFI_SUPPLICANT_SERVICE_ID          ((SomeipServiceId) (WIFI_SERVICE_ID_BASE + 0x0003))
#define WIFI_HOSTAPD_SERVICE_ID             ((SomeipServiceId) (WIFI_SERVICE_ID_BASE + 0x0004))
#define WIFI_QTIWIFI_SERVICE_ID             ((SomeipServiceId) (WIFI_SERVICE_ID_BASE + 0x0005))
