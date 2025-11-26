/*
 **************************************************************************************************
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include <sys/stat.h>
#include <string.h>
#include <cutils/properties.h>

#include "VideoPlatform.h"
#include "VideoSession.h"
#include "VideoDecoder.h"
#include "VideoRender.h"
#include "VideoMap.h"
#include <sstream>
using namespace videoMAP;
/* Global, default properties */
VideoStaticProperties gVideoDecProp;
int DEFAULT_WIDTH;
int DEFAULT_HEIGHT;
char* g_inputFilePtr;
u64 videoMapSize;

/* Global, default drm display params */
std::mutex plane_mutex;
std::vector<plane_config> plane_cfg;
std::vector<connector_config> connector_cfg;
int fd = -1;
drmModeAtomicReqPtr req;
int buf_idx = 0;

void initDecodeParams()
{
    strlcpy(gVideoDecProp.sInputRoot, DEC_INPUT_FILE_PATH, sizeof(DEC_INPUT_FILE_PATH));
    strlcpy(gVideoDecProp.sOutRoot, DEC_OUTPUT_FILE_PATH, sizeof(DEC_OUTPUT_FILE_PATH));
    strlcpy(gVideoDecProp.sInputFileName, INPUT_FILE_NAME, sizeof(INPUT_FILE_NAME));

    gVideoDecProp.nDecCapPortBufCnt = DEFAULT_NUM_CAPTURE_BUF;
    gVideoDecProp.nDecOutPortBufCnt = DEFAULT_NUM_OUTPUT_BUF;
    gVideoDecProp.nDisplayBufCnt    = 3;

    gVideoDecProp.nDefaultHeight = DEFAULT_HEIGHT;
    gVideoDecProp.nDefaultWidth  = DEFAULT_WIDTH;

    gVideoDecProp.inputDump       = false;
    gVideoDecProp.outputDump      = false;
    gVideoDecProp.bDynamicBufMode = false;

    gVideoDecProp.nPollTimeout   = 300000;
    gVideoDecProp.nThreadTimeout = 300000;

    gVideoDecProp.logMask = 0x0;
    gVideoDecProp.nFwLog  = 0x0;
}

int initDRM(char *card_path)
{
    int ret = 0, rc = 0;

    fd = open(card_path, O_RDWR, 0);

    if (fd < 0) {
       DEBUGT_PRINT_ERROR("failed to open %s\n", card_path);
       ret = 1;
       return ret;
    }

    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);

    ret = parse_display();
    if (ret)
    {
       DEBUGT_PRINT_ERROR("parse_display failed\n");
       goto CLOSE_FD;
    }

    ret = create_fb(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    if (ret)
    {
       DEBUGT_PRINT_ERROR("Failed to create framebuffer\n");
       goto CLOSE_FD;
    }

    /* init atomic commit */
    req = drmModeAtomicAlloc();
    if (!req) {
       DEBUGT_PRINT_ERROR("failed to init atomic commit\n");
       ret = 1;
       goto FREE_BUFS;
    }

    /* setup crtc / connector */
    ret = setup_connector(req);
    if (ret) {
       DEBUGT_PRINT_ERROR("failed to setup connector\n");
       goto CLEAN_DRM;
    }
    return ret;

CLEAN_DRM:
    drmModeAtomicFree(req);
FREE_BUFS:
    /* destroy buffer */
    rc = destroy_buff();
    if (rc)
    {
       DEBUGT_PRINT_ERROR("failed to clean framebuffers\n");
    }
CLOSE_FD:
    if (fd)
    {
       close(fd);
    }
    return ret;
}

int deinitDRM()
{
    int ret = 0;

    drmModeAtomicFree(req);

    /* destroy buffer */
    ret = destroy_buff();
    if (ret)
       return ret;

    /* close fd */
    close(fd);

    return 0;
}

static int wait_for_file(const char* file, int sleep_msec, int count)
{
    int i;
    int ret = 0;
    int delay = sleep_msec * 1000;
    for (i = 0, ret = -1; i < count; i++)
    {
       if (access(file, F_OK) == 0)
       {
            ret = 0;
            break;
       }
       usleep(delay);
    }
    return ret;
}

