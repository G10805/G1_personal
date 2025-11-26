/*--------------------------------------------------------------------------
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidaries.
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
/*
    Early Video/Ethernet RVC application
*/

#include <linux/version.h>
#include "omx_vdec_render.h"

#ifdef _ANDROID_
using namespace android;
#endif
/************************************************************************/
/*                            MACROS                                    */
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

/************************************************************************/
/*                        GLOBAL DECLARATIONS                           */
/************************************************************************/
typedef enum
{
    RESOLUTION_720P   = 0,
    RESOLUTION_1080P  = 1,
} resolution;

#ifdef _MSM8974_
typedef unsigned short int uint16;

const uint16 CRC_INIT = 0xFFFF ;

const uint16 crc_16_l_table[ 256 ] = {
  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
  0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
  0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
  0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
  0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
  0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
  0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
  0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
  0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
  0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
  0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
  0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
  0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
  0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
  0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
  0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
  0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
  0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
  0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
  0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
  0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
  0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
  0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
  0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
  0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
  0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
  0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
  0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
  0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
  0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
  0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

uint16 crc_16_l_step_nv12 (uint16 seed, const void *buf_ptr,
    unsigned int byte_len, unsigned int height, unsigned int width)
{
  (void) byte_len;
  uint16 crc_16 = ~seed;
  char *buf = (char *)buf_ptr;
  char *byte_ptr = buf;
  unsigned int i, j;
  const unsigned int width_align = 32;
  const unsigned int height_align = 32;
  unsigned int stride = (width + width_align -1) & (~(width_align-1));
  unsigned int scan_lines = (height + height_align -1) & (~(height_align-1));
  for (i = 0; i < height; i++) {
      for (j = 0; j < stride; j++) {
          if (j < width) {
              crc_16 = crc_16_l_table[ (crc_16 ^ *byte_ptr) & 0x00ff ] ^ (crc_16 >> 8);
          }
          byte_ptr++;
      }
  }
  byte_ptr = buf + (scan_lines * stride);
  for (i = scan_lines; i < scan_lines + height/2;i++) {
      for (j = 0; j < stride; j++) {
          if (j < width) {
              crc_16 = crc_16_l_table[ (crc_16 ^ *byte_ptr) & 0x00ff ] ^ (crc_16 >> 8);
          }
          byte_ptr++;
      }
  }
  return( ~crc_16 );
}
#endif

typedef enum {
  CODEC_FORMAT_H264 = 1,
  CODEC_FORMAT_MP4,
  CODEC_FORMAT_H263,
  CODEC_FORMAT_VC1,
  CODEC_FORMAT_DIVX,
  CODEC_FORMAT_MPEG2,
#ifdef _MSM8974_
  CODEC_FORMAT_VP8,
  CODEC_FORMAT_VP9,
  CODEC_FORMAT_HEVC,
#endif
  CODEC_FORMAT_MAX
} codec_format;

typedef enum {
  FILE_TYPE_RTP = 0,
  FILE_TYPE_DAT_PER_AU = 1,
  FILE_TYPE_ARBITRARY_BYTES,
  FILE_TYPE_MP4,
  FILE_TYPE_COMMON_CODEC_MAX,

  FILE_TYPE_START_OF_H264_SPECIFIC = 10,
  FILE_TYPE_264_NAL_SIZE_LENGTH = FILE_TYPE_START_OF_H264_SPECIFIC,
  FILE_TYPE_264_START_CODE_BASED,

  FILE_TYPE_START_OF_MP4_SPECIFIC = 20,
  FILE_TYPE_PICTURE_START_CODE = FILE_TYPE_START_OF_MP4_SPECIFIC,

  FILE_TYPE_START_OF_VC1_SPECIFIC = 30,
  FILE_TYPE_RCV = FILE_TYPE_START_OF_VC1_SPECIFIC,
  FILE_TYPE_VC1,

  FILE_TYPE_START_OF_DIVX_SPECIFIC = 40,
  FILE_TYPE_DIVX_4_5_6 = FILE_TYPE_START_OF_DIVX_SPECIFIC,
  FILE_TYPE_DIVX_311,

  FILE_TYPE_START_OF_MPEG2_SPECIFIC = 50,
  FILE_TYPE_MPEG2_START_CODE = FILE_TYPE_START_OF_MPEG2_SPECIFIC,

#ifdef _MSM8974_
  FILE_TYPE_START_OF_VP8_SPECIFIC = 60,
  FILE_TYPE_VP8_START_CODE = FILE_TYPE_START_OF_VP8_SPECIFIC,
  FILE_TYPE_VP8
#endif

} file_type;

typedef enum {
  EARLY_VIDEO = 0,
  ETHERNET_RVC
} op_mode;

typedef enum {
  GOOD_STATE = 0,
  PORT_SETTING_CHANGE_STATE,
  ERROR_STATE
} test_status;

typedef enum {
  FREE_HANDLE_AT_LOADED = 1,
  FREE_HANDLE_AT_IDLE,
  FREE_HANDLE_AT_EXECUTING,
  FREE_HANDLE_AT_PAUSE
} freeHandle_test;

struct temp_egl {
    int pmem_fd;
    int offset;
};

/************************************************************************/
/*                   GLOBALS FOR INPUT OPTIONS                          */
/************************************************************************/
char *in_filename = (char *)"/vendor/bin/bootvideo.mp4";

resolution video_resolution = RESOLUTION_720P;
op_mode operating_mode = EARLY_VIDEO;
codec_format  codec_format_option = CODEC_FORMAT_H264;
#ifdef EARLY_BOOTVIDEO
file_type     file_type_option = FILE_TYPE_MP4;
#else
file_type     file_type_option = FILE_TYPE_RTP;
#endif
freeHandle_test freeHandle_option = FREE_HANDLE_AT_LOADED;

unsigned int num_frames_to_decode = 0;
int nalSize = 2;
int realtime_display = 1;

int height = 1080;
int width = 1920;
int fps = 30;

static int socketaddr = 0;
static int nDestIP = 0;
static int nDestPort = 0;

char* nDestIPStr = NULL;

#ifdef USE_ION
bool first_start = true;
int gpio_value = 1;
#endif

/************************************************************************/
/*                                 GLOBAL INIT                          */
/************************************************************************/
int input_buf_cnt = 0;
int used_ip_buf_cnt = 0;
unsigned free_op_buf_cnt = 0;
volatile int event_is_done = 0;
int ebd_cnt= 0, fbd_cnt = 0;
int frame_count = 0;
int frames_dropped = 0;
int bInputEosReached = 0;
int bOutputEosReached = 0;
#ifdef _MSM8974_
char crclogname[512];
#endif
char seq_file_name[512];
unsigned char seq_enabled = 0;
bool anti_flickering = true;
unsigned char flush_input_progress = 0, flush_output_progress = 0;
unsigned cmd_data = ~(unsigned)0, etb_count = 0;

char curr_seq_command[100];
OMX_S64 timeStampLfile = 0;
unsigned int timestampInterval = 33333;
int sent_disabled = 0;
int waitForPortSettingsChanged = 1;
test_status currentStatus = GOOD_STATE;
struct timespec t_start = {0, 0}, t_end = {0, 0};

//* OMX Spec Version supported by the wrappers. Version = 1.1 */
const OMX_U32 CURRENT_OMX_SPEC_VERSION = 0x00000101;
OMX_COMPONENTTYPE* dec_handle = 0;

OMX_BUFFERHEADERTYPE  **pInputBufHdrs = NULL;
OMX_BUFFERHEADERTYPE  **pOutYUVBufHdrs= NULL;

static OMX_BOOL use_external_pmem_buf = OMX_FALSE;

int rcv_v1=0;
static struct temp_egl **p_eglHeaders = NULL;
unsigned char* use_buf_virt_addr[32];

OMX_QCOM_PLATFORM_PRIVATE_LIST      *pPlatformList = NULL;
OMX_QCOM_PLATFORM_PRIVATE_ENTRY     *pPlatformEntry = NULL;
OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *pPMEMInfo = NULL;
OMX_CONFIG_RECTTYPE crop_rect;

static int bHdrflag = 0;
static int video_playback_count = 1;

static int inputBufferFileFd = -1;
FILE * outputBufferFile;
#ifdef _MSM8974_
FILE * crcFile;
#endif
FILE * seqFile;

int takeYuvLog = 0;
int displayYuv = 0;
int thumbnailMode = 0;

Queue *etb_queue = NULL;
Queue *fbd_queue = NULL;

pthread_t ebd_thread_id;
pthread_t fbd_thread_id;
pthread_t gbm_thread_id;
pthread_t gpio_thread_id;

FileSource *m_pFileSource;
FileSourceStatus mStatus = FILE_SOURCE_FAIL;

#ifdef READ_FROM_SOCKET
#define READ_BUFFER_LEN 1280*720
#define RTCPINTERVAL 500 //millisecond
static uint8 destIP[16] = {0};
pthread_t rtp_thread_id;
uint8 *buffer;
bool bRtpReceiverStopped = FALSE;
pthread_mutex_t rtp_lock;
static RTPDataSource* m_pRTPDecoder = NULL;
static SourcePort *m_pSourcePort = NULL;
uint64 currentTime = 0;
uint64 diffTime = 0;
struct RTP_Socket {
  int nLocalPort;
  uint16 nDestPort;
  uint32 nDestIP;
  int nRTCPsocket;
  int payload_type;
  int rtcpIntervalMs;
};
struct RTP_Socket rtp_socket;
#endif

#ifdef _ANDROID_
Queue *queue_buffer_list; /* Contains rtp data */
int opendisplay = 1;
#elif defined _LINUX_
GQueue queue_buffer_list; /* Contains rtp data */
#endif

pthread_mutex_t etb_lock;
pthread_mutex_t fbd_lock;
pthread_mutex_t lock;
pthread_cond_t cond;
pthread_mutex_t eos_lock;
pthread_cond_t eos_cond;
pthread_mutex_t enable_lock;

sem_t etb_sem;
sem_t fbd_sem;
sem_t seq_sem;
sem_t in_flush_sem;
sem_t out_flush_sem;

OMX_PARAM_PORTDEFINITIONTYPE portFmt;
OMX_PORT_PARAM_TYPE portParam;
OMX_ERRORTYPE error;
OMX_COLOR_FORMATTYPE color_fmt;
static bool input_use_buffer = false;
static bool output_use_buffer = false;
QOMX_VIDEO_DECODER_PICTURE_ORDER picture_order;

#ifdef MAX_RES_1080P
unsigned int color_fmt_type = 1;
#else
unsigned int color_fmt_type = 0;
#endif

bool isdisplayopened = FALSE;

enum use_protocol protocol = USE_GBM_PROTOCOL;
static bool bmarkfirstdisplay = TRUE;
static int previous_vc1_au = 0;
pthread_t edrmdisplay_thread_id;
enum render enable_display = RENDER_DRM;

/**************************************************************************/
/*               FUNCTION DECLARATIONS                       */
/**************************************************************************/
int Init_Decoder();
int Play_Decoder();
int Decode_Display_Video();
static int (*Read_Buffer)(OMX_BUFFERHEADERTYPE  *pBufHdr );

static int open_video_file();
static void do_freeHandle_and_clean_up(bool isDueToError);

int disable_output_port();
int enable_output_port();
int output_port_reconfig();
void free_output_buffers();

void* ebd_thread(void*);
void* fbd_thread(void*);

#ifndef USE_ION
static bool align_pmem_buffers(int pmem_fd, OMX_U32 buffer_size,
                                  OMX_U32 alignment);
#endif
void getFreePmem();

#ifdef READ_FROM_SOCKET
static int Read_Buffer_From_Socket(OMX_BUFFERHEADERTYPE  *pBufHdr);
#endif
static int Read_Buffer_From_DAT_File(OMX_BUFFERHEADERTYPE  *pBufHdr );
static int Read_Buffer_From_H264_Start_Code_File(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_ArbitraryBytes(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_MP4_File(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_Vop_Start_Code_File(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_Mpeg2_Start_Code(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_Size_Nal(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_RCV_File_Seq_Layer(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_RCV_File(OMX_BUFFERHEADERTYPE  *pBufHdr);
#ifdef _MSM8974_
static int Read_Buffer_From_VP8_File(OMX_BUFFERHEADERTYPE  *pBufHdr);
#endif
static int Read_Buffer_From_VC1_File(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_DivX_4_5_6_File(OMX_BUFFERHEADERTYPE  *pBufHdr);
static int Read_Buffer_From_DivX_311_File(OMX_BUFFERHEADERTYPE  *pBufHdr);

static OMX_ERRORTYPE Allocate_Buffer ( OMX_COMPONENTTYPE *dec_handle,
                                       OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                       OMX_U32 nPortIndex,
                                       long bufCntMin, long bufSize);

static OMX_ERRORTYPE use_input_buffer(OMX_COMPONENTTYPE      *dec_handle,
                                OMX_BUFFERHEADERTYPE ***bufferHdr,
                                OMX_U32              nPortIndex,
                                OMX_U32              bufSize,
                                long                 bufcnt);

static OMX_ERRORTYPE use_output_buffer(OMX_COMPONENTTYPE      *dec_handle,
                                OMX_BUFFERHEADERTYPE ***bufferHdr,
                                OMX_U32              nPortIndex,
                                OMX_U32              bufSize,
                                long                 bufcnt);
#ifdef _ANDROID_
static OMX_ERRORTYPE use_output_buffer_multiple_fd ( OMX_COMPONENTTYPE *dec_handle,
                                   OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                   OMX_U32 nPortIndex,
                                   OMX_U32 bufSize,
                                   long bufCntMin);
#endif
static OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                                  OMX_IN OMX_PTR pAppData,
                                  OMX_IN OMX_EVENTTYPE eEvent,
                                  OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
                                  OMX_IN OMX_PTR pEventData);
static OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_PTR pAppData,
                                     OMX_IN OMX_BUFFERHEADERTYPE* pBuffer);
static OMX_ERRORTYPE FillBufferDone(OMX_OUT OMX_HANDLETYPE hComponent,
                                    OMX_OUT OMX_PTR pAppData,
                                    OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

/**************************************************************************/
/*              Function Definitions                      */
/**************************************************************************/
void place_marker(char const *name)
{
#ifdef ANDROID_U_AND_ABOVE
   int fd=open("/sys/kernel/debug/bootkpi/kpi_values", O_WRONLY);
   if (fd > 0)
   {
       char earlyapp[128] = {0};
       strlcpy(earlyapp, rstr(EARLYAPP), sizeof(earlyapp));
       strlcat(earlyapp, name, sizeof(earlyapp));
       write(fd, earlyapp, strlen(earlyapp));
       close(fd);
   }
#else
   /* debug info, hence boot_kpi string is not added.
    */
   ALOGE("%s", name);
#endif
}

int Char_To_WideChar(
                  const char * src, // address of string to map
                  int nSrc,      // number of bytes in string
                  wchar_t * dest,  // address of wide-character buffer
                  int nDest,        // size of buffer
                  boolean isUnicode = FALSE
                  )
{
  char *pDest = (char*)dest;
  if (nDest==0)
  {
    //return size of required output buffer
    return (int)strlen(src) + 1;
  }
  else
  {
    //see how many chars to convert
    int n = (nSrc < 0) ? (int)strlen(src)+1 : nSrc;
    int count = 0;

    //reset output memory
    memset(dest, 0, nDest);
    if ((n * sizeof(wchar_t))/(1+isUnicode) > (unsigned int)nDest)
    {
      return 0;
    }
    for (int i = 0; i < n; i++)
    {
      if (i<nDest)
      {
        //convert to unsigned first to avoid sign-extension.
        if(FALSE == isUnicode)
          dest[i]=(wchar_t)((unsigned char)src[i]);
        else //convert uint16 character into wchar(uint32) for LA platform
        {
          pDest[count * 4] = src[i];
          pDest[count * 4 + 1] = src[i + 1];
          i++;
        }
        count++;
      }
    }

    dest[count] = (wchar_t) 0;
    dest[nDest - 1] = (wchar_t) 0;

    return count;
  }
}

int clip2(int x)
{
        x = x -1;
        x = x | x >> 1;
        x = x | x >> 2;
        x = x | x >> 4;
        x = x | x >> 16;
        x = x + 1;
        return x;
}

void wait_for_event(void)
{
    DEBUGT_PRINT("Waiting for event");
    pthread_mutex_lock(&lock);
    while (event_is_done == 0) {
        pthread_cond_wait(&cond, &lock);
    }
    event_is_done = 0;
    pthread_mutex_unlock(&lock);
    DEBUGT_PRINT("Running .... get the event");
}

void event_complete(void )
{
    pthread_mutex_lock(&lock);
    if (event_is_done == 0) {
        event_is_done = 1;
        pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&lock);
}

int get_next_command(FILE *seq_file)
{
    int i = -1;
    do{
        i++;
        if(fread(&curr_seq_command[i], 1, 1, seq_file) != 1)
            return -1;
    }while(curr_seq_command[i] != '\n');
    curr_seq_command[i] = 0;
    DEBUGT_PRINT("cmd_str = %s", curr_seq_command);
    return 0;
}

int process_current_command(const char *seq_command)
{
    char *data_str = NULL;
    unsigned int data = 0;

    if(strstr(seq_command, "pause") == seq_command)
    {
        DEBUGT_PRINT(" $$$$$   PAUSE    $$$$$");
        data_str = (char*)seq_command + strlen("pause") + 1;
        data = atoi(data_str);
        DEBUGT_PRINT(" After frame number %u", data);
        cmd_data = data;
        sem_wait(&seq_sem);
        if (!bOutputEosReached && !bInputEosReached)
        {
            DEBUGT_PRINT(" Sending PAUSE cmd to OMX compt");
            OMX_SendCommand(dec_handle, OMX_CommandStateSet, OMX_StatePause,0);
            wait_for_event();
            DEBUGT_PRINT(" EventHandler for PAUSE DONE");
        }
        else
            seq_enabled = 0;
    }
    else if(strstr(seq_command, "sleep") == seq_command)
    {
        DEBUGT_PRINT(" $$$$$   SLEEP    $$$$$");
        data_str = (char*)seq_command + strlen("sleep") + 1;
        data = atoi(data_str);
        DEBUGT_PRINT(" Sleep Time = %u ms", data);
        usleep(data*1000);
    }
    else if(strstr(seq_command, "resume") == seq_command)
    {
        DEBUGT_PRINT(" $$$$$   RESUME    $$$$$");
        DEBUGT_PRINT(" Immediate effect");
        DEBUGT_PRINT(" Sending RESUME cmd to OMX compt");
        OMX_SendCommand(dec_handle, OMX_CommandStateSet, OMX_StateExecuting,0);
        wait_for_event();
        DEBUGT_PRINT(" EventHandler for RESUME DONE");
    }
    else if(strstr(seq_command, "flush") == seq_command)
    {
        DEBUGT_PRINT(" $$$$$   FLUSH    $$$$$");
        data_str = (char*)seq_command + strlen("flush") + 1;
        data = atoi(data_str);
        DEBUGT_PRINT(" After frame number %u", data);
        if (previous_vc1_au)
        {
            DEBUGT_PRINT(" Flush not allowed on Field boundary");
            return 0;
        }
        cmd_data = data;
        sem_wait(&seq_sem);
        if (!bOutputEosReached && !bInputEosReached)
        {
            DEBUGT_PRINT(" Sending FLUSH cmd to OMX compt");
            flush_input_progress = 1;
            flush_output_progress = 1;
            OMX_SendCommand(dec_handle, OMX_CommandFlush, OMX_ALL, 0);
            wait_for_event();
            DEBUGT_PRINT(" EventHandler for FLUSH DONE");
            DEBUGT_PRINT(" Post EBD_thread flush sem");
            sem_post(&in_flush_sem);
            DEBUGT_PRINT(" Post FBD_thread flush sem");
            sem_post(&out_flush_sem);
        }
        else
            seq_enabled = 0;
    }
    else if(strstr(seq_command, "disable_op") == seq_command)
    {
        DEBUGT_PRINT(" $$$$$   DISABLE OP PORT    $$$$$");
        data_str = (char*)seq_command + strlen("disable_op") + 1;
        data = atoi(data_str);
        DEBUGT_PRINT(" After frame number %u", data);
        cmd_data = data;
        sem_wait(&seq_sem);
        DEBUGT_PRINT(" Sending DISABLE OP cmd to OMX compt");
        if (disable_output_port() != 0)
        {
            DEBUGT_PRINT_ERROR(" ERROR: While DISABLE OP...");
            do_freeHandle_and_clean_up(true);
            return -1;
        }
        else
            DEBUGT_PRINT(" EventHandler for DISABLE OP");
    }
    else if(strstr(seq_command, "enable_op") == seq_command)
    {
        DEBUGT_PRINT(" $$$$$   ENABLE OP PORT    $$$$$");
        data_str = (char*)seq_command + strlen("enable_op") + 1;
        DEBUGT_PRINT(" Sending ENABLE OP cmd to OMX compt");
        if (enable_output_port() != 0)
        {
            DEBUGT_PRINT_ERROR(" ERROR: While ENABLE OP...");
            do_freeHandle_and_clean_up(true);
            return -1;
        }
        else
            DEBUGT_PRINT(" EventHandler for ENABLE OP");
    }
    else
    {
        DEBUGT_PRINT(" $$$$$   INVALID CMD    $$$$$");
        DEBUGT_PRINT(" seq_command[%s] is invalid", seq_command);
        seq_enabled = 0;
    }
    return 0;
}

void PrintFramePackArrangement(OMX_QCOM_FRAME_PACK_ARRANGEMENT framePackingArrangement)
{
   DEBUGT_PRINT("id (%d)",
          framePackingArrangement.id);
   DEBUGT_PRINT("cancel_flag (%d)",
          framePackingArrangement.cancel_flag);
   DEBUGT_PRINT("type (%d)",
          framePackingArrangement.type);
   DEBUGT_PRINT("quincunx_sampling_flag (%d)",
          framePackingArrangement.quincunx_sampling_flag);
   DEBUGT_PRINT("content_interpretation_type (%d)",
          framePackingArrangement.content_interpretation_type);
   DEBUGT_PRINT("spatial_flipping_flag (%d)",
          framePackingArrangement.spatial_flipping_flag);
   DEBUGT_PRINT("frame0_flipped_flag (%d)",
          framePackingArrangement.frame0_flipped_flag);
   DEBUGT_PRINT("field_views_flag (%d)",
          framePackingArrangement.field_views_flag);
   DEBUGT_PRINT("current_frame_is_frame0_flag (%d)",
          framePackingArrangement.current_frame_is_frame0_flag);
   DEBUGT_PRINT("frame0_self_contained_flag (%d)",
          framePackingArrangement.frame0_self_contained_flag);
   DEBUGT_PRINT("frame1_self_contained_flag (%d)",
          framePackingArrangement.frame1_self_contained_flag);
   DEBUGT_PRINT("frame0_grid_position_x (%d)",
          framePackingArrangement.frame0_grid_position_x);
   DEBUGT_PRINT("frame0_grid_position_y (%d)",
          framePackingArrangement.frame0_grid_position_y);
   DEBUGT_PRINT("frame1_grid_position_x (%d)",
          framePackingArrangement.frame1_grid_position_x);
   DEBUGT_PRINT("frame1_grid_position_y (%d)",
          framePackingArrangement.frame1_grid_position_y);
   DEBUGT_PRINT("reserved_byte (%d)",
          framePackingArrangement.reserved_byte);
   DEBUGT_PRINT("repetition_period (%d)",
          framePackingArrangement.repetition_period);
   DEBUGT_PRINT("extension_flag (%d)",
          framePackingArrangement.extension_flag);
}

void cbFileSourceStatus(FileSourceCallBackStatus status, void* pCbData)
{
    (void) pCbData;
    switch (status)
    {
    case FILE_SOURCE_OPEN_COMPLETE:
        mStatus = FILE_SOURCE_SUCCESS;
        break;
    case FILE_SOURCE_OPEN_FAIL:
        mStatus = FILE_SOURCE_FAIL;
        break;
    case FILE_SOURCE_OPEN_DATA_UNDERRUN:
        mStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
        break;
    case FILE_SOURCE_SEEK_COMPLETE:
        mStatus = FILE_SOURCE_SUCCESS;
        break;
    case FILE_SOURCE_SEEK_FAIL:
        mStatus = FILE_SOURCE_FAIL;
        break;
    default:
        mStatus = FILE_SOURCE_FAIL;
        break;
    }
    return;
}

#ifdef READ_FROM_SOCKET
void* rtp_thread(void* pArg)
{
   uint8 *buffer;
   uint32 nBytes = 0;
   uint32 max_size = 0;
   FileSourceMediaStatus error = FILE_SOURCE_DATA_OK;
   FileSourceStatus status;
   uint32 ulCount = 0;
   struct RTP_Socket *rtpsocket = (struct RTP_Socket *)pArg;
   place_marker(" === new rtp socket");
   switch(socketaddr)
    {
      case 1: /*for h264 ipv4*/
      {
        DEBUGT_PRINT("start RTPDataSource with payload h264 ipv4");
        m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_H264,
           false, 0, 0, nDestIP, 0, RTCPINTERVAL, 0, destIP, 0, 0, nDestIP, destIP, 0));
      }
      break;
      case 2: /*for h264 ipv6*/
      {
        DEBUGT_PRINT("start RTPDataSource with payload h264 ipv6");
        m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_H264,
            false, 0, 0, nDestIP, 0, RTCPINTERVAL, 1, destIP, 16, 0, nDestIP, destIP, 0));
      }
      break;
      case 3: /*for ts h264 ipv4*/
      {
         DEBUGT_PRINT("start RTPDataSource with payload MPEG2TS ipv4");
         m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_MPEG2TS,
             false, 0, 0, nDestIP, 0, RTCPINTERVAL, 0, destIP, 0, 0, nDestIP, destIP, 0));
      }
      break;
      case 4: /*for multicast ts h264 ipv4*/
      {
         int nRTCPsocket = 0;
         nRTCPsocket = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
         DEBUGT_PRINT("start RTPDataSource with payload multicast MPEG2TS ipv4");
         m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_MPEG2TS,
             false, 0, nDestPort, nDestIP, nRTCPsocket, RTCPINTERVAL, 0, destIP, 0, 1, nDestIP, destIP, 0));
      }
      break;
      case 5: /*for multicast ts h264 ipv6*/
      {
         int nRTCPsocket = 0;
         nRTCPsocket = socket(AF_INET6, SOCK_DGRAM,IPPROTO_UDP);
         DEBUGT_PRINT("start RTPDataSource with payload multicast MPEG2TS ipv4");
         m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_MPEG2TS,
             false, 0, nDestPort, nDestIP, nRTCPsocket, RTCPINTERVAL, 1, destIP, 16, 1, nDestIP, destIP, 16));
      }
      break;
      case 6: /*for multicast h264 ipv6*/
      {
         int nRTCPsocket = 0;
         nRTCPsocket = socket(AF_INET6, SOCK_DGRAM,IPPROTO_UDP);
         DEBUGT_PRINT("start RTPDataSource with payload multicast h264 ipv6");
         m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_H264,
             false, 0, nDestPort, nDestIP, nRTCPsocket, RTCPINTERVAL, 1, destIP, 16, 1, nDestIP, destIP, 16));
      }
      break;
      case 7: /*for ts h264 ipv6*/
      {
         int nRTCPsocket = 0;
         nRTCPsocket = socket(AF_INET6, SOCK_DGRAM,IPPROTO_UDP);
         DEBUGT_PRINT("start RTPDataSource with payload MPEG2TS ipv6");
         m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_MPEG2TS,
             false, 0, nDestPort, nDestIP, nRTCPsocket, RTCPINTERVAL, 1, destIP, 16, 0, nDestIP, destIP, 0));
      }
      break;
      case 8: /*for multicast h264 ipv4*/
      {
         int nRTCPsocket = 0;
         nRTCPsocket = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
         DEBUGT_PRINT("start RTPDataSource with payload multicast h264 ipv4");
         m_pRTPDecoder = MM_New_Args(RTPDataSource, (rtpsocket->nLocalPort, RTP_PAYLOAD_H264,
             false, 0, nDestPort, nDestIP, nRTCPsocket, RTCPINTERVAL, 0, destIP, 0, 1, nDestIP, destIP, 0));
      }
      break;
      default:
      break;
    }

   if(m_pRTPDecoder->start() != OK)
   {
      DEBUGT_PRINT_ERROR("MM-RTP-RECEIVER: RTP decoder start failed");
      return NULL;
   }
   bool firstrtp = FALSE;
   place_marker(" ===socket is started");
   if (socketaddr < 3 || socketaddr == 6 || socketaddr == 8) {
     buffer = (uint8 *)malloc(READ_BUFFER_LEN);
     while(! bRtpReceiverStopped)
     {
        nBytes = m_pRTPDecoder->readH264Nal(buffer);
        if(firstrtp == FALSE)
        {
          place_marker(" ===first rtp is received");
          firstrtp = TRUE;
        }
        if (nBytes > 0) {
            if(gpio_value) {
              struct queue_buffer *queue_buffer =
                     (struct queue_buffer *) malloc(sizeof(struct queue_buffer));
              queue_buffer->buffer = buffer;
              queue_buffer->len = nBytes;

              pthread_mutex_lock(&rtp_lock);
              push_queue(queue_buffer);
              DEBUGT_PRINT("queue rtp h64 buffer size %d queulen %d\n",
                 queue_buffer->len, queue_length());
              pthread_mutex_unlock(&rtp_lock);

              buffer = (uint8 *)malloc(READ_BUFFER_LEN);
            }
            else{
              memset(buffer, 0, READ_BUFFER_LEN);
            }
        }
        else {
          //DEBUGT_PRINT("no readH264Nal data ");
          usleep(1000);
        }
     }
  } else {
    int i;
    m_pSourcePort = new SourcePort(m_pRTPDecoder);
    m_pFileSource = new FileSource(cbFileSourceStatus, NULL, false);
    do {
        status = m_pFileSource->OpenFile(m_pSourcePort, FILE_SOURCE_MP2TS, true);
        if(FILE_SOURCE_SUCCESS != status) {
            return NULL;
        }
        if(FILE_SOURCE_DATA_NOTAVAILABLE == status) {
            usleep(1000); //! Sleep for 1ms
            ulCount++;
        }
        if(ulCount > 5) {
            return NULL;
        }
    } while(FILE_SOURCE_DATA_NOTAVAILABLE == status);
    FileSourceTrackIdInfoType trackList[FILE_SOURCE_MAX_NUM_TRACKS];
    uint32_t numTracks = m_pFileSource->GetWholeTracksIDList(trackList);
    DEBUGT_PRINT("numTracks = %d\n",numTracks);
    FileSourceMjMediaType majorType;
    FileSourceMnMediaType minorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
    FileSourceTrackIdInfoType trackInfo = {0};
    for (i = 0;i < numTracks ; i++)
    {
        trackInfo = trackList[i];
        error = FILE_SOURCE_DATA_OK;
        DEBUGT_PRINT("i %d, trackInfo.id %d\n",i,trackInfo.id);
        FileSourceStatus status = m_pFileSource->GetMimeType(trackInfo.id, majorType, minorType);
        if (majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
        {
          DEBUGT_PRINT("video tracks = %d",i);
          break;
        }
    }
    if (i == numTracks) {
      printf("no video tracks\n");
      return NULL;
    }
    max_size = m_pFileSource->GetTrackMaxFrameBufferSize(trackInfo.id);
    buffer = (uint8 *)malloc(max_size);
    if (buffer == NULL)
    {
      DEBUGT_PRINT_ERROR("buffer allocation failed");
      return NULL;
    }
    nBytes = max_size;
    while(! bRtpReceiverStopped && FILE_SOURCE_DATA_OK == error)
    {
      uint32 index = 0;
      FileSourceSampleInfo sampleInfo;
      memset(&sampleInfo, 0, sizeof(FileSourceSampleInfo));
      error = m_pFileSource->GetNextMediaSample(trackInfo.id,
                                                       (uint8 *)buffer,
                                                       &nBytes,
                                                       sampleInfo);
     if(currentTime == 0)
        currentTime = sampleInfo.startTime;
     else
     {
        diffTime = sampleInfo.startTime - currentTime;
        currentTime = sampleInfo.startTime;
     }
     if(error != FILE_SOURCE_DATA_OK) {
        DEBUGT_PRINT_ERROR("buf_len %d, error %p, i %d, trackInfo.id %d",nBytes,error,i, trackInfo.id);
        if (buffer)
          free (buffer);
        return NULL;
      }
      if (nBytes > 0) {
          if(firstrtp == FALSE)
          {
            place_marker(" ===first rtp is received");
            firstrtp = TRUE;
          }
          if(gpio_value) {
            struct queue_buffer *queue_buffer =
               (struct queue_buffer *) malloc(sizeof(struct queue_buffer));
            queue_buffer->buffer = buffer;
            queue_buffer->len = nBytes;

            pthread_mutex_lock(&rtp_lock);
            push_queue(queue_buffer);
            DEBUGT_PRINT("queue rtp h64 buffer size %d queulen %d",
               queue_buffer->len, queue_length());
            pthread_mutex_unlock(&rtp_lock);

            buffer = (uint8 *)malloc(max_size);
            nBytes = max_size;
          }
          else {
            memset(buffer, 0, max_size);
            nBytes = max_size;
          }
      } else {
        DEBUGT_PRINT("no ts data");
        usleep(1000);
      }
    }
  }
  return NULL;
}
#endif

