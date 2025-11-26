/*************************************************************************
* Company                       : Tata Elxsi Ltd
* File                          : TEL_FLACDec_Riff.h
* Author                        : TEL Audio Team
* Version                       : 1.0
* Date                          : 10-October-2009
* Modified Date                 : 14-October-2009
* Operating Environment         : Windows XP
* Compiler with Version         : Visual studio on PC
* Description                   : Contains forward declarations and
*                               : constants used for wav header
* Copyright                     : <2009> Tata Elxsi Ltd.
*                               : All rights reserved.
*                               : This document/code contains information
*                                 that is proprietary to Tata Elxsi Ltd.
*                                 No part of this document/code may be
*                                 reproduced or used in whole or part in
*                                 any form or by any mean - graphic,
*                                 electronic or mechanical without the
*                                 written permission of Tata Elxsi Ltd.
*************************************************************************/
#ifndef _TEL_FLACDEC__RIFF_H_
#define _TEL_FLACDEC__RIFF_H_
/* Number of Channels Supported */
#define MAXNUMCH            8

#define OUTPUTBUFF_SIZE 1000

#define RIFF_CHUNK_ID   0x46464952
#define FMT_CHUNK_ID    0x20746D66
#define FACT_CHUNK_ID   0x74636166
#define DATA_CHUNK_ID   0x61746164
#define WAVE_FILE_ID 0x45564157

#define PCM_WAVE_FORMAT_TAG 0x0001
#define WAVE_FORMAT_TAG 0x0450
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

/* macro for GNU gcc to force packing on one-byte boundary */
#ifndef BYTEPACKED
#ifdef __GNUC__

#define BYTEPACKED __attribute__ ((__packed__))

#else
#define BYTEPACKED
#endif /* __GNUC__ */
#endif /* BYTEPACKED */

#define ENDOFSTREAM 0

typedef struct _WINDOWS_GUID
{
    uint32       ui32Data1;
    uint16       ui16Data2;
    uint16       ui16Data3;
    uint8        ui8Data4[8];

} WINDOWS_GUID;

