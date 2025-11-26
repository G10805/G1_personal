/* ===========================================================================
 * Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "ais_log.h"
#include "qcarcam.h"
#include "qcarcam_diag_types.h"

#include "AEEStdDef.h"

#define TIMER_THREAD_USLEEP 1000000

#define QCARCAMDIAG_DBGMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_QCARCAM_DIAG, AIS_LOG_LVL_DBG,  "QCARCAMDIAG %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_DBG], ##__VA_ARGS__)

#define QCARCAMDIAG_INFOMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_QCARCAM_DIAG, AIS_LOG_LVL_HIGH,  "QCARCAMDIAG %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_MED], ##__VA_ARGS__)

#define QCARCAMDIAG_ERRORMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_QCARCAM_DIAG, AIS_LOG_LVL_ERROR,  "QCARCAMDIAG %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ERROR], ##__VA_ARGS__)

#define QCARCAMDIAG_ALWZMSG(_fmt_, ...) \
    ais_log(AIS_MOD_ID_QCARCAM_DIAG, AIS_LOG_LVL_ALWAYS,  "QCARCAMDIAG %s:%d %s " _fmt_ "\n", __FUNCTION__, __LINE__, AIS_LOG_LVL_STR[AIS_LOG_LVL_ALWAYS], ##__VA_ARGS__)


/**
 * API to parse_diagnostics
 *
 * @param diagnostic information structure
 *
 */

void parse_diagnostics(void* pDiagnosticInfo)
{
    if(!pDiagnosticInfo)
        return;
    QCarCamDiagInfo* diagnosticInfo = (QCarCamDiagInfo*)pDiagnosticInfo;
    //parsing client Diagnostic Info
    QCarCamDiagClientInfo* pDiagClientInfo = diagnosticInfo->aisDiagClientInfo;
    for (uint32 i = 0; i < MAX_USR_CLIENTS; i++)
    {
        if (!pDiagClientInfo[i].usrHdl)
        {
            break;
        }
        else
        {
            QCarCamDiagClientInfo *usrInfo = &pDiagClientInfo[i];

            QCARCAMDIAG_INFOMSG("usrHdl:%p usr_state:%d inputId:%d opMode:%d inputDevId:%d"
                "csiPhyDevId:%d ifeDevId:%d rdiId:%d timeStampStart:%llu sofCounter:%llu frameCounter:%llu frameRate:%llu",
                usrInfo->usrHdl, usrInfo->state, usrInfo->inputId, usrInfo->opMode,
                usrInfo->inputDevId, usrInfo->csiphyDevId, usrInfo->ifeDevId, usrInfo->rdiId,
                usrInfo->timeStampStart, usrInfo->sofCounter, usrInfo->frameCounter, usrInfo->frameRate);
            for (uint32 j = 0; j < QCARCAM_MAX_NUM_BUFFERS; j++)
            {
                QCarCamDiagBufferInfo *bufInfo = &usrInfo->bufInfo[j];
                QCARCAMDIAG_INFOMSG("bufId:%d bufStatus:%d", bufInfo->bufId, bufInfo->bufStatus);
            }
        }
    }

    //parsing Input device statistics info
    QCarCamDiagInputDevInfo* pDiagInputInfo = diagnosticInfo->aisDiagInputDevInfo;
    for(uint32 i = 0; i < MAX_NUM_INPUT_DEVICES; i++)
    {
        QCarCamDiagInputDevInfo* inputInfo = &pDiagInputInfo[i];

        QCARCAMDIAG_INFOMSG("DevId:%d numSensors:%d cciDevId:%d cciPortId:%d state:%d srcIdEnableMask:%d", inputInfo->inputDevId, inputInfo->numSensors,
            inputInfo->cciMap.cciDevId, inputInfo->cciMap.cciPortId, inputInfo->state, inputInfo->srcIdEnableMask);

        for (uint32 j = 0; j < (uint32)inputInfo->numSensors; j++)
        {
            QCarCamDiagInputSrcInfo* sourceInfo = &inputInfo->inputSourceInfo[j];

            QCARCAMDIAG_INFOMSG("inputSrcId:%d status:%d fps:%.2f width:%d height:%d format:%d",
                sourceInfo->inputSrcId, sourceInfo->status,
                sourceInfo->sensorMode.sources[0].fps, sourceInfo->sensorMode.sources[0].width,
                sourceInfo->sensorMode.sources[0].height, sourceInfo->sensorMode.sources[0].colorFmt);
        }
    }

    //parse csiphy device info
    QCarCamDiagCsiDevInfo* pDiagCsiInfo = diagnosticInfo->aisDiagCsiDevInfo;
    for(uint32 i = 0; i < MAX_NUM_CSIPHY_DEVICES; i++)
    {
        QCarCamDiagCsiDevInfo* csiInfo = &pDiagCsiInfo[i];

        QCARCAMDIAG_INFOMSG("DevId:%d laneMapping:%x numIfeMap:%d ifeMap:%x", csiInfo->csiphyDevId, csiInfo->csiLaneMapping,
            csiInfo->numIfeMap, csiInfo->ifeMap);
    }

    //parse ife device info
    QCarCamDiagIfeDevInfo* pDiagIfeInfo = diagnosticInfo->aisDiagIfeDevInfo;
    for (uint32 i = 0; i < MAX_NUM_IFE_DEVICES; i++)
    {
        QCarCamDiagIfeDevInfo* ifeInfo = &pDiagIfeInfo[i];

        QCARCAMDIAG_INFOMSG("DevId:%d csiDevId:%d numRdi:%d csidPktsReceived:%llu", ifeInfo->ifeDevId, ifeInfo->csiDevId,
            ifeInfo->numRdi, ifeInfo->csidPktsRcvd);

        for(uint32 j = 0; j < (uint32)ifeInfo->numRdi; j++)
        {
            QCarCamDiagRdiInfo *rdiInfo = &ifeInfo->rdiInfo[j];

            QCARCAMDIAG_INFOMSG("rdiId:%d rdiStatus:%d", rdiInfo->rdiId, rdiInfo->rdiStatus);
        }
    }

    //parse Error Info
    QCarCamDiagErrorInfo* pDiagErrorInfo = diagnosticInfo->aisDiagErrInfo;
    for(uint32 i = 0; i < MAX_ERR_QUEUE_SIZE; i++)
    {
        QCarCamDiagErrorInfo* errorInfo = &pDiagErrorInfo[i];

        QCARCAMDIAG_INFOMSG("errorDevice:%d errorStatus:%d usrHdl:%p inputId:%d inputDevId:%d csiphyId:%d"
            "ifeDevId:%d rdiId:%d errorTimeStamp:%llu", errorInfo->errorType, errorInfo->payload[0],
            errorInfo->usrHdl, errorInfo->inputSrcId, errorInfo->inputDevId, errorInfo->csiphyDevId,
            errorInfo->ifeDevId, errorInfo->rdiId, errorInfo->errorTimeStamp);
    }
}

