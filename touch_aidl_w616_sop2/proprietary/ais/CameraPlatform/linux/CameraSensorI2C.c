/**
 * @file CameraSensorI2C.c
 *
 * @brief Implementation of the camera sensor I2C abstraction API.
 *
 * Copyright (c) 2011-2012, 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/*============================================================================
                        INCLUDE FILES
============================================================================*/
#include "AEEstd.h"
#include "CameraPlatform.h"
#include "CameraOSServices.h"

/* ===========================================================================
                DEFINITIONS AND DECLARATIONS FOR MODULE
=========================================================================== */

/* --------------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* ===========================================================================
**                          Macro Definitions
** ======================================================================== */
/* ===========================================================================
**                          Internal Helper Functions
** ======================================================================== */

/* ===========================================================================
**                          External API Definitions
** ======================================================================== */
/*===========================================================================

FUNCTION
    CameraSensorI2C_Init

DESCRIPTION
    This function is used to intialize I2C communication.

DEPENDENCIES
    None

RETURN VALUE
    TRUE  - success
    FALSE - failed

SIDE EFFECTS
    None

===========================================================================*/
boolean CameraSensorI2C_Init(void)
{
    return TRUE;
}

/*===========================================================================

FUNCTION
    CameraSensorI2C_DeInit

DESCRIPTION
    This function is used to de-intialize I2C communication.

DEPENDENCIES
    None

RETURN VALUE
    TRUE  - success
    FALSE - failed

SIDE EFFECTS
    None

===========================================================================*/
boolean CameraSensorI2C_DeInit(void)
{
    return TRUE;
}

/*===========================================================================

FUNCTION
    CameraSensorI2C_PowerUp

DESCRIPTION
    This function is used to power up I2C communication.

DEPENDENCIES
    None

RETURN VALUE
    TRUE  - success
    FALSE - failed

SIDE EFFECTS
    None

===========================================================================*/
boolean CameraSensorI2C_PowerUp(void)
{
    return TRUE;
}

/*===========================================================================

FUNCTION
    CameraSensorI2C_PowerDown

DESCRIPTION
    This function is used to power down I2C communication.

DEPENDENCIES
    None

RETURN VALUE
    TRUE  - success
    FALSE - failed

SIDE EFFECTS
    None

===========================================================================*/
boolean CameraSensorI2C_PowerDown(void)
{
    return TRUE;
}

/*===========================================================================

FUNCTION
    CameraSensorI2C_Write

DESCRIPTION
    This function is used to write data over the I2C bus.

DEPENDENCIES
    None

RETURN VALUE
    TRUE - if successful
    FALSE - if failed

SIDE EFFECTS
    None

===========================================================================*/
boolean CameraSensorI2C_Write(void* cmd)
{
    return TRUE;
}

/*===========================================================================

FUNCTION
    CameraSensorI2C_Read

DESCRIPTION
    This function is used to read data from the I2C bus.

DEPENDENCIES
    None

RETURN VALUE
    TRUE  - success
    FALSE - failed

SIDE EFFECTS
    None

===========================================================================*/
boolean CameraSensorI2C_Read(void* cmd)
{
    return TRUE;
}

/*===========================================================================

FUNCTION
    CameraSensorI2C_SetClockFrequency

DESCRIPTION
    This function is used to set the I2C clock frequency of the bus.

DEPENDENCIES
    None

RETURN VALUE
    TRUE  - success
    FALSE - failed

SIDE EFFECTS
    None

===========================================================================*/
void CameraSensorI2C_SetClockFrequency(CameraSensorI2C_SpeedType I2CBusSpeed)
{
    //TODO
    CAM_MSG(ERROR, "%s UNSUPPORTED API", __FUNCTION__);
    return;
}