void* ebd_thread(void* pArg)
{
  (void)pArg;
  int signal_eos = 0;
  uint32_t end_playback = 0;
  while(currentStatus != ERROR_STATE)
  {
    int readBytes =0;
    OMX_BUFFERHEADERTYPE* pBuffer = NULL;
#ifdef _ANDROID_
#ifdef DISABLE_AFTER_BOOT
    static char boot_completed[PROPERTY_VALUE_MAX] = {0};
    property_get("sys.boot_completed", boot_completed, "0");
    end_playback = atoi(boot_completed);
    if (end_playback)
        place_marker(" === ebd_thread: end_playback");
#endif
#endif
    if(flush_input_progress)
    {
        DEBUGT_PRINT(" EBD_thread flush wait start");
        sem_wait(&in_flush_sem);
        DEBUGT_PRINT(" EBD_thread flush wait complete");
    }

    sem_wait(&etb_sem);
    pthread_mutex_lock(&etb_lock);
    pBuffer = (OMX_BUFFERHEADERTYPE *) pop(etb_queue);
    pthread_mutex_unlock(&etb_lock);
    if(pBuffer == NULL)
    {
      DEBUGT_PRINT_ERROR("Error - No etb pBuffer to dequeue");
      continue;
    }

    if (num_frames_to_decode && (etb_count >= num_frames_to_decode)) {
        DEBUGT_PRINT(" Signal EOS %d frames decoded ", num_frames_to_decode);
       signal_eos = 1;
    }

    pBuffer->nOffset = 0;
    if(((readBytes = Read_Buffer(pBuffer)) > 0) && !signal_eos && !end_playback) {
        pBuffer->nFilledLen = readBytes;
        DEBUGT_PRINT("%s: Timestamp sent(%lld)", __FUNCTION__, pBuffer->nTimeStamp);
        OMX_EmptyThisBuffer(dec_handle,pBuffer);
        etb_count++;
    }
    else
    {
        pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        bInputEosReached = true;
        pBuffer->nFilledLen = readBytes;
        DEBUGT_PRINT("%s: Timestamp sent(%lld)", __FUNCTION__, pBuffer->nTimeStamp);
        OMX_EmptyThisBuffer(dec_handle,pBuffer);
        DEBUGT_PRINT("EBD::Either EOS or Some Error while reading file");
        etb_count++;
        m_pFileSource->CloseFile();
        break;
    }
  }
  return NULL;
}

