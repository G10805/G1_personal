/**=============================================================================

@file
   fastcvSimpleTest.cpp

@brief
   Few simple fastcv API tests

Copyright (c) 2017, 2022 - 2023 Qualcomm Technologies, Inc.
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

fcvOperationMode modeVals[] = {
  FASTCV_OP_LOW_POWER,
  FASTCV_OP_PERFORMANCE,
  FASTCV_OP_CPU_OFFLOAD,
  FASTCV_OP_CPU_PERFORMANCE
};

fcvOperationModeExt targetVals[] = {
    FASTCV_OP_EXT_QDSP,
    FASTCV_OP_EXT_GPU,
    FASTCV_OP_EXT_VENUM
};

void printUsage(char *testName)
{
   fprintf( stderr, "USAGE: %s test_data_directory [-l loops] [-o output_file]\n"
                    "       [-M operationMode] / [-t targetMode] [-w dstWidth] [-h dstHeight]\n\n"
                    "       -M integer\n"
                    "       Integer value indicating Operation Mode.  Valid values are:\n"
                    "       FASTCV_OP_LOW_POWER       = 0\n"
                    "       PERFORMANCE               = 1\n"
                    "       CPU_OFFLOAD               = 2\n"
                    "       FASTCV_OP_CPU_PERFORMANCE = 3\n\n"
                    "       -t integer\n"
                    "       Integer value indicating extended target Mode.\n"
                    "       Will be ignored if -M is set. Valid values are:\n"
                    "       Extended DSP target       = 0\n"
                    "       Extended GPU target       = 1\n",
                    testName );
}

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

void runfastcvSimpleTest (uint32_t       loop,
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
   uint64_t startTime = 0, beforeScaleDown=0, beforeCornerHarris=0, afterCornerHarris=0, endTime = 0;
   unsigned int border = 2;
   unsigned int nCornersMax = dstWidth*dstHeight/4;  //Note: Its not a magic number/criteria. Can be set to desired value by the user.
   int32_t threshold = 0;

   uint32_t nCorners = 0;

   uint32_t *xy = (uint32_t *)fcvMemAlloc(nCornersMax * 2 * sizeof(uint32_t), 16);

   if(dst && xy )
   {
      getTime(&startTime);
      for(uint32_t i = 0; i < loop; i++)
      {
         getTime(&beforeScaleDown);
         fcvScaleDownMNu8(src,
                          srcWidth,
                          srcHeight,
                          srcStride,
                          dst,
                          dstWidth,
                          dstHeight,
                          dstStride);

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
      }


      getTime(&endTime);
      if(totalTime)
      {
         *totalTime = endTime - startTime;
      }

      // Let's have the corner for the base pyramid
      for (uint32_t k=0; k<nCorners; k++)
      {
         int32_t index = xy[k*2] + (xy[k*2+1] * dstWidth);
         dst[index] = 255;                      // Making the detected corners in the input image as white
      }

      FASTCV_TEST_PRINTF("   Execution time of fcvScaleDownMNu8 %lu usec", beforeCornerHarris-beforeScaleDown);
      FASTCV_TEST_PRINTF("   Execution time of fcvCornerHarrisu8 %lu usec", afterCornerHarris-beforeCornerHarris);

      if(xy)
      {
         fcvMemFree(xy);
         xy = NULL;
      }
   }
   else
   {
      FASTCV_TEST_ERROR("Error: Failed to allocate memory in fastcvSimpleTest");
   }
}

int runCommandLine( int argc, char* argv[], uint64_t *timer )
{
    if( argc < 4 )
    {
        printUsage(argv[0]);
        return 1;
    }

    char *pTestDir = argv[1];
    char *pOutputFile = NULL;
    uint32_t loop = 1;
    uint32_t opMode = (uint32_t) FASTCV_OP_RESERVED;
    uint32_t targetMode = (uint32_t) FASTCV_OP_EXT_VENUM;

    char modeString[20];
    strlcpy(modeString, "CPU_PERFORMANCE", sizeof(modeString));

    int ret = -1;
    uint32_t dstWidth = 640;
    uint32_t dstHeight = 480;

    for( int i=2; i<argc; ++i )
    {
        if( strncmp( argv[i], "-o", 2 ) == 0 )
        {
            ++i;
            if( i < argc )
            {
                pOutputFile = argv[i];
            }
            else
            {
                fprintf( stderr, "ERROR: missing param with option -o\n");
                printUsage(argv[0]);
                return 1;
            }
        }
        else if( strncmp( argv[i], "-l", 2 ) == 0 )
        {
            ++i;
            if( i < argc )
            {
                loop = atoi(argv[i]);
                loop = (loop == 0) ? 1:loop;
            }
            else
            {
                fprintf( stderr, "ERROR: missing param with option -l\n");
                printUsage(argv[0]);
                return 1;
            }
        }
        else if( strncmp( argv[i], "-M", 2 ) == 0 )
        {
            ++i;
            if( i < argc )
            {
                opMode = atoi(argv[i]);

                switch(opMode)
                {
                    case 0: strlcpy(modeString, "LOW_POWER",       sizeof(modeString)); break;
                    case 3: strlcpy(modeString, "CPU_PERFORMANCE", sizeof(modeString)); break;
                    case 2: strlcpy(modeString, "CPU_OFFLOAD", sizeof(modeString)); break;
                    case 1: strlcpy(modeString, "PERFORMANCE", sizeof(modeString)); break;
                    default:strlcpy(modeString, "Unknown", sizeof(modeString)); break;
                }
            }
            else
            {
                fprintf( stderr, "ERROR: missing param with option -M\n");
                printUsage(argv[0]);
                return 1;
            }
        }
        else if( strncmp( argv[i], "-t", 2 ) == 0 )
        {
            ++i;
            if( i < argc )
            {
                targetMode = atoi(argv[i]);

                switch(targetMode)
                {
                    case 0: strlcpy(modeString, "EXT_QDSP",sizeof(modeString)); break;
                    case 1: strlcpy(modeString, "EXT_GPU", sizeof(modeString)); break;
                    default:strlcpy(modeString, "Unknown", sizeof(modeString)); break;
                }
            }
            else
            {
                fprintf( stderr, "ERROR: missing param with option -t\n");
                printUsage(argv[0]);
                return 1;
            }
        }
        else if( strncmp( argv[i], "-w", 2 ) == 0 )
        {
            ++i;
            if( i < argc )
            {
                dstWidth = atoi(argv[i]);
            }
            else
            {
                fprintf( stderr, "ERROR: missing param with option -w\n");
                printUsage(argv[0]);
                return 1;
            }
        }
        else if( strncmp( argv[i], "-h", 2 ) == 0 )
        {
            ++i;
            if( i < argc )
            {
                dstHeight = atoi(argv[i]);
            }
            else
            {
                fprintf( stderr, "ERROR: missing param with option -h\n");
                printUsage(argv[0]);
                return 1;
            }
        }
    }

    char tmpFullPath[255] = {0};

    uint32_t width = 1920, height = 1080;

    FASTCV_TEST_PRINTF("\nFunction called from test application: fastcvSimpleTest, OperationMode: %s", modeString);
    FASTCV_TEST_PRINTF("Input image width = %d, height = %d  function to call:fastcvSimpleTest", width, height);

    /***************************************
    // FastCV initialization
     ****************************************/
    if (opMode < 4)
    {
        ret = fcvSetOperationMode(modeVals[opMode]);
    }
    else if (targetMode < 3)
    {
        ret = fcvSetOperationModeExt(targetVals[targetMode]);
    }
    else
    {
        fprintf( stderr, "\nFAILED: Choose among supported opMode values\n");
        return 1;
    }

    if( 0 != ret )
    {
        fprintf( stderr, "\nFAILED to set operation mode : %s\n", modeString);
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
    uint8_t *pSrc = (uint8_t *)fcvMemAlloc(width*height, 16);
    uint8_t *pDst = NULL;
    uint32_t bSize=64;                           //size of block of checkers board

    if( pSrc )
    {
        for(uint32_t i = 0; i < width*height; i++)       //initialise all with 0
            pSrc[i]=0;
        bool v=false;
        uint32_t i=0,shift=0, k, ptr;
        for(uint32_t j=0;j<height;j++){

            if(j%bSize==0)
                v=!v;                       //fill /unfil this block

            if(!v)                          //where to start putting blacks
                shift=bSize;

            ptr=width*j;

            for(i=i+shift;i<width;){        //at displacemnt of i+shift begin to fill white
                for(k=0;k<bSize;k++,i++){
                    pSrc[ptr+i]=127;
                }
                i=i+bSize;                  //skip that block in the row
            }
            shift=0;i=0;
        }

        uint64_t totalTime = 0;

        pDst = (uint8_t *)fcvMemAlloc(dstWidth*dstHeight, 16);
        if(pDst)
        {
            runfastcvSimpleTest(loop,&totalTime,pSrc,width,height,width,pDst,dstWidth,dstHeight,dstWidth);

            // Write the result
            if (NULL != pOutputFile)
            {
                snprintf( tmpFullPath, 255, "%s/%s.pgm", pTestDir, pOutputFile);
                FILE *fpOutput = fopen( tmpFullPath, "wb" );
                if (fpOutput)
                {
                    writeOutput(fpOutput,pDst,dstWidth,dstHeight);
                    fclose(fpOutput);
                    fpOutput = NULL;
                }
            }
        }


        if(timer)
        {
            *timer = totalTime/loop;
        }
        FASTCV_TEST_PRINTF("Average time (us/frame) for fastcvSimpleTest %s: %lu", modeString, totalTime/loop);
        if (NULL != pOutputFile)
        {
            FASTCV_TEST_PRINTF("Output file can be found at %s\n", tmpFullPath);
        }

    }

    if (pSrc)
    {
        fcvMemFree(pSrc);
        pSrc = NULL;
    }
    if (pDst)
    {
        fcvMemFree(pDst);
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

int main(int argc, char* argv[])
{
   uint64_t cpu_timer = 0;
   uint64_t qdsp_timer = 0;
   uint64_t lowPower_timer = 0;
   uint64_t gpu_timer = 0;

   if( argc < 2 )
   {
      {   // runfastcvSimpleTest

        /*******
        args-
            <test-name>
            <output location>
            -o <output file name>
            -w <width of the output image>
            -h <height of the output image>
            -l <number of loops>
            -M <operation mode>
            -t <Extended target mode>
        *******/
         char *argv_cpu[12] = {(char *)"fastcv_simple_test",(char *)"/data",(char *)"-o",(char *)"fastcvSimpleTest_output_OpMode_3",
                             (char *)"-w",(char *)"640",(char *)"-h",(char *)"480",(char *)"-l",(char *)"100",(char *)"-M",(char *)"3"};
         char *argv_lowPower[12] = {(char *)"fastcv_simple_test",(char *)"/data",(char *)"-o",(char *)"fastcvSimpleTest_output_OpMode_0",
                             (char *)"-w",(char *)"640",(char *)"-h",(char *)"480",(char *)"-l",(char *)"100",(char *)"-M",(char *)"0"};
         char *argv_qdsp[12] = {(char *)"fastcv_simple_test",(char *)"/data",(char *)"-o",(char *)"fastcvSimpleTest_output_target_0",
                             (char *)"-w",(char *)"640",(char *)"-h",(char *)"480",(char *)"-l",(char *)"100",(char *)"-t",(char *)"0"};
         char *argv_gpu[12] = {(char *)"fastcv_simple_test",(char *)"/data",(char *)"-o",(char *)"fastcvSimpleTest_output_target_1",
                             (char *)"-w",(char *)"640",(char *)"-h",(char *)"480",(char *)"-l",(char *)"100",(char *)"-t",(char *)"1"};
         runCommandLine(12, argv_cpu, &cpu_timer);
         runCommandLine(12, argv_lowPower, &lowPower_timer);
         runCommandLine(12, argv_qdsp, &qdsp_timer);
         runCommandLine(12, argv_gpu, &gpu_timer);
      }
   }
   else
   {
      runCommandLine(argc, argv, NULL);
   }
   return 0;
}
