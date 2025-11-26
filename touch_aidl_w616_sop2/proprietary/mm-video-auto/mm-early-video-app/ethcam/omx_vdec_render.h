/*--------------------------------------------------------------------------
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2010 - 2013, 2016 - 2017, The Linux Foundation. All rights reserved.
 * Copyright (c) 2011 Benjamin Franzke
 * Copyright (c) 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------*/

#ifndef __OMX_VDEC_RENDER_H__
#define __OMX_VDEC_RENDER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <media/msm_media_info.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "MMTime.h"
#include "MMTimer.h"

#include "OMX_Core.h"
#include "OMX_Component.h"
#include "OMX_QCOMExtns.h"

extern "C" {
#include "queue.h"
}

#ifdef READ_FROM_SOCKET
#include "RTPDataSource.h"
#include "filesourcetypes.h"
#include <arpa/inet.h>
#include "SourcePort.h"
#include "parserdatadef.h"
#include "RTPParser.h"
#include "filesource.h"
#endif

#ifdef USE_ION
#include <linux/msm_ion.h>
#endif

/************************************************************************/
/*              #DEFINES                            */
/************************************************************************/
#define DELAY 66
#define H264_START_CODE 0x00000001
#define VOP_START_CODE 0x000001B6
#define SHORT_HEADER_START_CODE 0x00008000
#define MPEG2_FRAME_START_CODE 0x00000100
#define MPEG2_SEQ_START_CODE 0x000001B3
#define VC1_START_CODE  0x00000100
#define VC1_FRAME_START_CODE  0x0000010D
#define VC1_FRAME_FIELD_CODE  0x0000010C
#define VC1_SEQUENCE_START_CODE 0x0000010F
#define VC1_ENTRY_POINT_START_CODE 0x0000010E
#define NUMBER_OF_ARBITRARYBYTES_READ  (4 * 1024)
#define VC1_SEQ_LAYER_SIZE_WITHOUT_STRUCTC 32
#define VC1_SEQ_LAYER_SIZE_V1_WITHOUT_STRUCTC 16

#define CONFIG_VERSION_SIZE(param) \
    param.nVersion.nVersion = CURRENT_OMX_SPEC_VERSION;\
    param.nSize = sizeof(param);

#define FAILED(result) (result != OMX_ErrorNone)

#define SUCCEEDED(result) (result == OMX_ErrorNone)
#define SWAPBYTES(ptrA, ptrB) { char t = *ptrA; *ptrA = *ptrB; *ptrB = t;}
#define SIZE_NAL_FIELD_MAX  4
#define MDP_DEINTERLACE 0x80000000

#ifdef _MSM8974_
#define PMEM_DEVICE "/dev/ion"
#elif MAX_RES_720P
#define PMEM_DEVICE "/dev/pmem_adsp"
#elif MAX_RES_1080P_EBI
#define PMEM_DEVICE "/dev/pmem_adsp"
#elif MAX_RES_1080P
#define PMEM_DEVICE "/dev/pmem_smipool"
#endif

#ifdef USE_OUTPUT_BUFFER
#define USE_EXTERN_PMEM_BUF
#undef USE_EGL_IMAGE_TEST_APP
#endif

#define DISABLE_AFTER_BOOT
#ifdef NATIVEWINDOW_DISPLAY
#undef DISABLE_AFTER_BOOT
#endif

/************************************************************************/
/*              GLOBAL DECLARATIONS                     */
/************************************************************************/
#ifdef _ANDROID_
#include "omx_vdec_render_android.h"
#define EARLYAPP earlyvideo
#define LOG_TAG "EARLY-VIDEO"
#else
#include "omx_vdec_render_linux.h"
#define EARLYAPP ethcam
#define LOG_TAG "ETHERNET-RVC"
#endif

#define rstr(x) Str(x)
#define Str(x) #x

enum use_protocol
{
    USE_GBM_PROTOCOL = 0,
    USE_DMA_PROTOCOL = 1,
};

enum render
{
    RENDER_NOTHING = 0,
    RENDER_DRM     = 1,
    RENDER_WESTON  = 2,
};

extern enum use_protocol protocol;

extern enum render enable_display;

extern int height;
extern int width;
extern int opendisplay;
extern bool isdisplayopened;
extern int displayYuv;
extern bool bRtpReceiverStopped;
extern int bInputEosReached;
extern int bOutputEosReached;

extern OMX_QCOM_PLATFORM_PRIVATE_LIST      *pPlatformList;
extern OMX_QCOM_PLATFORM_PRIVATE_ENTRY     *pPlatformEntry;
extern OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *pPMEMInfo;
extern unsigned char* use_buf_virt_addr[32];

#ifdef USE_ION
extern bool first_start;
extern int gpio_value;
#endif

void init();
void deInit();

void init_queue();
void clear_queue();
void clear_all_queue();
bool is_empty_queue();
struct queue_buffer* pop_queue();
void push_queue(struct queue_buffer *queue_buffer);
int queue_length();

int open_display();
void close_display();

void display_marker();
int display_render(struct OMX_BUFFERHEADERTYPE *pBuffer, int frameWidth, int frameHeight, uint32_t end_playback);
void free_ion(int index);
void free_outport_ion();
void place_marker(char const *name);
int clip2(int x);

#endif
