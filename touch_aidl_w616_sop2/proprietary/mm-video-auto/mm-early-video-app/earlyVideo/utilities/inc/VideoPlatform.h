/*
 **************************************************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#ifndef _VIDEOPLATFORM_H
#define _VIDEOPLATFORM_H

#include <VideoComDef.h>
#include <VideoStreamParserInterface.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"
#include "fcntl.h"
#include <string.h>

#include <linux/videodev2.h>
#include <errno.h>
#include <sys/mman.h>
#include <utils/Log.h>
#include <time.h>
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#include <linux/dma-buf.h>
#else
#include <linux/ion.h>
#endif
#include <sys/eventfd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <poll.h>
#include <linux/version.h>

#define ALIGN(x, align) (((x) + ((align)-1)) & ~((align)-1))

#ifndef _UBWC_
/* Source: msm_media_info.h */
#define COLOR_FMT_NV12_UBWC             4
#define COLOR_FMT_RGBA8888              6
#define COLOR_FMT_RGBA8888_UBWC         7

/* Source: videodev2.h */
/* UBWC 8-bit Y/CbCr 4:2:0  */
#define V4L2_PIX_FMT_NV12_UBWC                        \
                     v4l2_fourcc('Q', '1', '2', '8')

#endif

#define MSM_VIDC_PIC_STRUCT_PROGRESSIVE 0x1

#define platform_update_inputbfr_ptr(ppBuffer, vaddr)   (*(ppBuffer) = (uint8*)vaddr)
#define platform_write_inputbfr_ptr(vaddr, nFilledLen)
#define platform_write_update_outputbfr_ptr(pvaddr, bytesused)
#define platform_restore_outputbfr_ptr(ppBuffer)

#define MAX_FILENAME_LENGTH 128
#define MAX_NUM_OF_SESSION 256
#ifdef ANDROID_U_AND_ABOVE
#define LOCAL_STR_LEN 128
#else
#define LOCAL_STR_LEN 64
#endif
#define TAGDATA_ENABLE 1
#define READ4BYTECHUNK(size) size - 3