void* fbd_thread(void* pArg)
{
  (void)pArg;
  long unsigned act_time = 0, display_time = 0, render_time = 10e3, lipsync = 15e3;
  struct timespec t_avsync = {0, 0}, base_avsync = {0, 0};
  float total_time = 0;
  int canDisplay = 1, contiguous_drop_frame = 0, bytes_written = 0, ret = 0;
  OMX_S64 base_timestamp = 0, lastTimestamp = 0;
  OMX_BUFFERHEADERTYPE *pBuffer = NULL, *pPrevBuff = NULL;
  char value[PROPERTY_VALUE_MAX] = {0};
  uint32_t end_playback = 0;
  OMX_U32 aspectratio_prop = 0;
  pthread_mutex_lock(&eos_lock);
  DEBUGT_PRINT("First Inside %s", __FUNCTION__);
#ifdef _ANDROID_
  property_get("vidc.vdec.debug.aspectratio", value, "0");
#endif
  aspectratio_prop = atoi(value);
  DEBUGT_PRINT("Inside %s", __FUNCTION__);

  while(currentStatus != ERROR_STATE && !bOutputEosReached)
  {
#ifdef _ANDROID_
#ifdef DISABLE_AFTER_BOOT
    static char boot_completed[PROPERTY_VALUE_MAX] = {0};
    property_get("sys.boot_completed", boot_completed, "0");
    end_playback = atoi(boot_completed);
    if (end_playback)
        place_marker(" === fbd_thread: end_playback");
#endif
#endif
    pthread_mutex_unlock(&eos_lock);
    if(flush_output_progress)
    {
        DEBUGT_PRINT(" FBD_thread flush wait start");
        sem_wait(&out_flush_sem);
        DEBUGT_PRINT(" FBD_thread flush wait complete");
    }
    sem_wait(&fbd_sem);
    pthread_mutex_lock(&enable_lock);
    if (sent_disabled)
    {
      pthread_mutex_unlock(&enable_lock);
      pthread_mutex_lock(&fbd_lock);
      if (pPrevBuff != NULL ) {
        if(push(fbd_queue, (void *)pBuffer))
            DEBUGT_PRINT_ERROR("Error in enqueueing fbd_data");
        else
            sem_post(&fbd_sem);
        pPrevBuff = NULL;
      }
      if (free_op_buf_cnt == portFmt.nBufferCountActual)
        free_output_buffers();
      pthread_mutex_unlock(&fbd_lock);
      pthread_mutex_lock(&eos_lock);
      continue;
    }
    pthread_mutex_unlock(&enable_lock);
    if (anti_flickering)
      pPrevBuff = pBuffer;
    pthread_mutex_lock(&fbd_lock);
    pBuffer = (OMX_BUFFERHEADERTYPE *)pop(fbd_queue);
    pthread_mutex_unlock(&fbd_lock);
    if (pBuffer == NULL)
    {
      if (anti_flickering)
        pBuffer = pPrevBuff;
      DEBUGT_PRINT("Error - No pBuffer to dequeue");
      pthread_mutex_lock(&eos_lock);
      continue;
    }
    else if (pBuffer->nFilledLen > 0)
    {
      if (!frame_count)
      {
        clock_gettime(CLOCK_MONOTONIC, &t_start);
      }
      fbd_cnt++;
      frame_count++;

      if (frame_count > 31) {
        clock_gettime(CLOCK_MONOTONIC, &t_end);
        total_time = ((float) ((t_end.tv_sec - t_start.tv_sec) * 1e9
                     + t_end.tv_nsec - t_start.tv_nsec))/ 1e9;
        //total frames is frame_count - 1 since the start time is
        //recorded after the first frame is decoded.
        DEBUGT_PRINT_INFO("NumFrames = %d, total_time %f, Average frame rate = %f", (frame_count - 1), total_time, (frame_count - 1)/total_time);
        DEBUGT_PRINT_INFO("Number of frames dropped so far: %d", frames_dropped);
        frame_count = 0;
      }

      canDisplay = 1;
      if (realtime_display)
      {
        lastTimestamp = pBuffer->nTimeStamp;
        clock_gettime(CLOCK_MONOTONIC, &t_avsync);
        if ((!base_avsync.tv_sec && !base_avsync.tv_nsec) || // First frame
            (pBuffer->nTimeStamp < lastTimestamp)) // Just after a seek to the beginning
        {
          display_time = 0;
          base_avsync = t_avsync;
          base_timestamp = pBuffer->nTimeStamp;
          DEBUGT_PRINT("base_avsync Sec(%lu) nSec(%lu) base_timestamp(%lld)",
              base_avsync.tv_sec, base_avsync.tv_nsec, base_timestamp);
        }
        else
        {
          act_time = ((t_avsync.tv_sec - base_avsync.tv_sec) * 1e9
                     + t_avsync.tv_nsec - base_avsync.tv_nsec)/1e3; // usec
          display_time = pBuffer->nTimeStamp - base_timestamp;
          DEBUGT_PRINT("%s: act_time(%lu) display_time(%lu), pBuffer->nTimeStamp(%lld)",
              __FUNCTION__, act_time, display_time, pBuffer->nTimeStamp);
               //Frame rcvd on time
          if (((act_time + render_time) >= (display_time - lipsync) &&
               (act_time + render_time) <= (display_time + lipsync)) ||
               //Display late frame
               (contiguous_drop_frame > 5))
              display_time = 0;
          else if ((act_time + render_time) < (display_time - lipsync))
              //Delaying early frame
              display_time -= (lipsync + act_time + render_time);
          else
          {
              //Dropping late frame
              canDisplay = 0;
              frames_dropped++;
              contiguous_drop_frame++;
          }
        }
      }

      display_marker();

      if (bmarkfirstdisplay == TRUE) {
        place_marker(" === render first frame");
        bmarkfirstdisplay = FALSE;
      }
      if (displayYuv && canDisplay)
      {
          if (display_time)
              usleep(display_time);

          ret = display_render(pBuffer, portFmt.format.video.nFrameWidth, portFmt.format.video.nFrameHeight, end_playback);
          if (ret != 0)
          {
            DEBUGT_PRINT_ERROR("ERROR in display, disabling display!");
            close_display();
            displayYuv = 0;
          }
          usleep(render_time);
          contiguous_drop_frame = 0;
      }

      if (takeYuvLog)
      {
        if (color_fmt == (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m)
          {
             DEBUGT_PRINT(" width: %d height: %d", crop_rect.nWidth, crop_rect.nHeight);
             unsigned int stride = VENUS_Y_STRIDE(COLOR_FMT_NV12, portFmt.format.video.nFrameWidth);
             unsigned int scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12, portFmt.format.video.nFrameHeight);
             char *temp = (char *) pBuffer->pBuffer;
             int i = 0;

             temp += (stride * (int)crop_rect.nTop) +  (int)crop_rect.nLeft;
             for (i = 0; i < crop_rect.nHeight; i++) {
                bytes_written = fwrite(temp, crop_rect.nWidth, 1, outputBufferFile);
                temp += stride;
             }

             temp = (char *)pBuffer->pBuffer + stride * scanlines;
             temp += (stride * (int)crop_rect.nTop) +  (int)crop_rect.nLeft;
             for(i = 0; i < crop_rect.nHeight/2; i++) {
                 bytes_written += fwrite(temp, crop_rect.nWidth, 1, outputBufferFile);
                 temp += stride;
             }
         }
         else
         {
             bytes_written = fwrite((const char *)pBuffer->pBuffer,
                       pBuffer->nFilledLen,1,outputBufferFile);
         }
          if (bytes_written < 0) {
              DEBUGT_PRINT("FillBufferDone: Failed to write to the file");
          }
          else {
              DEBUGT_PRINT("FillBufferDone: Wrote %d YUV bytes to the file",
                            bytes_written);
          }
      }
#ifdef _MSM8974_
      if (crcFile) {
          uint16 crc_val;
          crc_val = crc_16_l_step_nv12(CRC_INIT, pBuffer->pBuffer,
                  pBuffer->nFilledLen, height, width);
          int num_bytes = fwrite(&crc_val, 1, sizeof(crc_val), crcFile);
          if (num_bytes < sizeof(crc_val)) {
              DEBUGT_PRINT_ERROR("Failed to write CRC value into file");
          }
      }
#endif
      if (pBuffer->nFlags & OMX_BUFFERFLAG_EXTRADATA)
      {
        OMX_OTHER_EXTRADATATYPE *pExtra;
        DEBUGT_PRINT(">> BUFFER WITH EXTRA DATA RCVD <<<");
        pExtra = (OMX_OTHER_EXTRADATATYPE *)
                 ((size_t)(pBuffer->pBuffer + pBuffer->nOffset +
                  pBuffer->nFilledLen + 3)&(~3));
        while(pExtra &&
              (OMX_U8*)pExtra < (pBuffer->pBuffer + pBuffer->nAllocLen) &&
              pExtra->eType != OMX_ExtraDataNone )
        {
          DEBUGT_PRINT("ExtraData : pBuf(%p) BufTS(%lld) Type(%x) DataSz(%u)",
               pBuffer, pBuffer->nTimeStamp, pExtra->eType, pExtra->nDataSize);
          switch (pExtra->eType)
          {
            case OMX_ExtraDataInterlaceFormat:
            {
              OMX_STREAMINTERLACEFORMAT *pInterlaceFormat = (OMX_STREAMINTERLACEFORMAT *)pExtra->data;
              DEBUGT_PRINT("OMX_ExtraDataInterlaceFormat: Buf(%p) TSmp(%lld) IntPtr(%p) Fmt(%x)",
                pBuffer->pBuffer, pBuffer->nTimeStamp,
                pInterlaceFormat, pInterlaceFormat->nInterlaceFormats);
              break;
            }
            case OMX_ExtraDataFrameInfo:
            {
              OMX_QCOM_EXTRADATA_FRAMEINFO *frame_info = (OMX_QCOM_EXTRADATA_FRAMEINFO *)pExtra->data;
              DEBUGT_PRINT("OMX_ExtraDataFrameInfo: Buf(%p) TSmp(%lld) PicType(%u) IntT(%u) ConMB(%u)",
                pBuffer->pBuffer, pBuffer->nTimeStamp, frame_info->ePicType,
                frame_info->interlaceType, frame_info->nConcealedMacroblocks);
              if (aspectratio_prop)
                DEBUGT_PRINT_ERROR(" FrmRate(%u), AspRatioX(%u), AspRatioY(%u) DispWidth(%u) DispHeight(%u)",
                frame_info->nFrameRate, frame_info->aspectRatio.aspectRatioX,
                frame_info->aspectRatio.aspectRatioY, frame_info->displayAspectRatio.displayHorizontalSize,
                frame_info->displayAspectRatio.displayVerticalSize);
              else
                DEBUGT_PRINT(" FrmRate(%u), AspRatioX(%u), AspRatioY(%u) DispWidth(%u) DispHeight(%u)",
                frame_info->nFrameRate, frame_info->aspectRatio.aspectRatioX,
                frame_info->aspectRatio.aspectRatioY, frame_info->displayAspectRatio.displayHorizontalSize,
                frame_info->displayAspectRatio.displayVerticalSize);
              DEBUGT_PRINT("PANSCAN numWindows(%d)", frame_info->panScan.numWindows);
              for (int i = 0; i < frame_info->panScan.numWindows; i++)
              {
                DEBUGT_PRINT("WINDOW Lft(%d) Tp(%d) Rgt(%d) Bttm(%d)",
                  frame_info->panScan.window[i].x,
                  frame_info->panScan.window[i].y,
                  frame_info->panScan.window[i].dx,
                  frame_info->panScan.window[i].dy);
              }
              break;
            }
            break;
            case OMX_ExtraDataConcealMB:
            {
              OMX_U8 data = 0, *data_ptr = (OMX_U8 *)pExtra->data;
              OMX_U32 concealMBnum = 0, bytes_cnt = 0;
              while (bytes_cnt < pExtra->nDataSize)
              {
                data = *data_ptr;
                while (data)
                {
                  concealMBnum += (data&0x01);
                  data >>= 1;
                }
                data_ptr++;
                bytes_cnt++;
              }
              DEBUGT_PRINT("OMX_ExtraDataConcealMB: Buf(%p) TSmp(%lld) ConcealMB(%u)",
                pBuffer->pBuffer, pBuffer->nTimeStamp, concealMBnum);
            }
            break;
            case OMX_ExtraDataMP2ExtnData:
            {
              DEBUGT_PRINT("OMX_ExtraDataMP2ExtnData");
              OMX_U8 *data_ptr = (OMX_U8 *)pExtra->data;
              OMX_U32 bytes_cnt = 0;
              while (bytes_cnt < pExtra->nDataSize)
              {
                DEBUGT_PRINT(" MPEG-2 Extension Data Values[%d] = 0x%x", bytes_cnt, *data_ptr);
                data_ptr++;
                bytes_cnt++;
              }
            }
            break;
            case OMX_ExtraDataMP2UserData:
            {
              DEBUGT_PRINT("OMX_ExtraDataMP2UserData");
              OMX_U8 *data_ptr = (OMX_U8 *)pExtra->data;
              OMX_U32 bytes_cnt = 0;
              while (bytes_cnt < pExtra->nDataSize)
              {
                DEBUGT_PRINT(" MPEG-2 User Data Values[%d] = 0x%x", bytes_cnt, *data_ptr);
                data_ptr++;
                bytes_cnt++;
              }
            }
            break;
            default:
              DEBUGT_PRINT_ERROR("Unknown Extrata!");
          }
          if (pExtra->nSize < (pBuffer->nAllocLen - (size_t)pExtra))
            pExtra = (OMX_OTHER_EXTRADATATYPE *) (((OMX_U8 *) pExtra) + pExtra->nSize);
          else
          {
            DEBUGT_PRINT_ERROR("ERROR: Extradata pointer overflow buffer(%p) extra(%p)",
              pBuffer, pExtra);
            pExtra = NULL;
          }
        }
      }
    }
    if(pBuffer->nFlags & QOMX_VIDEO_BUFFERFLAG_EOSEQ)
    {
        printf("\n");
        printf("***************************************************\n");
        printf("FillBufferDone: End Of Sequence Received\n");
        printf("***************************************************\n");
    }
    if(pBuffer->nFlags & OMX_BUFFERFLAG_DATACORRUPT)
    {
      printf("\n");
      printf("***************************************************\n");
      printf("FillBufferDone: OMX_BUFFERFLAG_DATACORRUPT Received\n");
      printf("***************************************************\n");
    }
    /********************************************************************/
    /* De-Initializing the open max and relasing the buffers and */
    /* closing the files.*/
    /********************************************************************/
    if ((pBuffer->nFlags & OMX_BUFFERFLAG_EOS) || end_playback)
    {
      OMX_QCOM_FRAME_PACK_ARRANGEMENT framePackingArrangement;
      OMX_GetConfig(dec_handle,
                   (OMX_INDEXTYPE)OMX_QcomIndexConfigVideoFramePackingArrangement,
                    &framePackingArrangement);
      PrintFramePackArrangement(framePackingArrangement);

      printf("***************************************************\n");
      printf("FillBufferDone: End Of Stream Reached\n");
      printf("***************************************************\n");
      pthread_mutex_lock(&eos_lock);
      bOutputEosReached = true;
      break;
    }

    pthread_mutex_lock(&enable_lock);
    if (flush_output_progress || sent_disabled)
    {
        pBuffer->nFilledLen = 0;
        pBuffer->nFlags &= ~OMX_BUFFERFLAG_EXTRADATA;
        pthread_mutex_lock(&fbd_lock);
        if ( pPrevBuff != NULL ) {
            if(push(fbd_queue, (void *)pPrevBuff))
                DEBUGT_PRINT_ERROR("Error in enqueueing fbd_data");
            else
               sem_post(&fbd_sem);
            pPrevBuff = NULL;
        }
        if(push(fbd_queue, (void *)pBuffer) < 0)
        {
          DEBUGT_PRINT_ERROR("Error in enqueueing fbd_data");
        }
        else
          sem_post(&fbd_sem);
        pthread_mutex_unlock(&fbd_lock);
    }
    else
    {
        if (!anti_flickering)
          pPrevBuff = pBuffer;
        if (pPrevBuff)
        {
          pthread_mutex_lock(&fbd_lock);
          pthread_mutex_lock(&eos_lock);
          if (!bOutputEosReached)
          {
              if ( OMX_FillThisBuffer(dec_handle, pPrevBuff) == OMX_ErrorNone ) {
                  free_op_buf_cnt--;
              }
          }
          pthread_mutex_unlock(&eos_lock);
          pthread_mutex_unlock(&fbd_lock);
        }
    }
    pthread_mutex_unlock(&enable_lock);
    if(cmd_data <= fbd_cnt)
    {
      sem_post(&seq_sem);
      DEBUGT_PRINT("Posted seq_sem Frm(%d) Req(%d)", fbd_cnt, cmd_data);
      cmd_data = ~(unsigned)0;
    }
    pthread_mutex_lock(&eos_lock);
  }
  if(seq_enabled)
  {
      seq_enabled = 0;
      sem_post(&seq_sem);
      DEBUGT_PRINT("Posted seq_sem in EOS ");
  }
  pthread_cond_broadcast(&eos_cond);
  pthread_mutex_unlock(&eos_lock);
  return NULL;
}

OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                           OMX_IN OMX_PTR pAppData,
                           OMX_IN OMX_EVENTTYPE eEvent,
                           OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
                           OMX_IN OMX_PTR pEventData)
{
    DEBUGT_PRINT("Function %s ", __FUNCTION__);

    switch(eEvent) {
        case OMX_EventCmdComplete:
            DEBUGT_PRINT(" OMX_EventCmdComplete ");
            if(OMX_CommandPortDisable == (OMX_COMMANDTYPE)nData1)
            {
                printf("*********************************************\n");
                printf("Recieved DISABLE Event Command Complete[%d]\n",nData2);
                printf("*********************************************\n");
            }
            else if(OMX_CommandPortEnable == (OMX_COMMANDTYPE)nData1)
            {
                printf("*********************************************\n");
                printf("Recieved ENABLE Event Command Complete[%d]\n",nData2);
                printf("*********************************************\n");
                if (currentStatus == PORT_SETTING_CHANGE_STATE)
                  currentStatus = GOOD_STATE;
                pthread_mutex_lock(&enable_lock);
                sent_disabled = 0;
                pthread_mutex_unlock(&enable_lock);
            }
            else if(OMX_CommandFlush == (OMX_COMMANDTYPE)nData1)
            {
                printf("*********************************************\n");
                printf("Received FLUSH Event Command Complete[%d]\n",nData2);
                printf("*********************************************\n");
                if (nData2 == 0)
                    flush_input_progress = 0;
                else if (nData2 == 1)
                    flush_output_progress = 0;
            }
            if (!flush_input_progress && !flush_output_progress)
                event_complete();
            break;

        case OMX_EventError:
            printf("*********************************************\n");
            printf("Received OMX_EventError Event Command !\n");
            printf("*********************************************\n");
            currentStatus = ERROR_STATE;
            place_marker(" === OMX_EventError");
            if (OMX_ErrorInvalidState == (OMX_ERRORTYPE)nData1 ||
                OMX_ErrorHardware == (OMX_ERRORTYPE)nData1)
            {
              DEBUGT_PRINT("Invalid State or hardware error ");
              if(event_is_done == 0)
              {
                DEBUGT_PRINT("Event error in the middle of Decode ");
                pthread_mutex_lock(&eos_lock);
                bOutputEosReached = true;
                pthread_mutex_unlock(&eos_lock);
                if(seq_enabled)
                {
                    seq_enabled = 0;
                    sem_post(&seq_sem);
                    DEBUGT_PRINT(" Posted seq_sem in ERROR");
                }
              }
            }
            if (waitForPortSettingsChanged)
            {
                waitForPortSettingsChanged = 0;
                event_complete();
            }
            sem_post(&etb_sem);
            sem_post(&fbd_sem);
            break;
        case OMX_EventPortSettingsChanged:
            DEBUGT_PRINT("OMX_EventPortSettingsChanged port[%d]", nData1);
            if (nData2 == OMX_IndexConfigCommonOutputCrop) {
                OMX_U32 outPortIndex = 1;
                 if (nData1 == outPortIndex) {
                     crop_rect.nPortIndex = outPortIndex;
                     OMX_ERRORTYPE ret = OMX_GetConfig(dec_handle,
                                                       OMX_IndexConfigCommonOutputCrop, &crop_rect);
                     if (FAILED(ret)) {
                         DEBUGT_PRINT_ERROR("Failed to get crop rectangle");
                         break;
                     } else
                         DEBUGT_PRINT("Got Crop Rect: (%d, %d) (%d x %d)",
                             crop_rect.nLeft, crop_rect.nTop, crop_rect.nWidth, crop_rect.nHeight);
                 }
                 currentStatus = GOOD_STATE;
                 break;
            }

#ifdef _MSM8974_
            if (nData2 != OMX_IndexParamPortDefinition)
              break;
#endif
            currentStatus = PORT_SETTING_CHANGE_STATE;
            if (waitForPortSettingsChanged)
            {
                waitForPortSettingsChanged = 0;
                event_complete();
            }
            else
            {
                pthread_mutex_lock(&eos_lock);
                pthread_cond_broadcast(&eos_cond);
                pthread_mutex_unlock(&eos_lock);
            }
            break;

        case OMX_EventBufferFlag:
            DEBUGT_PRINT("OMX_EventBufferFlag port[%d] flags[%x]", nData1, nData2);
            break;
        case OMX_EventIndexsettingChanged:
            DEBUGT_PRINT("OMX_EventIndexSettingChanged Interlace mode[%x]", nData1);
            break;
        default:
            DEBUGT_PRINT_ERROR("ERROR - Unknown Event ");
            break;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                              OMX_IN OMX_PTR pAppData,
                              OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    DEBUGT_PRINT("Function %s cnt[%d]", __FUNCTION__, ebd_cnt);
    ebd_cnt++;

    if(bInputEosReached) {
        printf("*****EBD:Input EoS Reached************\n");
        return OMX_ErrorNone;
    }

    pthread_mutex_lock(&etb_lock);
    if(push(etb_queue, (void *) pBuffer) < 0)
    {
       DEBUGT_PRINT_ERROR("Error in enqueue  ebd data");
       return OMX_ErrorUndefined;
    }
    pthread_mutex_unlock(&etb_lock);
    sem_post(&etb_sem);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE FillBufferDone(OMX_OUT OMX_HANDLETYPE hComponent,
                             OMX_OUT OMX_PTR pAppData,
                             OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    DEBUGT_PRINT("Inside %s callback_count[%d] ", __FUNCTION__, fbd_cnt);

    /* Test app will assume there is a dynamic port setting
     * In case that there is no dynamic port setting, OMX will not call event cb,
     * instead OMX will send empty this buffer directly and we need to clear an event here
     */
    if(waitForPortSettingsChanged)
    {
      waitForPortSettingsChanged = 0;
      event_complete();
    }

    pthread_mutex_lock(&fbd_lock);
    free_op_buf_cnt++;
    if(push(fbd_queue, (void *)pBuffer) < 0)
    {
      pthread_mutex_unlock(&fbd_lock);
      DEBUGT_PRINT_ERROR("Error in enqueueing fbd_data");
      return OMX_ErrorUndefined;
    }
    pthread_mutex_unlock(&fbd_lock);
    sem_post(&fbd_sem);

    return OMX_ErrorNone;
}

// Print usage showing how to use this utility
static void usage(const char *me) {
    fprintf(stderr, "usage: %s\n", me);
    fprintf(stderr, "       -a first_start flag: [0] Display by Weston [1] Display by DRM (default: 1)\n");
    fprintf(stderr, "       -c Codec format: [1] H.264, [2] MPEG4, [3] H.263, [4] VC1, [5] DIVX, [6] MPEG2, [7] VP8, [8] VP9, [9] HEVC. (default: 1)\n");
    fprintf(stderr, "       -d Destination IP address (Applicable only if -m 1)\n");
    fprintf(stderr, "       -D Decode/Display order: [0] Display order [1] Decode order (default:1)\n");
    fprintf(stderr, "       -e Destination Port number (Applicable only if -m 1)\n");
    fprintf(stderr, "       -f File type: [0] RTP, [1] DATA PER AU (only for H264 and MPEG4), [2] ARBITRARY_BYTES (Should be .264/.264c/.m4v/.263/.rcv/.vc1/.m2v), [3] MP4, \
                               [11] H264 NAL_SIZE_LENGTH, [12] H264 START_CODE_BASED, [21] MPEG4 PICTURE START CODE, [31] VC1_RCV (Simple or Main profile) \
                               [32] VC1 (Advanced profile), [41] DIVX_4_5_6, [42] DIVX_311, [51] MPEG2, [61] VP8/VP9 START_CODE, [62] VP8/VP9. (default: 3)\n");
    fprintf(stderr, "       -F FPS in Real-time display mode (default: 30)\n");
    fprintf(stderr, "       -h(elp)\n");
    fprintf(stderr, "       -i File name (Not Applicable if -m 1) (default: \"/vendor/bin/bootvideo.mp4\")\n");
    fprintf(stderr, "       -m Input mode: [0] File Input [1] Ethernet Camera (default: 0)\n");
    fprintf(stderr, "       -n NAL size:[0], [2] or [4] \n");
    fprintf(stderr, "       -N Number of frames to decode (default: 0 ==> Till the end of the clip)\n");
    fprintf(stderr, "       -o Output option: [0] No display and No YUV, [1] Display YUV, [2] Take YUV log, [3] Display YUV and Take YUV log. (default: 1)\n");
    fprintf(stderr, "       -p Local port number (Applicable only if -m 1) (default: 0)\n");
    fprintf(stderr, "       -r Input resolution: [0] 1280x720, [1] 1920x1080 (default: 0)\n");
    fprintf(stderr, "       -R Real-time display mode [0] Not real-time, [1] Real-time (Applicable only if -o 1 or -o 3. YUV log will be disabled if real-time display is enabled.) (default: 0)\n");
    fprintf(stderr, "       -s Socket address (Applicable only if -m 1) (default: 0)\n");
    fprintf(stderr, "       -t Test option: [1] Play the clip till the end, [2] Run compliance test ==> \"TEST SUCCESSFUL\" indicates pass, [3] Thumbnail decode mode (default: 1)\n");
}

/*
 * Main function of the native binary executable.
 */
int main(int argc, char **argv)
{
    /***************************************************/
    /*      Default input options and arguments        */
    /***************************************************/
    int outputOption = 1;
    int test_option = 1;
    int pic_order = 1;

#ifdef READ_FROM_SOCKET
    rtp_socket.nLocalPort = 0;
    rtp_socket.nDestPort = 0;
    rtp_socket.nDestIP = 0;
    rtp_socket.nRTCPsocket = 0;
#endif

    // set up the thread-pool (No-op for Android)
    init();

    /***************************************************/
    /*      Parse input options and arguments          */
    /***************************************************/
    int res;
    while ((res = getopt(argc, argv, "a:c:d:D:e:f:F:i:m:n:N:o:p:r:R:s:t:h")) >= 0) {
        switch (res) {
            case 'a':
            {
#ifndef _ANDROID_
                first_start = atoi(optarg);
                if ((first_start != 0) && (first_start != 1))
                {
                    DEBUGT_PRINT_ERROR("Invalid first_start flag: %d\n", first_start);
                    usage(argv[0]);
                }
                DEBUGT_PRINT("Selected first_start flag: %d\n", first_start);
#endif
                break;
            }

            case 'c':
            {
                codec_format_option = (codec_format)atoi(optarg);
                if ((codec_format_option < 1) || (codec_format_option > 9))
                {
                    DEBUGT_PRINT_ERROR("Invalid codec_format_option: %d\n", codec_format_option);
                    usage(argv[0]);
                }
                DEBUGT_PRINT("Selected codec_format_option: %d\n", codec_format_option);
                break;
            }

            case 'd':
            {
#ifdef READ_FROM_SOCKET
                nDestIPStr = optarg;
                rtp_socket.nDestIP = atoi(nDestIPStr);
#endif
                break;
            }

            case 'D':
            {
                pic_order = atoi(optarg);
                if ((pic_order != 0) && (pic_order != 1))
                {
                    DEBUGT_PRINT_ERROR("Invalid pic_order: %d\n", pic_order);
                    usage(argv[0]);
                }
                break;
            }

            case 'e':
            {
#ifdef READ_FROM_SOCKET
                nDestPort = atoi(optarg);
                rtp_socket.nDestPort = nDestPort;
#endif
                break;
            }

            case 'f':
            {
                file_type_option = (file_type)atoi(optarg);
                DEBUGT_PRINT_ERROR("Selected file_type_option: %d\n", file_type_option);
                break;
            }

            case 'F':
            {
                fps = atoi(optarg);
                break;
            }

            case 'i':
            {
                in_filename = optarg;
                break;
            }

            case 'm':
            {
                operating_mode = (op_mode)atoi(optarg);
                if ((operating_mode != EARLY_VIDEO) && (operating_mode != ETHERNET_RVC))
                {
                    DEBUGT_PRINT_ERROR("Invalid operating_mode: %d\n", operating_mode);
                    usage(argv[0]);
                }
                break;
            }

            case 'n':
            {
                nalSize = atoi(optarg);
                if (nalSize != 0 && nalSize != 2 && nalSize != 4)
                {
                    DEBUGT_PRINT_ERROR("Invalid nalSize: %d\n", nalSize);
                    usage(argv[0]);
                }
                break;
            }

            case 'N':
            {
                num_frames_to_decode = atoi(optarg);
                break;
            }

            case 'o':
            {
                outputOption = atoi(optarg);
                if ((outputOption < 0) || (outputOption > 3))
                {
                    DEBUGT_PRINT_ERROR("Invalid outputOption: %d\n", outputOption);
                    usage(argv[0]);
                }
                break;
            }

            case 'p':
            {
#ifdef READ_FROM_SOCKET
                rtp_socket.nLocalPort = atoi(optarg);
#endif
                break;
            }

            case 'r':
            {
                video_resolution = (resolution)atoi(optarg);
                if ((video_resolution != RESOLUTION_720P) && (video_resolution != RESOLUTION_1080P))
                {
                    DEBUGT_PRINT_ERROR("Invalid video_resolution: %d\n", video_resolution);
                    usage(argv[0]);
                }
                break;
            }

            case 'R':
            {
                realtime_display = atoi(optarg);
                if ((realtime_display != 0) && (realtime_display != 1))
                {
                    DEBUGT_PRINT_ERROR("Invalid realtime_display flag: %d\n", realtime_display);
                    usage(argv[0]);
                }
                break;
            }

            case 's':
            {
#ifdef READ_FROM_SOCKET
                socketaddr = atoi(optarg);
#endif
                break;
            }

            case 't':
            {
                test_option = atoi(optarg);
                if ((test_option < 1) || (test_option > 3))
                {
                    DEBUGT_PRINT_ERROR("Invalid test_option: %d\n", test_option);
                    usage(argv[0]);
                }
                break;
            }

            case 'T':
            {
                freeHandle_option = (freeHandle_test)atoi(optarg);
                if ((freeHandle_option < FREE_HANDLE_AT_LOADED) || (freeHandle_option > FREE_HANDLE_AT_PAUSE))
                {
                    DEBUGT_PRINT_ERROR("Invalid freeHandle_option: %d\n", freeHandle_option);
                    usage(argv[0]);
                }
                break;
            }

            case 'h':
            default:
            {
                usage(argv[0]);
                return 0;
            }
        }
    }

    /***************************************************/
    /*   Check clip presence and exit if not present   */
    /***************************************************/
#ifdef _ANDROID_
#ifdef EARLY_BOOTVIDEO
    struct stat clipstat;
    if(stat(in_filename, &clipstat)) {
        place_marker(" === enter, disabled");
        return -1;
    } else {
        place_marker(" === enter, enabled");
    }
#endif
#endif
    /***************************************************/
    /*        Open display in case of eDRM             */
    /***************************************************/
#ifdef _ANDROID_
#ifdef EARLY_BOOTVIDEO
#ifndef NATIVEWINDOW_DISPLAY
    if(0 != pthread_create(&edrmdisplay_thread_id, NULL, open_edrmdisplay_thread, NULL))
    {
      DEBUGT_PRINT_ERROR(" Error in Creating edrmdisplay_thread ");
      return -1;
    }
    pthread_join(edrmdisplay_thread_id, NULL);
#endif
#endif
#endif
    /***************************************************/
    /*    Set parameters based on input options       */
    /***************************************************/
#ifdef READ_FROM_SOCKET
    if(socketaddr == 4 || socketaddr == 8)
    {
       nDestIP = inet_addr(nDestIPStr);
    }
    else if(socketaddr == 5 || socketaddr == 6)
    {
       inet_pton(AF_INET6, nDestIPStr, &destIP);
    }
#endif

    if (test_option == 3)
          thumbnailMode = 1;

    if(video_resolution == RESOLUTION_720P)
    {
        width = 1280;
        height = 720;
    }
    crop_rect.nLeft = 0;
    crop_rect.nTop = 0;
    crop_rect.nWidth = width;
    crop_rect.nHeight = height;

    if (realtime_display)
    {
        takeYuvLog = 0;
        timestampInterval = 1e6 / fps; // usec
    }

    if (file_type_option >= FILE_TYPE_COMMON_CODEC_MAX)
    {
      switch (codec_format_option)
      {
        case CODEC_FORMAT_H264:
          file_type_option = (file_type)(FILE_TYPE_START_OF_H264_SPECIFIC + file_type_option - FILE_TYPE_COMMON_CODEC_MAX);
          break;
        case CODEC_FORMAT_DIVX:
          file_type_option = (file_type)(FILE_TYPE_START_OF_DIVX_SPECIFIC + file_type_option - FILE_TYPE_COMMON_CODEC_MAX);
          break;
        case CODEC_FORMAT_MP4:
        case CODEC_FORMAT_H263:
          file_type_option = (file_type)(FILE_TYPE_START_OF_MP4_SPECIFIC + file_type_option - FILE_TYPE_COMMON_CODEC_MAX);
          break;
        case CODEC_FORMAT_VC1:
          file_type_option = (file_type)(FILE_TYPE_START_OF_VC1_SPECIFIC + file_type_option - FILE_TYPE_COMMON_CODEC_MAX);
          break;
        case CODEC_FORMAT_MPEG2:
          file_type_option = (file_type)(FILE_TYPE_START_OF_MPEG2_SPECIFIC + file_type_option - FILE_TYPE_COMMON_CODEC_MAX);
          break;
#ifdef _MSM8974_
        case CODEC_FORMAT_VP8:
          break;
#endif
        default:
          DEBUGT_PRINT_ERROR("Error: Unknown code %d", codec_format_option);
      }
    }

    CONFIG_VERSION_SIZE(picture_order);
    picture_order.eOutputPictureOrder = QOMX_VIDEO_DECODE_ORDER;
    if (pic_order == 0)
      picture_order.eOutputPictureOrder = QOMX_VIDEO_DISPLAY_ORDER;

    if (outputOption == 0)
    {
      displayYuv = 0;
      takeYuvLog = 0;
      realtime_display = 0;
    }
    else if (outputOption == 1)
    {
      displayYuv = 1;
      takeYuvLog = 0;
    }
    else if (outputOption == 2)
    {
      displayYuv = 0;
      takeYuvLog = 1;
      realtime_display = 0;
    }
    else if (outputOption == 3)
    {
      displayYuv = 1;
      takeYuvLog = !realtime_display;
    }
    else
    {
      DEBUGT_PRINT("Wrong option. Assume you want to see the YUV display");
      displayYuv = 1;
    }

    if (test_option != 2)
    {
      freeHandle_option = (freeHandle_test)0;
    }

    /***************************************************/
    /*              Initializations               */
    /***************************************************/
#ifdef READ_FROM_SOCKET
    init_queue();
    pthread_mutex_init(&rtp_lock, 0);
#else
    printf("*******************************************************\n");
    printf("Input file name: inputfilename[%s]\n", in_filename);
    printf("*******************************************************\n");
#endif
    pthread_cond_init(&cond, 0);
    pthread_cond_init(&eos_cond, 0);
    pthread_mutex_init(&eos_lock, 0);
    pthread_mutex_init(&lock, 0);
    pthread_mutex_init(&etb_lock, 0);
    pthread_mutex_init(&fbd_lock, 0);
    pthread_mutex_init(&enable_lock, 0);
    if (-1 == sem_init(&etb_sem, 0, 0))
    {
      DEBUGT_PRINT_ERROR("Error - sem_init failed %d", errno);
    }
    if (-1 == sem_init(&fbd_sem, 0, 0))
    {
      DEBUGT_PRINT_ERROR("Error - sem_init failed %d", errno);
    }
    if (-1 == sem_init(&seq_sem, 0, 0))
    {
      DEBUGT_PRINT_ERROR("Error - sem_init failed %d", errno);
    }
    if (-1 == sem_init(&in_flush_sem, 0, 0))
    {
      DEBUGT_PRINT_ERROR("Error - sem_init failed %d", errno);
    }
    if (-1 == sem_init(&out_flush_sem, 0, 0))
    {
      DEBUGT_PRINT_ERROR("Error - sem_init failed %d", errno);
    }
    etb_queue = alloc_queue();
    if (etb_queue == NULL)
    {
      DEBUGT_PRINT_ERROR(" Error in Creating etb_queue");
      return -1;
    }

    fbd_queue = alloc_queue();
    if (fbd_queue == NULL)
    {
      DEBUGT_PRINT_ERROR(" Error in Creating fbd_queue");
      free_queue(etb_queue);
      return -1;
    }
#ifndef _ANDROID_
    if(0 != pthread_create(&gpio_thread_id, NULL, check_gpio_thread, NULL))
    {
      DEBUGT_PRINT_ERROR(" Error in Creating gbm_thread ");
      free_queue(etb_queue);
      free_queue(fbd_queue);
      return -1;
    }
#endif

#ifdef READ_FROM_SOCKET
   DEBUGT_PRINT("MM-RTP-RECEIVER: MM RTP receiver start");
   if ( socketaddr) {
      if(0 != pthread_create(&rtp_thread_id, NULL, rtp_thread, &rtp_socket))
      {
        DEBUGT_PRINT_ERROR(" Error in Creating rtp_thread ");
        clear_queue();
        return -1;
      }
  }
#endif
    if(0 != pthread_create(&fbd_thread_id, NULL, fbd_thread, NULL))
    {
      DEBUGT_PRINT_ERROR(" Error in Creating fbd_thread ");
      free_queue(etb_queue);
      free_queue(fbd_queue);
      return -1;
    }

    printf("Decode start\n");
    /***************************************************/
    /*      Decode and display the clip          */
    /***************************************************/
    /* The video decode and display function */
    Decode_Display_Video();

    /***************************************************/
    /*              Decode done, cleanup          */
    /***************************************************/
#ifdef READ_FROM_SOCKET
    if (socketaddr) {
       bRtpReceiverStopped =TRUE;
       if(m_pRTPDecoder->stop() != OK)
       {
          DEBUGT_PRINT_ERROR("MM-RTP-RECEIVER: RTP decoder stop failed");
          return -1;
       }
       if (m_pRTPDecoder)
       {
          MM_Delete(m_pRTPDecoder);
          m_pRTPDecoder = NULL;
       }
       if (m_pSourcePort) {
        delete m_pSourcePort;
       }
       m_pSourcePort = NULL;
       if (m_pFileSource)
       {
          MM_Delete(m_pFileSource);
          m_pFileSource = NULL;
       }
    }
#endif
    if (displayYuv){
        close_display();
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&etb_lock);
    pthread_mutex_destroy(&fbd_lock);
    pthread_mutex_destroy(&enable_lock);
    pthread_cond_destroy(&eos_cond);
    pthread_mutex_destroy(&eos_lock);
#ifdef READ_FROM_SOCKET
    clear_all_queue();
    pthread_mutex_destroy(&rtp_lock);
#endif
    if (-1 == sem_destroy(&etb_sem))
    {
      DEBUGT_PRINT_ERROR("Error - sem_destroy failed %d", errno);
    }
    if (-1 == sem_destroy(&fbd_sem))
    {
      DEBUGT_PRINT_ERROR("Error - sem_destroy failed %d", errno);
    }
    if (-1 == sem_destroy(&seq_sem))
    {
      DEBUGT_PRINT_ERROR("Error - sem_destroy failed %d", errno);
    }
    if (-1 == sem_destroy(&in_flush_sem))
    {
      DEBUGT_PRINT_ERROR("Error - sem_destroy failed %d", errno);
    }
    if (-1 == sem_destroy(&out_flush_sem))
    {
      DEBUGT_PRINT_ERROR("Error - sem_destroy failed %d", errno);
    }

    deInit();

    return 0;
}

int Decode_Display_Video()
{
  int cmd_error = 0;
  DEBUGT_PRINT("Inside %s", __FUNCTION__);
  waitForPortSettingsChanged = 1;

#ifdef READ_FROM_SOCKET
  if(file_type_option == FILE_TYPE_RTP) {
    Read_Buffer = Read_Buffer_From_Socket;
  }
  else if(file_type_option == FILE_TYPE_DAT_PER_AU) {
#else
  if(file_type_option == FILE_TYPE_DAT_PER_AU) {
#endif
    Read_Buffer = Read_Buffer_From_DAT_File;
  }
  else if(file_type_option == FILE_TYPE_ARBITRARY_BYTES) {
    Read_Buffer = Read_Buffer_ArbitraryBytes;
  }
  else if(file_type_option == FILE_TYPE_MP4) {
    Read_Buffer = Read_Buffer_From_MP4_File;
  }
  else if(codec_format_option == CODEC_FORMAT_H264) {
    if (file_type_option == FILE_TYPE_264_NAL_SIZE_LENGTH) {
       Read_Buffer = Read_Buffer_From_Size_Nal;
    } else if (file_type_option == FILE_TYPE_264_START_CODE_BASED) {
       Read_Buffer = Read_Buffer_From_H264_Start_Code_File;
    } else {
       DEBUGT_PRINT_ERROR("Invalid file_type_option(%d) for H264", file_type_option);
       return -1;
    }
  }
  else if((codec_format_option == CODEC_FORMAT_H263) ||
          (codec_format_option == CODEC_FORMAT_MP4)) {
    Read_Buffer = Read_Buffer_From_Vop_Start_Code_File;
  }
  else if (codec_format_option == CODEC_FORMAT_MPEG2) {
    Read_Buffer = Read_Buffer_From_Mpeg2_Start_Code;
  }
  else if(file_type_option == FILE_TYPE_DIVX_4_5_6) {
    Read_Buffer = Read_Buffer_From_DivX_4_5_6_File;
  }
#ifdef MAX_RES_1080P
  else if(file_type_option == FILE_TYPE_DIVX_311) {
    Read_Buffer = Read_Buffer_From_DivX_311_File;
  }
#endif
  else if(file_type_option == FILE_TYPE_RCV) {
    Read_Buffer = Read_Buffer_From_RCV_File;
  }
#ifdef _MSM8974_
  else if(file_type_option == FILE_TYPE_VP8) {
    Read_Buffer = Read_Buffer_From_VP8_File;
  }
#endif
  else if(file_type_option == FILE_TYPE_VC1) {
    Read_Buffer = Read_Buffer_From_VC1_File;
  }

  DEBUGT_PRINT("file_type_option %d!", file_type_option);

  switch(file_type_option)
  {
    case FILE_TYPE_RTP:
    case FILE_TYPE_DAT_PER_AU:
    case FILE_TYPE_ARBITRARY_BYTES:
    case FILE_TYPE_MP4:
    case FILE_TYPE_264_START_CODE_BASED:
    case FILE_TYPE_264_NAL_SIZE_LENGTH:
    case FILE_TYPE_PICTURE_START_CODE:
    case FILE_TYPE_MPEG2_START_CODE:
    case FILE_TYPE_RCV:
    case FILE_TYPE_VC1:
#ifdef _MSM8974_
    case FILE_TYPE_VP8:
#endif
    case FILE_TYPE_DIVX_4_5_6:
#ifdef MAX_RES_1080P
    case FILE_TYPE_DIVX_311:
#endif
      if(Init_Decoder()!= 0x00)
      {
        DEBUGT_PRINT_ERROR("Error - Decoder Init failed");
        return -1;
      }
      if(Play_Decoder() != 0x00)
      {
        return -1;
      }
      break;
    default:
      DEBUGT_PRINT_ERROR("Error - Invalid Entry...%d",file_type_option);
      break;
  }

  anti_flickering = true;
  if(strlen(seq_file_name))
  {
        seqFile = fopen (seq_file_name, "rb");
        if (seqFile == NULL)
        {
            DEBUGT_PRINT_ERROR("Error - Seq file %s could NOT be opened",
                              seq_file_name);
            return -1;
        }
        else
        {
            DEBUGT_PRINT("Seq file %s is opened ", seq_file_name);
            seq_enabled = 1;
            anti_flickering = false;
        }
  }

  pthread_mutex_lock(&eos_lock);
  while (bOutputEosReached == false && cmd_error == 0)
  {
    if(seq_enabled)
    {
        pthread_mutex_unlock(&eos_lock);
        if(!get_next_command(seqFile))
            cmd_error = process_current_command(curr_seq_command);
        else
        {
            DEBUGT_PRINT_ERROR(" Error in get_next_cmd or EOF");
            seq_enabled = 0;
        }
        pthread_mutex_lock(&eos_lock);
    }
    else
        pthread_cond_wait(&eos_cond, &eos_lock);

    if (currentStatus == PORT_SETTING_CHANGE_STATE)
    {
      pthread_mutex_unlock(&eos_lock);
      cmd_error = output_port_reconfig();
      pthread_mutex_lock(&eos_lock);
    }
  }
  pthread_mutex_unlock(&eos_lock);

  // Wait till EOS is reached...
  if(bOutputEosReached)
    do_freeHandle_and_clean_up(currentStatus == ERROR_STATE);
  return 0;
}

int Init_Decoder()
{
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    OMX_ERRORTYPE omxresult;
    char vdecCompNames[50];

    static OMX_CALLBACKTYPE call_back = {&EventHandler, &EmptyBufferDone, &FillBufferDone};

    /* Init. the OpenMAX Core */
    DEBUGT_PRINT("Initializing OpenMAX Core....");
    omxresult = OMX_Init();
    if(OMX_ErrorNone != omxresult) {
        DEBUGT_PRINT_ERROR(" Failed to Init OpenMAX core");
        return -1;
    }
    else {
        DEBUGT_PRINT_ERROR("OpenMAX Core Init Done");
    }

    if (codec_format_option == CODEC_FORMAT_H264)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.avc", 27);
      //strlcpy(vdecCompNames, "OMX.SEC.qcom.video.decoder.avc", 31);
    }
    else if (codec_format_option == CODEC_FORMAT_MP4)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.mpeg4", 29);
    }
    else if (codec_format_option == CODEC_FORMAT_H263)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.h263", 28);
    }
    else if (codec_format_option == CODEC_FORMAT_VC1)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.vc1", 27);
    }
    else if (codec_format_option == CODEC_FORMAT_MPEG2)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.mpeg2", 29);
    }
    else if (file_type_option == FILE_TYPE_RCV)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.wmv", 27);
    }
    else if (file_type_option == FILE_TYPE_DIVX_4_5_6)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.divx", 28);
    }
