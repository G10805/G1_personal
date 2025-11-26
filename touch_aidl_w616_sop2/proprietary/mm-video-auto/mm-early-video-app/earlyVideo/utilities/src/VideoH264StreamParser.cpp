/*
 **************************************************************************************************
 * Copyright (c) 2014-2020, 2023-2024 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <dlfcn.h>
#include <unistd.h>
#include "VideoH264StreamParser.h"
#include <limits.h>
#ifdef __ANDROID__
#include "utils/Log.h"
#endif // __ANDROID__

int debug_level = 0x7;

#define H264_NAL_UNIT_TYPE_MASK 0x1F
#define H264_NAL_UNIT_TYPE_SEI 0x06
#define H264_NAL_UNIT_TYPE_SPS 0x07
#define H264_NAL_UNIT_TYPE_PPS 0x08
#define H264_NAL_UNIT_TYPE_AUD 0x09
#define H264_NAL_UNIT_TYPE_IDR 0x05
#define H264_NAL_UNIT_TYPE_NONIDR 0x01

#define H264_NAL_UNIT_TYPE_OFFSET 5
#define NAL_UNIT_START_CODE 0x00000001
#define NAL_START_CODE_MASK 0x00FFFFFF
#define H264_FIRST_MB_IN_SLICE_MASK 0x80
#define H264_START_CODE_LENGTH 4
#define PRINT_FRMS_MULT_OF 1

#undef LOG_TAG
#define LOG_TAG "VIDEO_STREAM_PARSER"

enum {
   PARSER_ERROR = 0x1,
   PARSER_WARN = 0x1,
   PARSER_INFO = 0x2,
   PARSER_HIGH = 0x2,
   PARSER_MEDIUM = 0x2,
   PARSER_LOW = 0x4,
};

#ifdef __ANDROID__
#define VIDEO_STREAMPARSER_WARN(fmt, ...) ({ \
      if (debug_level & PARSER_WARN) \
          ALOGE("%s::%d " fmt, __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
      })
#define VIDEO_STREAMPARSER_ERROR(fmt, ...) ({ \
      if (debug_level & PARSER_ERROR) \
          ALOGE("%s::%d " fmt, __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
      })
#define VIDEO_STREAMPARSER_HIGH(fmt, ...) ({ \
      if (debug_level & PARSER_HIGH) \
          ALOGE("%s::%d " fmt, __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
      })
#define VIDEO_STREAMPARSER_MEDIUM(fmt, ...) ({ \
      if (debug_level & PARSER_MEDIUM) \
          ALOGE("%s::%d " fmt, __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
      })
#define VIDEO_STREAMPARSER_LOW(fmt, ...) ({ \
      if (debug_level & PARSER_LOW) \
          ALOGE("%s::%d " fmt, __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
      })
#define VIDEO_STREAMPARSER_INFO(fmt, ...) ({ \
      if (debug_level & PARSER_INFO) \
          ALOGE("%s::%d " fmt, __FUNCTION__, __LINE__,  ##__VA_ARGS__); \
      })
#else
#define VIDEO_STREAMPARSER_WARN printf
#define VIDEO_STREAMPARSER_ERROR printf
#define VIDEO_STREAMPARSER_HIGH printf
#define VIDEO_STREAMPARSER_MEDIUM printf
#define VIDEO_STREAMPARSER_LOW printf
#define VIDEO_STREAMPARSER_INFO printf
#endif

#define STORE_AND_UPDATE_CODE(pBuffer,nReadOffset,nCode)    (nCode) <<= 8; \
                                                            (nCode) |= (*pBuffer++ & 0xFF ); \
                                                            (nReadOffset)++;
#define MAX(a,b)  (((a)>(b)) ? (a) : (b))

struct File {
#ifdef _USE_FILE_DESCRIPTOR_
    int fd;
    int eof;
#else
    FILE *file;
#endif
};

File *open_file(const char *path, int write) {
    File *f;
    f = (File *)malloc(sizeof(*f));
    if (!f)
        return NULL;
#ifdef _USE_FILE_DESCRIPTOR_
    f->fd = open(path, write ? O_WRONLY : O_RDONLY);
    f->eof = 0;
    if (f->fd == -1) {
        free(f);
        return NULL;
    }
#else
    f->file = FOPEN(path, write ? "w+b" : "rb");
#endif
    return f;
}

void close_file(File *f) {
    if (!f)
        return;
#ifdef _USE_FILE_DESCRIPTOR_
    close(f->fd);
#else
    FCLOSE(f->file);
#endif
    free(f);
}

void seek_file(File *f, u64 pos, int whence) {
#ifdef _USE_FILE_DESCRIPTOR_
    f->eof = 0;
    lseek64(f->fd, pos, whence);
#else
    FSEEK(f->file, pos, whence);
#endif
}

int read_file(void *buf, size_t size, size_t count, File *f) {
#ifdef _USE_FILE_DESCRIPTOR_
    if (size <= 0 || count <= 0)
        return 0;
    int ret = read(f->fd, buf, size * count);
    if (ret < (size * count)) {
        f->eof = 1;
    }
    return ret / size;
#else
    return fread(buf, size, count, f->file);
#endif
}

int eof_file(File *f) {
#ifdef _USE_FILE_DESCRIPTOR_
    return f->eof;
#else
    return feof(f->file);
#endif
}

u64 curr_offset_file(File *f) {
#ifdef _USE_FILE_DESCRIPTOR_
    return lseek64(f->fd, 0, SEEK_CUR);
#else
    return FTELL(f->file);
#endif
}

void VideoStreamParserClose(VideoStreamParser* pParser);

/*
*  Does one time configuration. e.g open file, assign function pointers to parser based on codec types.
*  pConfig: incoming parameter which contains test config level information
*/
VidcStatus VideoStreamParserConfigure(VideoStreamParser* pParser,
    VideoSessionStaticConfig *pConfig, string sInRoot, BOOL inputDump,
    string sOutRoot, string sInFileName, string sTestID, BOOL bSecure);

