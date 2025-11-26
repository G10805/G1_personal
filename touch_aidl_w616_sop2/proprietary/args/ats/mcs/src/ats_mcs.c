/**
*==============================================================================
* \file ats_mcs.c
* \brief
*                  A T S  M C S  S O U R C E  F I L E
*
*     The Media Control Service(MCS) source file contains the
*     implementation for ATS to handle requests that operate the
*     platform media controller. This allows ATS to start and stop
*     playback and capture sessions.
*
* \copyright
*     Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
*     All Rights Reserved.
*     Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*==============================================================================
*/

/*------------------------------------------
* Includes
*------------------------------------------*/
#include "ats_mcs.h"
#include "mcs_api.h"
#include "ats_i.h"
#include "ats_common.h"
#include "acdb_utility.h"

/*------------------------------------------
* Defines and Constants Typedefs
*------------------------------------------*/

/*------------------------------------------
* Private Functions
*------------------------------------------*/

int32_t mcs_read_properties(mcs_properties_t *properties, uint8_t* buffer, uint32_t buffer_size, uint32_t* offset)
{
    int32_t status = AR_EOK;
    uint32_t sz_mcs_property = sizeof(mcs_properties_t) - sizeof(size_t);

    if (IsNull(properties) || IsNull(buffer) || IsNull(offset) || buffer_size < sizeof(mcs_properties_t))
    {
        status = AR_EBADPARAM;
        ATS_ERR("Error[%d]: Null parameter passed in.", status);
        return status;
    }

    ATS_MEM_CPY_SAFE(properties, sz_mcs_property,
        buffer + *offset, sz_mcs_property);
    *offset += sz_mcs_property;

    properties->channel_mapping = buffer + *offset;
    *offset += properties->num_channels * sizeof(uint8_t);

    return status;
}

int32_t mcs_read_playback_req(mcs_play_t *req, uint8_t *cmd_buf, uint32_t cmd_buf_size, uint32_t* bytes_read)
{
    int32_t status = AR_EOK;
    uint32_t offset = 0;
    uint32_t sz_session_info = 0;

    if (IsNull(req) || IsNull(cmd_buf) || IsNull(bytes_read) ||
        cmd_buf_size <= 0)
    {
        status = AR_EBADPARAM;
        ATS_ERR("Error[%d]: Null parameter passed in.", status);
        return status;
    }

    sz_session_info =
        sizeof(req->playback_mode) +
        sizeof(req->playback_duration_sec) +
        sizeof(req->filename_len);

    //Read Device and Stream Properties
    status = mcs_read_properties(&req->device_properties, cmd_buf, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read device properties.");
        return status;
    }

    status = mcs_read_properties(&req->stream_properties, cmd_buf, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read stream properties.");
        return status;
    }

    //Read GKV
    ATS_MEM_CPY_SAFE(&req->graph_key_vector.num_kvps, sizeof(uint32_t),
        cmd_buf + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    req->graph_key_vector.kvp =
        (mcs_key_value_pair_t*)(cmd_buf + offset);
    offset += req->graph_key_vector.num_kvps * 2UL * (uint32_t)sizeof(uint32_t);

    //Read Playback Info (Playback file, Duration, Playback Mode)
    //memcpy starting from the address of playback_mode
    ATS_MEM_CPY_SAFE(&req->playback_mode, sz_session_info,
        cmd_buf + offset, sz_session_info);

    offset += sz_session_info;

    ATS_MEM_CPY_SAFE(&req->filename, req->filename_len,
        cmd_buf + offset, req->filename_len);

    offset += req->filename_len;
    *bytes_read = offset;

    return status;
}

int32_t mcs_read_record_req(mcs_record_t *req, uint8_t *cmd_buf, uint32_t cmd_buf_size, uint32_t* bytes_read)
{
    int32_t status = AR_EOK;
    uint32_t offset = 0;
    uint32_t sz_session_info = 0;

    if (IsNull(req) || IsNull(cmd_buf) || IsNull(bytes_read) ||
        cmd_buf_size <= 0)
    {
        status = AR_EBADPARAM;
        ATS_ERR("Error[%d]: Null parameter passed in.", status);
        return status;
    }

    sz_session_info =
        sizeof(req->record_mode) +
        sizeof(req->write_to_file) +
        sizeof(req->record_duration_sec) +
        sizeof(req->filename_len);

    //Read Device and Stream Properties
    status = mcs_read_properties(&req->device_properties, cmd_buf, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read device properties.");
        return status;
    }

    status = mcs_read_properties(&req->stream_properties, cmd_buf, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read stream properties.");
        return status;
    }

    //Read GKV
    ATS_MEM_CPY_SAFE(&req->graph_key_vector.num_kvps, sizeof(uint32_t),
        cmd_buf + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    req->graph_key_vector.kvp =
        (mcs_key_value_pair_t*)(cmd_buf + offset);
    offset += req->graph_key_vector.num_kvps * 2UL * (uint32_t)sizeof(uint32_t);

    //Read Playback Info (Playback file, Duration, Playback Mode)
    //memcpy starting from the address of record_mode
    ATS_MEM_CPY_SAFE(&req->record_mode, sz_session_info,
        cmd_buf + offset, sz_session_info);

    offset += sz_session_info;

    ATS_MEM_CPY_SAFE(&req->filename, req->filename_len,
        cmd_buf + offset, req->filename_len);

    offset += req->filename_len;
    *bytes_read = offset;

    return status;
}

int32_t mcs_play(
        uint8_t *cmd_buf,
        uint32_t cmd_buf_size,
        uint8_t *rsp_buf,
        uint32_t rsp_buf_size,
        uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);
    __UNREFERENCED_PARAM(rsp_buf_bytes_filled);

    int32_t status = AR_EOK;
    mcs_play_t req = { 0 };
    uint32_t offset = 0;
    status = mcs_read_playback_req(&req, cmd_buf, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read MCS Playback Request");
        return status;
    }

    status = mcs_stream_cmd((uint32_t)MCS_CMD_PLAY,
        (uint8_t*)&req, sizeof(mcs_play_t), NULL, 0, NULL);

    return status;
}

int32_t mcs_rec(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);
    __UNREFERENCED_PARAM(rsp_buf_bytes_filled);

    int32_t status = AR_EOK;
    mcs_record_t req = {0};
    uint32_t offset = 0;

    status = mcs_read_record_req(&req, cmd_buf, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read MCS Record Request");
        return status;
    }

    status = mcs_stream_cmd((uint32_t)MCS_CMD_REC,
        (uint8_t*)&req, sizeof(mcs_record_t), NULL, 0, NULL);

    return status;
}

