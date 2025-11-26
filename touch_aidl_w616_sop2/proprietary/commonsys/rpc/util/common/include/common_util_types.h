/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include <condition_variable>
#include <mutex>

typedef uint16_t ResultType;

#define RESULT_SUCCESS  ((ResultType) 0x0000)
#define RESULT_FAILURE  ((ResultType) 0xFFFF)

#define IS_RESULT_SUCCESS(res)  ((res) == RESULT_SUCCESS)

/* uint16_t codes */
#define FE_RESULT_NO_MORE_EVENTS    ((ResultType) 0x0001)
#define FE_RESULT_INVALID_POINTER   ((ResultType) 0x0002)
#define FE_RESULT_INVALID_HANDLE    ((ResultType) 0x0003)
#define FE_RESULT_NO_MORE_MUTEXES   ((ResultType) 0x0004)
#define FE_RESULT_TIMEOUT           ((ResultType) 0x0005)
#define FE_RESULT_NO_MORE_THREADS   ((ResultType) 0x0006)
#define FE_RESULT_NO_MORE_TIMERS    ((ResultType) 0x0007)

struct Event
{
    std::condition_variable event;
    std::mutex mutex;
};

typedef struct Event EventHandle;