/*
* Reads next frame from the file and fills the buffer
* pFrameInfo: Some of the information in this structure has incoming and some of them are outgoing
* FrameNo: Reporesents which frameno to jump to.
*          0 then read next frame
*          1 then read 1st frame
*          n then read nth frame
*/
VidcStatus VideoStreamParserGetNextFrame(VideoStreamParser* pParser, ParserFrameInfoType *pFrameInfo);

VidcStatus ReadBufferFromH264File(VideoStreamParser* pParser, ParserFrameInfoType *pFrameInfo);
VidcStatus VideoStreamParserBuildOffsetTable(VideoStreamParser* pParser);
static uint32 ParseStringEnum(ParseCfgEnum *pConfigEnum, const char* pEnumName);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

inline uint32 get_bit(const uint8 * const base, uint32 offset)
{
    return ((*(base + (offset >> 0x3))) >> (0x7 - (offset & 0x7))) & 0x1;
}

// This function implement decoding of exp-Golomb codes of zero range (used in H.264).

uint32 DecodeUGolomb(const uint8 * const base, uint32 * const offset)
{
    uint32 zeros = 0;
    int i;

    // calculate zero bits. Will be optimized.
    while (0 == get_bit(base, (*offset)++))
        zeros++;

    // insert first 1 bit
    uint32 info = 1 << zeros;

    for (i = zeros; i > 0; i--)
    {
        info |= get_bit(base, (*offset)++) << (i - 1);
    }

    return (info - 1);
}

