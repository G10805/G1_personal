/*========================================================================
\file gpio_api.h
\brief
     This file contains gpio APIs
\copyright
 Copyright (c) 2022 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ====================================================================== */

#ifndef _GPIO_API_H_
#define _GPIO_API_H_

/*------------------------------------------------------------------------
 * Include files
 * -----------------------------------------------------------------------*/
#include "ar_defs.h"

/**
   Param ID for GPIO pins
   This ID should only be used under PRM module instance
*/
#define PARAM_ID_RSC_GPIO_PIN 0x080014F3

/** Defines the GPIO pull types for input GPIOs to specify the internal pull.*/
#define GPIO_DEVICE_LIST_TYPE_LPI 1
#define GPIO_DEVICE_LIST_TYPE_CHIP 2
#define GPIO_DEVICE_LIST_TYPE_MAX_LENGTH 3

/**< GPIO Direction Input. */
#define GPIO_DIR_IN 0x0

/**< GPIO Direction Output. */
#define GPIO_DIR_OUT 0x1

#include "spf_begin_pack.h"
typedef struct gpio_cfg_req_param_t
{
   uint32_t num_gpio;
   /**< Number of gpio pins being configured */
} gpio_cfg_req_param_t

#include "spf_end_pack.h"
   ;
/** Defines the GPIO configuration types used to configure a GPIO.
 */

#include "spf_begin_pack.h"

typedef struct gpio_pin_config_t
{
   uint32_t gpio_pin;
   /**< Pin number being configured.
   @values
   - valid non-zero unsigned integer */

   uint32_t gpio_type;
   /**< Used for selecting GPIO device type.
   @values
   - GPIO_DEVICE_LIST_TYPE_LPI = 1,
   - GPIO_DEVICE_LIST_TYPE_CHIP = 2, */

   uint32_t func_sel;
   /**< Used for selecting multiplexed functionality on
   the pin.
   @values
   - valid unsigned integer */

   uint32_t dir;
   /**< GPIO direction type for GPIO configuration.
   (#GPIODirType).
   @values
   - GPIO_DIR_IN = 0
   - GPIO_DIR_OUT = 1 */

   uint32_t pull;
   /**< GPIO pull type for an input GPIO
   (#GPIOPullType).
   @values
   - GPIO_PULL_STATE_NP = 0  No Pull.
   - GPIO_PULL_STATE_PD = 1 Pull Down.
   - GPIO_PULL_STATE_KP = 2 Keeper.
   - GPIO_PULL_STATE_PU = 3 Pull Up. */

   uint32_t drive;
   /**< Strength value of the GPIO drive.
   @values
   valid non-zero unsigned integer*/
} gpio_pin_config_t
#include "spf_end_pack.h"
   ;
#endif /* _GPIO_API_H_ */
