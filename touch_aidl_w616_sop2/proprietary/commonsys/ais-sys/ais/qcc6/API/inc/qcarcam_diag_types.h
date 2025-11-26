#ifndef _QCARCAM_DIAG_TYPES_H_
#define _QCARCAM_DIAG_TYPES_H_

/* ===========================================================================
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @file qcarcam_diag_types.h
 * @brief QCARCAM Diagnostic information header
 *
=========================================================================== */

#include "qcarcam_types.h"

#define MAX_USR_CLIENTS 64
#define MAX_NUM_INPUT_DEVICES 8
#define MAX_NUM_CSIPHY_DEVICES 4
#define MAX_NUM_IFE_DEVICES 8
#define MAX_SOURCES_PER_DEVICE 4
#define IFE_OUTPUT_RDI_MAX 4
#define MAX_ERR_QUEUE_SIZE 64
#define MAX_NUM_ERROR_PAYLOAD_SIZE 16

typedef struct
{
    unsigned int bufId;
    unsigned int bufStatus;
}QCarCamDiagBufferInfo;

typedef struct
{
    unsigned int rdiId;
    unsigned int rdiStatus;
}QCarCamDiagRdiInfo;

typedef struct
{
    unsigned int cciDevId;
    unsigned int cciPortId;
}QCarCamDiagCciInfo;

typedef struct
{
    QCarCamHndl_t usrHdl;
    unsigned int state;
    uint32_t inputId;
    QCarCamOpmode_e opMode;
    unsigned int inputDevId;
    unsigned int csiphyDevId;
    unsigned int ifeDevId;
    unsigned int rdiId;
    unsigned int frameRate;
    unsigned long long timeStampStart;
    unsigned long long sofCounter;
    unsigned long long frameCounter;
    QCarCamDiagBufferInfo bufInfo[QCARCAM_MAX_NUM_BUFFERS];
}QCarCamDiagClientInfo;

typedef struct
{
    unsigned int inputSrcId; /* Input source Id */
    unsigned int status; /* Input source Id - whether detected or not */
    QCarCamMode_t sensorMode;
}QCarCamDiagInputSrcInfo;

typedef struct
{
    unsigned int inputDevId;
    unsigned int numSensors;
    unsigned int state;
    unsigned int srcIdEnableMask;
    QCarCamDiagCciInfo cciMap;
    QCarCamDiagInputSrcInfo inputSourceInfo[MAX_SOURCES_PER_DEVICE];
}QCarCamDiagInputDevInfo;

typedef struct
{
    unsigned int csiphyDevId;
    unsigned int csiLaneMapping;
    unsigned int numIfeMap;
    unsigned int ifeMap;
}QCarCamDiagCsiDevInfo;

typedef struct
{
    unsigned int ifeDevId;
    unsigned int csiDevId;
    unsigned int numRdi;
    unsigned long long csidPktsRcvd;
    QCarCamDiagRdiInfo rdiInfo[IFE_OUTPUT_RDI_MAX];
}QCarCamDiagIfeDevInfo;

typedef struct
{
    unsigned int errorType;
    QCarCamHndl_t usrHdl;
    unsigned int inputSrcId;
    unsigned int inputDevId;
    unsigned int csiphyDevId;
    unsigned int ifeDevId;
    unsigned int rdiId;
    unsigned int dropFrameId;
    unsigned long long errorTimeStamp;
    unsigned int payload[MAX_NUM_ERROR_PAYLOAD_SIZE];
}QCarCamDiagErrorInfo;

typedef struct
{
    QCarCamDiagClientInfo aisDiagClientInfo[MAX_USR_CLIENTS];
    QCarCamDiagInputDevInfo aisDiagInputDevInfo[MAX_NUM_INPUT_DEVICES];
    QCarCamDiagCsiDevInfo aisDiagCsiDevInfo[MAX_NUM_CSIPHY_DEVICES];
    QCarCamDiagIfeDevInfo aisDiagIfeDevInfo[MAX_NUM_IFE_DEVICES];
    QCarCamDiagErrorInfo aisDiagErrInfo[MAX_ERR_QUEUE_SIZE];
}QCarCamDiagInfo;

#endif /* _QCARCAM_DIAG_TYPES_H_ */