#ifdef __cplusplus
extern "C" {
#endif

typedef enum color_fmts colorFMTs;

#ifdef __ANDROID_T_AND_ABOVE__
#define DEC_INPUT_FILE_PATH             "/vendor_early_services/etc/"
#define DEC_OUTPUT_FILE_PATH            "/vendor_early_services/run/"
#define DEFAULT_SYSTEM_HEAP             "/dev/dma_heap/qcom,system"
#else
#define DEC_INPUT_FILE_PATH             "/early_services/etc/"
#define DEC_OUTPUT_FILE_PATH            "/early_services/run/"
#endif

#define INPUT_FILE_NAME                 "earlyvideo_input_dump.h264"

#define VID_DEC_DEVICE                  "/dev/video32"
#ifdef __ANDROID_T_AND_ABOVE__
#define EARLY_VID_DEC_DEVICE            "/dev/video32"
#else
#define EARLY_VID_DEC_DEVICE            "/early_services/dev/video32"
#endif

#define MAX_BUFFER                      10
#define MAX_COUNT                       2
#define MAX_ITER                        300

extern int DEFAULT_WIDTH;
extern int DEFAULT_HEIGHT; 
extern char* g_inputFilePtr;
extern u64 videoMapSize;

#define DEFAULT_NUM_CAPTURE_BUF         10
#define DEFAULT_NUM_OUTPUT_BUF          6

#define EARLY_VIDEO_DRI_CARD            4

#define MAX_BUF_SZ                      256
#define NUM_FRMS_PARSE                  90

//Around 5sec delay observed for display&video drivers to be ready, accordingly the below MACROS have been set
//used in wait_for_file() call
#define WAIT_TIME                       10
#define WAIT_COUNT                      500

#define MSMDRM_INIT_PATH                "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/init_complete"
/* Wait a maximum of 5 seconds for DRM Initialization */
#define MSMDRM_INIT_WAIT_TO_MS          5000

static inline void place_marker(char const *name)
{
#ifdef ANDROID_U_AND_ABOVE
        int fd = open("/sys/kernel/boot_kpi/kpi_values", O_WRONLY);

        if (fd > 0)
        {
                char earlyapp[MAX_BUF_SZ] = {0};
                strlcat(earlyapp, name, sizeof(earlyapp));
                write(fd, earlyapp, strlen(earlyapp));
                close(fd);
        }
#else
        ALOGE("boot_kpi: %s", name);
#endif
}

static inline void klog(char const *name)
{
        int fd = open("/dev/kmsg", O_WRONLY);

        if (fd > 0)
        {
                char earlyapp[MAX_BUF_SZ] = {0};
                strlcat(earlyapp, name, sizeof(earlyapp));
                write(fd, earlyapp, strlen(earlyapp));
                close(fd);
        }
}

static inline void print_error_msg(char const *msg, char const *err)
{
        static char marker[LOCAL_STR_LEN];

        memset(marker, 0, LOCAL_STR_LEN);

        snprintf(marker, LOCAL_STR_LEN ,"Earlyvideo: %s with %s ", msg, err);
        klog(marker);
        place_marker(msg);
}

typedef struct VideoStaticProperties
{
    char   sOutRoot[MAX_STR_LEN];
    char   sInputRoot[MAX_STR_LEN];
    char   sInputFileName[MAX_STR_LEN];
    uint32 logMask;
    bool   inputDump;
    bool   outputDump;
    uint32 nDisplayBufCnt;
    bool   bDynamicBufMode;
    uint32 nDefaultHeight;
    uint32 nDefaultWidth;
    uint32 nPollTimeout;   // In milli seconds
    uint32 nThreadTimeout; // In milli seconds
    BOOL bGenerateCrc;
    BOOL bSkipCrcCheck;
    uint32 nDecCapPortBufCnt;
    uint32 nDecOutPortBufCnt;
    uint32 nFwLog;
    uint32 nOffsetDump;
    uint32 bExtraDataDump;
    uint32 nCachedBufferMask;
} VideoStaticProperties;

extern VideoStaticProperties gVideoDecProp;
#define ADEBUG

#ifdef ADEBUG
#define DEBUGT_PRINT       ALOGV
#define DEBUGT_PRINT_INFO  ALOGI
#define DEBUGT_PRINT_ERROR ALOGE

#else
#define DEBUGT_PRINT       printf
#define DEBUGT_PRINT_INFO  printf
#define DEBUGT_PRINT_ERROR printf

#endif

#define MSG_FUNC_BEGIN(fmt, ...) MSG_DBG(fmt"...BEGIN", ##__VA_ARGS__)
#define MSG_FUNC_END(fmt, ...) MSG_DBG(fmt"...END", ##__VA_ARGS__)

#define STRCMP(str1,str2) ((str1 && str2)? strcmp(str1,str2) : 1)

#define VIDCTF_PRINT_INFO(fmt, ...) { \
                                           if(gVideoDecProp.logMask & 0x8) {                          \
                                              ALOGE("INFO  : %s : " fmt "\n",__FUNCTION__, ##__VA_ARGS__); \
                                        } \
                                    }

#define VIDCTF_PRINT_HIGH(fmt, ...) { \
                                           if(gVideoDecProp.logMask & 0x4) {                         \
                                              ALOGE("HIGH : %s : " fmt "\n",__FUNCTION__, ##__VA_ARGS__);  \
                                        } \
                                    }
#define VIDCTF_PRINT_WARN(fmt, ...) { \
                                           if(gVideoDecProp.logMask & 0x2) {                          \
                                              ALOGE("WARN : %s : " fmt "\n",__FUNCTION__, ##__VA_ARGS__);  \
                                        } \
                                    }
#define VIDCTF_PRINT_RESULT(fmt, ...) { \
                                           ALOGE(fmt,##__VA_ARGS__);                                   \
                                    }

#define VIDEO_BAILOUT_ON_FAILURE(Msg,Result,bailout) \
         if( 0 != Result )\
         {\
            VIDCTF_PRINT_WARN(Msg"-Failed with %d", Result);\
            goto bailout;\
         }\
         else{\
            VIDCTF_PRINT_INFO(Msg"-Success at %s\n", __FUNCTION__);\
         }

#define ENOTSUPP    524 /* Operation is not supported */

/* VIDC_ERROR would return unsupported status If rc is ENOTSUPP
   It would return unknowerror for any other non zero rc value
   It would return success if the rc value is 0*/
#define VIDC_ERROR(rc) ((rc)?((rc==ENOTSUPP)?VidcStatusUnSupported:VidcStatusUnknownError):VidcStatusSuccess)

#define MAX(a,b)  (((a)>(b)) ? (a) : (b))

#define VIDCTF_PRINT_ERROR(handle,errnum,fmt,...) { \
                                                        if(gVideoDecProp.logMask & 0x1) { \
                                                           ALOGE("ERROR: " #errnum " - " fmt,##__VA_ARGS__); \
                                                    } \
                                                  }
#define LOG_FUNC_START { \
                                if(gVideoDecProp.logMask & 0x8) {              \
                                    ALOGE("INFO: Logs: %s start\n",__FUNCTION__); \
                            } \
                        }
