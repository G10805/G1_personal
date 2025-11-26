/**
*==============================================================================
* \file ats_command.c
* \brief
*                  A T S  C M D  S O U R C E  F I L E
*
*     The ATS CMD source file defines the implementation for processessing
*     requests from ATS clients and retrieves the response from ACDB.
*
* \copyright
*     Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
*     All Rights Reserved.
*     Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*==============================================================================
*/

#include "ats_command.h"
#include "acdb_command.h"
#include "acdb_file_mgr.h"

int32_t AtsCmdGetLoadedFileInfo(AcdbFileManBlob *rsp)
{
    int32_t status = AR_EOK;
    status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_LOADED_FILES_INFO,
        NULL, 0,
        (uint8_t*)rsp, sizeof(AcdbFileManBlob));

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to retrieve loaded acdb file info");
    }

    return status;
}

int32_t AtsCmdGetLoadedFileData(AcdbFileManGetFileDataReq *req, AcdbFileManBlob *rsp)
{

    int32_t status = AR_EOK;
    status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_LOADED_FILE_DATA,
        (uint8_t*)req, sizeof(AcdbFileManGetFileDataReq),
        (uint8_t*)rsp, sizeof(AcdbFileManBlob));

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to retrieve loaded acdb file info");
    }

    return status;
}

int32_t AtsCmdGetHeapEntryInfo(AcdbBufferContext *rsp)
{
    int32_t status = AR_EOK;

    status = DataProcGetHeapInfo(rsp);

    if (AR_FAILED(status))
    {
        ATS_ERR("Faild to get information from the ACDB heap");
    }
    return status;
}

int32_t AtsCmdGetHeapEntryData(AcdbGraphKeyVector *key_vector, AcdbBufferContext *rsp)
{
    int32_t status = AR_EOK;
    uint32_t rsp_offset = rsp->bytes_filled;

    status = DataProcGetHeapData(key_vector, rsp, &rsp_offset);
    if (AR_FAILED(status))
    {
        ATS_ERR("Faild to get heap data for key vector");
    }

    rsp->bytes_filled = rsp_offset;
    return status;
}

int32_t AtsCmdGetCalDataNonPersist(AtsGetSubgraphCalDataReq *req, AcdbBufferContext *rsp)
{
    int32_t status = AR_EOK;
    uint32_t blob_offset = 0;
    AcdbBlob blob = { 0 };

    blob.buf = rsp->buf;

    status = GetSubgraphCalData(ACDB_DATA_NON_PERSISTENT, ACDB_DATA_ATS_CLIENT,
        &req->cal_key_vector, req->subgraph_id, req->module_iid, req->param_id,
        FALSE, &blob_offset, &blob);

    if(AR_FAILED(status))
        status = AcdbGetParameterCalData(
        (AcdbParameterCalData*)(void*)req, &blob);

    switch (status)
    {
    case AR_EOK:
        break;
    case AR_ENOTEXIST:
        ATS_ERR("No non-persistent calibration found in heap/file for Subgraph(%x) Module Instance(%x) Parameter(%x)",
            req->subgraph_id, req->module_iid, req->param_id);
        break;
    default:
        ATS_ERR_MSG_1("Failed to get non-persistent subgraph calibration from file/heap for Subgraph(%x) Module Instance(%x) Parameter(%x)",
            status, req->subgraph_id, req->module_iid, req->param_id)
        break;
    }

    blob.buf = NULL;
    rsp->bytes_filled = blob.buf_size;

    return status;
}

int32_t AtsCmdSetCalDataNonPersist(AcdbGraphKeyVector* cal_key_vector,
    AcdbSubgraphParamData* subgraph_param_data)
{
    int32_t status = AR_EOK;

    subgraph_param_data->ignore_error_code = TRUE;
    status = DataProcSetCalData(cal_key_vector, subgraph_param_data);

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to set non-persistent subgraph calibration to heap");
        return status;
    }

    if (acdb_delta_data_is_persist_enabled())
    {
        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SAVE, NULL, 0, NULL, 0);
        if (AR_FAILED(status))
        {
            ATS_ERR("Error(%d) Failed to save delta file", status);
        }
    }
    else
    {
        ATS_ERR("Unable to save delta data. Is delta data persistance enabled?", status);
    }

    return status;
}

int32_t AtsCmdGetCalDataPersist(AtsGetSubgraphCalDataReq *req, AcdbBufferContext *rsp)
{
    int32_t status = AR_EOK;
    uint32_t blob_offset = 0;
    AcdbBlob blob = { 0 };

    blob.buf = rsp->buf;

    status = GetSubgraphCalData(ACDB_DATA_PERSISTENT, ACDB_DATA_ATS_CLIENT,
        &req->cal_key_vector, req->subgraph_id, req->module_iid, req->param_id,
        FALSE, &blob_offset, &blob);

    if (AR_FAILED(status))
        status = AcdbGetParameterCalData(
        (AcdbParameterCalData*)(void*)req, &blob);

    switch (status)
    {
    case AR_EOK:
        break;
    case AR_ENOTEXIST:
        ATS_ERR("No persistent calibration found in heap/file for Subgraph(%x) Module Instance(%x) Parameter(%x)",
            req->subgraph_id, req->module_iid, req->param_id);
        break;
    default:
        ATS_ERR_MSG_1("Failed to get persistent subgraph calibration from file/heap for Subgraph(%x) Module Instance(%x) Parameter(%x)",
            status, req->subgraph_id, req->module_iid, req->param_id)
            break;
    }

    blob.buf = NULL;
    rsp->bytes_filled = blob.buf_size;

    return status;
}

