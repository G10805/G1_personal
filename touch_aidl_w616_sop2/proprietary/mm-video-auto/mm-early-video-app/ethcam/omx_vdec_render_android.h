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


#ifndef __OMX_VDEC_RENDER_ANDROID_H__
#define __OMX_VDEC_RENDER_ANDROID_H__

#include "omx_vdec_render.h"

#include <inttypes.h>
#include <cutils/properties.h>
#include <binder/MemoryHeapBase.h>

using namespace android;
extern "C"{
#include<utils/Log.h>
}

#include <cutils/memory.h>
#include <unistd.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <media/stagefright/foundation/ADebug.h>

#ifdef NATIVEWINDOW_DISPLAY
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferMapper.h>
#include <android/native_window.h>
#include <system/window.h>
#endif

#include "OMX_QCOMExtns.h"

#define DEBUGT_PRINT ALOGV
#define DEBUGT_PRINT_INFO ALOGI
#define DEBUGT_PRINT_ERROR ALOGE

//#define __DEBUG_DIVX__ // Define this macro to print (through logcat)
                         // the kind of frames packed per buffer and
                         // timestamps adjustments for divx.

//#define TEST_TS_FROM_SEI // Define this macro to calculate the timestamps
                           // from the SEI and VUI data for H264

#ifdef READ_FROM_SOCKET
#include "RTPDataSource.h"
#include <arpa/inet.h>
#include "SourcePort.h"
#include "RTPParser.h"
#endif

#include "filesource.h"
#include "filesourcetypes.h"
#include "parserdatadef.h"

#define ALLOCATE_BUFFER 0

extern int height;
extern int width;
extern int displayYuv;
extern int opendisplay;
extern bool isdisplayopened;
extern Queue *queue_buffer_list;

struct queue_buffer {
  uint8 *buffer;
  int len;
};


int render_nativeWindow(struct OMX_BUFFERHEADERTYPE *pBufHdr);
int nativeWindow_buf_render(struct OMX_BUFFERHEADERTYPE *pBufHdr);

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
void* open_display_thread(void* pArg);
void* open_edrmdisplay_thread(void* pArg);

void display_marker();

#endif
