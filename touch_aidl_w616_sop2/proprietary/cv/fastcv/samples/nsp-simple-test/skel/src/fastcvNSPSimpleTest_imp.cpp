/**=============================================================================
 
@file
   fastcvNSPSimpleTest_imp.cpp
 
@brief
   skel implementation for FastCV API sequence in fastcvNSPSimpleTest_test
 
Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
 
=============================================================================**/
#include "fastcv.h"
#include "HAP_farf.h"
#include "AEEStdErr.h"
#include <AEEStdDef.h>
#include "AEEStdDef.h"
#include "fastcvNSPSimpleTest.h"
#include<stdio.h>
#include<stdlib.h>
#include <assert.h>
#include "HAP_perf.h"
#include "HAP_mem.h"
#include "qurt.h"
#include "dlfcn.h"

#define FULL_CACHE_OP_THRESHOLD 1024*1024
#define GET_TIME HAP_perf_get_time_us

uint64_t tCC_start = 0, tCC_end = 0;
uint64_t tSD_start = 0, tSD_end = 0;
uint64_t tCmbn_start = 0, tCmbn_end = 0;

typedef fcvStatus (*pfnfcvColorUYVYtoRGB888u8) (const uint8_t* src, uint32_t srcStride, uint32_t srcWidth,
                                        uint32_t srcHeight, uint8_t* dst, uint32_t dstStride);
typedef fcvStatus (*pfnfcvScaleDown888u8)(const uint8_t* __restrict src, uint32_t srcWidth, uint32_t srcHeight,
                                   uint32_t srcStride, uint8_t* __restrict dst, uint32_t dstWidth,
                                   uint32_t dstHeight, uint32_t dstStride,
                                   fcvImageFormat imageFormat);
typedef struct fastcvNSPSimpleTestCtxt
{
    pfnfcvColorUYVYtoRGB888u8 fcvColorUYVYtoRGB888u8;
    pfnfcvScaleDown888u8 fcvScaleDown888u8;
    void *phLib;
} fastcvNSPSimpleTestCtxt_t;

// -----------------------------------------------------------------------------
/// @brief
///   flushes cache lines corresponding to the memory region.
///
/// @param buf
///     pointer to the buffer.
///
/// @param bufLen
///     length of the buffer.
//----------------------------------------------------------------------------

static void fastcvNSPSimpleTest_CacheFlush( void* buf, uint32_t bufLen )
{
    if( (nullptr != buf) && ( 0 < bufLen ) )
    {
        if ( FULL_CACHE_OP_THRESHOLD < bufLen )
        {
            qurt_mem_cache_clean((qurt_addr_t) buf, bufLen, QURT_MEM_CACHE_FLUSH_INVALIDATE_ALL, QURT_MEM_DCACHE);
        }
        else
        {
            qurt_mem_cache_clean((qurt_addr_t) buf, bufLen, QURT_MEM_CACHE_FLUSH, QURT_MEM_DCACHE);
        }
    }
}

// -----------------------------------------------------------------------------
/// @brief
///   invalidates cache lines corresponding to the memory region.
///
/// @param buf
///     pointer to the buffer.
///
/// @param bufLen
///     length of the buffer.
//----------------------------------------------------------------------------

static void fastcvNSPSimpleTest_CacheInv( void* buf, uint32_t bufLen )
{
    if( (nullptr != buf) && ( 0 < bufLen ) )
    {
        if ( FULL_CACHE_OP_THRESHOLD < bufLen )
        {
            qurt_mem_cache_clean((qurt_addr_t) buf, bufLen, QURT_MEM_CACHE_FLUSH_INVALIDATE_ALL, QURT_MEM_DCACHE);
        }
        else
        {
            qurt_mem_cache_clean((qurt_addr_t) buf, bufLen, QURT_MEM_CACHE_INVALIDATE, QURT_MEM_DCACHE);
        }
    }
}

// -----------------------------------------------------------------------------
/// @brief
///   maps FD to virtual address and increment its reference count.
///
/// @param bufId
///     file descriptor.
//----------------------------------------------------------------------------

static void* fastcvNSPSimpleTest_GetBuf( int32_t bufId )
{
    void*       bufPtr  = nullptr;
    if( 0 < bufId )
    {
        HAP_mmap_get( bufId, (void**)&bufPtr, NULL );
    }
    return bufPtr;
}

// -----------------------------------------------------------------------------
/// @brief
///   Decrement the reference count of the file descriptor
//----------------------------------------------------------------------------

static void fastcvNSPSimpleTest_PutBuf( int32_t bufId )
{
    if( 0 < bufId )
    {
        HAP_mmap_put( bufId );
    }
}

// -----------------------------------------------------------------------------
/// @brief
///   Get current time of system in us
///
//----------------------------------------------------------------------------
static uint64_t getCurrentTimeUsType()
{
    return GET_TIME();
}

/*===========================================================================
REMOTED FUNCTION
===========================================================================*/