int32_t mcs_play_rec(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);
    __UNREFERENCED_PARAM(rsp_buf_bytes_filled);

    int32_t status = AR_EOK;
    mcs_play_rec_t req = { 0 };
    uint32_t offset = 0;

    status = mcs_read_playback_req(&req.playback_session, cmd_buf, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read MCS Playback Request");
        return status;
    }

    status = mcs_read_record_req(&req.record_session, cmd_buf + offset, cmd_buf_size, &offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to read MCS Record Request");
        return status;
    }

    status = mcs_stream_cmd((uint32_t)MCS_CMD_PLAY_REC,
        (uint8_t*)&req, sizeof(mcs_play_rec_t), NULL, 0, NULL);

    return status;
}

int32_t mcs_stop(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf);
    __UNREFERENCED_PARAM(cmd_buf_size);
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);
    __UNREFERENCED_PARAM(rsp_buf_bytes_filled);

    int32_t status = AR_EOK;

    status = mcs_stream_cmd((uint32_t)MCS_CMD_STOP, NULL, 0, NULL, 0, NULL);

    return status;
}

/*------------------------------------------
* Public Functions
*------------------------------------------*/


int32_t ats_mcs_ioctl(
    uint32_t svc_cmd_id,
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled
)
{
    int32_t status = AR_EOK;

    int32_t(*func_cb)(
        uint8_t *cmd_buf,
        uint32_t cmd_buf_size,
        uint8_t *rsp_buf,
        uint32_t rsp_buf_size,
        uint32_t *rsp_buf_bytes_filled
        ) = NULL;

    switch (svc_cmd_id)
    {
    case ATS_CMD_MCS_PLAY:
        func_cb = mcs_play;
        break;
    case ATS_CMD_MCS_RECORD:
        func_cb = mcs_rec;
        break;
    case ATS_CMD_MCS_PLAY_RECORD:
        func_cb = mcs_play_rec;
        break;
    case ATS_CMD_MCS_STOP:
        func_cb = mcs_stop;
        break;
    default:
        ATS_ERR("Command[%x] is not supported", svc_cmd_id);
        status = AR_EUNSUPPORTED;
    }

    if (status == AR_EOK)
    {
        status = func_cb(cmd_buf,
            cmd_buf_size,
            rsp_buf,
            rsp_buf_size,
            rsp_buf_bytes_filled);
    }

    return status;
}


int32_t ats_mcs_init(void)
{
    ATS_INFO("Registering Media Control Service...");

    int32_t status = AR_EOK;

    status = mcs_init();

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to initialize Media Control Service.", status);
        return status;
    }

    status = ats_register_service(ATS_MCS_SERVICE_ID, ats_mcs_ioctl);

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to register the Media Control Service.", status);
    }

    return status;
}

int32_t ats_mcs_deinit(void)
{
    ATS_INFO("Deregistering Media Control Service...");

    int32_t status = AR_EOK;

    status = mcs_deinit();

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to deinitialize Media Control Service.", status);
    }

    status = ats_deregister_service(ATS_MCS_SERVICE_ID);

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to deregister the Media Control Service.", status);
    }

    return status;
}

int32_t ats_mcs_get_version(uint32_t* major, uint32_t* minor)
{
    int32_t status = AR_EOK;
    struct mcs_version version = { 0 };

    if (IsNull(major) || IsNull(minor))
        return AR_EBADPARAM;

    status = mcs_get_api_version(&version);

    if (AR_FAILED(status))
    {
        /* Default to initial version. */
        *major = MCS_API_MAJOR_VERSION_1;
        *minor = MCS_API_MINOR_VERSION_0;
        ATS_ERR("Warning[%d]: Unable to get MCS API version. "
            "Defaulting to v%d.%d", status,
            *major, *minor);
        return AR_EOK;
    }

    *major = version.major;
    *minor = version.minor;

    return status;
}


