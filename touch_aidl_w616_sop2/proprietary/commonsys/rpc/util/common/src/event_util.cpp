/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <chrono>

#include "event_util.h"

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
 *----------------------------------------------------------------------------*/
EventHandle* EventCreate()
{
    return new EventHandle;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EventWait
 *
 *  DESCRIPTION
 *      Wait for the event to be set.
 *
 *  RETURNS
 *      Possible values:
 *          RESULT_SUCCESS                 in case of success
 *          FE_RESULT_TIMEOUT              in case of timeout
 *          FE_RESULT_INVALID_HANDLE       in case the eventHandle is invalid
 *----------------------------------------------------------------------------*/
uint16_t EventWait(EventHandle *eventHandle, uint16_t timeoutInMs)
{
    uint16_t res = RESULT_SUCCESS;

    if (eventHandle == NULL)
    {
        return FE_RESULT_INVALID_HANDLE;
    }

    std::unique_lock<std::mutex> lk(eventHandle->mutex);
    if ((timeoutInMs != 0) && (timeoutInMs != EVENT_WAIT_INFINITE))
    {
        std::cv_status status = std::cv_status::no_timeout;
        eventHandle->event.wait_until(lk, std::chrono::system_clock::now() +
                                      std::chrono::milliseconds(timeoutInMs));
        if (status != std::cv_status::no_timeout)
        {
            res = FE_RESULT_TIMEOUT;
        }
    }
    else
    {
        eventHandle->event.wait(lk);
    }

    return res;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      EventSet
 *
 *  DESCRIPTION
 *      Set an event.
 *
 *  RETURNS
 *      Possible values:
 *          RESULT_SUCCESS              in case of success
 *          FE_RESULT_INVALID_HANDLE    in case the eventHandle is invalid
 *
 *----------------------------------------------------------------------------*/
uint16_t EventSet(EventHandle *eventHandle)
{
    if (eventHandle == NULL)
    {
        return FE_RESULT_INVALID_HANDLE;
    }

    std::lock_guard<std::mutex> lk(eventHandle->mutex);
    eventHandle->event.notify_all();

    return RESULT_SUCCESS;
}

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
void EventDestroy(EventHandle *eventHandle)
{
    if (eventHandle == NULL)
    {
        return;
    }

    delete eventHandle;
}
