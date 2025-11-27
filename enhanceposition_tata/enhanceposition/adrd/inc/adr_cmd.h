/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/
#include <stdio.h>

#define ADR_COMMAND_NUM 9
#define ADR_ACK_NUM 9

/*!<the struct mark adr cmd, that may need to send to gnss first>*/
typedef struct padr_control_cmd {
    char *adr_cmd;
    char *adr_ack;
    char *gnss_cmd;
    char *gnss_ack;
} padr_control_cmd;

padr_control_cmd control_cmd[ADR_COMMAND_NUM] =
{
    {"$PADR000*37\r\n", "$PMTK001,000", NULL, NULL},
    {"$PADR101*37\r\n", "$PMTK001,101", "$PMTK101*32\r\n", "$PMTK010"},
    {"$PADR102*34\r\n", "$PMTK001,102", "$PMTK102*31\r\n", "$PMTK010"},
    {"$PADR103*35\r\n", "$PMTK001,103", "$PMTK103*30\r\n", "$PMTK010"},
    {"$PADR104*32\r\n", "$PMTK001,104", "$PMTK104*37\r\n", "$PMTK010"},
    {"$PADR299*35\r\n", "$PMTK001,299", NULL, NULL},
    {"$PADR605*34\r\n", "$PMTK705", NULL, NULL},
    {"$PADR740*34\r\n", "$PMTK001,740", NULL, NULL},
    {"$PADR741*35\r\n", "$PMTK001,741", NULL, NULL}
};

const char *adr_command[ADR_COMMAND_NUM] =
{
    "$PADR000", //ADR_TEST
    "$PADR101", //ADR_CMD_HOT_START
    "$PADR102", //ADR_CMD_WARM_START
    "$PADR103", //ADR_CMD_COLD_START
    "$PADR104", //ADR_CMD_FULL_COLD_START
    "$PADR299", //ADR_SET_OUTPUT_DEBUG
    "$PADR605", //ADR_Q_RELEASE
    "$PADR740", //ADR_DT_UTC
    "$PADR741" //ADR_DT_POS
};

const char *adr_ack[ADR_ACK_NUM] =
{
    "$PMTK001,000", //ADR_TEST
    "$PMTK001,101", //ADR_CMD_HOT_START
    "$PMTK001,102", //ADR_CMD_WARM_START
    "$PMTK001,103", //ADR_CMD_COLD_START
    "$PMTK001,104", //ADR_CMD_FULL_COLD_START
    "$PMTK001,299", //ADR_SET_OUTPUT_DEBUG
    "$PMTK001,740", //ADR_DT_UTC
    "$PMTK001,741", //ADR_DT_POS
    "$PMTK705"
};

static inline char* 
get_pmtk_from_padr(char *padr, int32_t len)
{
    int32_t cnt = 0;
    while(cnt < ADR_COMMAND_NUM) {
        if(!memcmp(control_cmd[cnt].adr_cmd, padr, len))
            return control_cmd[cnt].gnss_cmd;
        cnt++;
    }
    return NULL;
}