/***************************************************
 * fetch_msm_drm_init_status()
 * description: check the msm_drm init_complete status
 ***************************************************/
#ifndef __ANDROID_T_AND_ABOVE__
static int fetch_msm_init_status()
{
    int msm_init_fd = 0;
    unsigned char buf = 0;
    int bytes_read = 0;
    int ret = 0;

    msm_init_fd = open(MSMDRM_INIT_PATH, O_RDONLY, 0);
    if (msm_init_fd < 0)
    {
       DEBUGT_PRINT_INFO("%s: Failed to open msm_drm init_complete file: %s", __func__, strerror(errno));
    }
    else
    {
       bytes_read = read(msm_init_fd, (void*)&buf, sizeof(unsigned char));
       if (bytes_read < 0)
          DEBUGT_PRINT_ERROR("%s: Error reading msm_drm init_complete file: %s", __func__, strerror(errno));

       close(msm_init_fd);

       if (buf == '1')
          ret = 1;
    }
    return ret;
}
#endif

int main(int argc, char* argv[])
{
    VideoSession *pSession = NULL;
    char card_path[MAX_BUF_SZ], value[MAX_BUF_SZ], boot_complete[MAX_BUF_SZ];
    int card = EARLY_VIDEO_DRI_CARD, ret = 0, idx = 0, counter = 0;
    static char marker[LOCAL_STR_LEN];
    clock_t tstart = 0;
    clock_t tnow = 0;
    double telapsed = 0;
    int retry_count = 0;
    if (argc < 2)
    {
        print_error_msg("No Extra Command Line Argument Passed for earlyVideo Test App",argv[0]);
        DEBUGT_PRINT_ERROR("No Extra Command Line Argument Passed for earlyVideo Test App\n");
        ret = -1;
        return ret;
    }
    std::string sWidth(argv[1]);
    DEBUGT_PRINT_ERROR("earlyvideo test app command line argv:[%s]\n",argv[1]);//debugging purpose
    std::ostringstream oss;
    if(sWidth == "default") //default 1280x720 resolution video clip
    {
        DEFAULT_WIDTH = 1280;
        DEFAULT_HEIGHT = 720;
        g_inputFilePtr = (char *)videoMap_default;
        videoMapSize = sizeof(videoMap_default);
        oss << videoMapSize;
        std::string intAsString(oss.str());
        print_error_msg("earlyvideo default 1280 videoMapSize:",intAsString.c_str()); //debugging purpose
    }
    else if(sWidth == "1280") //1280x720 customer specific video clip
    {
        DEFAULT_WIDTH = 1280;
        DEFAULT_HEIGHT = 720;
        g_inputFilePtr = (char *)videoMap_1280;
        videoMapSize = sizeof(videoMap_1280);
        oss << videoMapSize;
        std::string intAsString(oss.str());
        print_error_msg("earlyvideo 1280 videoMapSize:",intAsString.c_str()); //debugging purpose
    }
    else if(sWidth == "1920") //1920x1080 customer specific video clip
    {
        DEFAULT_WIDTH = 1920;
        DEFAULT_HEIGHT = 1080;
        g_inputFilePtr = (char *)videoMap_1920;
        videoMapSize = sizeof(videoMap_1920);
        oss << videoMapSize;
        std::string intAsString(oss.str());
        print_error_msg("early video 1920 videoMapSize:",intAsString.c_str()); //debugging purpose
    }


#ifdef __EARLYSERVICES__
#ifdef __ANDROID_T_AND_ABOVE__
    snprintf(card_path, sizeof(card_path), "/dev/dri/card%d", card);
#else
    snprintf(card_path, sizeof(card_path), "/early_services/dev/dri/card%d", card);
#endif
#else
    snprintf(card_path, sizeof(card_path), "/dev/dri/card%d", card);
#endif

    ret = wait_for_file(card_path, WAIT_TIME, WAIT_COUNT);
    if (ret == 0)
    {
       ret = wait_for_file(EARLY_VID_DEC_DEVICE, WAIT_TIME, WAIT_COUNT);
       if (ret == 0)
       {
#ifdef __ANDROID_T_AND_ABOVE__
          ret = wait_for_file(DEFAULT_SYSTEM_HEAP, WAIT_TIME, WAIT_COUNT);
          if (ret == 0)
          {
             place_marker("M - earlyVideo START");
          }
          else
          {
             print_error_msg("DMA Heap not ready", strerror(errno));
          }
#else
          place_marker("M - earlyVideo START");
#endif
       }
       else
       {
          print_error_msg("VIDEO Card failed to open", strerror(errno));
       }
    }
    else
    {
       print_error_msg("DRM Card failed to open", strerror(errno));
    }

#ifndef __ANDROID_T_AND_ABOVE__
    if (ret == 0)
    {
       /* wait for msm_drm drivers intialization to complete */
       tstart = clock();
       while (!fetch_msm_init_status())
       {
          /* Break when time taken is greater than */
          tnow = clock();
          telapsed = (double)(tnow - tstart) / CLOCKS_PER_SEC * 1000;
          if (telapsed > MSMDRM_INIT_WAIT_TO_MS)
             break;
       }
       if (telapsed > MSMDRM_INIT_WAIT_TO_MS)
       {
          print_error_msg("DRM not ready", strerror(errno));
       }
       else
       {
          print_error_msg("DRM ready", "no error");
       }
    }
#endif

    if (ret == 0)
    {
       while (retry_count < WAIT_COUNT)
       {
          ret = initDRM(card_path);
          if (ret)
          {
             retry_count++;
             usleep(WAIT_TIME * 1000);
          }
          else
          {
             print_error_msg("DRM init success", "no error");
             break;
          }
       }
       if (ret)
       {
          print_error_msg("DRM init failed", strerror(errno));
          DEBUGT_PRINT_ERROR("DRM init failed\n");
          return ret;
       }
       initDecodeParams();

       pSession = VideoDecoderInit();
       if (pSession)
       {
          pSession->m_bIsThread = TRUE;
          pthread_create(&pSession->m_sessionThread, NULL, pSession->ThreadRun, pSession);
       }
       else
       {
          DEBUGT_PRINT_ERROR("VideoDecoderInit failed\n");
          ret = -1;
          goto CLEANUP;
       }

       if (pSession)
       {
          pthread_join(pSession->m_sessionThread, NULL);
       }

       /* video file too small, finished before boot animation */
       while (1) {
            counter++;

            /* update per crtc */
            for (int i = 0; i < (int) connector_cfg.size(); i++) {
                uint32_t flags = DRM_MODE_ATOMIC_ALLOW_MODESET;

                if (idx >= MAX_BUFFER)
                idx = 0;

                /* update fb */
                update_fb(req, i, idx);

                /* asynchronous commit */
                flags |= DRM_MODE_ATOMIC_NONBLOCK;

                ret = drmModeAtomicCommit(fd, req, flags, 0);
                if (ret) {
                update_possible_crtcs();
                }

                /* clear previous commit */
                drmModeAtomicSetCursor(req, 0);
            }
            idx++;

            /* sleep for 15ms, simulating 60fps */
            usleep(15 * 1000);

            if (counter > MAX_ITER)
            break;
       } /* animation loop ends */

        place_marker("K - Early Video - Exit");

CLEANUP:
       for (int i = 0; i < (int)connector_cfg.size(); i++) {
            /* clear previous commit */
            drmModeAtomicSetCursor(req, 0);

            /* set inactive */
            drmModeAtomicAddProperty(req, connector_cfg[i].crtc_id,
                                    connector_cfg[i].active_pid,
                                    0);
            /* last commit */
            ret = drmModeAtomicCommit(fd, req,
                                    DRM_MODE_ATOMIC_ALLOW_MODESET, 0);
            if (ret)
            DEBUGT_PRINT_ERROR("last commit failed");
       }

       ret = deinitDRM();
       if (ret)
       {
            DEBUGT_PRINT_ERROR("DRM deinit failed\n");
       }
    }
    return ret;
}