#define LOG_FUNC_END   { \
                                if(gVideoDecProp.logMask & 0x8) {            \
                                    ALOGE("INFO: Logs: %s end\n",__FUNCTION__); \
                            } \
                       }

typedef struct VideoSessionEnumMap
{
    uint32   eVal;
    char     sName[MAX_STR_LEN];
}VideoSessionEnumMap;


extern VideoSessionEnumMap g_errorMap[VIDC_ERR_MAX];

typedef enum{
    VideoSessionTypeUnknown,
    VideoSessionTypeDecode,
    VideoSessionTypeEncode,
    VideoSessionTypeTranscode, //Decode-Encode
    VideoSessionTypeVT, // Reverse Transcode > Encode-Decode
}VideoSessionType;

typedef enum
{                        //TODO: chinmays todo make xml config change
    VIDC_FRAME_FLAG_EOS0, /* EOS + Valid FrameData */
    VIDC_FRAME_FLAG_EOS1, /* ValidBuffer + 0 FrameData */
    VIDC_FRAME_FLAG_EOS2, /* NULL Buffer + 0 FrameData */
    VIDC_FRAME_FLAG_EOS3 /* NO EOS flag attached*/
}VideoSessionEosType;

typedef enum VidcClassificationType
{
    VIDC_CLASSIFICATION_THUMBNAIL = 1,
    VIDC_CLASSIFICATION_SMOOTH_STREAMING,
    VIDC_CLASSIFICATION_DECODE_ORDER,
    VIDC_CLASSIFICATION_FCN_DYN,
    VIDC_CLASSIFICATION_FCN_PB,
    VIDC_CLASSIFICATION_UNSUPPORTED,
    VIDC_CLASSIFICATION_PERF,
    VIDC_CLASSIFICATION_RANDOM_SEEK,
    VIDC_CLASSIFICATION_PIC_TYPE_DEC,
    VIDC_CLASSIFICATION_PIC_TYPE_DEC_RECONFIG,
    VIDC_CLASSIFICATION_RECONFIG,
    VIDC_CLASSIFICATION_SKYPE,
    VIDC_CLASSIFICATION_WFD,
    VIDC_CLASSIFICATION_MBI_SS,
    VIDC_CLASSIFICATION_MBI_T2T,
    VIDC_CLASSIFICATION_ERROR,
    VIDC_CLASSIFICATION_ROI,
    VIDC_CLASSIFICATION_API,
    VIDC_CLASSIFICATION_MBI_FRC,
    VIDC_CLASSIFICATION_ISDBTMM,
    VIDC_CLASSIFICATION_MAX
}VidcClassificationType;

typedef enum VidcCacheBufType
{
    VIDC_BUFTYPE_NONE = 0,
    VIDC_BUFTYPE_INPUT = 1,
    VIDC_BUFTYPE_OUTPUT = 2,
    VIDC_BUFTYPE_IN_EXTRADATA = 4,
    VIDC_BUFTYPE_OUT_EXTRADATA = 8,
}VidcCacheBufType;

typedef struct VideoSessionBeatStaticConfig{
    uint32  nFrameCnt;
    uint32  eCodecType;
    uint32  eColorFmt;  //Color format to be configured for capture port
    uint32  eConfiguredColorFmt;    //Color format in the XML file for decoder
    VidcClassificationType  eClassificationType;
    BOOL    bSecure;
    uint32  eBufferFormat;
    VideoSessionEosType  eEosType;
    VideoFrameRate nFrameRate;
    BOOL bDynamicBufMode;
    BOOL bThumbnailMode;
    uint32 nDisplayBufCnt;
    uint32 nSkipReadAfterNFrames;
}VideoSessionBeatStaticConfig;

#ifdef __cplusplus
}
#endif

#endif //end of _VIDEOPLATFORM_H
