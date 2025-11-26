#ifndef _FASTCVNSPSIMPLETEST_H
#define _FASTCVNSPSIMPLETEST_H
/**=============================================================================
 
@file
   fastcv_NSP_sample.idl
 
@brief
   IDL file for fastcv_NSP_sample.
 
Copyright (c) 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
 
=============================================================================**/
#include <AEEStdDef.h>
#include <remote.h>


#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE
#ifdef __cplusplus
extern "C" {
#endif
#if !defined(__QAIC_DMAHANDLE1_OBJECT_DEFINED__) && !defined(__DMAHANDLE1_OBJECT__)
#define __QAIC_DMAHANDLE1_OBJECT_DEFINED__
#define __DMAHANDLE1_OBJECT__
typedef struct _dmahandle1_s {
   int fd;
   uint32 offset;
   uint32 len;
} _dmahandle1_t;
#endif /* __QAIC_DMAHANDLE1_OBJECT_DEFINED__ */
/**
    * Opens the handle in the specified domain.  If this is the first
    * handle, this creates the session.  Typically this means opening
    * the device, aka open("/dev/adsprpc-smd"), then calling ioctl
    * device APIs to create a PD on the DSP to execute our code in,
    * then asking that PD to dlopen the .so and dlsym the skel function.
    *
    * @param uri, <interface>_URI"&_dom=aDSP"
    *    <interface>_URI is a QAIC generated uri, or
    *    "file:///<sofilename>?<interface>_skel_handle_invoke&_modver=1.0"
    *    If the _dom parameter is not present, _dom=DEFAULT is assumed
    *    but not forwarded.
    *    Reserved uri keys:
    *      [0]: first unamed argument is the skel invoke function
    *      _dom: execution domain name, _dom=mDSP/aDSP/DEFAULT
    *      _modver: module version, _modver=1.0
    *      _*: any other key name starting with an _ is reserved
    *    Unknown uri keys/values are forwarded as is.
    * @param h, resulting handle
    * @retval, 0 on success
    */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(fastcvNSPSimpleTest_open)(const char* uri, remote_handle64* h) __QAIC_HEADER_ATTRIBUTE;
/** 
    * Closes a handle.  If this is the last handle to close, the session
    * is closed as well, releasing all the allocated resources.

    * @param h, the handle to close
    * @retval, 0 on success, should always succeed
    */
__QAIC_HEADER_EXPORT int __QAIC_HEADER(fastcvNSPSimpleTest_close)(remote_handle64 h) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(fastcvNSPSimpleTest_startTimerQ)(remote_handle64 _h) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(fastcvNSPSimpleTest_endTimerQ)(remote_handle64 _h, uint64_t* res0, uint64_t* res1, uint64_t* res2) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(fastcvNSPSimpleTest_ExecQ)(remote_handle64 _h, int32_t fdSrc, uint32 srcWidth, uint32 srcHeight, uint32 srcStride, int32_t fdDst, uint32 dstWidth, uint32 dstHeight, uint32 dstStride, int32 fdScratchbuf, int32 nLoops) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(fastcvNSPSimpleTest_memMap)(remote_handle64 _h, int fd, uint32 fdOffset, uint32 fdLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(fastcvNSPSimpleTest_memUnmap)(remote_handle64 _h, int32_t bufId, uint32_t bufSize) __QAIC_HEADER_ATTRIBUTE;
#ifndef fastcvNSPSimpleTest_URI
#define fastcvNSPSimpleTest_URI "file:///libfastcvNSPSimpleTest_skel.so?fastcvNSPSimpleTest_skel_handle_invoke&_modver=1.0"
#endif /*fastcvNSPSimpleTest_URI*/
#ifdef __cplusplus
}
#endif
#endif //_FASTCVNSPSIMPLETEST_H
