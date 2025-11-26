/*
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifdef ANDROID
#include <utils/Log.h>
#endif
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "log_common.h"


#define BPR     16
#define BUFFER_COUNT    1024

#define DEFAULT_OUTPUT_FILE    "/data/vendor/log.txt"

typedef struct
{
    bool                enableLog;
    pthread_mutex_t     lock;
    uint32_t            outputMask;
    uint32_t            outputMethod;
    FILE               *outputFile;
    bool                initialized;
    char                outputBuffer[BUFFER_COUNT];
} DebugZones;

static DebugZones   zoneData;

static void Output(void)
{
    if ((zoneData.outputMethod & OUTPUT_MODE_FILE) && zoneData.outputFile)
    {
        fprintf(zoneData.outputFile, "%s\n", zoneData.outputBuffer);
    }

    if (zoneData.outputMethod & OUTPUT_MODE_CONSOLE)
    {
#ifdef ANDROID
        ALOGD("%s", zoneData.outputBuffer);
#endif
    }
}

void DebugOut(uint32_t mask, char *format, ...)
{
    size_t len = 0;
    int size_remained = 0;
    va_list arglist;

    if ((!(mask & zoneData.outputMask)) || !zoneData.initialized)
    {
        return;
    }

    pthread_mutex_lock(&zoneData.lock);

    if (mask & DEBUG_ERROR)
    {
        snprintf((char *) zoneData.outputBuffer, sizeof(zoneData.outputBuffer), "%s ", "[ERR]");
    }
    else if (mask & DEBUG_WARN)
    {
        snprintf((char *) zoneData.outputBuffer, sizeof(zoneData.outputBuffer), "%s ", "[WARN]");
    }
    else if (mask & DEBUG_INFO)
    {
        snprintf((char *) zoneData.outputBuffer, sizeof(zoneData.outputBuffer), "%s", "");
    }
    else if (mask & DEBUG_OUTPUT)
    {
        snprintf((char *) zoneData.outputBuffer, sizeof(zoneData.outputBuffer), "%s", "");
    }
    else
    {
        snprintf((char *) zoneData.outputBuffer, sizeof(zoneData.outputBuffer), "%s", "");
    }

    len = strlen(zoneData.outputBuffer);

    size_remained = sizeof(zoneData.outputBuffer) - len - 1;

    if (size_remained > 0)
    {
        va_start(arglist, format);
        vsnprintf((char *) &zoneData.outputBuffer[len], size_remained, format, arglist);
        va_end(arglist);
    }

    Output();

    pthread_mutex_unlock(&zoneData.lock);
}

void DumpBuff(uint32_t mask, const uint8_t *buffer, uint32_t bufferSize)
{
    char szLine[5 + 7 + 2 + 4 * BPR];
    char *p = NULL;
    uint32_t i = 0;

    if (!(mask & zoneData.outputMask) || !zoneData.initialized || !buffer || !bufferSize)
    {
        return;
    }

    for (i = 0 ; i < bufferSize ; i += BPR)
    {
        int bpr = bufferSize - i;
        int j;

        if (bpr > BPR)
            bpr = BPR;

        snprintf(szLine, sizeof(szLine), "%04x ", i);
        p = szLine + strlen (szLine);

        for (j = 0 ; j < bpr ; ++j)
        {
            char c = (buffer[i + j] >> 4) & 0xf;

            if (c > 9) c += 'a' - 10;
            else c += '0';

            *p++ = c;
            c = buffer[i + j] & 0xf;

            if (c > 9) c += 'a' - 10;
            else c += '0';

            *p++ = c;
            *p++ = ' ';
        }

        for ( ; j < BPR ; ++j)
        {
            *p++ = ' ';
            *p++ = ' ';
            *p++ = ' ';
        }

        *p++ = ' ';
        *p++ = ' ';
        *p++ = ' ';
        *p++ = '|';
        *p++ = ' ';
        *p++ = ' ';
        *p++ = ' ';

        for (j = 0 ; j < bpr ; ++j)
        {
            char c = buffer[i + j];

            if ((c < ' ') || (c >= 127))
                c = '.';

            *p++ = c;
        }

        for ( ; j < BPR ; ++j)
        {
            *p++ = ' ';
        }

        *p++ = '\0';

        DebugOut(mask, TEXT("%s"), szLine);
    }
}

void DebugInitialize(uint32_t outputMode, uint32_t outputMask, const char *outputFilename)
{
    pthread_mutex_init(&zoneData.lock, NULL);
    pthread_mutex_lock(&zoneData.lock);

    zoneData.outputMethod = outputMode;
    zoneData.outputMask   = outputMask;

    if (zoneData.outputMethod & OUTPUT_MODE_FILE)
    {
        if (outputFilename)
        {
            zoneData.outputFile = fopen((char *) outputFilename, "w");
        }
        else
        {
            zoneData.outputMethod &= ~OUTPUT_MODE_FILE;
        }
    }

    pthread_mutex_unlock(&zoneData.lock);
}

void DebugFlushBuffers(void)
{
    pthread_mutex_lock(&zoneData.lock);

    fflush(zoneData.outputFile);

    pthread_mutex_unlock(&zoneData.lock);
}

void DebugUninitialize(void)
{
    if (zoneData.initialized)
    {
        pthread_mutex_lock(&zoneData.lock);

        if (zoneData.outputFile)
        {
            fflush(zoneData.outputFile);
            fclose(zoneData.outputFile);
        }

        zoneData.outputFile   = NULL;
        zoneData.outputMethod = 0;
        zoneData.initialized  = false;

        pthread_mutex_unlock(&zoneData.lock);
        pthread_mutex_destroy(&zoneData.lock);
    }
}

static bool isLogEnabled(void)
{
    return true;
}

void InitLog()
{
    InitLogExt(DEFAULT_OUTPUT_MODE, DEFAULT_OUTPUT_MASK, DEFAULT_OUTPUT_FILE);
}

void InitLogExt(uint32_t outputMode, uint32_t outputMask, const char *outputFile)
{
    memset(&zoneData, 0, sizeof(DebugZones));

    zoneData.enableLog = isLogEnabled();

    if (!zoneData.enableLog)
        return;

    DebugInitialize(outputMode, outputMask, outputFile);

    zoneData.initialized = true;
}

void DeinitLog()
{
    if (!zoneData.enableLog)
        return;

    DebugUninitialize();
}
