/**
*==============================================================================
* \file ats_online.c
* \brief
*                  A T S  O N L I N E  S O U R C E  F I L E
*
*      The Online Calibration Service(ONC) source file contains the
*      implementation for ATS to handle requests that operate ACDB SW
*
* \copyright
*     Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
*     All Rights Reserved.
*     Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*==============================================================================
*/

/*------------------------------------------
* Includes
*------------------------------------------*/
#include "ats_online.h"
#include "ats_i.h"
#include "acdb_utility.h"
#include "ats_common.h"
#include "ats_command.h"
#include "ar_osal_mutex.h"
#include "acdb_common.h"

/*------------------------------------------
* Defines and Constants
*------------------------------------------*/
//uint8_t *ats_main_buffer = NULL;

ar_osal_mutex_t acdb_ats_online_lock;

/*------------------------------------------
* Private Functions
*------------------------------------------*/

/**
* \brief get_service_info
*		Queries ATS for information about each service (FTS, MCS, RTS, ADIE, etc?)
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_service_info(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf);
    __UNREFERENCED_PARAM(cmd_buf_size);

    int32_t status = AR_EOK;

    if (IsNull(rsp_buf) || (rsp_buf_size < sizeof(uint32_t)))
    {
        return AR_ENOMEMORY;
    }
    else
    {
        AtsCmdGetServiceInfoRsp rsp = { 0 };

        rsp.services_info = (AtsServiceInfo*)(rsp_buf + sizeof(uint32_t));

        status = ats_get_service_info(&rsp, rsp_buf_size - sizeof(uint32_t));

        if (AR_SUCCEEDED(status))
        {
            ACDB_MEM_CPY_SAFE(rsp_buf, sizeof(uint32_t), &rsp.service_count, sizeof(uint32_t));

            *rsp_buf_bytes_filled = sizeof(rsp.service_count) + (rsp.service_count * sizeof(AtsServiceInfo));
        }

        return AR_EOK;
    }

    //return status;
}

/**
* \brief get_online_version
*		Queries ATS for the Online Calibration service version.
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
//int32_t get_online_version(
//    uint8_t *cmd_buf,
//    uint32_t cmd_buf_size,
//    uint8_t *rsp_buf,
//    uint32_t rsp_buf_size,
//    uint32_t *rsp_buf_bytes_filled)
//{
//    int32_t status = AR_EOK;
//
//    UNUSED(cmd_buf_size);
//    UNUSED(*cmd_buf);
//
//    if ((NULL == rsp_buf) || (rsp_buf_size < 2 * sizeof(uint32_t)))
//    {
//        return AR_ENOMEMORY;
//    }
//    else
//    {
//        AtsVersion version = { ATS_ONC_MAJOR_VERSION , ATS_ONC_MINOR_VERSION };
//        ACDB_MEM_CPY_SAFE(rsp_buf, sizeof(AtsVersion), &version, sizeof(AtsVersion));
//        *rsp_buf_bytes_filled = sizeof(AtsVersion);
//        return AR_EOK;
//    }
//
//    return status;
//}

/**
* \brief check_connection
*		Checks to see if a connection between ATS and its client can be established or is still alive.
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t check_connection(
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

    int32_t status = AR_EOK;

    *rsp_buf_bytes_filled = 0;
    return status;
}

/**
* \brief get_max_buffer_length
*		Queries ATS for its maximum buffer length.
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_max_buffer_length(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf);
    __UNREFERENCED_PARAM(cmd_buf_size);

    uint32_t ats_buf_len = rsp_buf_size;

    if ((NULL == rsp_buf) || (rsp_buf_size < sizeof(uint32_t)))
    {
        return AR_ENOMEMORY;
    }
    else
    {
        ACDB_MEM_CPY_SAFE((void*)(rsp_buf), sizeof(uint32_t), (void*)&ats_buf_len, sizeof(uint32_t));
        *rsp_buf_bytes_filled = sizeof(uint32_t);
        return AR_EOK;
    }
}

/**
* \brief set_max_buffer_length
*		Queries ATS for its maximum buffer length.
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t set_max_buffer_length(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf_size);

    int32_t ats_max_buf_len = 0;

    ACDB_MEM_CPY_SAFE(&ats_max_buf_len, sizeof(uint32_t), &cmd_buf, sizeof(uint32_t));

    uint32_t ats_buf_len = rsp_buf_size;

    if ((NULL == rsp_buf) || (rsp_buf_size < sizeof(uint32_t)))
    {
        //add generic function that will fill in rsp_buf
        return AR_ENOMEMORY;
    }
    else
    {
        ACDB_MEM_CPY_SAFE((void*)(rsp_buf), sizeof(uint32_t), (void*)&ats_buf_len, sizeof(uint32_t));
        *rsp_buf_bytes_filled = sizeof(uint32_t);
        return AR_EOK;
    }
}

/**
* \brief get_acdb_files_info
*		Queries ATS for ACDB file information for all acdb files loaded into RAM.
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_acdb_files_info(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf);
    __UNREFERENCED_PARAM(cmd_buf_size);

    int32_t status = AR_EOK;
    AcdbFileManBlob rsp = { 0 };

    if (IsNull(rsp_buf))
    {
        return AR_EBADPARAM;
    }

    rsp.size = rsp_buf_size;
    rsp.buf = rsp_buf;

    status = AtsCmdGetLoadedFileInfo(&rsp);

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to retrieve loaded acdb file info");
    }

    *rsp_buf_bytes_filled = rsp.bytes_filled;

    return status;
}

/**
* \brief get_acdb_file
*		Downloads the requested ACDB data file from the target.
*
* \sa ats_online_ioctl
* \sa get_acdb_files_info
*/
int32_t get_acdb_file(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf_size);

    int32_t status = AR_EOK;
    AcdbFileManBlob rsp = { 0 };
    AcdbFileManGetFileDataReq req = { 0 };
    int32_t sz_request = sizeof(AcdbFileManGetFileDataReq) - sizeof(size_t);

    if (IsNull(rsp_buf))
    {
        return AR_EBADPARAM;
    }

    ACDB_MEM_CPY_SAFE(&req, sz_request, cmd_buf, sz_request);
    req.file_name = &cmd_buf[sz_request];

    rsp.size = rsp_buf_size;
    rsp.buf = rsp_buf;

    status = AtsCmdGetLoadedFileData(&req, &rsp);

    if (AR_FAILED(status))
    {
        ATS_ERR("Failed to retrieve loaded acdb file info");
    }

    *rsp_buf_bytes_filled = rsp.bytes_filled;
    return status;
}