int32_t AtsCmdSetCalDataPersist(AcdbGraphKeyVector* cal_key_vector,
    AcdbSubgraphParamData* subgraph_param_data)
{
    int32_t status = AR_EOK;

    subgraph_param_data->ignore_error_code = TRUE;
    status = DataProcSetCalData(cal_key_vector, subgraph_param_data);

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to set persistent subgraph calibration to heap");
        return status;
    }

    if (acdb_delta_data_is_persist_enabled())
    {
        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SAVE, NULL, 0, NULL, 0);
        if (AR_FAILED(status))
        {
            ATS_ERR("Error(%d) Failed to save delta file", status);
        }
    }
    else
    {
        ATS_ERR("Unable to save delta data. Is delta data persistance enabled?", status);
    }

    return status;
}

int32_t AtsCmdGetTagData(AtsGetSubgraphTagDataReq* req, AcdbBufferContext *rsp)
{
    int32_t status = AR_EOK;
    uint32_t blob_offset = 0;
    AcdbBlob blob = { 0 };
    AcdbDataPersistanceType persist_type = ACDB_DATA_UNKNOWN;
    AcdbParameterTagData tag_req = { 0 };
    status = DataProcGetPersistenceType(req->param_id, &persist_type);
    if (AR_FAILED(status))
    {
        ATS_ERR("Unable to determine persitence type for Parameter(0x%x)", req->param_id);
        return status;
    }

    blob.buf = rsp->buf;

    status = GetSubgraphCalData(ACDB_DATA_PERSISTENT, ACDB_DATA_ATS_CLIENT,
        &req->tag_key_vector, req->subgraph_id, req->module_iid, req->param_id,
        FALSE, &blob_offset, &blob);

    if (AR_FAILED(status))
    {
        tag_req.subgraph_id = req->subgraph_id;
        tag_req.module_tag.tag_id = req->tag_id;
        tag_req.module_iid = req->module_iid;
        tag_req.parameter_id = req->param_id;
        tag_req.module_tag.tag_key_vector = req->tag_key_vector;
        status = AcdbGetParameterTagData(
            &tag_req, &blob);
    }

    switch (status)
    {
    case AR_EOK:
        break;
    case AR_ENOTEXIST:
        ATS_ERR("No heap tag data exists for Subgraph(%x) Module Instance(%x) Parameter(%x) Tag(%x)",
            req->subgraph_id, req->module_iid, req->param_id, req->tag_id);
        break;
    default:
        ATS_ERR_MSG_1("Failed to get tag data from heap for Subgraph(%x) Module Instance(%x) Parameter(%x) Tag(%x)",
            status, req->subgraph_id, req->module_iid, req->param_id, req->tag_id)
            break;
    }

    blob.buf = NULL;
    rsp->bytes_filled = blob.buf_size;

    return status;
}

int32_t AtsCmdSetTagData(AcdbModuleTag* module_tag,
    AcdbSubgraphParamData* subgraph_param_data)
{
    int32_t status = AR_EOK;

    subgraph_param_data->ignore_error_code = TRUE;
    status = DataProcSetTagData(module_tag, subgraph_param_data);

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to set subgraph tag to heap");
        return status;
    }

    if (acdb_delta_data_is_persist_enabled())
    {
        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SAVE, NULL, 0, NULL, 0);
        if (AR_FAILED(status))
        {
            ATS_ERR("Error(%d) Failed to save delta file", status);
        }
    }
    else
    {
        ATS_ERR("Unable to save delta data. Is delta data persistance enabled?", status);
    }

    return status;
}

int32_t AtsCmdDeleteDeltaFiles(void)
{
    int32_t status = AR_EOK;

    status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_DELETE_ALL_FILES, NULL, 0, NULL, 0);

    return status;
}

int32_t AtsCmdGetTempFilePath(AtsGetTempPath *path)
{
    int32_t status = AR_EOK;
    AcdbFileManPathInfo tmp_path = { 0 };
    if (IsNull(path))
    {
        ATS_ERR("Error[%d]: Input path parameter is null"
            , AR_EBADPARAM);
        return status;
    }

    status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_TEMP_PATH_INFO,
        NULL, 0,
        (uint8_t*)&tmp_path, sizeof(AcdbFileManPathInfo));
    if (AR_FAILED(status))
    {
        ATS_ERR("Error[%d]: Failed to retrieve temporary path "
            "info", status);
        return status;
    }

    path->path_len = tmp_path.path_len;
    ATS_MEM_CPY_SAFE(path->path, tmp_path.path_len,
        &tmp_path.path[0], tmp_path.path_len);

    return status;
}
