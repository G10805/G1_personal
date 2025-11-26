/*************************************************************************
Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/
#ifndef __FLACDEC_COMMON_H__
#define __FLACDEC_COMMON_H__

// Redefine for LA
#ifdef _ANDROID_
#include<utils/Log.h>
#define printf ALOGE
#define printf ALOGV
#define printf ALOGVV
#else
#define ALOGE printf
#define ALOGV printf
#define ALOGVV printf
#endif

#ifdef _MBCS
#define __func__ __FUNCTION__
#elif _ANDROID_

#endif


#endif /* __FLACDEC_COMMON_H__ */