#ifdef _MSM8974_
    else if (codec_format_option == CODEC_FORMAT_VP8)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.vp8", 27);
    }
    else if (codec_format_option == CODEC_FORMAT_VP9)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.vp9", 27);
    }
#endif
    else if (codec_format_option == CODEC_FORMAT_HEVC)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.hevc", 28);
    }
#ifdef MAX_RES_1080P
    else if (file_type_option == FILE_TYPE_DIVX_311)
    {
      strlcpy(vdecCompNames, "OMX.qcom.video.decoder.divx311", 31);
    }
#endif
    else
    {
      DEBUGT_PRINT_ERROR("Error: Unsupported codec %d", codec_format_option);
      return -1;
    }
    place_marker(" === OMX Gethandle");
    omxresult = OMX_GetHandle((OMX_HANDLETYPE*)(&dec_handle),
                              (OMX_STRING)vdecCompNames, NULL, &call_back);
    if (FAILED(omxresult)) {
        DEBUGT_PRINT_ERROR("Failed to Load the component:%s", vdecCompNames);
        return -1;
    }
    else
    {
        DEBUGT_PRINT("Component %s is in LOADED state", vdecCompNames);
    }
    place_marker(" === OMX Gethandle done");
    QOMX_VIDEO_QUERY_DECODER_INSTANCES decoder_instances;
    omxresult = OMX_GetConfig(dec_handle,
                 (OMX_INDEXTYPE)OMX_QcomIndexQueryNumberOfVideoDecInstance,
                              &decoder_instances);
    DEBUGT_PRINT(" Number of decoder instances %d",
                      decoder_instances.nNumOfInstances);

    /* Get the port information */
    CONFIG_VERSION_SIZE(portParam);
    omxresult = OMX_GetParameter(dec_handle, OMX_IndexParamVideoInit,
                                (OMX_PTR)&portParam);

    if(FAILED(omxresult)) {
        DEBUGT_PRINT_ERROR("ERROR - Failed to get Port Param");
        return -1;
    }
    else
    {
        DEBUGT_PRINT("portParam.nPorts:%d", portParam.nPorts);
        DEBUGT_PRINT("portParam.nStartPortNumber:%d", portParam.nStartPortNumber);
    }

    /* Set the compression format on i/p port */
    if (codec_format_option == CODEC_FORMAT_H264)
    {
      portFmt.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
    }
    else if (codec_format_option == CODEC_FORMAT_MP4)
    {
      portFmt.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if (codec_format_option == CODEC_FORMAT_H263)
    {
      portFmt.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;
    }
    else if (codec_format_option == CODEC_FORMAT_VC1)
    {
      portFmt.format.video.eCompressionFormat = OMX_VIDEO_CodingWMV;
    }
    else if (codec_format_option == CODEC_FORMAT_DIVX)
    {
      portFmt.format.video.eCompressionFormat =
          (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingDivx;
    }
    else if (codec_format_option == CODEC_FORMAT_MPEG2)
    {
      portFmt.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG2;
    }
    else if (codec_format_option == CODEC_FORMAT_HEVC)
    {
      portFmt.format.video.eCompressionFormat = (OMX_VIDEO_CODINGTYPE)QOMX_VIDEO_CodingHevc;
    }
    else
    {
      DEBUGT_PRINT_ERROR("Error: Unsupported codec %d", codec_format_option);
    }

    if (thumbnailMode == 1) {
        QOMX_ENABLETYPE thumbNailMode;
        thumbNailMode.bEnable = OMX_TRUE;
        OMX_SetParameter(dec_handle,(OMX_INDEXTYPE)OMX_QcomIndexParamVideoSyncFrameDecodingMode,
                     (OMX_PTR)&thumbNailMode);
        DEBUGT_PRINT("Enabled Thumbnail mode");
    }
#ifdef USE_OUTPUT_BUFFER
    use_external_pmem_buf = OMX_TRUE;
#endif
    return 0;
}

int Play_Decoder()
{
    OMX_VIDEO_PARAM_PORTFORMATTYPE videoportFmt;
    int i, bufCnt, index = 0;
    int frameSize=0;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    /* open the i/p and o/p files based on the video file format passed */
#ifdef EARLY_BOOTVIDEO
    if(open_video_file()) {
        DEBUGT_PRINT_ERROR("Error in opening video file");
        return -1;
    }
#endif
    OMX_QCOM_PARAM_PORTDEFINITIONTYPE inputPortFmt;
    memset(&inputPortFmt, 0, sizeof(OMX_QCOM_PARAM_PORTDEFINITIONTYPE));
    CONFIG_VERSION_SIZE(inputPortFmt);
    inputPortFmt.nPortIndex = 0;  // input port
    switch (file_type_option)
    {
      case FILE_TYPE_DAT_PER_AU:
      case FILE_TYPE_PICTURE_START_CODE:
      case FILE_TYPE_MPEG2_START_CODE:
      case FILE_TYPE_264_START_CODE_BASED:
      case FILE_TYPE_RCV:
      case FILE_TYPE_VC1:
#ifdef MAX_RES_1080P
      case FILE_TYPE_DIVX_311:
#endif
      {
        inputPortFmt.nFramePackingFormat = OMX_QCOM_FramePacking_OnlyOneCompleteFrame;
        break;
      }
      case FILE_TYPE_RTP:
      case FILE_TYPE_ARBITRARY_BYTES:
      case FILE_TYPE_MP4:
      case FILE_TYPE_264_NAL_SIZE_LENGTH:
      case FILE_TYPE_DIVX_4_5_6:
      {
        inputPortFmt.nFramePackingFormat = OMX_QCOM_FramePacking_Arbitrary;
        break;
      }
#ifdef _MSM8974_
      case FILE_TYPE_VP8:
      {
        inputPortFmt.nFramePackingFormat = OMX_QCOM_FramePacking_OnlyOneCompleteFrame;
        break;
      }
#endif
      default:
        inputPortFmt.nFramePackingFormat = OMX_QCOM_FramePacking_Unspecified;
    }
    OMX_SetParameter(dec_handle,(OMX_INDEXTYPE)OMX_QcomIndexPortDefn,
                     (OMX_PTR)&inputPortFmt);
#ifdef USE_EXTERN_PMEM_BUF
    OMX_QCOM_PARAM_PORTDEFINITIONTYPE outPortFmt;
    memset(&outPortFmt, 0, sizeof(OMX_QCOM_PARAM_PORTDEFINITIONTYPE));
    CONFIG_VERSION_SIZE(outPortFmt);
    outPortFmt.nPortIndex = 1;  // output port
    outPortFmt.nCacheAttr = OMX_QCOM_CacheAttrNone;
    outPortFmt.nMemRegion = OMX_QCOM_MemRegionEBI1;
    OMX_SetParameter(dec_handle,(OMX_INDEXTYPE)OMX_QcomIndexPortDefn,
                     (OMX_PTR)&outPortFmt);

    OMX_QCOM_PLATFORMPRIVATE_EXTN outPltPvtExtn;
    memset(&outPltPvtExtn, 0, sizeof(OMX_QCOM_PLATFORMPRIVATE_EXTN));
    CONFIG_VERSION_SIZE(outPltPvtExtn);
    outPltPvtExtn.nPortIndex = 1;  // output port
    outPltPvtExtn.type = OMX_QCOM_PLATFORM_PRIVATE_PMEM;
    OMX_SetParameter(dec_handle,(OMX_INDEXTYPE)OMX_QcomIndexPlatformPvt,
                     (OMX_PTR)&outPltPvtExtn);
    use_external_pmem_buf = OMX_TRUE;
#endif
    QOMX_ENABLETYPE extra_data;
    extra_data.bEnable = OMX_TRUE;

    OMX_SetParameter(dec_handle,(OMX_INDEXTYPE)OMX_QcomIndexParamFrameInfoExtraData,
                     (OMX_PTR)&extra_data);

#ifdef TEST_TS_FROM_SEI
    OMX_SetParameter(dec_handle,(OMX_INDEXTYPE)OMX_QcomIndexParamH264TimeInfo,
                     (OMX_PTR)&extra_data);
#endif

    /* Query the decoder outport's min buf requirements */
    CONFIG_VERSION_SIZE(portFmt);

    /* Port for which the Client needs to obtain info */
    portFmt.nPortIndex = portParam.nStartPortNumber;

    OMX_GetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
    DEBUGT_PRINT("Dec: Min Buffer Count %d", portFmt.nBufferCountMin);
    DEBUGT_PRINT("Dec: Buffer Size %d", portFmt.nBufferSize);

    if(OMX_DirInput != portFmt.eDir) {
        DEBUGT_PRINT ("Dec: Expect Input Port");
        return -1;
    }
#ifdef MAX_RES_1080P
    if( (codec_format_option == CODEC_FORMAT_DIVX) &&
        (file_type_option == FILE_TYPE_DIVX_311) ) {

            int off;

            if ( read(inputBufferFileFd, &width, 4 ) == -1 ) {
                DEBUGT_PRINT_ERROR("Failed to read width for divx");
                return  -1;
            }

            DEBUGT_PRINT("Width for DIVX = %d", width);

            if ( read(inputBufferFileFd, &height, 4 ) == -1 ) {
                DEBUGT_PRINT_ERROR("Failed to read height for divx");
                return  -1;
            }

            DEBUGT_PRINT("Height for DIVX = %u", height);
    }
#endif
#ifdef _MSM8974_
    if( (codec_format_option == CODEC_FORMAT_VC1) &&
        (file_type_option == FILE_TYPE_RCV) ) {
        //parse struct_A data to get height and width information
        unsigned int temp;
        lseek64(inputBufferFileFd, 0, SEEK_SET);
        if (read(inputBufferFileFd, &temp, 4) < 0) {
            DEBUGT_PRINT_ERROR("Failed to read vc1 data");
            return -1;
        }
        //Refer to Annex L of SMPTE 421M-2006 VC1 decoding standard
        //We need to skip 12 bytes after 0xC5 in sequence layer data
        //structure to read struct_A, which includes height and
        //width information.
        if ((temp & 0xFF000000) == 0xC5000000) {
            lseek64(inputBufferFileFd, 12, SEEK_SET);

            if ( read(inputBufferFileFd, &height, 4 ) < -1 ) {
                DEBUGT_PRINT_ERROR("Failed to read height for vc-1");
                return  -1;
            }
            if ( read(inputBufferFileFd, &width, 4 ) == -1 ) {
                DEBUGT_PRINT_ERROR("Failed to read width for vc-1");
                return  -1;
            }
            lseek64(inputBufferFileFd, 0, SEEK_SET);
        }
	if ((temp & 0xFF000000) == 0x85000000) {
		lseek64(inputBufferFileFd, 0, SEEK_SET);
	}
        DEBUGT_PRINT(" RCV clip width = %u height = %u ",width, height);
    }
#endif
    crop_rect.nWidth = width;
    crop_rect.nHeight = height;

    bufCnt = 0;
    portFmt.format.video.nFrameHeight = height;
    portFmt.format.video.nFrameWidth  = width;
    portFmt.format.video.xFramerate = fps;
    OMX_SetParameter(dec_handle,OMX_IndexParamPortDefinition, (OMX_PTR)&portFmt);
    OMX_GetParameter(dec_handle,OMX_IndexParamPortDefinition, &portFmt);
    DEBUGT_PRINT("Dec: New Min Buffer Count %d", portFmt.nBufferCountMin);
    CONFIG_VERSION_SIZE(videoportFmt);
#ifdef MAX_RES_720P
    if(color_fmt_type == 0)
    {
        color_fmt = OMX_COLOR_FormatYUV420SemiPlanar;
    }
    else
    {
        color_fmt = (OMX_COLOR_FORMATTYPE)
           QOMX_COLOR_FormatYUV420PackedSemiPlanar64x32Tile2m8ka;
    }
#elif _MSM8974_
        color_fmt = (OMX_COLOR_FORMATTYPE)
		QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m;
#else
       color_fmt = (OMX_COLOR_FORMATTYPE)
           QOMX_COLOR_FormatYUV420PackedSemiPlanar64x32Tile2m8ka;
#endif

    while (ret == OMX_ErrorNone)
    {
        videoportFmt.nPortIndex = 1;
        videoportFmt.nIndex = index;
        ret = OMX_GetParameter(dec_handle, OMX_IndexParamVideoPortFormat,
          (OMX_PTR)&videoportFmt);

        if((ret == OMX_ErrorNone) && (videoportFmt.eColorFormat ==
           color_fmt))
        {
            DEBUGT_PRINT(" Format[%u] supported by OMX Decoder", color_fmt);
            break;
        }
        index++;
    }

    if(ret == OMX_ErrorNone)
    {
        if(OMX_SetParameter(dec_handle, OMX_IndexParamVideoPortFormat,
            (OMX_PTR)&videoportFmt) != OMX_ErrorNone)
        {
            DEBUGT_PRINT_ERROR(" Setting Tile format failed");
            return -1;
        }
    }
    else
    {
        DEBUGT_PRINT_ERROR(" Error in retrieving supported color formats");
        return -1;
    }
    picture_order.nPortIndex = 1;
    DEBUGT_PRINT("Set picture order");
    if(OMX_SetParameter(dec_handle,
       (OMX_INDEXTYPE)OMX_QcomIndexParamVideoDecoderPictureOrder,
       (OMX_PTR)&picture_order) != OMX_ErrorNone)
    {
        DEBUGT_PRINT_ERROR(" ERROR: Setting picture order!");
        return -1;
    }
    DEBUGT_PRINT("Video format: W x H (%d x %d)",
      portFmt.format.video.nFrameWidth,
      portFmt.format.video.nFrameHeight);
    if(codec_format_option == CODEC_FORMAT_H264 ||
       codec_format_option == CODEC_FORMAT_HEVC)
    {
        OMX_VIDEO_CONFIG_NALSIZE naluSize;
        naluSize.nNaluBytes = nalSize;
        DEBUGT_PRINT(" Nal length is %d index %d",nalSize,OMX_IndexConfigVideoNalSize);
        OMX_SetConfig(dec_handle,OMX_IndexConfigVideoNalSize,(OMX_PTR)&naluSize);
        DEBUGT_PRINT("SETTING THE NAL SIZE to %d",naluSize.nNaluBytes);
    }

    // Set smooth streaming param
    if(OMX_SetParameter(dec_handle,
       (OMX_INDEXTYPE)OMX_QcomIndexParamEnableSmoothStreaming,
       (OMX_PTR)&portFmt) != OMX_ErrorNone)
    {
        DEBUGT_PRINT_ERROR(" ERROR: Setting Smooth Streaming !");
        return -1;
    }

    DEBUGT_PRINT("OMX_SendCommand Decoder -> IDLE");
    OMX_SendCommand(dec_handle, OMX_CommandStateSet, OMX_StateIdle,0);

    input_buf_cnt = portFmt.nBufferCountActual;
    DEBUGT_PRINT("Transition to Idle State succesful...");

#if ALLOCATE_BUFFER
       // Allocate buffer on decoder's i/p port
       error = Allocate_Buffer(dec_handle, &pInputBufHdrs, portFmt.nPortIndex,
                               portFmt.nBufferCountActual, portFmt.nBufferSize);
       if (error != OMX_ErrorNone) {
           DEBUGT_PRINT_ERROR("Error - OMX_AllocateBuffer Input buffer error");
           return -1;
       }
       else {
           DEBUGT_PRINT("OMX_AllocateBuffer Input buffer success");
       }
#else
       // Use buffer on decoder's i/p port
          input_use_buffer = true;
          DEBUGT_PRINT_ERROR(" before OMX_UseBuffer %p", &pInputBufHdrs);
          error =  use_input_buffer(dec_handle,
                             &pInputBufHdrs,
                              portFmt.nPortIndex,
                              portFmt.nBufferSize,
                              portFmt.nBufferCountActual);
          if (error != OMX_ErrorNone) {
             DEBUGT_PRINT_ERROR("ERROR - OMX_UseBuffer Input buffer failed");
             return -1;
          }
          else {
             DEBUGT_PRINT("OMX_UseBuffer Input buffer success");
          }
#endif
       portFmt.nPortIndex = portParam.nStartPortNumber+1;
       // Port for which the Client needs to obtain info

    OMX_GetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);

#ifndef _ANDROID_
    //set the correct output buffer count
    {
        ret = OMX_GetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
        if (ret != OMX_ErrorNone) {
            DEBUGT_PRINT_ERROR("%s: OMX_GetParameter failed: %d",__FUNCTION__, ret);
            return -1;
        }
        portFmt.nBufferCountMin = portFmt.nBufferCountActual = 6;
        ret = OMX_SetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
        if (ret != OMX_ErrorNone) {
            DEBUGT_PRINT_ERROR("%s: OMX_SetParameter failed: %d",__FUNCTION__, ret);
            return -1;
        }
    }
#endif

    DEBUGT_PRINT("nMin Buffer Count=%d", portFmt.nBufferCountMin);
    DEBUGT_PRINT("nBuffer Size=%d", portFmt.nBufferSize);
    if(OMX_DirOutput != portFmt.eDir) {
        DEBUGT_PRINT_ERROR("Error - Expect Output Port");
        return -1;
    }


    if (anti_flickering) {
        ret = OMX_GetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
        if (ret != OMX_ErrorNone) {
            DEBUGT_PRINT_ERROR("%s: OMX_GetParameter failed: %d",__FUNCTION__, ret);
            return -1;
        }
        portFmt.nBufferCountActual += 1;
        ret = OMX_SetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
        if (ret != OMX_ErrorNone) {
            DEBUGT_PRINT_ERROR("%s: OMX_SetParameter failed: %d",__FUNCTION__, ret);
            return -1;
        }
    }

#ifndef USE_EGL_IMAGE_TEST_APP
    if (use_external_pmem_buf)
    {
        DEBUGT_PRINT_ERROR(" Use External pmem buf: OMX_UseBuffer %p", &pInputBufHdrs);
        error =  use_output_buffer_multiple_fd(dec_handle,
                                               &pOutYUVBufHdrs,
                                               portFmt.nPortIndex,
                                               portFmt.nBufferSize,
                                               portFmt.nBufferCountActual);
    }
    else
    {
        /* Allocate buffer on decoder's o/p port */
        error = Allocate_Buffer(dec_handle, &pOutYUVBufHdrs, portFmt.nPortIndex,
                                portFmt.nBufferCountActual, portFmt.nBufferSize);
    }
    free_op_buf_cnt = portFmt.nBufferCountActual;
    if (error != OMX_ErrorNone) {
        DEBUGT_PRINT_ERROR("Error - OMX_AllocateBuffer Output buffer error");
        return -1;
    }
    else
    {
        DEBUGT_PRINT("OMX_AllocateBuffer Output buffer success");
    }
#else
    DEBUGT_PRINT_ERROR(" before OMX_UseBuffer %p", &pInputBufHdrs);
    error =  use_output_buffer(dec_handle,
                       &pOutYUVBufHdrs,
                        portFmt.nPortIndex,
                        portFmt.nBufferSize,
                        portFmt.nBufferCountActual);
    free_op_buf_cnt = portFmt.nBufferCountActual;
    if (error != OMX_ErrorNone) {
       DEBUGT_PRINT_ERROR("ERROR - OMX_UseBuffer Input buffer failed");
       return -1;
    }
    else {
       DEBUGT_PRINT("OMX_UseBuffer Input buffer success");
    }
#endif
    wait_for_event();
    if (currentStatus == ERROR_STATE)
    {
      do_freeHandle_and_clean_up(true);
      return -1;
    }

    if (freeHandle_option == FREE_HANDLE_AT_IDLE)
    {
      OMX_STATETYPE state = OMX_StateInvalid;
      OMX_GetState(dec_handle, &state);
      if (state == OMX_StateIdle)
      {
        DEBUGT_PRINT("Decoder is in OMX_StateIdle and trying to call OMX_FreeHandle ");
        do_freeHandle_and_clean_up(false);
      }
      else
      {
        DEBUGT_PRINT_ERROR("Error - Decoder is in state %d and trying to call OMX_FreeHandle ", state);
        do_freeHandle_and_clean_up(true);
      }
      return -1;
    }
    DEBUGT_PRINT("OMX_SendCommand Decoder -> Executing");
    OMX_SendCommand(dec_handle, OMX_CommandStateSet, OMX_StateExecuting,0);
    wait_for_event();
    if (currentStatus == ERROR_STATE)
    {
      do_freeHandle_and_clean_up(true);
      return -1;
    }
    if (pOutYUVBufHdrs == NULL)
    {
        DEBUGT_PRINT_ERROR("Error - pOutYUVBufHdrs is NULL");
        return -1;
    }
    for(bufCnt=0; bufCnt < portFmt.nBufferCountActual; ++bufCnt) {
        DEBUGT_PRINT("OMX_FillThisBuffer on output buf no.%d",bufCnt);
        if (pOutYUVBufHdrs[bufCnt] == NULL)
        {
            DEBUGT_PRINT_ERROR("Error - pOutYUVBufHdrs[%d] is NULL", bufCnt);
            return -1;
        }
        pOutYUVBufHdrs[bufCnt]->nOutputPortIndex = 1;
        pOutYUVBufHdrs[bufCnt]->nFlags &= ~OMX_BUFFERFLAG_EOS;
        ret = OMX_FillThisBuffer(dec_handle, pOutYUVBufHdrs[bufCnt]);
        if (OMX_ErrorNone != ret)
            DEBUGT_PRINT_ERROR("Error - OMX_FillThisBuffer failed with result %d", ret);
        else
        {
            DEBUGT_PRINT("OMX_FillThisBuffer success!");
            free_op_buf_cnt--;
        }
    }

    used_ip_buf_cnt = input_buf_cnt;

    rcv_v1 = 0;

    //QPERF_START(client_decode);
    if (codec_format_option == CODEC_FORMAT_VC1)
    {
      pInputBufHdrs[0]->nOffset = 0;
      if(file_type_option == FILE_TYPE_RCV)
      {
      frameSize = Read_Buffer_From_RCV_File_Seq_Layer(pInputBufHdrs[0]);
      pInputBufHdrs[0]->nFilledLen = frameSize;
          DEBUGT_PRINT("After Read_Buffer_From_RCV_File_Seq_Layer, "
              "frameSize %d", frameSize);
      }
      else if(file_type_option == FILE_TYPE_VC1)
      {
          bHdrflag = 1;
          pInputBufHdrs[0]->nFilledLen = Read_Buffer(pInputBufHdrs[0]);
          bHdrflag = 0;
          DEBUGT_PRINT_ERROR("After 1st Read_Buffer for VC1, "
              "pInputBufHdrs[0]->nFilledLen %d", pInputBufHdrs[0]->nFilledLen);
      }
      else
      {
          pInputBufHdrs[0]->nFilledLen = Read_Buffer(pInputBufHdrs[0]);
          DEBUGT_PRINT("After Read_Buffer pInputBufHdrs[0]->nFilledLen %d",
              pInputBufHdrs[0]->nFilledLen);
      }

      pInputBufHdrs[0]->nInputPortIndex = 0;
      pInputBufHdrs[0]->nOffset = 0;
#ifndef _MSM8974_
      pInputBufHdrs[0]->nFlags = 0;
#endif
      ret = OMX_EmptyThisBuffer(dec_handle, pInputBufHdrs[0]);
      if (ret != OMX_ErrorNone)
      {
          DEBUGT_PRINT_ERROR("ERROR - OMX_EmptyThisBuffer failed with result %d", ret);
          do_freeHandle_and_clean_up(true);
          return -1;
      }
      else
      {
          etb_count++;
          DEBUGT_PRINT("OMX_EmptyThisBuffer success!");
      }
      i = 1;
#ifdef _MSM8974_
      pInputBufHdrs[0]->nFlags = 0;
#endif
    }
    else
    {
      i = 0;
    }

    for (i; i < used_ip_buf_cnt;i++) {
      pInputBufHdrs[i]->nInputPortIndex = 0;
      pInputBufHdrs[i]->nOffset = 0;
      if((frameSize = Read_Buffer(pInputBufHdrs[i])) <= 0 ){
        DEBUGT_PRINT("NO FRAME READ");
        pInputBufHdrs[i]->nFilledLen = frameSize;
        pInputBufHdrs[i]->nInputPortIndex = 0;
        pInputBufHdrs[i]->nFlags |= OMX_BUFFERFLAG_EOS;;
        bInputEosReached = true;

        OMX_EmptyThisBuffer(dec_handle, pInputBufHdrs[i]);
        etb_count++;
        DEBUGT_PRINT("File is small::Either EOS or Some Error while reading file");
        break;
      }
      pInputBufHdrs[i]->nFilledLen = frameSize;
      pInputBufHdrs[i]->nInputPortIndex = 0;
      pInputBufHdrs[i]->nFlags = 0;

      DEBUGT_PRINT("%s: Timestamp sent(%lld)", __FUNCTION__, pInputBufHdrs[i]->nTimeStamp);
      ret = OMX_EmptyThisBuffer(dec_handle, pInputBufHdrs[i]);
      if (OMX_ErrorNone != ret) {
          DEBUGT_PRINT_ERROR("ERROR - OMX_EmptyThisBuffer failed with result %d", ret);
          do_freeHandle_and_clean_up(true);
          return -1;
      }
      else {
          DEBUGT_PRINT("OMX_EmptyThisBuffer success!");
          etb_count++;
      }
    }
    if(0 != pthread_create(&ebd_thread_id, NULL, ebd_thread, NULL))
    {
      DEBUGT_PRINT_ERROR(" Error in Creating fbd_thread ");
      free_queue(etb_queue);
      free_queue(fbd_queue);
      return -1;
    }

#ifdef _ANDROID_
// wait for event port settings changed event
    DEBUGT_PRINT("wait_for_event: dyn reconfig");
    wait_for_event();
    DEBUGT_PRINT("wait_for_event: dyn reconfig rcvd, currentStatus %d\n",
            currentStatus);
    if (currentStatus == ERROR_STATE) {
        printf("Error - ERROR_STATE\n");
        do_freeHandle_and_clean_up(true);
        return -1;
    } else if (currentStatus == PORT_SETTING_CHANGE_STATE) {
        if (output_port_reconfig() != 0) {
            DEBUGT_PRINT("output_port_reconfig - ERROR_STATE\n");
            do_freeHandle_and_clean_up(true);
            return -1;
        }
    }
#endif

    if (freeHandle_option == FREE_HANDLE_AT_EXECUTING)
    {
      OMX_STATETYPE state = OMX_StateInvalid;
      OMX_GetState(dec_handle, &state);
      if (state == OMX_StateExecuting)
      {
        DEBUGT_PRINT("Decoder is in OMX_StateExecuting and trying to call OMX_FreeHandle ");
        do_freeHandle_and_clean_up(false);
      }
      else
      {
        DEBUGT_PRINT_ERROR("Error - Decoder is in state %d and trying to call OMX_FreeHandle ", state);
        do_freeHandle_and_clean_up(true);
      }
      return -1;
    }
    else if (freeHandle_option == FREE_HANDLE_AT_PAUSE)
    {
      OMX_SendCommand(dec_handle, OMX_CommandStateSet, OMX_StatePause,0);
      wait_for_event();

      OMX_STATETYPE state = OMX_StateInvalid;
      OMX_GetState(dec_handle, &state);
      if (state == OMX_StatePause)
      {
        DEBUGT_PRINT("Decoder is in OMX_StatePause and trying to call OMX_FreeHandle ");
        do_freeHandle_and_clean_up(false);
      }
      else
      {
        DEBUGT_PRINT_ERROR("Error - Decoder is in state %d and trying to call OMX_FreeHandle ", state);
        do_freeHandle_and_clean_up(true);
      }
      return -1;
    }

    return 0;
}

