////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2018, 2020 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file  cdkutils.h
/// @brief CDK OS specific utility implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CDKUTILS_H
#define CDKUTILS_H

#include <stdio.h>

#include "camxcdktypes.h"
#include "chxutils.h"

#if defined (_WIN32)
#include <windows.h>
#endif // _WIN32

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief CDK OS specific utility class implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOWHINE CP017,CP018: All static class does not need copy/assignment overrides
class CdkUtils
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FileNo
    ///
    /// @brief  gets the File Handle for the Descriptor
    ///
    /// @param  pFile File Handle
    ///
    /// @return File Descriptor
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CDK_VISIBILITY_PUBLIC inline INT FileNo(
        FILE* pFile)
    {
#if defined (_LINUX) || defined(__QNXNTO__)
        return fileno(pFile);
#endif // _LINUX
#if defined (_WIN32)
        return fileno(pFile);
#endif // _WIN32
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FOpen
    ///
    /// @brief  Opens a file on the filesystem
    ///
    /// @param  pFilename File to open
    /// @param  pMode     File access mode string
    ///
    /// @return File handle or NULL on failure
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CDK_VISIBILITY_PUBLIC inline FILE* FOpen(
        const CHAR* pFilename,
        const CHAR* pMode)
    {
#if defined (_LINUX) || defined(__QNXNTO__)
        return fopen(pFilename, pMode);
#endif // _LINUX
#if defined (_WIN32)
        FILE* pFile;
        if (0 != fopen_s(&pFile, pFilename, pMode))
        {
            pFile = NULL;
        }
        return pFile;
#endif // _WIN32
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FClose
    ///
    /// @brief  Closes the given file handle
    ///
    /// @param  pFile File handle to close
    ///
    /// @return CDKResultSuccess on success, CamxResultEFailed otherwise
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CDK_VISIBILITY_PUBLIC inline CDKResult FClose(
        FILE* pFile)
    {
        CDKResult result = CDKResultSuccess;

        if (0 != fclose(pFile))
        {
            result = CDKResultEFailed;
        }

        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// FWrite
    ///
    /// @brief  Writes data to a file handle
    ///
    /// @param  pSrc     Source memory
    /// @param  size     Size of objects to write
    /// @param  count    Number of objects to write
    /// @param  pFile    File handle to write to
    ///
    /// @return Number of objects written successfully
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CDK_VISIBILITY_PUBLIC inline SIZE_T FWrite(
        const VOID* pSrc,
        SIZE_T      size,
        SIZE_T      count,
        FILE*       pFile)
    {
        return fwrite(pSrc, size, count, pFile);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SNPrintF
    ///
    /// @brief Write formatted data from variable argument list to sized buffer
    ///
    /// @param pDst     Destination buffer
    /// @param sizeDst  Size of the destination buffer
    /// @param pFormat  Format string, printf style
    /// @param ...      Parameters required by format
    ///
    /// @return Number of characters written
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static CDK_VISIBILITY_PUBLIC inline INT SNPrintF(
        CHAR*       pDst,
        SIZE_T      sizeDst,
        const CHAR* pFormat,
        ...)
    {
        INT     numCharWritten;
        va_list args;

        va_start(args, pFormat);
        numCharWritten = VSNPrintF(pDst, sizeDst, pFormat, args);
        va_end(args);

        return numCharWritten;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// VSNPrintF
    ///
    /// @brief  Write formatted data from variable argument list to sized buffer
    ///
    /// @param  pDst     Destination buffer
    /// @param  sizeDst  Size of the destination buffer
    /// @param  pFormat  Format string, printf style
    /// @param  argptr   Parameters required by format
    ///
    /// @return Number of characters written
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static INT CDK_VISIBILITY_PUBLIC inline VSNPrintF(
        CHAR*       pDst,
        SIZE_T      sizeDst,
        const CHAR* pFormat,
        va_list     argptr)
    {
        INT numCharWritten = 0;
#if defined (_LINUX) || defined(__QNXNTO__)
        numCharWritten = vsnprintf(pDst, sizeDst, pFormat, argptr);
#endif // _LINUX
#if defined (_WIN32)
        numCharWritten = vsnprintf_s(pDst, sizeDst, _TRUNCATE, pFormat, argptr);
#endif // _WIN32
        CHX_ASSERT(numCharWritten >= 0);
        if ((numCharWritten >= static_cast<INT>(sizeDst)) && (sizeDst > 0))
        {
            // Message length exceeds the buffer limit size
            CHX_ASSERT(FALSE);
            numCharWritten = -1;
        }

        return numCharWritten;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// StrLCpy
    ///
    /// @brief  Copy string from source pointer to destination
    ///
    /// @param  pDst     Destination buffer
    /// @param  pSrc     Source buffer
    /// @param  sizeDst  Size of the destination buffer
    ///
    /// @return Number of characters written
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static SIZE_T CDK_VISIBILITY_PUBLIC inline StrLCpy(
        CHAR*       pDst,
        const CHAR* pSrc,
        SIZE_T      sizeDst)
    {
#if defined(ANDROID)
        return strlcpy(pDst, pSrc, sizeDst);
#elif defined(__QNXNTO__) || defined(CAMERA_UNITTEST)
    size_t nLen = strlen(pSrc);

    if (0 < sizeDst) {
       int n;

       n = (nLen <= (sizeDst - 1)) ? nLen :  (sizeDst - 1);
       (void)memmove(pDst, pSrc, n);

       pDst[n] = 0;
    }
    return nLen;
#else // ANDROID
        return g_strlcpy(pDst, pSrc, sizeDst);
#endif // ANDROID
    }

private:
    CdkUtils() = default;
};

#endif // CDKUTILS_H