/**
* \brief get_heap_entry_count
*		Queries ATS for the number of heap entries in the ACDB SW Heap.
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_heap_entry_info(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf);
    __UNREFERENCED_PARAM(cmd_buf_size);

    int32_t status = AR_EOK;
    AcdbBufferContext rsp = { 0 };
    rsp.buf = rsp_buf;
    rsp.size = rsp_buf_size;

    status = AtsCmdGetHeapEntryInfo(&rsp);

    *rsp_buf_bytes_filled = rsp.bytes_filled;
    return status;
}

/**
* \brief
* Retrieves the entries in the ACDB SW Heap
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_heap_entry_data(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf_size);

    int32_t status = AR_EOK;
    uint32_t num_key_vectors = 0;
    uint32_t num_key_vectors_found = 0;
    AcdbBufferContext rsp = { 0 };
    AcdbGraphKeyVector key_vector = { 0 };
    uint32_t offset = 0;
    uint32_t rsp_offset = sizeof(num_key_vectors_found);

    rsp.buf = &rsp_buf[rsp_offset];//make room to write num key vectors found
    rsp.size = rsp_buf_size;
    rsp.bytes_filled = 0;

    ACDB_MEM_CPY_SAFE(&num_key_vectors, sizeof(num_key_vectors), cmd_buf, sizeof(num_key_vectors));
    offset += sizeof(num_key_vectors);

    for (uint32_t i = 0; i < num_key_vectors; i++)
    {
        ACDB_MEM_CPY_SAFE(&key_vector.num_keys, sizeof(key_vector.num_keys), &cmd_buf[offset], sizeof(key_vector.num_keys));
        offset += sizeof(key_vector.num_keys);

        key_vector.graph_key_vector = (AcdbKeyValuePair*)&cmd_buf[offset];
        offset += key_vector.num_keys * sizeof(AcdbKeyValuePair);

        status = AtsCmdGetHeapEntryData(&key_vector, &rsp);
        if (AR_FAILED(status))
            break;//todo: print key vector that failed
        else
            num_key_vectors_found++;
    }

    ACDB_MEM_CPY_SAFE(rsp_buf, sizeof(num_key_vectors_found), &num_key_vectors_found, sizeof(num_key_vectors_found));

    *rsp_buf_bytes_filled = rsp_offset + rsp.bytes_filled;
    return status;
}

/**
* \brief
* Get non-persistent calibration data from the ACDB SW Heap or the data files.
*
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_cal_data_non_persist(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)

{
    int32_t status = AR_EOK;
    uint32_t offset = 0;
    AtsGetSubgraphCalDataReq req = { 0 };
    AcdbBufferContext rsp = { 0 };

    if (IsNull(cmd_buf) || cmd_buf_size == 0)
    {
        ATS_ERR("Error[%d]: The request recieved is empty.",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    ATS_READ_SEEK_UI32(req.subgraph_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.module_iid, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.param_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.cal_key_vector.num_keys, cmd_buf, offset);

    if (req.cal_key_vector.num_keys)
    {
        req.cal_key_vector.graph_key_vector =
            (AcdbKeyValuePair*)&cmd_buf[offset];
    }

    rsp.buf = rsp_buf;
    rsp.size = rsp_buf_size;

    status = AtsCmdGetCalDataNonPersist(&req, &rsp);

    *rsp_buf_bytes_filled = rsp.bytes_filled;
    return status;
}

/**
* \brief
* Set non-persistent calibration data to the ACDB SW Heap or the data files.
*
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t set_cal_data_non_persist(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)

{
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);

    int32_t status = AR_EOK;
    uint32_t offset = 0;
    AcdbGraphKeyVector ckv = { 0 };
    AcdbSubgraphParamData subgraph_param_data = {0};

    if (IsNull(cmd_buf) || cmd_buf_size == 0)
    {
        ATS_ERR("Error[%d]: The request recieved is empty.",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    *rsp_buf_bytes_filled = 0;

    ATS_READ_SEEK_UI32(subgraph_param_data.subgraph_id_list.count,
        cmd_buf, offset);
    subgraph_param_data.subgraph_id_list.list =
    (uint32_t*)&cmd_buf[offset];
    offset += subgraph_param_data.subgraph_id_list.count * sizeof(uint32_t);

    ATS_READ_SEEK_UI32(ckv.num_keys, cmd_buf, offset);

    if (ckv.num_keys)
    {
        ckv.graph_key_vector =
            (AcdbKeyValuePair*)&cmd_buf[offset];
        offset += ckv.num_keys
            * 2UL * (uint32_t)sizeof(uint32_t);
    }

    ATS_READ_SEEK_UI32(subgraph_param_data.data_size, cmd_buf, offset);

    if (subgraph_param_data.data_size)
    {
        subgraph_param_data.data = &cmd_buf[offset];
    }

    status = AtsCmdSetCalDataNonPersist(&ckv, &subgraph_param_data);

    return status;
}

/**
* \brief
* Queries ATS for persistent calibration data from the ACDB SW Heap or the data files.
*
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_cal_data_persist(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    int32_t status = AR_EOK;
    uint32_t offset = 0;
    AtsGetSubgraphCalDataReq req = { 0 };
    AcdbBufferContext rsp = { 0 };

    if (IsNull(cmd_buf) || cmd_buf_size == 0)
    {
        ATS_ERR("Error[%d]: The request recieved is empty.",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    ATS_READ_SEEK_UI32(req.subgraph_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.module_iid, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.param_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.cal_key_vector.num_keys, cmd_buf, offset);

    if (req.cal_key_vector.num_keys)
    {
        req.cal_key_vector.graph_key_vector =
            (AcdbKeyValuePair*)&cmd_buf[offset];
    }

    rsp.buf = rsp_buf;
    rsp.size = rsp_buf_size;

    status = AtsCmdGetCalDataPersist(&req, &rsp);

    *rsp_buf_bytes_filled = rsp.bytes_filled;
    return status;
}

/**
* \brief
* Set persistent calibration data to the ACDB SW Heap or the data files.
*
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t set_cal_data_persist(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)

{
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);

    int32_t status = AR_EOK;
    uint32_t offset = 0;
    AcdbGraphKeyVector ckv = { 0 };
    AcdbSubgraphParamData subgraph_param_data = { 0 };

    if (IsNull(cmd_buf) || cmd_buf_size == 0)
    {
        ATS_ERR("Error[%d]: The request recieved is empty.",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    *rsp_buf_bytes_filled = 0;

    ATS_READ_SEEK_UI32(subgraph_param_data.subgraph_id_list.count,
        cmd_buf, offset);
    subgraph_param_data.subgraph_id_list.list =
        (uint32_t*)&cmd_buf[offset];
    offset += subgraph_param_data.subgraph_id_list.count * sizeof(uint32_t);

    ATS_READ_SEEK_UI32(ckv.num_keys, cmd_buf, offset);

    if (ckv.num_keys)
    {
        ckv.graph_key_vector =
            (AcdbKeyValuePair*)&cmd_buf[offset];
        offset += ckv.num_keys
            * 2UL * (uint32_t)sizeof(uint32_t);
    }

    ATS_READ_SEEK_UI32(subgraph_param_data.data_size, cmd_buf, offset);

    if (subgraph_param_data.data_size)
    {
        subgraph_param_data.data = &cmd_buf[offset];
    }

    status = AtsCmdSetCalDataPersist(&ckv, &subgraph_param_data);

    return status;
}


/**
* \brief
* Get tag data from the ACDB SW Heap or the delta file(s).
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t get_tag_data(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)

{
    int32_t status = AR_EOK;
    uint32_t offset = 0;
    AtsGetSubgraphTagDataReq req = { 0 };
    AcdbBufferContext rsp = { 0 };

    if (IsNull(cmd_buf) || cmd_buf_size == 0)
    {
        ATS_ERR("Error[%d]: The request recieved is empty.",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    ATS_READ_SEEK_UI32(req.subgraph_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.tag_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.module_iid, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.param_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(req.tag_key_vector.num_keys, cmd_buf, offset);

    if (req.tag_key_vector.num_keys)
    {
        req.tag_key_vector.graph_key_vector =
            (AcdbKeyValuePair*)&cmd_buf[offset];
    }

    rsp.buf = rsp_buf;
    rsp.size = rsp_buf_size;

    status = AtsCmdGetTagData(&req, &rsp);

    *rsp_buf_bytes_filled = rsp.bytes_filled;
    return status;
}

/**
* \brief
* Set tag data to the ACDB SW Heap or the delta file(s).
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t set_tag_data(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)

{
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);

    int32_t status = AR_EOK;
    uint32_t offset = 0;
    AcdbModuleTag tag = { 0 };
    AcdbSubgraphParamData subgraph_param_data = { 0 };

    if (IsNull(cmd_buf) || cmd_buf_size == 0)
    {
        ATS_ERR("Error[%d]: The request recieved is empty.",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    *rsp_buf_bytes_filled = 0;

    ATS_READ_SEEK_UI32(subgraph_param_data.subgraph_id_list.count,
        cmd_buf, offset);
    subgraph_param_data.subgraph_id_list.list =
        (uint32_t*)&cmd_buf[offset];
    offset += subgraph_param_data.subgraph_id_list.count * sizeof(uint32_t);

    ATS_READ_SEEK_UI32(tag.tag_id, cmd_buf, offset);
    ATS_READ_SEEK_UI32(tag.tag_key_vector.num_keys,
        cmd_buf, offset);

    if (tag.tag_key_vector.num_keys)
    {
        tag.tag_key_vector.graph_key_vector =
            (AcdbKeyValuePair*)&cmd_buf[offset];
        offset += tag.tag_key_vector.num_keys
            * 2UL * (uint32_t)sizeof(uint32_t);
    }

    ATS_READ_SEEK_UI32(subgraph_param_data.data_size, cmd_buf, offset);

    if (subgraph_param_data.data_size)
    {
        subgraph_param_data.data = &cmd_buf[offset];
    }

    status = AtsCmdSetTagData(&tag, &subgraph_param_data);

    return status;
}

/**
* \brief delete_delta_files
* Removes the *.acdbdelta files from the targets file system
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t delete_delta_files(
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

    int32_t status = AR_EOK;

    status = AtsCmdDeleteDeltaFiles();
    *rsp_buf_bytes_filled = 0;

    return status;
}

/**
* \brief
* Check to see if the target supports delta data persistence.
*
* \sa ats_online_ioctl
* \return 0 on success, non-zero on failure
*/
int32_t is_delta_data_supported(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf);
    __UNREFERENCED_PARAM(cmd_buf_size);

    int32_t status = AR_EOK;
    int32_t is_delta_data_supported = acdb_delta_data_is_persist_enabled();

    ACDB_MEM_CPY_SAFE(rsp_buf, rsp_buf_size, &is_delta_data_supported, sizeof(int32_t));
    *rsp_buf_bytes_filled = sizeof(is_delta_data_supported);

    return status;
}

