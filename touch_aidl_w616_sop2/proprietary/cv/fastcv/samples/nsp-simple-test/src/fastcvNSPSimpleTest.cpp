/**=============================================================================
 
@file
   fastcvNSPSimpleTest.cpp
 
@brief
   Sample program to demonstrate FastCV NSP pipeline.
 
Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
 
=============================================================================**/

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "fastcvNSPSimpleTest.h"
#include <unistd.h>
#include "AEEStdErr.h"
#include "rpcmem.h"

#define nLOOPS 100
#pragma weak remote_session_control

// -----------------------------------------------------------------------------
/// @brief
///   Get current time of the system in us
///
//----------------------------------------------------------------------------
static int64_t getCurrentTimeUsType()
{
    struct timeval tv;
    gettimeofday(&tv,nullptr);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

// -----------------------------------------------------------------------------
/// @brief
///   Print usages
///
//----------------------------------------------------------------------------
void printUsage()
{
    printf(
        "Sample prgram to demonstrate FastCV DSP pipeline for UYVY to RGB color conversion and downscaling\n\n"
        "Copyright (c) 2023 Qualcomm Technologies, Inc.\n"
        "All Rights Reserved.\n"
        "Confidential and Proprietary - Qualcomm Technologies, Inc.\n\n"
        "Usage : fastcvNSPSimpleTest [options]\n"
        "     -in  <input image>                Specifies path to UYVY input image\n"
        "    -out  <output image>               Specifies path to output RGB888 downscale image\n"
        "     -iW  <input_width>                Input image width\n"
        "     -iH  <input_height>               Input image height\n"
        "     -oW  <output_width>               Output image width, must be < iW\n"
        "     -oH  <output_height>              Output image height, must be < iH\n"
        "      -l  <loop_count>                 Specifies iteration count for performance measurement\n"
        "  --help                               To print help string\n"
    );
    std::exit( EXIT_FAILURE );
}


// -----------------------------------------------------------------------------
/// @brief
///   Utility function to read raw image
///
/// @param fpInput
///     file discriptor for file to read
///
/// @param img
///     Buffer to read file
///
/// @param height
///     expected height of the input image to read
///
/// @param stride
///     expected stride of the input image to read
///
/// @return int
///      0 if data read successfully
///     -1 if data read failed
///
//----------------------------------------------------------------------------
static int readImage(FILE *fpInput, uint8_t *img, uint32_t height, uint32_t stride)
{
    for (uint32_t i = 0; i < height; ++i)
    {
        uint32_t nRead = fread(img + i * stride, sizeof(uint8_t), stride, fpInput);
        if ((uint32_t)nRead != stride)
        {
            printf("Error: Read input file %d bytes, expected %d, line %d/%d\n", nRead, (int32_t)(stride * sizeof(uint8_t)), i, height);
            return -1;
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
/// @brief
///   Utility function to Write raw image to file
///
/// @param fpOutput
///     file discriptor for file to read
///
/// @param img
///     Buffer to write file
///
/// @param height
///     Height of output image
///
/// @param stride
///     expected stride of the output image to write
///
/// @return int
///      0 if data read successfully
///     -1 if data read failed
///
//----------------------------------------------------------------------------
static void writeImage(FILE *fpOutput, uint8_t *buf, uint32_t height, uint32_t stride)
{
    for (uint32_t i = 0; i < height; ++i)
    {
        fwrite(buf + i * stride, sizeof(uint8_t), stride, fpOutput);
    }
}

// -----------------------------------------------------------------------------
/// @brief
///   Execute function for FastCV NSP pipeline
///
/// @param inImage
///     Input image file path
///
/// @param height
///     Input Image height
///
/// @param width
///     Input image width
///
/// @param outImage
///     Output image file path
///
/// @param loops
///     Loop count
///
//----------------------------------------------------------------------------
static void Exec( char* inImage, uint32_t inW, uint32_t inH, char* outImage, uint32_t outW, uint32_t outH, uint32_t loops)
{
    int nErr = AEE_SUCCESS;
    int ret = 0;
    uint64_t tCC_Q = 0, tSD_Q = 0, tCmbn_Q = 0, tHlos_start = 0, tHlos_end = 0;
    int32_t fdSrc = -1, fdDst = -1, fdScratchBuf = -1;

    if(remote_session_control)
    {
        struct remote_rpc_control_unsigned_module data;
        data.domain = CDSP_DOMAIN_ID;
        data.enable = 1;
        if (AEE_SUCCESS != (nErr = remote_session_control(DSPRPC_CONTROL_UNSIGNED_MODULE, (void*)&data,
            sizeof(data))))
        {
            printf("ERROR 0x%x: Unsigned PD enabement: remote_session_control failed\n", nErr);
            return;
        }
    }
    else
    {
        nErr = AEE_EUNSUPPORTED;
        printf("ERROR 0x%x: remote_session_control interface is not supported on this device\n", nErr);
        return;
    }

    const char *uri = fastcvNSPSimpleTest_URI CDSP_DOMAIN;
    remote_handle64 handleSum = -1;
    if (AEE_SUCCESS != (nErr = fastcvNSPSimpleTest_open(uri, &handleSum)))
    {
        printf("ERROR::Failed to open %s\n", uri);
        return;
    }

    FILE *fpInput = nullptr;
    FILE *fpOutput = nullptr;
    uint8_t *src = nullptr;
    uint8_t *dst = nullptr;
    uint8_t *scratchBuf = nullptr;

    //Stride computation for buffers
    //Input UYVY buffer
    uint32_t inS = inW * 2;
    //Intermediate RGB buffer
    uint32_t scratchBufS = inW * 3;
    //Downscale RGB buffer
    uint32_t outS = outW * 3;

    //Allocate input buffer
    src =  (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, (inH * inS * sizeof(uint8_t)));
    if( nullptr == src )
    {
        printf("ERROR::rpcmem_alloc failed to allocate source memory\n");
        goto exit;
    }
    memset(src, 0, inH * inS * sizeof(uint8_t));
    fdSrc = rpcmem_to_fd(src);
    fastcvNSPSimpleTest_memMap(handleSum, fdSrc, 0, inH * inS * sizeof(uint8_t));

    //Allocate intermediate buffer to store converted RGB image
    scratchBuf = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, (inH* scratchBufS * sizeof(uint8_t)));
    if( nullptr == scratchBuf )
    {
        printf("ERROR::rpcmem_alloc failed to allocate scratch memory of size %lu \n",(unsigned long)(inH* scratchBufS * sizeof(uint8_t)));
        goto exit;
    }
    memset(scratchBuf, 0, inH * scratchBufS * sizeof(uint8_t));
    fdScratchBuf = rpcmem_to_fd(scratchBuf);
    fastcvNSPSimpleTest_memMap(handleSum, fdScratchBuf, 0, (inH * scratchBufS * sizeof(uint8_t)));

    //Allocate output buffer to store downscaled RGB image
    dst = (uint8_t*) rpcmem_alloc(RPCMEM_HEAP_ID_SYSTEM, RPCMEM_DEFAULT_FLAGS, ( outH * outS * sizeof(uint8_t)));
    if( nullptr == dst )
    {
        printf("ERROR::rpcmem_alloc failed to allocate destination memory\n");
        goto exit;
    }
    memset(dst, 0, outH * outS * sizeof(int8_t));
    fdDst = rpcmem_to_fd(dst);
    fastcvNSPSimpleTest_memMap(handleSum, fdDst, 0, (outH * outS * sizeof(int8_t)));

    fpInput = fopen( inImage, "rb");
    if (fpInput == nullptr)
    {
        printf("ERROR::Unable to open input image %s", inImage);
        goto exit;
    }

    fpOutput = fopen( outImage, "wb");
    if (fpOutput == nullptr)
    {
        printf("ERROR::Unable to open output image %s\n", outImage);
        goto exit;
    }

    readImage(fpInput, src, inH, inS);
    if( 0 != ret )
    {
        printf("ERROR::Input read error\n");
        goto exit;
    }

    (void)fastcvNSPSimpleTest_startTimerQ(handleSum);

    tHlos_start = getCurrentTimeUsType();
    if (AEE_SUCCESS != (nErr = fastcvNSPSimpleTest_ExecQ( handleSum, fdSrc, inW, inH, inS,
            fdDst, outW, outH, outS, fdScratchBuf, loops) ) )
    {
        printf("ERROR: Failed to execute execute NSP pipeline\n");
        goto exit;
    }
    tHlos_end = getCurrentTimeUsType() - tHlos_start;

    (void)fastcvNSPSimpleTest_endTimerQ(handleSum, &tCC_Q, &tSD_Q, &tCmbn_Q);

    printf("\n\nfastcvNSPSimpleTest execution Success \n");
    printf("Execution summary::\n");
    printf("\t\t UYVY to RGB888u8 conversion (avg of %d loops)    : %8lu us (DSP)\n",loops, (unsigned long)(tCC_Q/loops));
    printf("\t\t RGB888 scaledown  (avg of %d loops)              : %8lu us (DSP)\n",loops, (unsigned long)(tSD_Q/loops));
    printf("\t\t Combined execution time (avg of %d loops)        : %8lu us (HLOS)\n",loops,(unsigned long)(tHlos_end/loops));
    printf("\t\t Combined execution time (avg of %d loops)        : %8lu us (DSP)\n",loops, (unsigned long)(tCmbn_Q/loops));

    writeImage(fpOutput, dst, outH , outS);
exit:
    if(nullptr != src)
    {
        fastcvNSPSimpleTest_memUnmap(handleSum,fdSrc, inH * inS * sizeof(uint8_t));
        rpcmem_free(src);
    }
    if(nullptr != scratchBuf)
    {
        fastcvNSPSimpleTest_memUnmap(handleSum, fdScratchBuf, (inH * scratchBufS * sizeof(uint8_t)));
        rpcmem_free(scratchBuf);
    }
    if(nullptr != dst)
    {
        fastcvNSPSimpleTest_memUnmap(handleSum, fdDst, (outH * outS * sizeof(int8_t)));
        rpcmem_free(dst);
    }
    if(nullptr != fpInput)
    {
        fclose(fpInput);
    }
    if(nullptr != fpOutput)
    {
        fclose(fpOutput);
    }

    fastcvNSPSimpleTest_close(handleSum);
    return;
}

int main(int argc, char* argv[])
{
    char* inImage = nullptr;
    char* outImage = nullptr;
    uint32_t inW = 0, inH = 0, outW = 0, outH = 0;
    uint32_t loops = nLOOPS;

    if(argc < 2)
    {
        printUsage();
    }
    else
    {
        for(int i=1; i < argc; i++)
        {
            if (strncmp(argv[i], "--help", 6) == 0)
            {
                printUsage();
            }

            // Input Width
            if (strncmp(argv[i], "-iW", 3) == 0)
            {
                i++;
                if (i < argc)
                {
                    inW = std::stoi( argv[i] );
                }
                else
                {
                    printf("ERROR::Input Width value is missing\n");
                    printUsage();
                }
            }

            // Input Height
            if (strncmp(argv[i], "-iH", 3) == 0)
            {
                i++;
                if (i < argc)
                {
                    inH = std::stoi( argv[i] );
                }
                else
                {
                    printf("ERROR::Input Height value is missing\n");
                    printUsage();
                }
            }

            // output Width
            if (strncmp(argv[i], "-oW", 3) == 0)
            {
                i++;
                if (i < argc)
                {
                    outW = std::stoi( argv[i] );
                }
                else
                {
                    printf("ERROR::Output Width value is missing\n");
                    printUsage();
                }
            }

            // Output Height
            if (strncmp(argv[i], "-oH", 3) == 0)
            {
                i++;
                if (i < argc)
                {
                    outH = std::stoi( argv[i] );
                }
                else
                {
                    printf("ERROR::Output Height value is missing\n");
                    printUsage();
                }
            }

            // Input
            if (strncmp(argv[i], "-in", 3) == 0)
            {
                i++;
                if(i<argc)
                {
                    inImage =  argv[i];
                }
                else
                {
                    printf("ERROR::Input filename is missing\n");
                    printUsage();
                }
            }

            // Output
            if (strncmp(argv[i], "-out", 3) == 0)
            {
                i++;
                if(i<argc)
                {
                    outImage =  argv[i];
                }
                else
                {
                    printf("ERROR::Output filename is missing\n");
                    printUsage();
                }
            }


            // Loop count
            if (strncmp(argv[i], "-l", 2) == 0)
            {
                i++;
                if (i < argc)
                {
                    loops = std::stoi( argv[i] );
                }
                else
                {
                    printf("ERROR::Height value is missing\n");
                    printUsage();
                }
            }

        }

        if( (nullptr == inImage) || (nullptr == outImage) ||
            (!inW) || (!inH) || (!outW) || (!outH) )
        {
            printf("ERROR::Incorrect App usage %d %d %d %d\n",inW,inH,outW,outH);
            printUsage();
        }

        if( (inH < outH) || (inW < outW) )
        {
            printf("ERROR:: Incorrect Input/Output dimensions\n");
            printUsage();
        }
        Exec(inImage, inW, inH, outImage, outW, outH, loops);
    }

    return 0;
}
