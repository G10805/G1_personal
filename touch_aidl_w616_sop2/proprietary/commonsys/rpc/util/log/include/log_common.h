/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_ERROR             (0x00000001)
#define DEBUG_WARN              (0x00000002)
#define DEBUG_INFO              (0x00000004)
#define DEBUG_OUTPUT            (0x00000008)

#define OUTPUT_MODE_NONE        (0x00000000)
#define OUTPUT_MODE_CONSOLE     (0x00000001)
#define OUTPUT_MODE_FILE        (0x00000002)

#define DEFAULT_OUTPUT_MODE     (OUTPUT_MODE_CONSOLE)

#define DEFAULT_OUTPUT_MASK     (DEBUG_ERROR | DEBUG_WARN)

#define TEXT(str)               (char *)(str)

#define FUNC_ENTER()            IFDBG(DebugOut(DEBUG_INFO, TEXT("%s: +++"), __func__))
#define FUNC_LEAVE()            IFDBG(DebugOut(DEBUG_INFO, TEXT("%s: ---"), __func__))

#define OUTPUT_FUNC()           IFDBG(DebugOut(DEBUG_INFO, TEXT("%s"), __func__))


void DumpBuff(uint32_t mask, const uint8_t *buffer, uint32_t bufferSize);
void DebugOut(uint32_t mask, char *format, ...);
void DebugInitialize(uint32_t outputMode, uint32_t outputMask, const char *outputFilename);
void DebugFlushBuffers(void);
void DebugUninitialize(void);

#define IFDBG(c)    c

void InitLog(void);
void InitLogExt(uint32_t outputMode, uint32_t outputMask, const char *outputFile);
void DeinitLog(void);

#ifdef __cplusplus
}
#endif