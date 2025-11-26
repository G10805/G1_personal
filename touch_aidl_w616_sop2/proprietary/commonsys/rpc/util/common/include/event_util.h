/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <thread>
#include <iostream>

#include "common_util_types.h"

#define EVENT_WAIT_INFINITE         ((uint16_t) 0xFFFF)

/*----------------------------------------------------------------------------*
 *  NAME
 *      EventCreate
 *
 *  DESCRIPTION
 *      Creates an event and returns a handle to the created event.
 *
 *  RETURNS
 *      Possible values:
 *          RESULT_SUCCESS             in case of success
 *          FE_RESULT_NO_MORE_EVENTS   in case of out of event resources
 *          FE_RESULT_INVALID_POINTER  in case the eventHandle pointer is invalid
 *
 *----------------------------------------------------------------------------*/
EventHandle* EventCreate();

/*----------------------------------------------------------------------------*
 *  NAME
 *      EventWait
 *
 *  DESCRIPTION
 *      Wait for the event to be set
 *
 *  RETURNS
 *      Possible values:
 *          RESULT_SUCCESS                 in case of success
 *          FE_RESULT_TIMEOUT              in case of timeout
 *          FE_RESULT_INVALID_HANDLE       in case the eventHandle is invalid
 *
 *----------------------------------------------------------------------------*/
uint16_t EventWait(EventHandle *eventHandle, uint16_t timeoutInMs);

/*----------------------------------------------------------------------------*
 *  NAME
 *      EventSet
 *
 *  DESCRIPTION
 *      Set an event.
 *
 *  RETURNS
 *      Possible values:
 *          RESULT_SUCCESS                 in case of success
 *          FE_RESULT_INVALID_HANDLE       in case the eventHandle is invalid
 *
 *----------------------------------------------------------------------------*/
uint16_t EventSet(EventHandle *eventHandle);

/*----------------------------------------------------------------------------*
 *  NAME
 *      EventDestroy
 *
 *  DESCRIPTION
 *      Destroy the event associated.
 *
 *  RETURNS
 *      void
 *
 *----------------------------------------------------------------------------*/
void EventDestroy(EventHandle *eventHandle);