// -----------------------------------------------------------------------------
/// @brief
///     DSP skel open function
///     Whenever the fastcvNSPSimpleTest_open is called by the APPS process,
///     FastRPC generates a unique apps-side handle and remembers the mapping
///     from the apps-side handle to the DSP handle *handle that we assign here.
///
/// @param uri
///     fastcvNSPSimpleTest skel URI
///
/// @param handle
///     handle to store fastcvNSPSimpleTest handle
///
/// @retval
///     0 for success, should always succeed
//----------------------------------------------------------------------------
int fastcvNSPSimpleTest_open(const char*uri, remote_handle64* handle)
{
    void *tptr = NULL;
    fastcvNSPSimpleTestCtxt_t *ctxt = NULL;
    ctxt = (fastcvNSPSimpleTestCtxt_t *)malloc(sizeof(fastcvNSPSimpleTestCtxt_t));
    memset(ctxt, 0, sizeof(fastcvNSPSimpleTestCtxt_t));

    // Load the fastCV library
    ctxt->phLib = dlopen("libfastcvadsp.so", RTLD_NOW);
    if (ctxt->phLib == NULL)
    {
        FARF(ERROR,"Failed to open libfastcvadsp.so file ");
        goto exit;
    }

    //Load symbols for required APIs
    ctxt->fcvColorUYVYtoRGB888u8 = (pfnfcvColorUYVYtoRGB888u8) dlsym(ctxt->phLib, "fcvColorUYVYtoRGB888u8");
    if ( nullptr == ctxt->fcvColorUYVYtoRGB888u8 )
    {
        FARF(ERROR,"Failed to load the symbol: %s from libfastcvadsp.so","fcvColorUYVYtoRGB888u8");
        goto exit;
    }

    ctxt->fcvScaleDown888u8 = (pfnfcvScaleDown888u8) dlsym(ctxt->phLib, "fcvScaleDown888u8");
    if ( nullptr == ctxt->fcvScaleDown888u8)
    {
        FARF(ERROR,"Failed to load the symbol: %s from libfastcvadsp.so", "fcvScaleDown888u8");
        goto exit;
    }
    tptr = ctxt;
exit:
    if ( NULL == tptr)
    {
        if ( NULL != ctxt->phLib)
        {
            free ( ctxt->phLib );
        }

        if ( NULL != ctxt)
        {
            free(ctxt);
        }
    }

    *handle = (remote_handle64)tptr;
    return 0;
}

// -----------------------------------------------------------------------------
/// @brief
///     DSP skel close function
///
/// @param handle
///     the value returned by open
///
/// @retval
///     0 for success, should always succeed
//----------------------------------------------------------------------------

int fastcvNSPSimpleTest_close(remote_handle64 handle)
{
    if ( NULL != handle )
    {
        fastcvNSPSimpleTestCtxt_t *ctxt = (fastcvNSPSimpleTestCtxt_t*) handle;
        if ( NULL != ctxt->phLib )
        {
            dlclose( ctxt->phLib );
        }

        free((void*)ctxt );
    }
    return 0;
}

// -----------------------------------------------------------------------------
/// @brief
///   Reset profiling timers value
///
/// @param handle
///     the value returned by open
//----------------------------------------------------------------------------
AEEResult fastcvNSPSimpleTest_startTimerQ(remote_handle64 handle)
{
    tCC_start = 0,     tCC_end = 0;
    tSD_start = 0,     tSD_end = 0;
    tCmbn_start = 0,   tCmbn_end = 0;
    return 0;
}

// -----------------------------------------------------------------------------
/// @brief
///   Get profiling time
///
/// @param handle
///     the value returned by open
///
/// @param res0
///     return runtime for execution of fcvColorUYVYtoRGB888u8
///
/// @param res1
///     return runtime for execution of fcvScaleDown888u8
///
/// @param res2
///     return overall execution time for both APIs.
///
//----------------------------------------------------------------------------
AEEResult fastcvNSPSimpleTest_endTimerQ(remote_handle64 handle, uint64_t* res0, uint64_t* res1, uint64_t* res2)
{
    *res0 = tCC_end;
    *res1 = tSD_end;
    *res2 = tCmbn_end;
    return 0;
}


// -----------------------------------------------------------------------------
/// @brief
///   Converts UYVY to RGB888 and downscale it.
///
/// @param fdSrc
///    file descriptor for input 8-bit UYVY source image.
///
/// @param srcW
///   Input image width.
///
/// @param srcH
///     Input image height.
///
/// @param srcS
///     Stride (bytes) for src [srcS >= srcW * 2].
///
/// @param fdDst
///   file descriptor for output RGB888 downscaled image.
///
/// @param dstW
///   Output image width
///
/// @param dstH
///     Output image height
///
/// @param dstS
///     Stride (bytes) for dst [dstS >= dstW * 3].
///
/// @param fdScratchbuf
///     file descriptor for scratch buffer to store intermediate
///     output of fcvColorUYVYtoRGB888u8. Should be of size (( srcW * 3 ) * srcH).
///
/// @param nLoops
///     Number of profiling loops
///
/// @return
///     Execution status
///
//----------------------------------------------------------------------------

