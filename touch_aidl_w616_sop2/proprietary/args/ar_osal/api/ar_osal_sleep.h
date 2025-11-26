#ifndef AR_OSAL_SLEEP_H
#define AR_OSAL_SLEEP_H

/**
*
* \file ar_osal_sleep.h
*
* \brief
*      This file contains API(s) to suspend thread execution for the required
*      duration.
* \cond
*  Copyright (c) 2020 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
* \endcond
*
*/

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
	/* =======================================================================
	INCLUDE FILES FOR MODULE
	========================================================================== */
#include "ar_osal_types.h"

	/****************************************************************************
	** Sleep
	*****************************************************************************/
	/**
	Micro second sleep function. Sleep internal may not be accurate
	as requested, it would vary based on resolution of system clock ticks.
	Note that a ready thread is not guaranteed to run immediately. 
	Consequently, the thread may not run until some time after the sleep interval 
	elapses.

	@param[in]  micro_seconds: sleep duration in micro seconds.

	@return
	0 -- Success
	Nonzero -- Failure

	@dependencies
	None. @newpage
	*/
	int32_t ar_osal_micro_sleep(uint64_t micro_seconds);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* #ifndef AR_OSAL_SLEEP_H */

