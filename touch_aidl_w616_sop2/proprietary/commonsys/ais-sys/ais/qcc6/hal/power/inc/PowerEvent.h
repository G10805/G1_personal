/* ===========================================================================
 *Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *All rights reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#ifndef _POWER_EVENTS_H_
#define _POWER_EVENTS_H_

typedef enum
{
     AIS_PM_SUSPEND,
     AIS_PM_RESUME
} pm_event_t;

typedef int (*PowerEventCb) (pm_event_t event, void *pUsrCtxt);

class PowerPolicyService;
extern std::shared_ptr<PowerPolicyService> PowerEventInit(PowerEventCb cb,void *pUsrCtxt);

#endif //_POWER_EVENTS_H_
