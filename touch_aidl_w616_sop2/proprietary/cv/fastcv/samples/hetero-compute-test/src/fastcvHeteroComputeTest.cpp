/**=============================================================================

@file
   fastcvSimpleTest.cpp

@brief
   FastCV sample test to demonstrate heterogenous computation

Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================**/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include "fastcv.h"
#include "fastcvExt.h"
#include <dlfcn.h>

#define MAX_BUF_LEN        1024

#define FASTCV_TEST_PRINTF( FMT, ... ) \
   printf( FMT "\n", ##__VA_ARGS__ );
#define FASTCV_TEST_ERROR( FMT, ... ) \
   fprintf( stderr, FMT "\n", ##__VA_ARGS__ );

void getTime(uint64_t *time)
{
   struct timeval tv;
   struct timezone tz;

   gettimeofday(&tv, &tz);

   *time = tv.tv_sec * 1000000ULL + tv.tv_usec;
}

void writeOutput(FILE *fpOutput, uint8_t *pImage, uint32_t width, uint32_t height)
{
	if (fpOutput && pImage)
   {
      fprintf( fpOutput, "P5\n%d %d\n255\n", width, height );
      for (uint32_t i = 0; i < height; ++i)
      {
         uint32_t nWrite = fwrite(pImage+i*width, 1, width, fpOutput);
         if (nWrite != width)
         {
            FASTCV_TEST_ERROR("Error: Write output file %d bytes, expected %d, line %d/%d", nWrite, width, i, height);
         }
      }
   }
}

static void runTest (uint32_t       loop,
                     uint64_t*      totalTime,
                     const uint8_t* src,
                     uint32_t       srcWidth,
                     uint32_t       srcHeight,
                     uint32_t       srcStride,
                     uint8_t*       dst,
                     uint32_t       dstWidth,
                     uint32_t       dstHeight,
                     uint32_t       dstStride )
{
    uint64_t startTime = 0, endTime = 0, beforeScaleDown = 0, afterScaleDown = 0, scaleDownTime = 0,
             beforeFilter = 0, afterFilter = 0, filterTime = 0, beforeCornerHarris = 0, afterCornerHarris = 0,
             cornerHarrisTime = 0;
    unsigned int border = 2;
    unsigned int nCornersMax = dstWidth*dstHeight/4;  //Note: Its not a magic number/criteria. Can be set to desired value by the user.
    int32_t threshold = 0;
    uint32_t nCorners = 0;
    uint32_t *xy = NULL;
    uint8_t  *scratchBuf = NULL;

    xy = (uint32_t *)fcvMemAlloc(nCornersMax * 2 * sizeof(uint32_t), 16);
    scratchBuf = (uint8_t *)fcvMemAlloc( ( dstWidth * dstStride * sizeof(uint8_t) ), 16);

    if( xy && scratchBuf)
    {
        memset(xy, 0, nCornersMax * 2 * sizeof(uint32_t) );
        getTime( &startTime );

        for( uint32_t i = 0; i < loop; i++ )
        {
            // Execution as per configured FASTCV_OP_CPU_PERFORMANCE mode
            getTime(&beforeScaleDown);
            fcvScaleDownMNu8( src,
                    srcWidth,
                    srcHeight,
                    srcStride,
                    scratchBuf,
                    dstWidth,
                    dstHeight,
                    dstStride );
            getTime(&afterScaleDown);
            scaleDownTime += afterScaleDown - beforeScaleDown;

            // Override execution of fcvFilterGaussian3x3u8_v2 to run on GPU
            fcvSetFunctionApiToCore(e_fcvFilterGaussian3x3u8_v2, FCV_IMPL_Gpu_ext);
            getTime(&beforeFilter);
            fcvFilterGaussian3x3u8_v2( scratchBuf,
                    dstWidth,
                    dstHeight,
                    dstStride,
                    dst,
                    dstStride,
                    1 );
            getTime(&afterFilter);
            filterTime += afterFilter - beforeFilter;
            // Override execution of fcvCornerHarrisu8 to run on DSP
            fcvSetFunctionApiToCore(e_fcvCornerHarrisu8, FCV_IMPL_Qdsp_ext);
            getTime(&beforeCornerHarris);
            fcvCornerHarrisu8(dst,
                    dstWidth,
                    dstHeight,
                    dstStride,
                    border,
                    xy,
                    nCornersMax,
                    &nCorners,
                    threshold);
            getTime(&afterCornerHarris);
            cornerHarrisTime += afterCornerHarris - beforeCornerHarris;
        }

        getTime(&endTime);
        if(totalTime)
        {
            *totalTime = endTime - startTime;
        }

        // Let's have the corner for the base pyramid
        for ( uint32_t k = 0; k < nCorners; k++ )
        {
            int32_t index = xy[k*2] + (xy[k*2+1] * dstWidth);
            // Making the detected corners in the input image as white
            dst[index] = 255;
        }

        FASTCV_TEST_PRINTF("   Avg Execution time of fcvScaleDownMNu8 on CPU_PERF mode %lu usec",
                                 ( scaleDownTime / loop ) );
        FASTCV_TEST_PRINTF("   Avg Execution time of fcvFilterGaussian3x3u8_v2 on GPU %lu usec",
                                 ( filterTime / loop ) );
        FASTCV_TEST_PRINTF("   Avg Execution time of fcvCornerHarrisu8 on NSP %lu usec",
                                 ( cornerHarrisTime / loop ) );

        fcvMemFree(xy);
        fcvMemFree(scratchBuf);
        scratchBuf = NULL;
        xy = NULL;
    }
    else
    {
        FASTCV_TEST_ERROR("Error: Failed to allocate scratch buffers!!");
    }
}

int main()
{
    int ret = -1;
    // Input image of 1920x1080 resolution
    uint32_t width = 1920;
    uint32_t height = 1080;
    // Scaled down output image of 640x480 resolution
    uint32_t dstWidth = 640;
    uint32_t dstHeight = 480;
    uint32_t loop = 1;
    char tmpFullPath[] = "/data/fastcvHeteroComputeTest_output.pgm";

    /***************************************
    // FastCV initialization
     ****************************************/
    ret = fcvSetOperationMode( FASTCV_OP_CPU_PERFORMANCE );

    if( 0 != ret )
    {
        fprintf( stderr, "\nFAILED to set operation mode : FASTCV_OP_CPU_PERFORMANCE\n");
        return 1;
    }

    fcvMemInit();
    /***************************************
    // FastCV initialization ends
     ****************************************/

    /***********************************************
    //Creating Checker-board src image
    NOTE : Input image can be read from a file here.
     ************************************************/
    uint8_t *pSrc = NULL;
    uint8_t *pDst = NULL;
    uint32_t bSize = 64;                           //size of block of checkers board

    pSrc = (uint8_t *)fcvMemAlloc(width*height, 16);

    if( pSrc )
    {
        for(uint32_t i = 0; i < width*height; i++)       //initialise all with 0
        {
            pSrc[i]=0;
        }

        bool v=false;
        uint32_t i=0,shift=0, k, ptr;

        for( uint32_t j = 0; j < height; j++ )
        {
            if( ( j % bSize ) == 0 )
            {
                v = !v;                         //fill /unfil this block
            }
            if( !v )                            //where to start putting blacks
            {
                shift = bSize;
            }

            ptr = width * j;

            for( i = i + shift; i < width; )    //at displacemnt of i+shift begin to fill white
            {
                for( k = 0; k < bSize; k++, i++ )
                {
                    pSrc[ptr+i] = 127;
                }

                i = i + bSize;                  //skip that block in the row
            }

            shift = 0;
            i = 0;
        }

        uint64_t totalTime = 0;

        pDst = (uint8_t *)fcvMemAlloc( dstWidth*dstHeight, 16 );

        if ( pDst )
        {
            memset( pDst, 0, dstWidth * dstHeight );
            runTest( loop, &totalTime, pSrc, width, height, width,
                    pDst, dstWidth, dstHeight, dstWidth );

            // Write the result
            FILE *fpOutput = fopen( tmpFullPath, "wb" );
            if (fpOutput)
            {
                writeOutput(fpOutput,pDst,dstWidth,dstHeight);
                fclose(fpOutput);
                fpOutput = NULL;
            }

            FASTCV_TEST_PRINTF("Average time (us/frame) for fastcvHeteroComputeTest: %lu", totalTime/loop);
            FASTCV_TEST_PRINTF("Output file can be found at %s\n", tmpFullPath);
        }
        else
        {
            FASTCV_TEST_ERROR("ERROR:: DST allocation failed!!\n");
        }
    }
    else
    {
        FASTCV_TEST_ERROR("ERROR:: SRC allocation failed!!\n");
    }

    if ( pSrc )
    {
        fcvMemFree( pSrc );
        pSrc = NULL;
    }
    if ( pDst )
    {
        fcvMemFree( pDst );
        pDst = NULL;
    }

    /***************************************
    // FastCV deinitialization
     ****************************************/
    fcvMemDeInit();
    fcvCleanUp();
    /***************************************
    // FastCV deinitialization ends
     ****************************************/
    return 0;
}