static OMX_ERRORTYPE Allocate_Buffer ( OMX_COMPONENTTYPE *dec_handle,
                                       OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                       OMX_U32 nPortIndex,
                                       long bufCntMin, long bufSize)
{
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    OMX_ERRORTYPE error=OMX_ErrorNone;
    long bufCnt=0;

    DEBUGT_PRINT("pBufHdrs = 0x%p,bufCntMin = %ld", pBufHdrs, bufCntMin);
    *pBufHdrs= (OMX_BUFFERHEADERTYPE **)
                   malloc(sizeof(OMX_BUFFERHEADERTYPE)*bufCntMin);

    for(bufCnt=0; bufCnt < bufCntMin; ++bufCnt) {
        DEBUGT_PRINT("OMX_AllocateBuffer No %ld ", bufCnt);
        error = OMX_AllocateBuffer(dec_handle, &((*pBufHdrs)[bufCnt]),
                                   nPortIndex, NULL, bufSize);
    }

    return error;
}

static OMX_ERRORTYPE use_input_buffer ( OMX_COMPONENTTYPE *dec_handle,
                                  OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                  OMX_U32 nPortIndex,
                                  OMX_U32 bufSize,
                                  long bufCntMin)
{
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    OMX_ERRORTYPE error=OMX_ErrorNone;
    long bufCnt=0;
    OMX_U8* pvirt = NULL;

    *pBufHdrs= (OMX_BUFFERHEADERTYPE **)
                   malloc(sizeof(OMX_BUFFERHEADERTYPE)* bufCntMin);
    if(*pBufHdrs == NULL){
        DEBUGT_PRINT_ERROR(" m_inp_heap_ptr Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }

    for(bufCnt=0; bufCnt < bufCntMin; ++bufCnt) {
        // allocate input buffers
      DEBUGT_PRINT("OMX_UseBuffer No %ld %d ", bufCnt, bufSize);
      pvirt = (OMX_U8*) malloc (bufSize);
      if(pvirt == NULL){
        DEBUGT_PRINT_ERROR(" pvirt Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }
      error = OMX_UseBuffer(dec_handle, &((*pBufHdrs)[bufCnt]),
                              nPortIndex, NULL, bufSize, pvirt);
       }
    return error;
}

static OMX_ERRORTYPE use_output_buffer ( OMX_COMPONENTTYPE *dec_handle,
                                  OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                  OMX_U32 nPortIndex,
                                  OMX_U32 bufSize,
                                  long bufCntMin)
{
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    OMX_ERRORTYPE error=OMX_ErrorNone;
    long bufCnt=0;
    OMX_U8* pvirt = NULL;

    *pBufHdrs= (OMX_BUFFERHEADERTYPE **)
                   malloc(sizeof(OMX_BUFFERHEADERTYPE)* bufCntMin);
    if(*pBufHdrs == NULL){
        DEBUGT_PRINT_ERROR(" m_inp_heap_ptr Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }
    output_use_buffer = true;
    p_eglHeaders = (struct temp_egl **)
                    malloc(sizeof(struct temp_egl *)* bufCntMin);
    if (!p_eglHeaders){
        DEBUGT_PRINT_ERROR(" EGL allocation failed");
        return OMX_ErrorInsufficientResources;
    }

    for(bufCnt=0; bufCnt < bufCntMin; ++bufCnt) {
        // allocate input buffers
      DEBUGT_PRINT("OMX_UseBuffer No %ld %d ", bufCnt, bufSize);
      p_eglHeaders[bufCnt] = (struct temp_egl*)
                         malloc(sizeof(struct temp_egl));
      if(!p_eglHeaders[bufCnt]) {
          DEBUGT_PRINT_ERROR(" EGL allocation failed");
          return OMX_ErrorInsufficientResources;
      }
      p_eglHeaders[bufCnt]->pmem_fd = open(PMEM_DEVICE,O_RDWR);
      p_eglHeaders[bufCnt]->offset = 0;
      if(p_eglHeaders[bufCnt]->pmem_fd < 0) {
          DEBUGT_PRINT_ERROR(" open failed %s",PMEM_DEVICE);
          return OMX_ErrorInsufficientResources;
      }
#ifndef USE_ION
      /* TBD - this commenting is dangerous */
      align_pmem_buffers(p_eglHeaders[bufCnt]->pmem_fd, bufSize,
                                  8192);
#endif
      DEBUGT_PRINT_ERROR(" allocation size %d pmem fd %d",bufSize,p_eglHeaders[bufCnt]->pmem_fd);
      pvirt = (unsigned char *)mmap(NULL,bufSize,PROT_READ|PROT_WRITE,
        MAP_SHARED,p_eglHeaders[bufCnt]->pmem_fd,0);
      DEBUGT_PRINT_ERROR(" Virtaul Address %p Size %d",pvirt,bufSize);
      if (pvirt == MAP_FAILED) {
        DEBUGT_PRINT_ERROR(" mmap failed for buffers");
        return OMX_ErrorInsufficientResources;
      }
        use_buf_virt_addr[bufCnt] = pvirt;
        error = OMX_UseEGLImage(dec_handle, &((*pBufHdrs)[bufCnt]),
                              nPortIndex, pvirt,(void *)p_eglHeaders[bufCnt]);
       }
    return error;
}

static void do_freeHandle_and_clean_up(bool isDueToError)
{
    int bufCnt = 0;
    OMX_STATETYPE state = OMX_StateInvalid;
    OMX_GetState(dec_handle, &state);
    if (state == OMX_StateExecuting || state == OMX_StatePause)
    {
      DEBUGT_PRINT("Requesting transition to Idle");
      OMX_SendCommand(dec_handle, OMX_CommandStateSet, OMX_StateIdle, 0);
      wait_for_event();
    }
    OMX_GetState(dec_handle, &state);
    if (state == OMX_StateIdle)
    {
      DEBUGT_PRINT("Requesting transition to Loaded");
      OMX_SendCommand(dec_handle, OMX_CommandStateSet, OMX_StateLoaded, 0);
      for(bufCnt=0; bufCnt < input_buf_cnt; ++bufCnt)
      {
         if (pInputBufHdrs[bufCnt]->pBuffer && input_use_buffer)
         {
            free(pInputBufHdrs[bufCnt]->pBuffer);
            pInputBufHdrs[bufCnt]->pBuffer = NULL;
            DEBUGT_PRINT_ERROR("Free(pInputBufHdrs[%d]->pBuffer)",bufCnt);
         }
         OMX_FreeBuffer(dec_handle, 0, pInputBufHdrs[bufCnt]);
      }
      if (pInputBufHdrs)
      {
         free(pInputBufHdrs);
         pInputBufHdrs = NULL;
      }
      for(bufCnt = 0; bufCnt < portFmt.nBufferCountActual; ++bufCnt) {
        if (output_use_buffer && p_eglHeaders) {
            if(p_eglHeaders[bufCnt]) {
               munmap (pOutYUVBufHdrs[bufCnt]->pBuffer,
                       pOutYUVBufHdrs[bufCnt]->nAllocLen);
               close(p_eglHeaders[bufCnt]->pmem_fd);
               p_eglHeaders[bufCnt]->pmem_fd = -1;
               free(p_eglHeaders[bufCnt]);
               p_eglHeaders[bufCnt] = NULL;
            }
        }
        if (use_external_pmem_buf)
        {
            DEBUGT_PRINT("Freeing in external pmem case: buffer=0x%p, pmem_fd=0x%lu",
                              pOutYUVBufHdrs[bufCnt]->pBuffer,
                              pPMEMInfo[bufCnt].pmem_fd);
            if (pOutYUVBufHdrs[bufCnt]->pBuffer)
            {
                munmap (pOutYUVBufHdrs[bufCnt]->pBuffer,
                        pOutYUVBufHdrs[bufCnt]->nAllocLen);
            }
            if (&pPMEMInfo[bufCnt])
            {
                close(pPMEMInfo[bufCnt].pmem_fd);
                pPMEMInfo[bufCnt].pmem_fd = -1;
            }
        }
        OMX_FreeBuffer(dec_handle, 1, pOutYUVBufHdrs[bufCnt]);
      }
      if(p_eglHeaders) {
          free(p_eglHeaders);
          p_eglHeaders = NULL;
      }
      if (pPMEMInfo)
      {
          DEBUGT_PRINT("Freeing in external pmem case:PMEM");
          free(pPMEMInfo);
          pPMEMInfo = NULL;
      }
      if (pPlatformEntry)
      {
          DEBUGT_PRINT("Freeing in external pmem case:ENTRY");
          free(pPlatformEntry);
          pPlatformEntry = NULL;
      }
      if (pPlatformList)
      {
          DEBUGT_PRINT("Freeing in external pmem case:LIST");
          free(pPlatformList);
          pPlatformList = NULL;
      }
      wait_for_event();
    }

    DEBUGT_PRINT("[OMX Vdec Test] - Free handle decoder\n");
    OMX_ERRORTYPE result = OMX_FreeHandle(dec_handle);
    if (result != OMX_ErrorNone)
    {
       DEBUGT_PRINT_ERROR("[OMX Vdec Test] - OMX_FreeHandle error. Error code: %d", result);
    }
    dec_handle = NULL;

    /* Deinit OpenMAX */
    DEBUGT_PRINT("[OMX Vdec Test] - De-initializing OMX ");
    OMX_Deinit();

    DEBUGT_PRINT("[OMX Vdec Test] - closing all files");
    if(inputBufferFileFd != -1)
    {
        close(inputBufferFileFd);
        inputBufferFileFd = -1;
    }

    DEBUGT_PRINT("[OMX Vdec Test] - after free inputfile");

    if (takeYuvLog && outputBufferFile) {
        fclose(outputBufferFile);
        outputBufferFile = NULL;
    }
#ifdef _MSM8974_
    if (crcFile) {
        fclose(crcFile);
        crcFile = NULL;
    }
#endif
    DEBUGT_PRINT("[OMX Vdec Test] - after free outputfile");

    if(etb_queue)
    {
      free_queue(etb_queue);
      etb_queue = NULL;
    }
    DEBUGT_PRINT("[OMX Vdec Test] - after free etb_queue ");
    if(fbd_queue)
    {
      free_queue(fbd_queue);
      fbd_queue = NULL;
    }
    DEBUGT_PRINT("[OMX Vdec Test] - after free iftb_queue");
    printf("*****************************************\n");
    if (isDueToError)
      printf("************...TEST FAILED...************\n");
    else
      printf("**********...TEST SUCCESSFUL...*********\n");
    printf("*****************************************\n");
}
boolean first_buf = 1;
#ifdef READ_FROM_SOCKET
static int Read_Buffer_From_Socket(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    int len = 0;
    while( ! bRtpReceiverStopped ) {
       if(!first_buf)
         MM_Timer_Sleep(diffTime/1000);
       pthread_mutex_lock(&rtp_lock);
       if ( ! is_empty_queue()) {
         pthread_mutex_unlock(&rtp_lock);
         struct queue_buffer *queue_buffer = pop_queue();
         len = queue_buffer->len;
         if (len == 0) {
            DEBUGT_PRINT("get Zero buffer frame socket");
            return 0;
         }
         DEBUGT_PRINT("read len = %d",len);
         memcpy(pBufHdr->pBuffer,queue_buffer->buffer,len);
         first_buf = 0;
#ifdef TEST_TS_FROM_SEI
         if (timeStampLfile == 0)
           pBufHdr->nTimeStamp = 0;
         else
           pBufHdr->nTimeStamp = LLONG_MAX;
#else
         pBufHdr->nTimeStamp = timeStampLfile;
#endif
         timeStampLfile += timestampInterval;

         if (queue_buffer->buffer)
           free(queue_buffer->buffer);
         free(queue_buffer);
         return len;
       }
       else {
         DEBUGT_PRINT("read buffer g_queue_is_empty !");
         pthread_mutex_unlock(&rtp_lock);
         usleep(1000);
      }
    }
    return 0;
}
#endif
static int Read_Buffer_From_DAT_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    long frameSize=0;
    char temp_buffer[10];
    int bytes_read=0;
    char inputFrameSize[12];
    int count =0, cnt =0;
    memset(temp_buffer, 0, sizeof(temp_buffer));

    DEBUGT_PRINT("Inside %s ", __FUNCTION__);

    while (cnt < 10)
    /* Check the input file format, may result in infinite loop */
    {
        DEBUGT_PRINT("loop[%d] count[%d]",cnt,count);
        count = read( inputBufferFileFd, &inputFrameSize[cnt], 1);
        if(inputFrameSize[cnt] == '\0' )
          break;
        cnt++;
    }
    inputFrameSize[cnt]='\0';
    frameSize = atoi(inputFrameSize);
    pBufHdr->nFilledLen = 0;

    /* get the frame length */
    lseek64(inputBufferFileFd, -1, SEEK_CUR);
    bytes_read = read(inputBufferFileFd, pBufHdr->pBuffer, frameSize);

    DEBUGT_PRINT("Actual frame Size [%ld] bytes_read using fread[%d]",
                  frameSize, bytes_read);

    if(bytes_read == 0 || bytes_read < frameSize ) {
        DEBUGT_PRINT("Bytes read Zero After Read frame Size ");
        DEBUGT_PRINT("Checking VideoPlayback Count:video_playback_count is:%d",
                       video_playback_count);
        return 0;
    }
    pBufHdr->nTimeStamp = timeStampLfile;
    timeStampLfile += timestampInterval;
    return bytes_read;
}

static int Read_Buffer_From_H264_Start_Code_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    int bytes_read = 0;
    int cnt = 0;
    unsigned int code = 0;
    int naluType = 0;
    int newFrame = 0;
    char *dataptr = (char *)pBufHdr->pBuffer;
    DEBUGT_PRINT("Inside %s", __FUNCTION__);
    do
    {
        newFrame = 0;
        bytes_read = read(inputBufferFileFd, &dataptr[cnt], 1);
        if (!bytes_read)
        {
            DEBUGT_PRINT("%s: Bytes read Zero", __FUNCTION__);
            break;
        }
        code <<= 8;
        code |= (0x000000FF & dataptr[cnt]);
        cnt++;
        if ((cnt == 4) && (code != H264_START_CODE))
        {
            DEBUGT_PRINT_ERROR("%s: ERROR: Invalid start code found 0x%x", __FUNCTION__, code);
            cnt = 0;
            break;
        }
        if ((cnt > 4) && (code == H264_START_CODE))
        {
            DEBUGT_PRINT("%s: Found H264_START_CODE", __FUNCTION__);
            bytes_read = read(inputBufferFileFd, &dataptr[cnt], 1);
            if (!bytes_read)
            {
                DEBUGT_PRINT("%s: Bytes read Zero", __FUNCTION__);
                break;
            }
            DEBUGT_PRINT("%s: READ Byte[%d] = 0x%x", __FUNCTION__, cnt, dataptr[cnt]);
            naluType = dataptr[cnt] & 0x1F;
            cnt++;
            if ((naluType == 1) || (naluType == 5))
            {
                DEBUGT_PRINT("%s: Found AU", __FUNCTION__);
                bytes_read = read(inputBufferFileFd, &dataptr[cnt], 1);
                if (!bytes_read)
                {
                    DEBUGT_PRINT("%s: Bytes read Zero", __FUNCTION__);
                    break;
                }
                DEBUGT_PRINT("%s: READ Byte[%d] = 0x%x", __FUNCTION__, cnt, dataptr[cnt]);
                newFrame = (dataptr[cnt] & 0x80);
                cnt++;
                if (newFrame)
                {
                    lseek64(inputBufferFileFd, -6, SEEK_CUR);
                    cnt -= 6;
                    DEBUGT_PRINT("%s: Found a NAL unit (type 0x%x) of size = %d", __FUNCTION__, (dataptr[4] & 0x1F), cnt);
                    break;
                }
                else
                {
                    DEBUGT_PRINT("%s: Not a New Frame", __FUNCTION__);
                }
            }
            else
            {
                lseek64(inputBufferFileFd, -5, SEEK_CUR);
                cnt -= 5;
                DEBUGT_PRINT("%s: Found NAL unit (type 0x%x) of size = %d", __FUNCTION__, (dataptr[4] & 0x1F), cnt);
                break;
            }
        }
    } while (1);

#ifdef TEST_TS_FROM_SEI
    if (timeStampLfile == 0)
      pBufHdr->nTimeStamp = 0;
    else
      pBufHdr->nTimeStamp = LLONG_MAX;
#else
    pBufHdr->nTimeStamp = timeStampLfile;
#endif
    timeStampLfile += timestampInterval;

    return cnt;
}

static int Read_Buffer_ArbitraryBytes(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    int bytes_read=0;
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    bytes_read = read(inputBufferFileFd, pBufHdr->pBuffer, NUMBER_OF_ARBITRARYBYTES_READ);
    if(bytes_read == 0) {
        DEBUGT_PRINT("Bytes read Zero After Read frame Size ");
        DEBUGT_PRINT("Checking VideoPlayback Count:video_playback_count is:%d",
                      video_playback_count);
        return 0;
    }
#ifdef TEST_TS_FROM_SEI
    if (timeStampLfile == 0)
      pBufHdr->nTimeStamp = 0;
    else
      pBufHdr->nTimeStamp = LLONG_MAX;
#else
    pBufHdr->nTimeStamp = timeStampLfile;
#endif
    timeStampLfile += timestampInterval;
    return bytes_read;
}

static int Read_Buffer_From_MP4_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    unsigned int i = 0;
    uint32 nBytes = 0;
    static uint32 max_size = 0;
    FileSourceMediaStatus error = FILE_SOURCE_DATA_OK;
    FileSourceStatus status = FILE_SOURCE_SUCCESS;
    static unsigned int readOffset = 0;
    static int openfile_status = 0;

    FileSourceMjMediaType majorType;
    FileSourceMnMediaType minorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
    static FileSourceTrackIdInfoType trackInfo;
    FileSourceSampleInfo sampleInfo;

    wchar_t pStr[512];

    if(openfile_status==0){
        m_pFileSource = new FileSource(cbFileSourceStatus, NULL, false);

        DEBUGT_PRINT("OpenFile start for MP4 = %s\n", in_filename);

        //int count = Char_To_WideChar(in_filename, strlen(in_filename), pStr, 512, FALSE);
        int count = mbstowcs(pStr, in_filename, 512);

        status = m_pFileSource->OpenFile(NULL, pStr, NULL, FILE_SOURCE_MPEG4, true);
        if(FILE_SOURCE_SUCCESS != status) {
            DEBUGT_PRINT_ERROR("OpenFile failed for MP4\n");
            m_pFileSource->CloseFile();
            return 0;
        }

        FileSourceTrackIdInfoType trackList[FILE_SOURCE_MAX_NUM_TRACKS];
        uint32_t numTracks = m_pFileSource->GetWholeTracksIDList(trackList);
        DEBUGT_PRINT("numTracks = %d\n",numTracks);

        for (i = 0;i < numTracks ; i++)
        {
            trackInfo = trackList[i];
            error = FILE_SOURCE_DATA_OK;
            DEBUGT_PRINT("i %d, trackInfo.id %d\n",i,trackInfo.id);
            status = m_pFileSource->GetMimeType(trackInfo.id, majorType, minorType);
            if(FILE_SOURCE_SUCCESS != status) {
                DEBUGT_PRINT_ERROR("OpenFile failed for MP4\n");
                m_pFileSource->CloseFile();
                return 0;
            }
            if (majorType == FILE_SOURCE_MJ_TYPE_VIDEO)
            {
                DEBUGT_PRINT("video tracks = %d",i);
                break;
            }
        }
        if (i == numTracks) {
            DEBUGT_PRINT_ERROR("error no video tracks\n");
            return 0;
        }
        DEBUGT_PRINT("has video tracks\n");

        max_size = m_pFileSource->GetTrackMaxFrameBufferSize(trackInfo.id);
        DEBUGT_PRINT("max_size = %d\n", max_size);

        nBytes = max_size;
        memset(&sampleInfo, 0, sizeof(FileSourceSampleInfo));
        status = m_pFileSource->GetFormatBlock(trackInfo.id,
                                                       pBufHdr->pBuffer,
                                                       &nBytes,
                                                       false);
        openfile_status = 1;
    }else{
        nBytes = max_size;
        memset(&sampleInfo, 0, sizeof(FileSourceSampleInfo));
        error = m_pFileSource->GetNextMediaSample(trackInfo.id,
                                                       pBufHdr->pBuffer,
                                                       &nBytes,
                                                       sampleInfo);
    }
    printf("Read_Buffer_From_MP4_File framenumber = %d, nBytes = %d\n", readOffset, nBytes);
    if(nBytes!=0){
        readOffset++;
    }else{
        DEBUGT_PRINT_ERROR("Read_Buffer_From_MP4_File error for size 0\n");
        m_pFileSource->CloseFile();
        return 0;
    }

    if(error != FILE_SOURCE_DATA_OK){
        if(error == FILE_SOURCE_DATA_END){
            DEBUGT_PRINT_ERROR("Read_Buffer_From_MP4_File END - Seek to First\n");
            uint64 current_time = m_pFileSource->GetMediaCurrentPosition(trackInfo.id);
            status = m_pFileSource->SeekAbsolutePosition(0, true, current_time, FS_SEEK_DEFAULT);
            if(status != FILE_SOURCE_SUCCESS)
                DEBUGT_PRINT_ERROR("Failed to Seek to Zero\n");
        } else {
            DEBUGT_PRINT_ERROR("buf_len %d, error %d, i %d, trackInfo.id %d",nBytes,error,i, trackInfo.id);
        }
    }

    pBufHdr->nTimeStamp = sampleInfo.startTime;

    return nBytes;
}

static int Read_Buffer_From_Vop_Start_Code_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    unsigned int readOffset = 0;
    int bytes_read = 0;
    unsigned int code = 0;
    pBufHdr->nFilledLen = 0;
    static unsigned int header_code = 0;

    DEBUGT_PRINT("Inside %s", __FUNCTION__);

    do
    {
      //Start codes are always byte aligned.
      bytes_read = read(inputBufferFileFd, &pBufHdr->pBuffer[readOffset], 1);
      if(bytes_read == 0 || bytes_read == -1)
      {
          DEBUGT_PRINT("Bytes read Zero ");
          break;
      }
      code <<= 8;
      code |= (0x000000FF & pBufHdr->pBuffer[readOffset]);
      //VOP start code comparision
      if (readOffset>3)
      {
        if(!header_code ){
          if( VOP_START_CODE == code)
          {
            header_code = VOP_START_CODE;
          }
          else if ( (0xFFFFFC00 & code) == SHORT_HEADER_START_CODE )
          {
            header_code = SHORT_HEADER_START_CODE;
          }
        }
        if ((header_code == VOP_START_CODE) && (code == VOP_START_CODE))
        {
            //Seek backwards by 4
            lseek64(inputBufferFileFd, -4, SEEK_CUR);
            readOffset-=3;
            break;
        }
        else if (( header_code == SHORT_HEADER_START_CODE ) && ( SHORT_HEADER_START_CODE == (code & 0xFFFFFC00)))
        {
            //Seek backwards by 4
            lseek64(inputBufferFileFd, -4, SEEK_CUR);
            readOffset-=3;
            break;
        }
      }
      readOffset++;
    }while (1);
    pBufHdr->nTimeStamp = timeStampLfile;
    timeStampLfile += timestampInterval;
    return readOffset;
}
static int Read_Buffer_From_Mpeg2_Start_Code(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
  unsigned int readOffset = 0;
  int bytesRead = 0;
  unsigned int code = 0;
  pBufHdr->nFilledLen = 0;
  static unsigned int firstParse = true;
  unsigned int seenFrame = false;

  DEBUGT_PRINT("Inside %s", __FUNCTION__);

  /* Read one byte at a time. Construct the code every byte in order to
   * compare to the start codes. Keep looping until we've read in a complete
   * frame, which can be either just a picture start code + picture, or can
   * include the sequence header as well
   */
  while (1) {
    bytesRead = read(inputBufferFileFd, &pBufHdr->pBuffer[readOffset], 1);

    /* Exit the loop if we can't read any more bytes */
    if (bytesRead == 0 || bytesRead == -1) {
      break;
    }

    /* Construct the code one byte at a time */
    code <<= 8;
    code |= (0x000000FF & pBufHdr->pBuffer[readOffset]);

    /* Can't compare the code to MPEG2 start codes until we've read the
     * first four bytes
     */
    if (readOffset >= 3) {

      /* If this is the first time we're reading from the file, then we
       * need to throw away the system start code information at the
       * beginning. We can just look for the first sequence header.
       */
      if (firstParse) {
        if (code == MPEG2_SEQ_START_CODE) {
          /* Seek back by 4 bytes and reset code so that we can skip
           * down to the common case below.
           */
          lseek(inputBufferFileFd, -4, SEEK_CUR);
          code = 0;
          readOffset -= 3;
          firstParse = false;
          continue;
        }
      }

      /* If we have already parsed a frame and we see a sequence header, then
       * the sequence header is part of the next frame so we seek back and
       * break.
       */
      if (code == MPEG2_SEQ_START_CODE) {
        if (seenFrame) {
          lseek(inputBufferFileFd, -4, SEEK_CUR);
          readOffset -= 3;
          break;
        }
        /* If we haven't seen a frame yet, then read in all the data until we
         * either see another frame start code or sequence header start code.
         */
      } else if (code == MPEG2_FRAME_START_CODE) {
        if (!seenFrame) {
          seenFrame = true;
        } else {
          lseek(inputBufferFileFd, -4, SEEK_CUR);
          readOffset -= 3;
          break;
        }
      }
    }

    readOffset++;
  }

  pBufHdr->nTimeStamp = timeStampLfile;
  timeStampLfile += timestampInterval;
  return readOffset;
}


static int Read_Buffer_From_Size_Nal(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    // NAL unit stream processing
    char temp_size[SIZE_NAL_FIELD_MAX];
    int i = 0;
    int j = 0;
    unsigned int size = 0;   // Need to make sure that uint32 has SIZE_NAL_FIELD_MAX (4) bytes
    int bytes_read = 0;

    // read the "size_nal_field"-byte size field
    bytes_read = read(inputBufferFileFd, pBufHdr->pBuffer + pBufHdr->nOffset, nalSize);
    if (bytes_read == 0 || bytes_read == -1)
    {
      DEBUGT_PRINT("Failed to read frame or it might be EOF");
      return 0;
    }

    for (i=0; i<SIZE_NAL_FIELD_MAX-nalSize; i++)
    {
      temp_size[SIZE_NAL_FIELD_MAX - 1 - i] = 0;
    }

    /* Due to little endiannes, Reorder the size based on size_nal_field */
    for (j=0; i<SIZE_NAL_FIELD_MAX; i++, j++)
    {
      temp_size[SIZE_NAL_FIELD_MAX - 1 - i] = pBufHdr->pBuffer[pBufHdr->nOffset + j];
    }
    size = (unsigned int)(*((unsigned int *)(temp_size)));

    // now read the data
    bytes_read = read(inputBufferFileFd, pBufHdr->pBuffer + pBufHdr->nOffset + nalSize, size);
    if (bytes_read != size)
    {
      DEBUGT_PRINT_ERROR("Failed to read frame");
    }

    pBufHdr->nTimeStamp = timeStampLfile;
    timeStampLfile += timestampInterval;

    return bytes_read + nalSize;
}

static int Read_Buffer_From_RCV_File_Seq_Layer(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    unsigned int readOffset = 0, size_struct_C = 0;
    unsigned int startcode = 0;
    pBufHdr->nFilledLen = 0;
#ifdef _MSM8974_
    pBufHdr->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
#else
    pBufHdr->nFlags = 0;
#endif

    DEBUGT_PRINT("Inside %s ", __FUNCTION__);

    if (read(inputBufferFileFd, &startcode, 4) <= 0)
        DEBUGT_PRINT_ERROR("Error while reading");

    /* read size of struct C as it need not be 4 always*/
    if (read(inputBufferFileFd, &size_struct_C, 4) <= 0)
        DEBUGT_PRINT_ERROR("Error while reading");

#ifndef _MSM8974_
    /* reseek to beginning of sequence header */
    lseek64(inputBufferFileFd, -8, SEEK_CUR);
#endif
    if ((startcode & 0xFF000000) == 0xC5000000)
    {

      DEBUGT_PRINT("Read_Buffer_From_RCV_File_Seq_Layer size_struct_C: %d", size_struct_C);
#ifdef _MSM8974_
      readOffset = read(inputBufferFileFd, pBufHdr->pBuffer, size_struct_C);
      lseek64(inputBufferFileFd, 24, SEEK_CUR);
#else
      readOffset = read(inputBufferFileFd, pBufHdr->pBuffer, VC1_SEQ_LAYER_SIZE_WITHOUT_STRUCTC + size_struct_C);
#endif
    }
    else if((startcode & 0xFF000000) == 0x85000000)
    {
      // .RCV V1 file

      rcv_v1 = 1;

      DEBUGT_PRINT("Read_Buffer_From_RCV_File_Seq_Layer size_struct_C: %d", size_struct_C);
#ifdef _MSM8974_
      readOffset = read(inputBufferFileFd, pBufHdr->pBuffer, size_struct_C);
      lseek64(inputBufferFileFd, 8, SEEK_CUR);
#else
      readOffset = read(inputBufferFileFd, pBufHdr->pBuffer, VC1_SEQ_LAYER_SIZE_V1_WITHOUT_STRUCTC + size_struct_C);
#endif

    }
    else
    {
      DEBUGT_PRINT_ERROR("Error: Unknown VC1 clip format %x", startcode);
    }

    return readOffset;
}

static int Read_Buffer_From_RCV_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    unsigned int readOffset = 0;
    unsigned int len = 0;
    unsigned int key = 0;
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);

    DEBUGT_PRINT("Read_Buffer_From_RCV_File - nOffset %d", pBufHdr->nOffset);
    if(rcv_v1)
    {
      /* for the case of RCV V1 format, the frame header is only of 4 bytes and has
         only the frame size information */
        readOffset = read(inputBufferFileFd, &len, 4);
        DEBUGT_PRINT("Read_Buffer_From_RCV_File - framesize %d %x", len, len);

    }
    else
    {
      /* for a regular RCV file, 3 bytes comprise the frame size and 1 byte for key*/
        readOffset = read(inputBufferFileFd, &len, 3);
        DEBUGT_PRINT("Read_Buffer_From_RCV_File - framesize %d %x", len, len);

        readOffset = read(inputBufferFileFd, &key, 1);
      if ( (key & 0x80) == false)
      {
        DEBUGT_PRINT("Read_Buffer_From_RCV_File - Non IDR frame key %x", key);
       }

    }

    if(!rcv_v1)
    {
      /* There is timestamp field only for regular RCV format and not for RCV V1 format*/
        readOffset = read(inputBufferFileFd, &pBufHdr->nTimeStamp, 4);
        DEBUGT_PRINT("Read_Buffer_From_RCV_File - timeStamp %lld", pBufHdr->nTimeStamp);
        pBufHdr->nTimeStamp *= 1000;
    }
    else
    {
        pBufHdr->nTimeStamp = timeStampLfile;
        timeStampLfile += timestampInterval;
    }

    if(len > pBufHdr->nAllocLen)
    {
       DEBUGT_PRINT_ERROR("Error in sufficient buffer framesize %d, allocalen %d noffset %d",len,pBufHdr->nAllocLen, pBufHdr->nOffset);
       readOffset = read(inputBufferFileFd, pBufHdr->pBuffer+pBufHdr->nOffset,
                         pBufHdr->nAllocLen - pBufHdr->nOffset);

       loff_t off = (len - readOffset)*1LL;
       lseek64(inputBufferFileFd, off ,SEEK_CUR);
       return readOffset;
    }
    else {
        readOffset = read(inputBufferFileFd, pBufHdr->pBuffer+pBufHdr->nOffset, len);
    }
    if (readOffset != len)
    {
      DEBUGT_PRINT("EOS reach or Reading error %d, %s ", readOffset, strerror( errno ));
      return 0;
    }

    return readOffset;
}