static const WINDOWS_GUID KSDATAFORMAT_SUBTYPE_PCM =
{
    0x00000001, 0x0000, 0x0010,
    {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
};


#ifdef _MSC_VER /*Microsoft compiler */
  #pragma pack(push, 1)
#endif /*_MSC_VER */

struct stWavParams
{
    uint16 ui16nFormatTag;      /*Format type */
    uint16 ui16nChannels;       /* Number of channels */
    uint32 ui32nSamplesPerSec;  /*  Sample rate */
    uint32 ui32nAvgBytesPerSec; /*  Average data rate in bytes per second */
    uint16 ui16nBlockAlign;     /* Block alignment in bytes */
    uint16 ui16nBitsPerSample;  /* Bits per sample for the format type */
    uint16 i16cbSize;

    union
    {
        uint16  ui16wValidBitsPerSample;
        uint16  ui16wSamplesPerBlock;
        uint16  ui16wReserved;
    } Samples;
    uint32 ui32dwChannelMask;
    WINDOWS_GUID SubFormat;
} BYTEPACKED;

struct PCM_WAVEFORMATEX
{
    uint16 ui16nFormatTag;      /* Format type */
    uint16 ui16nChannels;       /* Number of channels */
    uint32 ui32nSamplesPerSec;  /* Sample rate */
    uint32 ui32nAvgBytesPerSec; /* Average data rate in bytes per second */
    uint16 ui16nBlockAlign;     /* Block alignment in bytes */
    uint16 ui16nBitsPerSample;  /* Bits per sample for the format type */

} BYTEPACKED;

struct FLAC_WAVEFORMAT
{
    uint16 ui16nFormatTag;          /* Format type */
    uint16 ui16nChannels;           /* Number of channels */
    uint32 ui32nSamplesPerSec;      /* Sample rate */
    uint32 ui32nAvgBytesPerSec;     /* Average data rate in bytes per second */
    uint16 ui16nBlockAlign;         /* Block alignment in bytes */
    uint16 ui16nBitsPerSample;      /* Bits per sample for the format type */
    uint16 ui16nSize;               /* Size in bytes of the extra
                                       information (after nSize) */
    uint16 ui16nVersion;            /* Bitstream version */
    uint32 ui32nOriginalBitRate;    /* Original bit rate */
    uint32 ui32nFlags;              /* Flags */
    uint32 ui32nOriginalSamplesPerSec; /* Original sample rate */
    uint16 ui16nAMODE;              /* channel configuration */
} BYTEPACKED;

struct CHUNK_HEADER
{
    uint32 ui32nChunkID;    /* Chunk identifier */
    uint32 ui32nLength;     /* Length of chunk data */
} BYTEPACKED;


#ifdef _MSC_VER /* Microsoft compiler */
  #pragma pack(pop)
#endif /* _MSC_VER */

uint32 FLACDEC_FindChunk
(
    FILE *pFile,
    uint32 ui32nChunkID,
    struct CHUNK_HEADER *pChunkHeader
);

uint32 FLACDEC_ReadRIFFChunk
(
    FILE *pFile,
    uint32 *ui32pLength
);

uint32 FLACDEC_ReadFMTChunkPCM
(
    FILE *pFile,
    struct stWavParams *pPCMWAVEFORMAT
);


/*************************************************************************
* Function Name     :FLACDEC_ReadFMTChunk()
* Description       :Reads the FMT chunk from the specified  file.
* Parameters        :FILE *pFile -> File pointer
*                   :FLAC_WAVEFORMAT *pstWaveFormat -> format header
* Called functions  :FLACDEC_FindChunk
*                   :CFSwapInt16LittleToHost
*                   :CFSwapInt32LittleToHost
* Global Data       :none
* Return Value      :0 if no error,
*                   :1 if error
* Exceptions        :none
*************************************************************************/
uint32 FLACDEC_ReadFMTChunk(FILE *pFile,
                            struct FLAC_WAVEFORMAT *pWaveFormat);


/*************************************************************************
* Function Name     : FLACDEC_ReadFACTChunk()
* Description       : Reads the FACT chunk from the specified file.
* Parameters        : FILE *pFile -> File pointer
*                   : uint32 *pui32TotalSamples -> Pointer for storing
*                   : number of samples
* Called functions  : CFSwapInt32LittleToHost
*                   : FLACDEC_FindChunk
* Global Data       : none
* Return Value      : 0 if no error,
*                   : 1 if error
* Exceptions        : none
*************************************************************************/
uint32 FLACDEC_ReadFACTChunk(FILE *pFile,
                            uint32 *ui32pTotalSamples);


/***********************************************************************
* Function Name     : FLACDEC_ReadDATAChunk()
* Description       : Reads the DATA chunk from the specified file.
* Parameters        : FILE *pFile -> File pointer
*                   : uint32 *pui32Length -> Pointer for stoing chunk
*                   :                       length
* Called functions  : CFSwapInt32LittleToHost
*                   : FLACDEC_FindChunk
* Global Data       : none
* Return Value      : 0 if no error,
*                   : 1 if error
* Exceptions        : none
***********************************************************************/
uint32 FLACDEC_ReadDATAChunk(FILE *pFile,
                            uint32 *ui32pLength);


/*************************************************************************
* Function Name     : FLACDEC_ReadPCMSamples()
* Description       : Reads PCM samples from the specified file.
* Parameters        : FILE *pFile -> File pointer
*                   : uint8 *pui8Data ->Buffer for storing data
*                   : uint32 ui32Nlength _> Number of bytes to be read
*                   : struct stWavParams *pstPCMWaveFormat -> Wave format
* Called functions  :CFSwapInt32LittleToHost
* Global Data       :none
* Return Value      :Byte count of samples read, or 0 if error
* Exceptions        :none
*************************************************************************/
uint32 FLACDEC_ReadPCMSamples(FILE *pFile,
                            uint8* ui8Data,
                            uint32 ui32nLength,
                            struct stWavParams *pPCMWaveFormat);


/*************************************************************************
* Function Name     : FLACDEC_WriteRIFFChunk()
* Description       : Writes the RIFF chunk to the specified file.
* Parameters        : FILE *pFile -> File Pointer
*                   : uint32 ui32Nlength -> Length
* Called functions  : CFSwapInt32LittleToHost
* Global Data       : none
* Return Value      : 0 if no error,
*                   : 1 if error
* Exceptions        : none
*************************************************************************/
uint32 FLACDEC_WriteRIFFChunk(FILE *pFile,
                              uint32 ui32nLength);


/*************************************************************************
* Function Name     : FLACDEC_WriteFMTChunkPCM()
* Description       : Writes the FMT chunk with WAVEFORMAT(EXTENSIBLE) format
*                   : to the specified PCM file.
*                   : This function will write a smaller PCM_WAVEFORMAT struct
*                   : if i32cbSize is set to the size of that struct.
* Parameters        : FILE *pFile -> File pointer
*                   : struct stWavParams *pstPCMWaveFormat -> Wav format
*                   : uint32 ui32SampleSize -> Sample size
*                   : uint32 ui32NumChannels -> Number of channels
* Called functions  : none
* Global Data       : none
* Return Value      : 0 if no error,
*                   : 1 if error
* Exceptions        : none
*************************************************************************/
uint32 FLACDEC_WriteFMTChunkPCM(FILE *pFile,
                        struct stWavParams *pPCMWaveFormat,
                        uint32 ui32SampleSize,
                        uint32 ui32NumChannels);


/***********************************************************************
* Function Name     : FLACDEC_WriteFMTChunk()
* Description       : Writes the FMT chunk to the specified  file.
* Parameters        : FILE *pFile -> File pointer
*                   : struct FLAC_WAVEFORMAT *pstWaveFormat -> Wav format
* Called functions  : CFSwapInt32LittleToHost
* Global Data       : none
* Return Value      : 0 if no error,
*                   : 1 if error
* Exceptions        : none
***********************************************************************/
uint32 FLACDEC_WriteFMTChunk(FILE *pFile,
                            struct FLAC_WAVEFORMAT *pWaveFormat);


/*************************************************************************
* Function Name     : FLACDEC_WriteFACTChunk()
* Description       : Writes the FACT chunk to the specified file.
* Parameters        : FILE *pFile -> file pointer
*                   : uint32 ui32NtotalSamples -> Total number of samples
* Called functions  : CFSwapInt32LittleToHost
* Global Data       : none
* Return Value      : 0 if no error,
*                   : 1 if error
* Exceptions        : none
*************************************************************************/
uint32 FLACDEC_WriteFACTChunk(FILE *pFile,
                              uint32 i32nTotalSamples);


/*************************************************************************
* Function Name     :FLACDEC_WriteDATAChunk()
* Description       :Writes the DATA chunk to the specified file.
* Parameters        :FILE *pFile -> File popinter
*                   :uint32 ui32Nlength -> Length
* Called functions  :CFSwapInt32LittleToHost
* Global Data       :none
* Return Value      :0 if no error,
*                   :1 if error
* Exceptions        :none
*************************************************************************/
uint32 FLACDEC_WriteDATAChunk(FILE *pFile,
                            uint32 ui32nLength);


/*************************************************************************
* Function Name     : FLACDEC_WritePCMSamples()
* Description       :Writes PCM samples to the specified file.
* Parameters        :FILE *pFile -> File pointer
*                   : uint8 *pui8Data -> Data to be written
*                   : uint32 ui32Nlength -> byte count of samples
*                   :ui32Nlength
* Called functions  :none
* Global Data       :none
* Return Value      :Byte count written
* Exceptions        :none
*************************************************************************/
uint32 FLACDEC_WritePCMSamples(FILE *pFile,
                        uint8* ui8pData,
                        uint32 ui32nLength);


/*************************************************************************
* Function Name     : FLACDEC_SeekBytes()
* Description       : Seek to specified byte offset in file.
*                   : Seek over 2GB not supported
* Parameters        : FILE *pFile -> File pointer
*                   : uint32 ui32Nbytes -> Number of bytes to seek
* Called functions  : none
* Global Data       : none
* Return Value      : 0 if no error,
*                   : non-zero if error
* Exceptions        : none
*************************************************************************/
uint32 FLACDEC_SeekBytes(FILE *pFile,
                        uint32 ui32nBytes);


/*************************************************************************
* Function Name     : FLACDEC_SeekChunk()
* Description       : Finds the specified chunk in the specified file. The
*                   : FILE pointer will be set to the start of the header.
* Parameters        : FILE *pFile -> File pointer
*                   : uint32 ui32NchunkID -> Chunk ID
*                   : struct CHUNK_HEADER *pstChunkHeader -> Chunk header
* Called functions  :CFSwapInt32LittleToHost
* Global Data       :none
* Return Value      :0 if no error,
*                   :1 if chunk not found
* Exceptions        :none
*************************************************************************/
uint32 FLACDEC_SeekChunk(FILE *pFile,
                    uint32 ui32nChunkID,
                    struct CHUNK_HEADER *pChunkHeader);


/*************************************************************************
* Function Name     : FLACDEC_SeekChunk()
* Description       : Finds the specified chunk in the specified file. The
*                   : FILE pointer will be set to the start of the header.
* Parameters        : FILE *pFile -> File pointer
*                   : uint32 ui32NchunkID -> Chunk ID
*                   : struct CHUNK_HEADER *pstChunkHeader -> Chunk header
* Called functions  :CFSwapInt32LittleToHost
* Global Data       :none
* Return Value      :0 if no error,
*                   :1 if chunk not found
* Exceptions        :none
*************************************************************************/
int32 FLACDEC_Deinterleave(const uint8 *ui8pnInputBuffer,
                        const uint32 ui32nSizeInputBuffer,
                        uint8 **ppnOutputBuffers,
                        const uint32 ui32nNumOutputBuffers,
                        const uint32 ui32nOutputBufferSize,
                        const int32 i32nOutputBufferBitsPerSample);


/*************************************************************************
* Function Name     : FLACDec_OpenOutFiles()
* Description       : This function opens out put files
* Parameters        : const int8 *InputFile -> Input file file
                         FILE **ppOutputFiles
* Called functions  : none
* Global Data       : none
* Return Value      : 0 if successful
* Exceptions        : none
*************************************************************************/
int32 FLACDec_OpenOutFiles(const int8 *InputFile,
                         FILE **ppOutputFiles);


/*************************************************************************
* Function Name     : FLACDec_InitializeWavChunks()
* Description       : This function initializes the wave chunk
* Parameters        : FILE *pFile -> File pointer
*                   : uint32 ui32NumChannels -> Number of channels
*                   : uint32 ui32SampleRate -> Sampling rate
*                   : uint32 ui32SampleSize -> Sample size
*                   : const int32 i32nOutputDataLength -> Data Length
* Called functions  : none
* Global Data       : none
* Return Value      : 1 if successful
* Exceptions        : none
*************************************************************************/
int32 FLACDec_InitializeWavChunks(FILE *pFile,
                          uint32 ui32NumChannels,
                          uint32 ui32SampleRate,
                          uint32 ui32SampleSize,
                          const int32 i32nOutputDataLength);


/***********************************************************************
* Function Name     : FLACDec_WriteRIFFDataLengths()
* Parameters        : This function writes RIFF Data Length to file
* Description       : pFile
*                   : i32ZeroBytes
* Called functions  : FLACDEC_WritePCMSamples
*                   : FLACDEC_WriteRIFFChunk
*                   : FLACDEC_SeekChunk
*                   : FLACDEC_WriteDATAChunk
* Global Data       : none
* Return Value      : 1 if successful
*                   : 0 if Error finding data chunk
*                   : or Error writing the data chunk
*                   : or if Error writing RIFF header RIFF chunk.
* Exceptions        : none
***********************************************************************/
int32 FLACDec_WriteRIFFDataLengths(FILE *pFile,
                                   int32 i32ZeroBytes);


/***********************************************************************
* Function Name     : FLACDec_WriteWAVFrameData()
* Description       : This function writes samples to output FILE
* Parameters        : int32* pi32OutData -> Output data
*                   : FILE *fpOutput -> File pointer
*                   : uint32 ui32nBitsPerSample -> Bits per sample
*                   : uint32 ui32Numchannels -> Number of channels
*                   : uint32 ui32BlockSize -> Block size
* Called functions  : none
* Global Data       : none
* Return Value      : 1 if successful
* Exceptions        : none
***********************************************************************/
int32 FLACDec_WriteWAVFrameData(void* piOutData,
                                FILE *fpOutput,
                                uint32 ui32NbitsPerSample,
                                uint32 ui32Numchannels,
                                uint32 ui32BlockSize);

#endif /* _TEL_FLACDEC__RIFF_H_ */

/*************************************************************************
 * End of file
 ************************************************************************/

