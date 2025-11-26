/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "MCS-TEST"
#include "mcs.h"
#include "ats_i.h"
#include "acdb.h"
#include <unistd.h>

static ATS_CALLBACK service;

void execute_playback()
{
    struct ats_cmd_mcs_play_req_t playbuf;
    uint32_t cmd;
    uint32_t response;
    int status;
    char *filename = "/usr/bin/yesterday_48KHz.wav";
    uint8_t *cmd_buf;
    AcdbKeyValuePair gkvpair[3];
    gkvpair[0].key = STREAMTX;
    gkvpair[0].value = PCM_LL_PLAYBACK;
    gkvpair[1].key = INSTANCE;
    gkvpair[1].value = INSTANCE_1;
    gkvpair[2].key = DEVICERX;
    gkvpair[2].value = SPEAKER;


    AcdbGraphKeyVector graph_key;
    graph_key.num_keys = 3;
    graph_key.graph_key_vector = gkvpair;

    playbuf.stream_properties.sample_rate = 48000;
    playbuf.stream_properties.num_channels = 2;
    playbuf.stream_properties.channel_mapping = (uint8_t *)(intptr_t)3;
    playbuf.stream_properties.bit_width = 16;
    cmd = ATS_CMD_MCS_PLAY;
    AR_LOG_ERR(LOG_TAG,"command service id %x", cmd);
    playbuf.graph_key_vector = graph_key;
    playbuf.playback_mode = 1;
    playbuf.playback_duration_sec = 90;
    playbuf.filename_len = (int32_t)strlen(filename);
    strlcpy(playbuf.filename, filename, playbuf.filename_len+1);

    cmd_buf = (uint8_t*)&playbuf;
    status = mcs_stream_cmd(cmd, cmd_buf, sizeof(struct ats_cmd_mcs_play_req_t), NULL, 0, &response);
    AR_LOG_INFO(LOG_TAG,"calling stop ");
    cmd = ATS_CMD_MCS_STOP;
    status = mcs_stream_cmd(cmd, cmd_buf, sizeof(struct ats_cmd_mcs_play_req_t), NULL, 0, &response);
}

void execute_record()
{
    struct ats_cmd_mcs_record_req_t recbuf;
    uint32_t cmd;
    uint32_t response;
    char *filename = "/data/pcmrecord.dat";
    uint8_t *cmd_buf;
    AcdbKeyValuePair gkvpair[2];
    gkvpair[0].key = STREAMTX;
    gkvpair[0].value = PCM_RECORD;
    gkvpair[1].key = DEVICETX;
    gkvpair[1].value = HANDSETMIC;

    AcdbGraphKeyVector graph_key;
    graph_key.num_keys = 2;
    graph_key.graph_key_vector = gkvpair;

    recbuf.stream_properties.sample_rate = 48000;
    recbuf.stream_properties.num_channels = 2;
    recbuf.stream_properties.channel_mapping = (uint8_t *)(intptr_t)3;
    recbuf.stream_properties.bit_width = 16;
    cmd = ATS_CMD_MCS_RECORD;
    AR_LOG_ERR(LOG_TAG,"command service id %x", cmd);
    recbuf.graph_key_vector = graph_key;
    recbuf.write_to_file = 1;
    recbuf.record_duration_sec = 30;
    recbuf.filename_len = (int32_t)strlen(filename);
    strlcpy(recbuf.filename, filename, recbuf.filename_len+1);

    cmd_buf = (uint8_t*)&recbuf;
    mcs_stream_cmd(cmd, cmd_buf, sizeof(struct ats_cmd_mcs_play_req_t), NULL, 0, &response);
    cmd = ATS_CMD_MCS_STOP;
    AR_LOG_INFO(LOG_TAG,"calling stop ");
    mcs_stream_cmd(cmd, cmd_buf, sizeof(struct ats_cmd_mcs_play_req_t), NULL, 0, &response);
}

int32_t ats_register_service(uint32_t service_id, ATS_CALLBACK service_callback)
{
    if(service_id != ATS_MCS_SERVICE_ID) {
        AR_LOG_ERR(LOG_TAG,"Invalid service id %x", service_id);
        return AR_EFAILED;
    }
    service = service_callback;
    return 0;
}

int main()
{
    mcs_init();
//    execute_playback();
    //sleep(630);
    execute_record();
    return 0;
}