static int Read_Buffer_From_VC1_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    static int timeStampLfile = 0;
    OMX_U8 *pBuffer = pBufHdr->pBuffer + pBufHdr->nOffset;
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    unsigned int readOffset = 0;
    int bytes_read = 0;
    unsigned int code = 0, total_bytes = 0;
    int startCode_cnt = 0;
    int bSEQflag = 0;
    int bEntryflag = 0;
    unsigned int SEQbytes = 0;
    int numStartcodes = 0;

    numStartcodes = bHdrflag?1:2;

    do
    {
      if (total_bytes == pBufHdr->nAllocLen)
      {
        DEBUGT_PRINT_ERROR("Buffer overflow!");
        break;
      }
      //Start codes are always byte aligned.
      bytes_read = read(inputBufferFileFd, &pBuffer[readOffset],1 );

      if(!bytes_read)
      {
          DEBUGT_PRINT(" Bytes read Zero ");
          break;
      }
      total_bytes++;
      code <<= 8;
      code |= (0x000000FF & pBufHdr->pBuffer[readOffset]);

     if(!bSEQflag && (code == VC1_SEQUENCE_START_CODE)) {
        if(startCode_cnt) bSEQflag = 1;
      }

      if(!bEntryflag && ( code == VC1_ENTRY_POINT_START_CODE)) {
         if(startCode_cnt) bEntryflag = 1;
      }

      if(code == VC1_FRAME_START_CODE || code == VC1_FRAME_FIELD_CODE)
      {
         startCode_cnt++ ;
      }

      //VOP start code comparision
      if(startCode_cnt == numStartcodes)
      {
        if (VC1_FRAME_START_CODE == (code & 0xFFFFFFFF) ||
            VC1_FRAME_FIELD_CODE == (code & 0xFFFFFFFF))
        {
          previous_vc1_au = 0;
          if(VC1_FRAME_FIELD_CODE == (code & 0xFFFFFFFF))
          {
              previous_vc1_au = 1;
          }

          if(!bHdrflag && (bSEQflag || bEntryflag)) {
             lseek(inputBufferFileFd,-(SEQbytes+4),SEEK_CUR);
             readOffset -= (SEQbytes+3);
          }
          else {
            //Seek backwards by 4
            lseek64(inputBufferFileFd, -4, SEEK_CUR);
            readOffset-=3;
          }

          while(pBufHdr->pBuffer[readOffset-1] == 0)
            readOffset--;

          break;
        }
      }
      readOffset++;
      if(bSEQflag || bEntryflag) {
        SEQbytes++;
      }
    }while (1);

    pBufHdr->nTimeStamp = timeStampLfile;
    timeStampLfile += timestampInterval;

    return readOffset;
}