VidcStatus VideoStreamParserInit(VideoStreamParser* pParser)
{
    VidcStatus status = VidcStatusSuccess;
    if (!pParser)
    {
        return VidcStatusBadParamError;
    }

    debug_level = pParser->m_nlogMask;
    pParser->m_nStartCodeLen = H264_START_CODE_LENGTH;
    pParser->m_nTimeStamp = 0;
    pParser->m_nTimeStampDelta = 33333;
    pParser->m_bAutoRestart = FALSE;
    pParser->m_nFrames = 0;
    pParser->m_nFrameHeight = 0;
    pParser->m_nFrameWidth = 0;
    pParser->m_fReadBuffer = NULL;
    pParser->m_bSecureSession = FALSE;
    pParser->m_eCodecFormat = V4L2_PIX_FMT_H264;
    pParser->m_bEosReached = TRUE;
    pParser->m_bRCVFullSeqLayer = FALSE;
    pParser->m_bRCVIsV2Format = FALSE;
    pParser->m_eIVFType = 0;
    pParser->m_nTotalInputFrames = 0;
    pParser->m_pYuvScratch = NULL;
    pParser->m_pIntBuf = NULL;

    pParser->m_nFrameRate.nFrameNum = 30;
    pParser->m_nFrameRate.nFrameDen = 1;
    list_init(&pParser->m_FrameOffsetTable);

    pParser->m_bitsDump = NULL;
    pParser->m_InputBitsOffsetFile = NULL;
    pParser->m_bReadDirection = FORWARD;

    pParser->m_nSkipReadAfterNFrames = 0;
    pParser->m_nSuperFrame = 1;
    pParser->m_nReverseReadingBasis = 0;
    list_init(&pParser->m_LoadedInputBufferList);

    pParser->Close     = VideoStreamParserClose;
    pParser->Configure = VideoStreamParserConfigure;
    pParser->GetNextFrame = VideoStreamParserGetNextFrame;
    pParser->BuildOffsetTable = VideoStreamParserBuildOffsetTable;
    return status;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void VideoStreamParserClose(VideoStreamParser* pParser)
{
    FileOffsetMap *pEntry;
    LoadedBufferInfo *pBufferEntry;
    if (!pParser) return;
    close_file(pParser->m_InputFile);
    FCLOSE(pParser->m_bitsDump);
    FCLOSE(pParser->m_InputBitsOffsetFile);
    FCLOSE(pParser->m_InputOffsetFile);
    list_clear (FileOffsetMap, pEntry, &pParser->m_FrameOffsetTable, list);
    FREE(pParser->m_pYuvScratch);
    FREE(pParser->m_pIntBuf);
}

VidcStatus GetAllSeqHdrBeforeSeekPoint(VideoStreamParser* pParser, ParserFrameInfoType *pFrameInfo)
{
    FileOffsetMap *entry = NULL;
    uint32 nFirstEntry = 0;
    LIST_NODE *pMap = &pParser->m_FrameOffsetTable;
    VidcStatus status = VidcStatusUnknownError;

    utils_list_for_each_entry(FileOffsetMap,entry,pMap,list)
    {
        if (entry->nFrameNum > pFrameInfo->nFrameCounter)
        {
            break;
        }
        if (entry->nFrameNum < pFrameInfo->nFrameCounter &&
            entry->val.eFrameType == PARSER_FRAME_TYPE_SEQ_HEADER )
        {
            status = VidcStatusSuccess;
            memcpy(pFrameInfo->pBuffer + pFrameInfo->nFilledLen, g_inputFilePtr + entry->val.nOffset, entry->val.nLength);
            pFrameInfo->nFilledLen += entry->val.nLength;
            VIDEO_STREAMPARSER_HIGH("Frame Num= %d frame_size=0x%llx pBuffer=%p fileoffset:0x%llx nal_type:%d\n",
               entry->nFrameNum, entry->val.nLength, pFrameInfo->pBuffer, entry->val.nOffset, entry->val.eFrameType);
        }
    }
    return status;
}
/* Get Sequence header Offset/Length at a specified Frame Number from FrameOffset Table*/
VidcStatus GetAllSeqHdrAtFrame(VideoStreamParser* pParser, ParserFrameInfoType *pFrameInfo, FileOffset* pOffset, BOOL isFirstFrame)
{
    FileOffsetMap *entry = NULL;
    uint32 nFirstEntry = 0;
    LIST_NODE *pMap = &pParser->m_FrameOffsetTable;
    VidcStatus status = VidcStatusUnknownError;
    uint32 finalOffset=0;

    //For S only mode, frame number starts from 0. However first node in offset table starts from 1.
    uint32 frameCounter = ((isFirstFrame == TRUE) ? (pFrameInfo->nFrameCounter + 1) : pFrameInfo->nFrameCounter);
    if (!pOffset)
        return VidcStatusUnknownError;

    memset(pOffset, 0x00, sizeof(FileOffset));

    pOffset->nOffset = (uint32)-1;
    utils_list_for_each_entry(FileOffsetMap,entry,pMap,list)
    {
        if (entry->nFrameNum > frameCounter)
        {
            break;
        }
        if (entry->nFrameNum == frameCounter &&
            ((entry->val.eFrameType == PARSER_FRAME_TYPE_SEQ_HEADER) ||
             (entry->val.eFrameType == PARSER_FRAME_TYPE_VC1_RCV_STRCUT_C)))
        {
            status = VidcStatusSuccess;
            if (pOffset->nOffset == (uint32)-1)
            {
                pOffset->nOffset = entry->val.nOffset;
            }
            pOffset->nLength += entry->val.nLength;
            pOffset->isLastFrame = entry->val.isLastFrame;
            finalOffset = entry->val.nOffset + entry->val.nLength;
            VIDEO_STREAMPARSER_INFO("Frame Num= %d frame_size=0x%llx pBuffer=%p fileoffset:0x%llx nal_type:%d\n",
                entry->nFrameNum, entry->val.nLength, pFrameInfo->pBuffer, entry->val.nOffset, entry->val.eFrameType);
        }
    }
    pOffset->nLength = finalOffset - pOffset->nOffset;

    return status;
}

VidcStatus GetFullFrameOffsetFromMap(LIST_NODE *pMap, uint32* nFrameNum, FileOffset* pOffset, uint32 isSyncRequest, uint32 nCombineNFrames)
{
    FileOffsetMap *entry = NULL;
    uint32 nFirstEntry = 0;
    VidcStatus status = VidcStatusUnknownError;

    if (!pOffset)
        return VidcStatusUnknownError;

    if (isSyncRequest)
    {
        status = VidcStatusNoIDRFound;
        utils_list_for_each_entry(FileOffsetMap,entry,pMap,list)
        {
            if (entry->nFrameNum >= *nFrameNum && entry->val.eFrameType == PARSER_FRAME_TYPE_SYNC)
            {
                *nFrameNum = entry->nFrameNum;
                *pOffset = entry->val;
                status = VidcStatusSuccess;
                break;
            }
        }
    }
    else
    {
        status = VidcStatusMaxSeekError;
        memset(pOffset, 0x00, sizeof(FileOffset));
        utils_list_for_each_entry(FileOffsetMap,entry,pMap,list)
        {
            if (entry->nFrameNum > (*nFrameNum + nCombineNFrames - 1))
            {
               break;
            }
            if (entry->nFrameNum >= *nFrameNum && entry->nFrameNum < (*nFrameNum + nCombineNFrames) &&
               (entry->val.eFrameType == PARSER_FRAME_TYPE_SYNC ||
                entry->val.eFrameType == PARSER_FRAME_TYPE_NON_SYNC ||
                entry->val.eFrameType ==  PARSER_FRAME_TYPE_VC1_RCV_STRCUT_C))
            {
                if (entry->nFrameNum == *nFrameNum)
                {
                    *pOffset = entry->val;
                    status = VidcStatusSuccess;
                }
                else
                {
                    pOffset->nLength += entry->val.nLength;
                    pOffset->isLastFrame = entry->val.isLastFrame;
                    status = VidcStatusSuccess;
                }
            }
        }
    }
    return status;
}

VidcStatus VideoStreamParserBuildOffsetTable(VideoStreamParser* pParser)
{
    LOG_FUNC_START;
    VidcStatus status = VidcStatusSuccess;
    ParserFrameInfoType frameInfo;
    ParserFrameInfoType *pFrameInfo = &frameInfo;
    FileOffsetMap *last_entry = NULL;
    uint32 nMaxBitStreamBufSize = 3 * 8192 * 4320;

    memset(pFrameInfo, 0x00, sizeof(ParserFrameInfoType));
    pFrameInfo->pBuffer = (uint8 *)malloc(nMaxBitStreamBufSize);
    if (NULL == pFrameInfo->pBuffer)
    {
        VIDEO_STREAMPARSER_WARN("NULL buffer pointer, frame.no:%d", pFrameInfo->nFrameCounter);
        return VidcStatusAllocError;
    }
    VIDEO_STREAMPARSER_INFO("Indexing the clip:");
    pFrameInfo->bLastFrame = FALSE;
    pFrameInfo->nAllocLen = nMaxBitStreamBufSize;

    if (pParser->m_InputOffsetFile && pParser->nOffsetDump == 2)
    {
        //TODO:Need to update last entry EOS flag true
        return status;
    }
    while (!pFrameInfo->bLastFrame)
    {
        pFrameInfo->nFrameCounter++;
        pFrameInfo->nFilledLen = 0;

        status = (pParser->m_fReadBuffer)(pParser, pFrameInfo);
        if (status){
            VIDEO_STREAMPARSER_WARN("m_fReadBuffer failed");
            goto bailout;
        }
        if (pFrameInfo->nFilledLen >= pFrameInfo->nAllocLen)
        {
            VIDEO_STREAMPARSER_WARN("Buffer Size not Sufficient");
            status = VidcStatusBufferOverFlowError;
            goto bailout;
        }

        if (pParser->m_InputOffsetFile)
        {
            last_entry = list_get_node_data(list_get_tail(&pParser->m_FrameOffsetTable), FileOffsetMap, list);
            fprintf(pParser->m_InputOffsetFile, "%llx,%llu,%d\n", last_entry->val.nOffset, last_entry->val.nLength, last_entry->val.eFrameType);
        }

        if (pFrameInfo->nFrameCounter == pParser->m_nFrames)
        {
            pFrameInfo->bLastFrame = TRUE;
        }

        if (pFrameInfo->bLastFrame == TRUE)
        {

            list_for_each_entry_reverse(FileOffsetMap,last_entry,&pParser->m_FrameOffsetTable,list)
            {
                last_entry->val.isLastFrame = TRUE;
                if (last_entry->val.eFrameType == PARSER_FRAME_TYPE_SYNC || last_entry->val.eFrameType == PARSER_FRAME_TYPE_NON_SYNC)
                {
                    break;
                }
                VIDEO_STREAMPARSER_WARN("Last Entry with non data frame\nFrame Num= %d frame_size=0x%llx fileoffset:0x%llx status:0x%x\n",
                    last_entry->nFrameNum, last_entry->val.nLength, last_entry->val.nOffset, last_entry->val.eFrameType);
            }
        }
        pParser->m_bEosReached = pFrameInfo->bLastFrame;

        if ((pFrameInfo->nFrameCounter % PRINT_FRMS_MULT_OF) == 0)
        {
            VIDEO_STREAMPARSER_HIGH("Frame Num= %d file_offset=0x%x frame_size=0x%x pBuffer=%p fileoffset:0x%x status:0x%x\n",
            pFrameInfo->nFrameCounter,
            pFrameInfo->nOffset,
            pFrameInfo->nFilledLen, pFrameInfo->pBuffer, (pParser->m_nCurFilePos - pFrameInfo->nFilledLen), status);
        }
        pParser->m_nMaxInputFrameSize = MAX(pParser->m_nMaxInputFrameSize, pFrameInfo->nFilledLen);
        { printf("#"); fflush(stdout); }
    }
    { printf("\n"); fflush(stdout); }

    pParser->m_bEosReached = FALSE;
    pParser->m_nTotalInputFrames = pFrameInfo->nFrameCounter;
    VIDEO_STREAMPARSER_HIGH("Total Input Frames: %d\n", pParser->m_nTotalInputFrames);
bailout:
    FREE(pFrameInfo->pBuffer);
    LOG_FUNC_END;
    return status;
}

VidcStatus VideoStreamParserConfigure(
    VideoStreamParser* pParser,
    VideoSessionStaticConfig *pConfig,
    string sInRoot,
    BOOL inputDump,
    string sOutRoot,
    string sInFileName,
    string sTestID,
    BOOL bSecure)
{
    LOG_FUNC_START;
    VidcStatus result = VidcStatusSuccess;
    char fileName[MAX_STR_LEN];

    if (!pParser) return VidcStatusBadParamError;
    pParser->m_InputOffsetFile = NULL; //not using any input file anymore

    pParser->m_eCodecFormat = ParseStringEnum( pCodecTypeMap, pConfig->sCodecType);
    pParser->m_eBufferFormat = ParseStringEnum( pYUVFormatMap, pConfig->sBufferFormat);
    pParser->m_nSuperFrame = pConfig->nSuperFrame;
    pParser->m_nMaxInputFrameSize = 0;
    pParser->m_bIsDivxPacked = FALSE;
    pParser->m_nPrefixHeaderMode = V4L2_DEC_S_ONLY;

    pParser->m_nFrames = pConfig->nFrameCnt;
    pParser->m_nSkipReadAfterNFrames = pConfig->nSkipReadAfterNFrames;

    pParser->m_nFileSize = videoMapSize;
    pParser->m_nCurFilePos = 0;

    if (pParser->m_eCodecFormat == V4L2_PIX_FMT_H264)
    {
        pParser->m_fReadBuffer = &ReadBufferFromH264File;
    }
    else
    {
        VIDEO_STREAMPARSER_HIGH("Unsupported type Codec type:%s BufferForamt %s",
                                                             pConfig->sCodecType,
                                                          pConfig->sBufferFormat);
        return VidcStatusXmlCfgError;
    }
    LOG_FUNC_END;
    return result;
}

//Save the loaded input buffers' filled length for skipping mode in decoders
VidcStatus SaveLoadedBufferFilledLength(LIST_NODE *pMap, ParserFrameInfoType *pFrameInfo)
{
    LoadedBufferInfo *entry = NULL;
    BOOL found = FALSE;
    utils_list_for_each_entry(LoadedBufferInfo, entry, pMap, list)
    {
        if(entry->pbuffer == pFrameInfo->pBuffer)
        {
            found = TRUE;
            break;
        }
    }

    if(found)
    {
        //save updated filled length
        entry->filledLength = pFrameInfo->nFilledLen;
        VIDEO_STREAMPARSER_WARN("Updated loaded buffer %p filled length = 0x%llx", pFrameInfo->pBuffer, entry->filledLength);
    }
    else
    {
        VIDEO_STREAMPARSER_WARN("Found a new loaded buffer %p filled length = 0x%llx", pFrameInfo->pBuffer, entry->filledLength);
        LoadedBufferInfo *pEntry = (LoadedBufferInfo*)malloc(sizeof(LoadedBufferInfo));
        if (!pEntry) return VidcStatusAllocError;
        memset(pEntry, 0x0, sizeof(LoadedBufferInfo));
        pEntry->pbuffer = pFrameInfo->pBuffer;
        pEntry->filledLength = pFrameInfo->nFilledLen;
        list_insert_tail(&pEntry->list, pMap);
    }

    return VidcStatusSuccess;
}

//Copy back the last-time saved filled length for re-using this input buffer and skipping reading
VidcStatus CopyLoadedBufferFilledLength(LIST_NODE* pMap, ParserFrameInfoType *pFrameInfo)
{
    LoadedBufferInfo *entry = NULL;
    BOOL found = FALSE;
    utils_list_for_each_entry(LoadedBufferInfo, entry, pMap, list)
    {
        if(entry->pbuffer == pFrameInfo->pBuffer)
        {
            found = TRUE;
            break;
        }
    }

    if(found)
    {
        //copy updated filled length
        VIDEO_STREAMPARSER_WARN("Found loaded buffer %p filled length = 0x%llx", pFrameInfo->pBuffer, entry->filledLength);
        pFrameInfo->nFilledLen = entry->filledLength;
    }
    else
    {
        VIDEO_STREAMPARSER_WARN("Cannot Found loaded buffer %p info", pFrameInfo->pBuffer, entry->filledLength);
        return VidcStatusUnknownError;
    }

    return VidcStatusSuccess;
}

VidcStatus VideoStreamParserGetNextFrame(VideoStreamParser* pParser,
        ParserFrameInfoType *pFrameInfo)
{
    VidcStatus status = VidcStatusSuccess;
    FileOffset val;
    uint32 nFrmCnt = pFrameInfo->nFrameCounter;

    memset(&val, 0, sizeof(FileOffset));

    /* This is needed bcoz for SF mode framecounter start with 0 but offset table has entries from 1
       without the hack sequence header will be parsed twice and will run into crash in cdsp */
    if(pParser->m_eCodecFormat == V4L2_PIX_FMT_VC1_ANNEX_L && pFrameInfo->nFrameCounter == 1)
    {
        pFrameInfo->nFrameCounter++;
        VIDEO_STREAMPARSER_HIGH("putting hack to increment framecounter to correct value for RCV clips");
    }
    if (NULL == pFrameInfo->pBuffer)
    {
        VIDEO_STREAMPARSER_WARN("NULL buffer pointer, frame.no:%d", pFrameInfo->nFrameCounter);
        return VidcStatusBadParamError;
    }
    if (pParser->m_bEosReached)
    {
        VIDEO_STREAMPARSER_HIGH("Already Reached EOS");
        pFrameInfo->bLastFrame = TRUE;
        pFrameInfo->nFilledLen = 0;
        return status;
    }

    if (pParser->BuildOffsetTable)
    {
        uint32 bytes_read = 0;

        if (pParser->m_bIsGetSeqHdr)
        {
            pParser->m_bIsGetSeqHdr = FALSE;

            if (pParser->m_nPrefixHeaderMode == V4L2_DEC_S_ONLY &&  pFrameInfo->nFrameCounter == 0)
            {
                //First frame case in which we need to parse seq header and set codec config flag
                status = GetAllSeqHdrAtFrame(pParser, pFrameInfo, &val, TRUE);

                if (status == VidcStatusSuccess)
                {

                    memcpy(pFrameInfo->pBuffer + pFrameInfo->nFilledLen, g_inputFilePtr + val.nOffset, val.nLength);
                    VIDEO_STREAMPARSER_HIGH("Copied Sequence Header with length:%llu", pFrameInfo->nFilledLen);
                    pFrameInfo->nFlags = V4L2_BUF_FLAG_CODECCONFIG;
                    pParser->nHdrLengthCopied = val.nLength;
                    pFrameInfo->nFilledLen += val.nLength;
                    if(pParser->m_nSkipReadAfterNFrames > 0)
                    {
                        status = SaveLoadedBufferFilledLength(&pParser->m_LoadedInputBufferList, pFrameInfo);
                    }
                }
                else
                {
                    VIDEO_STREAMPARSER_WARN("Unable to find sequence header at first frame, proceeding with direct frame decoding!!");
                    //Incrementing the frame counter to account for the missing seq header at first frame
                    pFrameInfo->nFrameCounter = 1;
                }
            }
            else
            {
                //seek case
                status = GetAllSeqHdrBeforeSeekPoint(pParser, pFrameInfo);
            }
        }

        if (pParser->m_nPrefixHeaderMode == V4L2_DEC_SF ||
            pFrameInfo->nFlags != V4L2_BUF_FLAG_CODECCONFIG)
        {
            pFrameInfo->nFlags = 0;

            status = GetFullFrameOffsetFromMap(&pParser->m_FrameOffsetTable, &pFrameInfo->nFrameCounter, &val, FALSE, pParser->m_nSuperFrame);

            //if sequence hdr already submitted to driver, substract it from full frame.
            if (pParser->m_nPrefixHeaderMode == V4L2_DEC_S_ONLY && pParser->nHdrLengthCopied)
            {
                val.nLength = val.nLength - pParser->nHdrLengthCopied;
                val.nOffset = val.nOffset + pParser->nHdrLengthCopied;
                pParser->nHdrLengthCopied = 0;
            }
            if (status != VidcStatusSuccess)
            {
                VIDEO_STREAMPARSER_WARN("Failed to fatch full frame from table number: %d", pFrameInfo->nFrameCounter);
                goto bailout;
            }

            if (val.nLength >= pFrameInfo->nAllocLen)
            {
                VIDEO_STREAMPARSER_WARN("Input data length %llu is more than input buffer allocated length %u", val.nLength, pFrameInfo->nAllocLen);
                status = VidcStatusBufferOverFlowError;
                goto bailout;
            }
            if(pParser->m_nSkipReadAfterNFrames == 0 || pFrameInfo->nFrameCounter <= pParser->m_nSkipReadAfterNFrames)
            {
                memcpy(pFrameInfo->pBuffer + pFrameInfo->nFilledLen, g_inputFilePtr + val.nOffset, val.nLength);
                pFrameInfo->nFilledLen += val.nLength;
                if(pParser->m_nSkipReadAfterNFrames > 0)
                {
                    status = SaveLoadedBufferFilledLength(&pParser->m_LoadedInputBufferList, pFrameInfo);
                }
            }
            else
            {
                VIDEO_STREAMPARSER_WARN("skip frame reading for frame %u",pFrameInfo->nFrameCounter);
                status = CopyLoadedBufferFilledLength(&pParser->m_LoadedInputBufferList, pFrameInfo);
            }
        }
        pFrameInfo->nTimeStamp = pFrameInfo->nFrameCounter * 1000000;
        if (pParser->m_eCodecFormat == V4L2_PIX_FMT_VC1_ANNEX_L && val.eFrameType == PARSER_FRAME_TYPE_VC1_RCV_STRCUT_C)
        {
            pFrameInfo->nFlags |= V4L2_BUF_FLAG_CODECCONFIG;
        }
        if ((pFrameInfo->nFrameCounter == NUM_FRMS_PARSE) || val.isLastFrame) //check for last frame
        {
            pFrameInfo->bLastFrame = TRUE;
        }
    }
    else //yuv case
    {
        pFrameInfo->nTimeStamp = pFrameInfo->nFrameCounter * 1000000;
        // YUVParse will overwrite the TS according to FrameRate;
        status = (pParser->m_fReadBuffer)(pParser, pFrameInfo);
        if (status){
            VIDEO_STREAMPARSER_WARN("m_fReadBuffer failed");
            return status;
        }
        if (pFrameInfo->nFilledLen > pFrameInfo->nAllocLen)
        {
            VIDEO_STREAMPARSER_WARN("Buffer Size not Sufficient");
            status = VidcStatusBufferOverFlowError;
            return status;
        }

        u64 curr_offset = (u64)curr_offset_file(pParser->m_InputFile);
        val.nOffset = curr_offset > pFrameInfo->nFilledLen ? curr_offset - pFrameInfo->nFilledLen : 0;
        if (pParser->m_InputOffsetFile)
        {
            fprintf(pParser->m_InputOffsetFile, "%#llx,%llu\n", val.nOffset, val.nLength);
        }
    }

    if (pParser->inputDump)
    {
        fwrite(pFrameInfo->pBuffer, pFrameInfo->nFilledLen, 1, pParser->m_bitsDump);
        fprintf(pParser->m_InputBitsOffsetFile, "%#llx,%llu\n", val.nOffset, pFrameInfo->nFilledLen);
    }

    if (!(pFrameInfo->nFlags & V4L2_BUF_FLAG_CODECCONFIG) && pFrameInfo->nFrameCounter == pParser->m_nFrames)
    {
        pFrameInfo->bLastFrame = TRUE;
    }
    pParser->m_bEosReached = pFrameInfo->bLastFrame;

    //Increment a frame counter for the next frame.
    if ((pParser->m_nPrefixHeaderMode == V4L2_DEC_S_ONLY) && (pFrameInfo->nFlags & V4L2_BUF_FLAG_CODECCONFIG))
        pFrameInfo->nFrameCounter++; //S_Only in which ETB will have only seq header hence incremment by 1 only
    else
        pFrameInfo->nFrameCounter += pParser->m_nSuperFrame;

    VIDEO_STREAMPARSER_HIGH("Frame Num= %d buffoffset=%#x frame_size=%#llx fileoffset:%#llx pBuffer=%#p allocLen:%u ionfd:%d ionHeap:%u status:%#x",
        nFrmCnt, pFrameInfo->nOffset, pFrameInfo->nFilledLen, val.nOffset,
        pFrameInfo->pBuffer, pFrameInfo->nAllocLen, pFrameInfo->nIonfd,
        pFrameInfo->nIonHeap, status);
bailout:
    return status;
}

VidcStatus ReadBufferFromH264File(VideoStreamParser* pParser, ParserFrameInfoType *pFrameInfo)
{
    uint8      *pBuffer = NULL;
    uint32      nCode = 0xFFFFFFFF;
    u64         nReadOffset = 0;
    BOOL        bMatched = TRUE;
    VidcStatus  status = VidcStatusSuccess;
    u64 nBytesToReset = 0;
    u64 nFrameStartOffset = pParser->m_nCurFilePos;
    u64 size;
    uint32 nNumStartCode = 0;
    FileOffsetMap *pEntry = NULL;
    BOOL isVCLFoundWithoutFMS = FALSE; //indicates that VCL unit found but it does not has FMS. This is used to determine frame boundry for seq
                                       //header if it is present just before that clip: 720p_h264_dec_high_32_s1_be_slh_I.264 and test case id: 1012000C_Dec_AVC_ADV_EOS1
    uint32 prevFMS = ~(0U);
    uint32 curFMS = 0;

    pBuffer = (pFrameInfo->pBuffer + pFrameInfo->nOffset);

    while (1)
    {
        if (nReadOffset >= pFrameInfo->nAllocLen - 5)
        {
            VIDEO_STREAMPARSER_WARN("Buffer is full\n");
            break;
        }

        *(pBuffer) = (uint8)*(g_inputFilePtr + pParser->m_nCurFilePos); //assign current byte pointed by g_inputFilePtr to pBuffer and increment file offset
        pParser->m_nCurFilePos++;

        // Store one byte read and update the current nCode
        STORE_AND_UPDATE_CODE(pBuffer, nReadOffset, nCode);  //pBuffer pointer and nReadOffset value are incremented & ncode value is updated
        bMatched = FALSE;

        if (nCode == NAL_UNIT_START_CODE) //checks for the start code
        {
            nBytesToReset = 4;
            bMatched = TRUE;
        }
        else if (NAL_UNIT_START_CODE == (nCode & NAL_START_CODE_MASK))
        {
            nBytesToReset = 3;
            bMatched = TRUE;
        }

        if (bMatched)
        {
            uint8 nNalType = 0;
            *(pBuffer) = (uint8)*(g_inputFilePtr + pParser->m_nCurFilePos);
            pParser->m_nCurFilePos++;

            nNalType = (*pBuffer++) & H264_NAL_UNIT_TYPE_MASK; //checked for NalType
            nReadOffset++;
            nBytesToReset++;

            if ((H264_NAL_UNIT_TYPE_SEI == nNalType ||
                H264_NAL_UNIT_TYPE_SPS == nNalType ||
                H264_NAL_UNIT_TYPE_PPS == nNalType ||
                H264_NAL_UNIT_TYPE_AUD == nNalType))
            {
                if (nNumStartCode == 1)
                {
                    nNumStartCode++;
                }
                if (pEntry)
                {
                    pEntry->val.nLength = pParser->m_nCurFilePos - pEntry->val.nOffset - nBytesToReset;
                    list_insert_tail(&pEntry->list, &pParser->m_FrameOffsetTable); //update pEntry length and insert buffer read as far to FrameOffsetTable
                    pEntry = NULL;
                }
                //if 2 then it is going to break a loop also create node only if its either SPS or PPS
                if ((H264_NAL_UNIT_TYPE_SPS == nNalType || H264_NAL_UNIT_TYPE_PPS == nNalType) && nNumStartCode != 2)
                {
                    pEntry = (FileOffsetMap*)malloc(sizeof(FileOffsetMap));
                    if (!pEntry) return VidcStatusAllocError;
                    pEntry->val.isLastFrame = FALSE;
                    pEntry->val.eFrameType = PARSER_FRAME_TYPE_SEQ_HEADER;
                    pEntry->val.nOffset = pParser->m_nCurFilePos - nBytesToReset;
                    pEntry->nFrameNum = pFrameInfo->nFrameCounter;
                }
            }
            else if (nNalType >= H264_NAL_UNIT_TYPE_NONIDR &&  nNalType <= H264_NAL_UNIT_TYPE_IDR) //Need to support all VCL units which are in range of 1 to 5 inclusive, clip: 720p_h264_dec_high_32_s1_be_sps.264
            {
                uint8* decodeByte = pBuffer; //decodeByte used in DecodeUGolomb points to pBuffer before incrementing by the following four bytes
                uint32 decodeByteOffset = 0;

                //read four bytes
                *pBuffer++ = (uint8)*(g_inputFilePtr + pParser->m_nCurFilePos);
                pParser->m_nCurFilePos++;
                *pBuffer++ = (uint8)*(g_inputFilePtr + pParser->m_nCurFilePos);
                pParser->m_nCurFilePos++;
                *pBuffer++ = (uint8)*(g_inputFilePtr + pParser->m_nCurFilePos);
                pParser->m_nCurFilePos++;
                *pBuffer++ = (uint8)*(g_inputFilePtr + pParser->m_nCurFilePos);
                pParser->m_nCurFilePos++;
                nBytesToReset += 4;
                if (pEntry && pEntry->val.eFrameType == PARSER_FRAME_TYPE_SEQ_HEADER)
                {
                    pEntry->val.nLength = pParser->m_nCurFilePos - pEntry->val.nOffset - nBytesToReset;
                    list_insert_tail(&pEntry->list, &pParser->m_FrameOffsetTable); //check for seq header and insert into FrameOffsetTable
                    pEntry = NULL;
                    isVCLFoundWithoutFMS = TRUE;
                }
                nReadOffset += 4;
                curFMS = DecodeUGolomb(decodeByte, &decodeByteOffset);

                // First MB in slice starts from 0 in each new frame hence check for curFMS<= prevFMS
                if (curFMS <= prevFMS)
                {
                    isVCLFoundWithoutFMS = FALSE;
                    nNumStartCode++;
                    // copy the byte read to the bitstream buffer and check if it is 1st MB in slice
                    if (pEntry)
                    {
                        pEntry->val.nLength = pParser->m_nCurFilePos - pEntry->val.nOffset - nBytesToReset; //frame size updated for pEntry
                        list_insert_tail(&pEntry->list, &pParser->m_FrameOffsetTable); //insert bytes read into FrameOffsetTable before one whole frame is done and break the while loop with (nNumStartCode > 1) becoming true
                        pEntry = NULL;
                    }
                    if (nNumStartCode != 2) //if 2 then it is going to break a loop
                    {
                        pEntry = (FileOffsetMap*)malloc(sizeof(FileOffsetMap));
                        if (!pEntry) return VidcStatusAllocError;
                        pEntry->val.isLastFrame  = FALSE;
                        pEntry->val.eFrameType = (nNalType == H264_NAL_UNIT_TYPE_IDR)? PARSER_FRAME_TYPE_SYNC : PARSER_FRAME_TYPE_NON_SYNC;
                        pEntry->val.nOffset = nFrameStartOffset;
                        pEntry->nFrameNum = pFrameInfo->nFrameCounter;
                    }
                }
                prevFMS = curFMS;
            }
        }

        if (nNumStartCode > 1)
        {
            //end of frame condition for all frames except the last frame
            size = pParser->m_nCurFilePos - nFrameStartOffset - nBytesToReset;
            pParser->m_nCurFilePos -= nBytesToReset;
            pFrameInfo->nFilledLen = size; //calculate the frame size using the offsets and assign
            break;
        }
        else if (pParser->m_nCurFilePos >= pParser->m_nFileSize) //file offset equals size of videoMap for last frame only
        {
            //end of frame condition for the last frame as nNumStartCode will not be incremented to 2 for last frame
            size = pParser->m_nCurFilePos - nFrameStartOffset;
            pFrameInfo->nFilledLen = size; //calculate the frame size using the offsets and assign

            if (!pEntry && isVCLFoundWithoutFMS)
            {
                pEntry = (FileOffsetMap*)malloc(sizeof(FileOffsetMap));
                if (!pEntry) return VidcStatusAllocError;
                pEntry->val.isLastFrame = FALSE;
                pEntry->val.eFrameType = PARSER_FRAME_TYPE_NON_SYNC;
                pEntry->val.nOffset = nFrameStartOffset;
                pEntry->nFrameNum = pFrameInfo->nFrameCounter;
            }
            if (pEntry)
            {
                //when nNumStartCode is 0 mean no FMS has been discoved. it would be only non-VCL unit is there and end of file is reached. Hence node length should be actual length
                // clip: 720p_h264_dec_high_32_s1_be_pps_I.264 and test case id: 10120009_Dec_AVC_ADV_EOS1
                if (nNumStartCode == 0)
                {
                    pEntry->val.nLength = pParser->m_nCurFilePos - pEntry->val.nOffset;
                }
                else
                {
                    pEntry->val.nLength = pFrameInfo->nFilledLen; //for last frame, frame size is updated for pEntry
                }
                pEntry->val.isLastFrame = TRUE; //for last frame
                list_insert_tail(&pEntry->list, &pParser->m_FrameOffsetTable); //insert last frame data into FrameOffsetTable
                pEntry = NULL;
            }
            if (nNumStartCode == 0 && !isVCLFoundWithoutFMS)
            {
                VIDEO_STREAMPARSER_INFO("Special case. Non VCL last frame. Mark 0 length data.\n");
                pFrameInfo->nFilledLen = 0;
                pFrameInfo->bLastFrame = TRUE;
                return status;
            }
            break;
        }
    }
    return status;
}

uint32 ParseStringEnum(ParseCfgEnum *pConfigEnum, const char* pEnumName)
{

    uint32 idx = 0;
    while (pConfigEnum[idx].pEnumName) {
        if (STRCMP(pEnumName,pConfigEnum[idx].pEnumName) == 0) {
            return pConfigEnum[idx].eEnumVal;
        }
        idx++;
    }
    return -1;
}