int32_t get_temp_file_path(
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

    int32_t status = AR_EOK;
    AtsGetTempPath path = { 0 };
    path.path = (char*)(rsp_buf + sizeof(uint32_t));
    status = AtsCmdGetTempFilePath(&path);
    if (AR_FAILED(status))
    {
        return status;
    }

    ACDB_MEM_CPY_SAFE(rsp_buf, sizeof(uint32_t),
        &path.path_len, sizeof(uint32_t));

    *rsp_buf_bytes_filled = sizeof(path.path_len)
        + path.path_len;

    return status;
}

int32_t reinit_acdb(
    uint8_t *cmd_buf,
    uint32_t cmd_buf_size,
    uint8_t *rsp_buf,
    uint32_t rsp_buf_size,
    uint32_t *rsp_buf_bytes_filled)
{
    __UNREFERENCED_PARAM(cmd_buf_size);
    __UNREFERENCED_PARAM(rsp_buf);
    __UNREFERENCED_PARAM(rsp_buf_size);
    __UNREFERENCED_PARAM(rsp_buf_bytes_filled);

    int32_t status = AR_EOK;
    uint32_t offset = 0;
    //AcdbFile *delta_file_path = NULL;
    AcdbFile delta_file_path = { 0 };
    AtsAcdbReinitReq *req = NULL;
    AtsGetTempPath path = { 0 };

    *rsp_buf_bytes_filled = 0;

    req = ACDB_MALLOC(AtsAcdbReinitReq, 1);

    if (IsNull(req))
    {
        ATS_ERR("Error[%d]: Unable to allocate reinit"
            " request structure.", AR_ENOMEMORY);
        return AR_ENOMEMORY;
    }

    ACDB_MEM_CPY_SAFE(&req->acdb_files.num_files, sizeof(uint32_t),
        &cmd_buf[offset], sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (0 == req->acdb_files.num_files)
    {
        status = AR_EBADPARAM;
        ATS_ERR("Error[%d]: Cannot reinitialize ACDB SW with 0 files.", status);
        goto end;
    }

    for (uint32_t i = 0; i < req->acdb_files.num_files; i++)
    {
        ACDB_MEM_CPY_SAFE(
            &req->acdb_files.acdbFiles[i].fileNameLen, sizeof(uint32_t),
            &cmd_buf[offset], sizeof(uint32_t));
        offset += sizeof(uint32_t);

        ACDB_MEM_CPY_SAFE(
            &req->acdb_files.acdbFiles[i].fileName,
            req->acdb_files.acdbFiles[i].fileNameLen,
            &cmd_buf[offset],
            req->acdb_files.acdbFiles[i].fileNameLen);
        offset += req->acdb_files.acdbFiles[i].fileNameLen;
    }

    ACDB_MEM_CPY_SAFE(&req->is_delta_data_supported, sizeof(uint32_t),
        &cmd_buf[offset], sizeof(uint32_t));
    offset += sizeof(uint32_t);

    path.path = delta_file_path.fileName;
    status = AtsCmdGetTempFilePath(&path);
    if (AR_FAILED(status))
    {
        ATS_ERR("Error[%d]: Failed to get temp file path from File Manager.");
        goto end;
    }

    delta_file_path.fileNameLen = path.path_len;

    if (req->is_delta_data_supported)
    {
        status = acdb_ioctl((uint32_t)ACDB_CMD_ENABLE_PERSISTANCE,
            NULL, 0, NULL, 0);
        if (AR_FAILED(status))
        {
            ATS_ERR("Error[%d]: Failed to enable delta data "
                "support. Skipping..", status);
        }
    }

    ATS_INFO("Re-initializing ACDB SW..");

    status = acdb_deinit();
    if (AR_FAILED(status))
    {
        ATS_ERR("Error[%d]: Acdb deinit failed during re-init.", status);
        goto end;
    }

    status = acdb_init(&req->acdb_files, &delta_file_path);
    if (AR_FAILED(status))
    {
        ATS_ERR("Error[%d]: Acdb init failed during re-init.", status);
        goto end;
    }

end:
    ACDB_FREE(req);

    return status;
}
/*------------------------------------------
* Public Functions
*------------------------------------------*/

/**
* \brief ats_online_ioctl
*		The Online Service IOCTL
* \param [in] svc_cmd_id: The command to be issued
* \param [in] cmd_buf: Pointer to the command structure.
* \param [in] cmd_buf_size: Size of the command structure
* \param [out] rsp_buf: The response structre
* \param [in] rsp_buf_size: The size of the response
* \param [out] rsp_buf_bytes_filled: Number of bytes written to the response buffer
* \return 0 on success, non-zero on failure
*/
int32_t ats_online_ioctl(
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
    case ATS_CMD_ONC_GET_SERVICE_INFO:
        func_cb = get_service_info;
        break;
    case ATS_CMD_ONC_CHECK_CONNECTION:
        func_cb = check_connection;
        break;
    case ATS_CMD_ONC_GET_MAX_BUFFER_LENGTH:
        func_cb = get_max_buffer_length;
        break;
    case ATS_CMD_ONC_SET_MAX_BUFFER_LENGTH:
        func_cb = set_max_buffer_length;
        break;
    case ATS_CMD_ONC_GET_ACDB_FILES_INFO:
        func_cb = get_acdb_files_info;
        break;
    case ATS_CMD_ONC_GET_ACDB_FILE:
        func_cb = get_acdb_file;
        break;
    case ATS_CMD_ONC_GET_HEAP_ENTRY_INFO:
        func_cb = get_heap_entry_info;
        break;
    case ATS_CMD_ONC_GET_HEAP_ENTRY_DATA:
        func_cb = get_heap_entry_data;
        break;
    case ATS_CMD_ONC_GET_CAL_DATA_NON_PERSIST:
        func_cb = get_cal_data_non_persist;
        break;
    case ATS_CMD_ONC_SET_CAL_DATA_NON_PERSIST:
        func_cb = set_cal_data_non_persist;
        break;
    case ATS_CMD_ONC_GET_CAL_DATA_PERSIST:
        func_cb = get_cal_data_persist;
        break;
    case ATS_CMD_ONC_SET_CAL_DATA_PERSIST:
        func_cb = set_cal_data_persist;
        break;
    case ATS_CMD_ONC_GET_TAG_DATA:
        func_cb = get_tag_data;
        break;
    case ATS_CMD_ONC_SET_TAG_DATA:
        func_cb = set_tag_data;
        break;
    case ATS_CMD_ONC_DELETE_DELTA_FILES:
        func_cb = delete_delta_files;
        break;
    case ATS_CMD_ONC_IS_DELTA_DATA_SUPPORTED:
        func_cb = is_delta_data_supported;
        break;
    case ATS_CMD_ONC_GET_TEMP_PATH:
        func_cb = get_temp_file_path;
        break;
    case ATS_CMD_ONC_ACDB_REINIT:
        func_cb = reinit_acdb;
        break;
    default:
        ATS_ERR("Command[%x] is not supported", svc_cmd_id);
        status = AR_EUNSUPPORTED;
    }

    if (status == AR_EOK)
    {
        ACDB_MUTEX_LOCK(acdb_ats_online_lock);

        status = func_cb(cmd_buf,
            cmd_buf_size,
            rsp_buf,
            rsp_buf_size,
            rsp_buf_bytes_filled);

        ACDB_MUTEX_UNLOCK(acdb_ats_online_lock);
    }

    return status;
}

