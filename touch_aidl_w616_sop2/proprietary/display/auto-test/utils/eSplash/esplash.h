/*--------------------------------------------------------------------------
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2010 - 2013, 2016 - 2017,  The Linux Foundation. All rights reserved.
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


#ifndef __ESPLASH_H__
#define __ESPLASH_H__


#include <inttypes.h>
#include <cutils/properties.h>

extern "C"{
#include<utils/Log.h>
}

#include <cutils/memory.h>
#include <unistd.h>
#include <utils/Log.h>
#include <errno.h>
#include <logo.h>

#define ADEBUG

#ifdef ADEBUG
#define DEBUGT_PRINT ALOGV
#define DEBUGT_PRINT_INFO ALOGI
#define DEBUGT_PRINT_ERROR ALOGE
#else
#define DEBUGT_PRINT printf
#define DEBUGT_PRINT_INFO printf
#define DEBUGT_PRINT_ERROR printf

#endif

#define MAX_BUFFER 30
#define MAX_COUNT 10
#define ERR_NO -1
#endif
