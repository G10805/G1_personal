/********************************************************************
 Copyright (c) 2024 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/
#ifndef _CLOSENOTIFIER_H
#define _CLOSENOTIFIER_H

#include "object.h"
#include "qlist.h"
#include "msforwarder.h"
#include "VmOsal.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct CloseNotifier CloseNotifier;

typedef void (*CloseHandlerFunc)(void *data, int32_t event);

#define EVENT_CLOSE       (0x00000020u)
#define EVENT_CRASH       (EVENT_CLOSE - 1)
#define EVENT_DELETE      (EVENT_CLOSE - 2)
#define EVENT_DETACH      (EVENT_CLOSE - 3)
#define EVENT_UNKNOWN     (0x0000F000u)

#define isCloseEvent(event)    (((event) & EVENT_CLOSE) != 0)

int32_t CloseNotifier_new(CloseHandlerFunc func, void *data, Object target,
                          Object *objOut);
int32_t CloseNotifier_subRegister(Object target, Object handler, Object *subNotifier);
int32_t CloseNotifier_popFromMSForwarder(CloseNotifier **me, MSForwarder *msFwd);
void CloseNotifier_notify(CloseNotifier *me, uint32_t event);

#if defined (__cplusplus)
}
#endif

#endif // _CLOSENOTIFIER_H