AEEResult fastcvNSPSimpleTest_ExecQ( remote_handle64  handle,
                                   int32_t          fdSrc,
                                   uint32           srcW,
                                   uint32           srcH,
                                   uint32           srcS,
                                   int32_t          fdDst,
                                   uint32           dstW,
                                   uint32           dstH,
                                   uint32           dstS,
                                   int32_t          fdScratchbuf,
                                   int32            nLoops)
{
    int i = 0;
    tCmbn_start = getCurrentTimeUsType();
    fcvStatus sts = FASTCV_EFAIL;
    AEEResult ret = AEE_SUCCESS;

    fastcvNSPSimpleTestCtxt_t *ctxt = (fastcvNSPSimpleTestCtxt_t*) handle;
    if ( NULL == ctxt )
    {
        FARF(ERROR, "Invalid handle");
        return AEE_EFAILED;
    }

    uint8_t* src   = (uint8_t*) fastcvNSPSimpleTest_GetBuf(fdSrc);
    uint8_t* dst   = (uint8_t*) fastcvNSPSimpleTest_GetBuf(fdDst);
    uint8_t* scratchbuf   = (uint8_t*) fastcvNSPSimpleTest_GetBuf(fdScratchbuf);

    fastcvNSPSimpleTest_CacheInv(src, (srcS * srcH));

    for(i=0; i < nLoops; i++)
    {
        tCC_start = getCurrentTimeUsType();
        sts = ctxt->fcvColorUYVYtoRGB888u8(src,srcS, srcW, srcH, scratchbuf, (srcW * 3));
        tCC_end += (getCurrentTimeUsType() - tCC_start);

        if( sts != FASTCV_SUCCESS )
        {
            FARF(ALWAYS, "ERROR:: fcvColorUYVYtoRGB888u8 failed with errorcode:%x", sts);
            ret = AEE_EFAILED;
            break;
        }

        tSD_start = getCurrentTimeUsType();
        sts = ctxt->fcvScaleDown888u8(scratchbuf, srcW, srcH, (srcW * 3), dst, dstW, dstH, dstS, FASTCV_RGB);
        if( sts != FASTCV_SUCCESS )
        {
            FARF(ALWAYS, "ERROR:: fcvScaleDown888u8 failed with errorcode:%x", sts);
            ret = AEE_EFAILED;
            break;
        }

        tSD_end += (getCurrentTimeUsType() - tSD_start);
    }
    tCmbn_end += (getCurrentTimeUsType() - tCmbn_start);

    fastcvNSPSimpleTest_PutBuf(fdSrc);
    fastcvNSPSimpleTest_PutBuf(fdDst);
    fastcvNSPSimpleTest_PutBuf(fdScratchbuf);

    fastcvNSPSimpleTest_CacheFlush(dst, (dstS * dstH));
    fastcvNSPSimpleTest_CacheFlush(scratchbuf, ( (srcW * 3) * srcH));

    return ret;
}

// -----------------------------------------------------------------------------
/// @brief
///   Maps memory corresponding to fd to DSP SMMU.
///
/// @param handle
///     The value returned by open.
///
/// @param bufOffset
///     offset from the starting memory.
///
/// @param bufSize
///     length of the buffer.
///
/// @return
///     Execution status
///
//----------------------------------------------------------------------------

AEEResult fastcvNSPSimpleTest_memMap(remote_handle64 handle,
                                   int             bufId,
                                   uint32_t        bufOffset,
                                   uint32_t        bufSize )
{
    AEEResult   status  = AEE_EFAILED;
    int32_t     prot    = HAP_PROT_READ | HAP_PROT_WRITE;
    int32_t     flags   = 0;

    void *bufPtr = HAP_mmap( nullptr, bufSize, prot, flags, bufId, bufOffset );
    if( (void*)0xFFFFFFFF != bufPtr )
    {
        status = AEE_SUCCESS;
    }

    return status;
}

// -----------------------------------------------------------------------------
/// @brief
///   Unmaps memory corresponding to fd to DSP SMMU.
///
/// @param handle
///     The value returned by open.
///
/// @param bufId
///     file descriptor, memory corresponding to which to be unmapped.
///
/// @param bufSize
///     length of the bufferxecution of fcvScaleDown888u8.
///
/// @return
///     Execution status.
///
//----------------------------------------------------------------------------

AEEResult fastcvNSPSimpleTest_memUnmap( remote_handle64 handle,
                                      int32_t         bufId,
                                      uint32_t        bufSize )
{
    AEEResult   status  = AEE_EFAILED;
    int32_t     err     = -1;
    uint32_t    iterCnt = 100;

    do
    {
        //decrement user count to 0
        err = HAP_mmap_put(bufId);
        iterCnt--;
    } while( ( 0 == err ) && (0 < iterCnt) );

    status = (AEEResult)HAP_munmap( fastcvNSPSimpleTest_GetBuf(bufId), bufSize );
    return status;
}