/**
 * API to query system diagnostic info
 *
 */
static void qcarcam_test_query_diagnostics(QCarCamDiagInfo *diagnosticInfo)
{
    int ret = 0;

    ret = QCarCamQueryDiagnostics(diagnosticInfo);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAMDIAG_ERRORMSG("QCarCamQueryDiagnostics failed with ret:%d", ret);
    }
    else
    {
        parse_diagnostics(diagnosticInfo);
        QCARCAMDIAG_INFOMSG("QCarCamQueryDiagnostics is success and parsed");
    }
}

int main(int argc, char **argv)
{
    const char* tok = NULL;
    unsigned int diagnostic_interval = TIMER_THREAD_USLEEP;

    QCARCAMDIAG_ALWZMSG("Welcome to qcarcam_diagnostics");

    for (int i = 1; i < argc; i++)
    {
        if(!strncmp(argv[i],"-interval=", strlen("-interval=")))
        {
            tok = argv[i]+strlen("-interval");
            diagnostic_interval = atoi(tok);
        }
        else
        {
            QCARCAMDIAG_ERRORMSG("Invalid argument, argv[%d]=%s", i, argv[i]);
            exit(-1);
        }
    }

    QCarCamInit_t qcarcam_init = {0};
    qcarcam_init.apiVersion = QCARCAM_VERSION;
    //qcarcam_init.debug_tag = (char *)"qcarcam_diag";

    int ret = QCarCamInitialize(&qcarcam_init);
    if (ret != QCARCAM_RET_OK)
    {
        QCARCAMDIAG_ERRORMSG("QCarCamInitialize failed %d", ret);
        exit(-1);
    }

    printf("Starting qcarcam_diagnostics...press e to exit\n");

    QCarCamDiagInfo* diagnosticInfo = (QCarCamDiagInfo*)calloc(1, sizeof(QCarCamDiagInfo));
    if (!diagnosticInfo)
    {
        QCARCAMDIAG_ERRORMSG("No memory allocated for diagnostics");
        return -1;
    }

    while (1)
    {
        uint32 option = 0;
        char buf[2];
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;

        int num = select(1, &readfds, NULL, NULL, &tv);
        if (num > 0)
        {
            if (fgets(buf, sizeof(buf), stdin) != NULL)
            {
                option = buf[0];
                if ('e' == option)
                {
                    break;
                }
            }
        }

        usleep(diagnostic_interval);
        memset(diagnosticInfo, 0x0, sizeof(QCarCamDiagInfo));
        qcarcam_test_query_diagnostics(diagnosticInfo);
    }

    QCarCamUninitialize();
    free(diagnosticInfo);
    return 0;
}