/**
* \brief ats_online_init
*		Initializes the online service to allow interaction between
*		ACDB SW and and QST clients.
* \return AR_EOK, AR_EFAILED, AR_EHANDLE
*/
int32_t ats_online_init(void)
{
    ATS_DBG("Registering Online Service...");

    int32_t status = AR_EOK;

    status = ats_register_service(ATS_ONLINE_SERVICE_ID, ats_online_ioctl);

    if (AR_FAILED(status))
    {
        ATS_DBG_MSG_1("Failed to register the Online Service.", status, 0);
    }

    if (!acdb_ats_online_lock)
    {
        status = ar_osal_mutex_create(&acdb_ats_online_lock);
        if (status)
        {
            ACDB_ERR("failed to create mutex: %d", status);
        }
    }


    return status;
}

/**
* \brief ats_online_deinit
*		Initializes the online service to allow interaction between
*		ACDB SW and and QST clients.
* \return AR_EOK, AR_EFAILED, AR_EHANDLE
*/
int32_t ats_online_deinit(void)
{
    ATS_DBG("Deregistering Online Service...");

    int32_t status = AR_EOK;

    status = ats_deregister_service(ATS_ONLINE_SERVICE_ID);

    if (AR_FAILED(status))
    {
        ATS_DBG_MSG_1("Failed to deregister the Online Service.", status, 0);
    }

    status = ar_osal_mutex_destroy(acdb_ats_online_lock);
    if (status)
    {
        ACDB_ERR("failed to destroy mutex: %d", status);
    }
    else
    {
        acdb_ats_online_lock = NULL;
    }

    return status;
}