static int Read_Buffer_From_DivX_4_5_6_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
#define MAX_NO_B_FRMS 3 // Number of non-b-frames packed in each buffer
#define N_PREV_FRMS_B 1 // Number of previous non-b-frames packed
                        // with a set of consecutive b-frames
#define FRM_ARRAY_SIZE (MAX_NO_B_FRMS + N_PREV_FRMS_B)
    char *p_buffer = NULL;
    unsigned int offset_array[FRM_ARRAY_SIZE];
    int byte_cntr, pckt_end_idx;
    unsigned int read_code = 0, bytes_read, byte_pos = 0, frame_type;
    unsigned int i, b_frm_idx, b_frames_found = 0, vop_set_cntr = 0;
    bool pckt_ready = false;
#ifdef __DEBUG_DIVX__
    char pckt_type[20];
    int pckd_frms = 0;
    static unsigned long long int total_bytes = 0;
    static unsigned long long int total_frames = 0;
#endif //__DEBUG_DIVX__

    DEBUGT_PRINT("Inside %s ", __FUNCTION__);

    do {
      p_buffer = (char *)pBufHdr->pBuffer + byte_pos;

      bytes_read = read(inputBufferFileFd, p_buffer, NUMBER_OF_ARBITRARYBYTES_READ);
      byte_pos += bytes_read;
      for (byte_cntr = 0; byte_cntr < bytes_read && !pckt_ready; byte_cntr++) {
        read_code <<= 8;
        ((char*)&read_code)[0] = p_buffer[byte_cntr];
        if (read_code == VOP_START_CODE) {
          if (++byte_cntr < bytes_read) {
            frame_type = p_buffer[byte_cntr];
            frame_type &= 0x000000C0;
#ifdef __DEBUG_DIVX__
            switch (frame_type) {
              case 0x00: pckt_type[pckd_frms] = 'I'; break;
              case 0x40: pckt_type[pckd_frms] = 'P'; break;
              case 0x80: pckt_type[pckd_frms] = 'B'; break;
              default: pckt_type[pckd_frms] = 'X';
            }
            pckd_frms++;
#endif // __DEBUG_DIVX__
            offset_array[vop_set_cntr] = byte_pos - bytes_read + byte_cntr - 4;
            if (frame_type == 0x80) { // B Frame found!
              if (!b_frames_found) {
                // Try to packet N_PREV_FRMS_B previous frames
                // with the next consecutive B frames
                i = N_PREV_FRMS_B;
                while ((vop_set_cntr - i) < 0 && i > 0) i--;
                b_frm_idx = vop_set_cntr - i;
                if (b_frm_idx > 0) {
                  pckt_end_idx = b_frm_idx;
                  pckt_ready = true;
#ifdef __DEBUG_DIVX__
                  pckt_type[b_frm_idx] = '\0';
                  total_frames += b_frm_idx;
#endif //__DEBUG_DIVX__
                }
              }
              b_frames_found++;
            } else if (b_frames_found) {
              pckt_end_idx = vop_set_cntr;
              pckt_ready = true;
#ifdef __DEBUG_DIVX__
              pckt_type[pckd_frms - 1] = '\0';
              total_frames += pckd_frms - 1;
#endif //__DEBUG_DIVX__
            } else if (vop_set_cntr == (FRM_ARRAY_SIZE -1)) {
              pckt_end_idx = MAX_NO_B_FRMS;
              pckt_ready = true;
#ifdef __DEBUG_DIVX__
              pckt_type[pckt_end_idx] = '\0';
              total_frames += pckt_end_idx;
#endif //__DEBUG_DIVX__
            } else
              vop_set_cntr++;
          } else {
            // The vop start code was found in the last 4 bytes,
            // seek backwards by 4 to include this start code
            // with the next buffer.
            lseek64(inputBufferFileFd, -4, SEEK_CUR);
            byte_pos -= 4;
#ifdef __DEBUG_DIVX__
            pckd_frms--;
#endif //__DEBUG_DIVX__
          }
        }
      }
      if (pckt_ready) {
          loff_t off = (byte_pos - offset_array[pckt_end_idx]);
          if ( lseek64(inputBufferFileFd, -1LL*off , SEEK_CUR) == -1 ){
              DEBUGT_PRINT_ERROR("lseek64 with offset = %lld failed with errno %d"
                                ", current position =0x%llx", -1LL*off,
                                errno, (unsigned long long)lseek64(inputBufferFileFd, 0, SEEK_CUR));
          }
      }
      else {
          char eofByte;
          int ret = read(inputBufferFileFd, &eofByte, 1 );
          if ( ret == 0 ) {
              offset_array[vop_set_cntr] = byte_pos;
              pckt_end_idx = vop_set_cntr;
              pckt_ready = true;
#ifdef __DEBUG_DIVX__
              pckt_type[pckd_frms] = '\0';
              total_frames += pckd_frms;
#endif //__DEBUG_DIVX__
          }
          else if (ret == 1){
              if ( lseek64(inputBufferFileFd, -1, SEEK_CUR ) == -1 ){
                  DEBUGT_PRINT_ERROR("lseek64 failed with errno = %d, "
                                    "current fileposition = %llx",
                                    errno,
                                    (unsigned long long)lseek64(inputBufferFileFd, 0, SEEK_CUR));
              }
          }
          else {
              DEBUGT_PRINT_ERROR("Error when checking for EOF");
          }
      }
    } while (!pckt_ready);
    pBufHdr->nFilledLen = offset_array[pckt_end_idx];
    pBufHdr->nTimeStamp = timeStampLfile;
    timeStampLfile += timestampInterval;
#ifdef __DEBUG_DIVX__
    total_bytes += pBufHdr->nFilledLen;
    ALOGE("[DivX] Packet: Type[%s] Size[%u] TS[%lld] TB[%llx] NFrms[%lld]",
      pckt_type, pBufHdr->nFilledLen, pBufHdr->nTimeStamp,
	  total_bytes, total_frames);
#endif //__DEBUG_DIVX__
    return pBufHdr->nFilledLen;
}

static int Read_Buffer_From_DivX_311_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    static OMX_S64 timeStampLfile = 0;
    char *p_buffer = NULL;
    unsigned int bytes_read = 0;
    unsigned int frame_size = 0;
    unsigned int num_bytes_size = 4;
    unsigned int n_offset = 0;

    DEBUGT_PRINT("Inside %s ", __FUNCTION__);

    pBufHdr->nTimeStamp = timeStampLfile;

    if (pBufHdr != NULL)
    {
        n_offset = pBufHdr->nOffset;
        p_buffer = (char *)pBufHdr->pBuffer + pBufHdr->nOffset;
    }
    else
    {
        DEBUGT_PRINT(" ERROR:Read_Buffer_From_DivX_311_File: pBufHdr is NULL");
        return 0;
    }

    if (p_buffer == NULL)
    {
        DEBUGT_PRINT(" ERROR:Read_Buffer_From_DivX_311_File: p_bufhdr is NULL");
        return 0;
    }

    //Read first frame based on size
    //DivX 311 frame - 4 byte header with size followed by the frame

    bytes_read = read(inputBufferFileFd, &frame_size, num_bytes_size);

    DEBUGT_PRINT("Read_Buffer_From_DivX_311_File: Frame size = %d", frame_size);
    n_offset += read(inputBufferFileFd, p_buffer, frame_size);

    pBufHdr->nTimeStamp = timeStampLfile;

    timeStampLfile += timestampInterval;

    //the packet is ready to be sent
    DEBUGT_PRINT("Returning Read Buffer from Divx 311: TS=[%ld], Offset=[%d]",
           (long int)pBufHdr->nTimeStamp,
           n_offset );

    return n_offset;
}

#ifdef _MSM8974_
static int Read_Buffer_From_VP8_File(OMX_BUFFERHEADERTYPE  *pBufHdr)
{
    char *p_buffer = NULL;
    unsigned int bytes_read = 0;
    unsigned int frame_size = 0;
    unsigned int n_offset = 0;
    unsigned long long time_stamp;
    static int ivf_header_read;

    if (pBufHdr != NULL)
    {
        n_offset = pBufHdr->nOffset;
        p_buffer = (char *)pBufHdr->pBuffer + pBufHdr->nOffset;
    }
    else
    {
        DEBUGT_PRINT(" ERROR:Read_Buffer_From_DivX_311_File: pBufHdr is NULL");
        return 0;
    }

    if (p_buffer == NULL)
    {
        DEBUGT_PRINT(" ERROR:Read_Buffer_From_DivX_311_File: p_bufhdr is NULL");
        return 0;
    }

    if(ivf_header_read == 0) {
        bytes_read = read(inputBufferFileFd, p_buffer, 32);
        ivf_header_read = 1;
        if(p_buffer[0] == 'D' && p_buffer[1] == 'K' && p_buffer[2] == 'I' && p_buffer[3] == 'F')
        {
            DEBUGT_PRINT("  IVF header found  ");
        } else
        {
            DEBUGT_PRINT("  No IVF header found  ");
            lseek(inputBufferFileFd, -32, SEEK_CUR);
        }
    }
    bytes_read = read(inputBufferFileFd, &frame_size, 4);
    bytes_read = read(inputBufferFileFd, &time_stamp, 8);
    n_offset += read(inputBufferFileFd, p_buffer, frame_size);
    pBufHdr->nTimeStamp = time_stamp;
    return n_offset;
}
#endif
static int open_video_file ()
{
    int error_code = 0;
    char outputfilename[512];
    if ( !socketaddr ) {
       DEBUGT_PRINT("Inside %s filename=%s", __FUNCTION__, in_filename);

       if ( (inputBufferFileFd = open( in_filename, O_RDONLY | O_LARGEFILE) ) == -1 ){
         DEBUGT_PRINT_ERROR("Error - i/p file %s could NOT be opened errno = %d",
                          in_filename, errno);
         error_code = -1;
       }
       else {
        DEBUGT_PRINT_ERROR("i/p file %s is opened ", in_filename);
      }
    }

    if (takeYuvLog) {
        strlcpy(outputfilename, "yuvframes.yuv", 14);
        outputBufferFile = fopen (outputfilename, "ab");
        if (outputBufferFile == NULL)
        {
          DEBUGT_PRINT_ERROR("ERROR - o/p file %s could NOT be opened", outputfilename);
          error_code = -1;
        }
        else
        {
          DEBUGT_PRINT("O/p file %s is opened ", outputfilename);
        }
    }
    return error_code;
}

void swap_byte(char *pByte, int nbyte)
{
  int i=0;

  for (i=0; i<nbyte/2; i++)
  {
    pByte[i] ^= pByte[nbyte-i-1];
    pByte[nbyte-i-1] ^= pByte[i];
    pByte[i] ^= pByte[nbyte-i-1];
  }
}

int disable_output_port()
{
    DEBUGT_PRINT("DISABLING OP PORT");
    pthread_mutex_lock(&enable_lock);
    sent_disabled = 1;
    // Send DISABLE command
    OMX_SendCommand(dec_handle, OMX_CommandPortDisable, 1, 0);
    pthread_mutex_unlock(&enable_lock);
    // wait for Disable event to come back
    wait_for_event();
    if(p_eglHeaders) {
        free(p_eglHeaders);
        p_eglHeaders = NULL;
    }
    if (pPMEMInfo)
    {
        DEBUGT_PRINT("Freeing in external pmem case:PMEM");
        free(pPMEMInfo);
        pPMEMInfo = NULL;
    }
    if (pPlatformEntry)
    {
        DEBUGT_PRINT("Freeing in external pmem case:ENTRY");
        free(pPlatformEntry);
        pPlatformEntry = NULL;
    }
    if (pPlatformList)
    {
        DEBUGT_PRINT("Freeing in external pmem case:LIST");
        free(pPlatformList);
        pPlatformList = NULL;
    }
    if (currentStatus == ERROR_STATE)
    {
      do_freeHandle_and_clean_up(true);
      return -1;
    }
    DEBUGT_PRINT("OP PORT DISABLED!");
    return 0;
}

int enable_output_port()
{
    int bufCnt = 0;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    DEBUGT_PRINT("ENABLING OP PORT");
    // Send Enable command
    OMX_SendCommand(dec_handle, OMX_CommandPortEnable, 1, 0);
#ifndef USE_EGL_IMAGE_TEST_APP
    /* Allocate buffer on decoder's o/p port */
    portFmt.nPortIndex = 1;

    if (anti_flickering) {
        ret = OMX_GetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
        if (ret != OMX_ErrorNone) {
            DEBUGT_PRINT_ERROR("%s: OMX_GetParameter failed: %d",__FUNCTION__, ret);
            return -1;
        }
        portFmt.nBufferCountActual += 1;
        ret = OMX_SetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
        if (ret != OMX_ErrorNone) {
            DEBUGT_PRINT_ERROR("%s: OMX_SetParameter failed: %d",__FUNCTION__, ret);
            return -1;
        }
    }

    if (use_external_pmem_buf)
    {
        DEBUGT_PRINT("Enable op port: calling use_buffer_mult_fd");
        error =  use_output_buffer_multiple_fd(dec_handle,
                                               &pOutYUVBufHdrs,
                                               portFmt.nPortIndex,
                                               portFmt.nBufferSize,
                                               portFmt.nBufferCountActual);
    }
    else
    {
        error = Allocate_Buffer(dec_handle, &pOutYUVBufHdrs, portFmt.nPortIndex,
                                portFmt.nBufferCountActual, portFmt.nBufferSize);
    }
    if (error != OMX_ErrorNone) {
        DEBUGT_PRINT_ERROR("Error - OMX_AllocateBuffer Output buffer error");
        return -1;
    }
    else
    {
        DEBUGT_PRINT("OMX_AllocateBuffer Output buffer success");
        free_op_buf_cnt = portFmt.nBufferCountActual;
    }
#else
    error =  use_output_buffer(dec_handle,
                       &pOutYUVBufHdrs,
                        portFmt.nPortIndex,
                        portFmt.nBufferSize,
                        portFmt.nBufferCountActual);
    free_op_buf_cnt = portFmt.nBufferCountActual;
    if (error != OMX_ErrorNone) {
       DEBUGT_PRINT_ERROR("ERROR - OMX_UseBuffer Input buffer failed");
       return -1;
    }
    else {
       DEBUGT_PRINT("OMX_UseBuffer Input buffer success");
    }

#endif
    // wait for enable event to come back
    wait_for_event();
    if (currentStatus == ERROR_STATE)
    {
      do_freeHandle_and_clean_up(true);
      return -1;
    }
    if (pOutYUVBufHdrs == NULL)
    {
        DEBUGT_PRINT_ERROR("Error - pOutYUVBufHdrs is NULL");
        return -1;
    }
    for(bufCnt=0; bufCnt < portFmt.nBufferCountActual; ++bufCnt) {
        DEBUGT_PRINT("OMX_FillThisBuffer on output buf no.%d",bufCnt);
        if (pOutYUVBufHdrs[bufCnt] == NULL)
        {
            DEBUGT_PRINT_ERROR("Error - pOutYUVBufHdrs[%d] is NULL", bufCnt);
            return -1;
        }
        pOutYUVBufHdrs[bufCnt]->nOutputPortIndex = 1;
        pOutYUVBufHdrs[bufCnt]->nFlags &= ~OMX_BUFFERFLAG_EOS;
        ret = OMX_FillThisBuffer(dec_handle, pOutYUVBufHdrs[bufCnt]);
        if (OMX_ErrorNone != ret) {
            DEBUGT_PRINT_ERROR("ERROR - OMX_FillThisBuffer failed with result %d", ret);
        }
        else
        {
            DEBUGT_PRINT("OMX_FillThisBuffer success!");
            free_op_buf_cnt--;
        }
    }
    DEBUGT_PRINT("OP PORT ENABLED!");
    return 0;
}

int output_port_reconfig()
{
    DEBUGT_PRINT("PORT_SETTING_CHANGE_STATE");
    if (disable_output_port() != 0)
        return -1;

    /* Port for which the Client needs to obtain info */
    portFmt.nPortIndex = 1;
    OMX_GetParameter(dec_handle,OMX_IndexParamPortDefinition,&portFmt);
    DEBUGT_PRINT("Min Buffer Count=%d", portFmt.nBufferCountMin);
    DEBUGT_PRINT("Buffer Size=%d", portFmt.nBufferSize);
    if(OMX_DirOutput != portFmt.eDir) {
        DEBUGT_PRINT_ERROR("Error - Expect Output Port");
        return -1;
    }
    height = portFmt.format.video.nFrameHeight;
    width = portFmt.format.video.nFrameWidth;

    crop_rect.nWidth = width;
    crop_rect.nHeight = height;

    if (enable_output_port() != 0)
        return -1;
    DEBUGT_PRINT("PORT_SETTING_CHANGE DONE!");
    return 0;
}

void free_output_buffers()
{
    int index = 0;
    OMX_BUFFERHEADERTYPE *pBuffer = (OMX_BUFFERHEADERTYPE *)pop(fbd_queue);
    while (pBuffer) {
        DEBUGT_PRINT(" pOutYUVBufHdrs %p p_eglHeaders %p output_use_buffer %d",
               pOutYUVBufHdrs,p_eglHeaders,output_use_buffer);
        if(pOutYUVBufHdrs && p_eglHeaders && output_use_buffer)
        {
           index = pBuffer - pOutYUVBufHdrs[0];
           DEBUGT_PRINT(" Index of free buffer %d",index);
           DEBUGT_PRINT(" Address freed %p size freed %d",pBuffer->pBuffer,
                  pBuffer->nAllocLen);
           munmap((void *)use_buf_virt_addr[index],pBuffer->nAllocLen);
           if(p_eglHeaders[index])
           {
               close(p_eglHeaders[index]->pmem_fd);
               free(p_eglHeaders[index]);
               p_eglHeaders[index] = NULL;
           }
        }

        if (pOutYUVBufHdrs && use_external_pmem_buf)
        {
            index = pBuffer - pOutYUVBufHdrs[0];
            DEBUGT_PRINT(" Address freed %p size freed %d,virt=%p,pmem_fd=%lu",
                              pBuffer->pBuffer,
                              pBuffer->nAllocLen,
                              use_buf_virt_addr[index],
                              pPMEMInfo[index].pmem_fd);
            munmap((void *)use_buf_virt_addr[index],pBuffer->nAllocLen);
            getFreePmem();
            use_buf_virt_addr[index] = NULL;
            if(protocol != USE_GBM_PROTOCOL){
               if (&pPMEMInfo[index])
                {
                   close(pPMEMInfo[index].pmem_fd);
                   pPMEMInfo[index].pmem_fd = -1;
                 }
            }
//#ifdef USE_OUTPUT_BUFFER
#ifdef USE_ION
            free_ion(index);
#endif
        }
        DEBUGT_PRINT(" Free output buffer");
        OMX_FreeBuffer(dec_handle, 1, pBuffer);
        pBuffer = (OMX_BUFFERHEADERTYPE *)pop(fbd_queue);
    }
//#ifdef USE_OUTPUT_BUFFER
#ifdef USE_ION
    free_outport_ion();
#endif
}

#ifndef USE_ION
static bool align_pmem_buffers(int pmem_fd, OMX_U32 buffer_size,
                                  OMX_U32 alignment)
{
  struct pmem_allocation allocation;
  allocation.size = buffer_size;
  allocation.align = clip2(alignment);

  if (allocation.align < 4096)
  {
    allocation.align = 4096;
  }
  if (ioctl(pmem_fd, PMEM_ALLOCATE_ALIGNED, &allocation) < 0)
  {
    DEBUGT_PRINT_ERROR(" Aligment failed with pmem driver");
    return false;
  }
  return true;
}
#endif

