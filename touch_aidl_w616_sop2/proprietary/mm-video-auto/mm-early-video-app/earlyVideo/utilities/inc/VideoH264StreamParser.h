/*
 **************************************************************************************************
 * Copyright (c) 2014-2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _VIDEOSTREAMPARSER_H
#define _VIDEOSTREAMPARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <VideoStreamParserInterface.h>
#include <stdlib.h>
#include <VideoComDef.h>
#include <list.h>
#include "VideoPlatform.h"

#define FORWARD 1
#define BACKWARD 0

/*  Four-character-code (FOURCC) */
#define v4l2_fourcc(a, b, c, d)\
((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))

typedef enum ParserFrameType{
  PARSER_FRAME_TYPE_SEQ_HEADER = 0x00,
  PARSER_FRAME_TYPE_SYNC = 0x01,
  PARSER_FRAME_TYPE_NON_SYNC = 0x02,
  PARSER_FRAME_TYPE_VC1_RCV_STRCUT_C = 0x03
}ParserFrameType;

/* use the same string as exposed through xml for kernel and omx test-app */
static ParseCfgEnum pCodecTypeMap[] =
{
    { "VIDEO_CodingAVC",   V4L2_PIX_FMT_H264 },
    { 0, 0 }
};

typedef struct FileOffset
{
    ParserFrameType eFrameType;
    u64 nOffset;     //Buffer data read start offset
    u64 nLength;     //The length of the data in the buffer
    uint32 isLastFrame;
}FileOffset;

typedef struct FileOffsetMap
{
    uint32     nFrameNum;
    FileOffset val;
    LIST_NODE list;
}FileOffsetMap;

//Structure for saving buffer filled length for skipping mode in decoders
typedef struct LoadedBufferInfo
{
    uint8*    pbuffer;
    u64    filledLength;
    LIST_NODE list;
}LoadedBufferInfo;

typedef signed char        Int8;
typedef unsigned char      UInt8;
typedef signed short int   Int16;
typedef unsigned short     UInt16;
typedef signed long        Int32;
typedef unsigned long      UInt32;
typedef long long          Int64;
typedef unsigned long long UInt64;    /** Unsigned 64 bit type */

#ifdef __cplusplus
}
#endif

#endif // #ifndef _VIDEOSTREAMPARSER_H
