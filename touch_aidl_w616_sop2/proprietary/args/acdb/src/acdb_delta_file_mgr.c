/**
*=============================================================================
* \file acdb_delta_file_mgr.c
*
* \brief
*		Contains the implementation of the delta acdb file manager
*		interface.
*
* \copyright
*  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/

#include "ar_osal_error.h"
#include "ar_osal_file_io.h"
#include "acdb_delta_file_mgr.h"
#include "acdb_init_utility.h"
#include "acdb_common.h"
#include "acdb_heap.h"
#include "acdb_data_proc.h"
#include "acdb_utility.h"

/* ---------------------------------------------------------------------------
* Preprocessor Definitions and Constants
*--------------------------------------------------------------------------- */
#define INF 4294967295U

/* ---------------------------------------------------------------------------
* Type Declarations
*--------------------------------------------------------------------------- */
typedef struct _AcdbDeltaDataFileInfo AcdbDeltaDataFileInfo;
#include "acdb_begin_pack.h"
struct _AcdbDeltaDataFileInfo
{
	uint32_t file_count;
	AcdbCmdDeltaFileInfo file_info[MAX_ACDB_FILE_COUNT];
}
#include "acdb_end_pack.h"
;

/* ---------------------------------------------------------------------------
* Static Variable Definitions
*--------------------------------------------------------------------------- */

/**< Maintains a list of AcdbDeltaDataFileInfo.*/
static AcdbDeltaDataFileInfo gDeltaDataFileInfo;

/**< Manages the state of the delta persistence.*/
static AcdbDeltaPersistState g_delta_persist_state;

/* ---------------------------------------------------------------------------
* Private Functions
*--------------------------------------------------------------------------- */

int32_t AcdbDeltaDataCmdInit(AcdbCmdDeltaFileInfo *info)
{
	int32_t status = AR_EOK;
	uint32_t idx = 0;

	if (info == NULL)
	{
		ACDB_ERR("Error[%d]: delta info is null", AR_EBADPARAM);
		return AR_EBADPARAM;
	}

	if (gDeltaDataFileInfo.file_count >= MAX_ACDB_FILE_COUNT)
	{
		ACDB_ERR("Error[%d]: Max delta acdb file count exceeded. "
            "The max is %d. The current count is %d",
            AR_EBADPARAM, gDeltaDataFileInfo.file_count + 1);
		return AR_EBADPARAM;
	}

	idx = gDeltaDataFileInfo.file_count++;
    gDeltaDataFileInfo.file_info[idx].file_index = info->file_index;
    gDeltaDataFileInfo.file_info[idx].acdb_file_index = info->acdb_file_index;
	gDeltaDataFileInfo.file_info[idx].file_info = info->file_info;
	gDeltaDataFileInfo.file_info[idx].is_updated = FALSE;
	gDeltaDataFileInfo.file_info[idx].exists = info->exists;
	gDeltaDataFileInfo.file_info[idx].file_size = info->file_size;
	gDeltaDataFileInfo.file_info[idx].file_handle = info->file_handle;
	gDeltaDataFileInfo.file_info[idx].delta_file_path.fileNameLen =
        info->delta_file_path.fileNameLen;

    status = ACDB_STR_CPY_SAFE(
		gDeltaDataFileInfo.file_info[idx].delta_file_path.fileName,
		MAX_FILENAME_LENGTH,
		info->delta_file_path.fileName,
		info->delta_file_path.fileNameLen
	);
	if (AR_FAILED(status))
	{
        ACDB_ERR("Error[%d]: Failed to copy delta file path", status);
	}

	return status;
}

int32_t AcdbDeltaDataClearFileInfo(void)
{
	int32_t result = AR_EOK;
	uint32_t idx = 0;

	for (idx = 0; idx < gDeltaDataFileInfo.file_count; idx++)
	{
		if (gDeltaDataFileInfo.file_info[idx].file_handle != NULL)
		{
			int ret = ar_fclose(gDeltaDataFileInfo.file_info[idx].file_handle);
			if (ret != 0)
			{
				ACDB_ERR("AcdbDeltaDataClearFileInfo() failed, delta file close was unsuccessful %d", ret);
				return AR_EFAILED;
			}
			gDeltaDataFileInfo.file_info[idx].file_handle = NULL;
			gDeltaDataFileInfo.file_info[idx].file_size = 0;
		}
	}

	return result;
}

int32_t AcdbDeltaDataCmdGetDeltaVersion(uint32_t findex, uint32_t *delta_major, uint32_t *delta_minor)
{
	int32_t result = AR_EOK;
	uint32_t offset = 0;
	size_t bytes_read = 0;

    if (findex >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, findex, gDeltaDataFileInfo.file_count)
            return AR_EBADPARAM;
    }

	if (gDeltaDataFileInfo.file_info[findex].exists == TRUE && gDeltaDataFileInfo.file_info[findex].file_handle != NULL)
	{
		offset = 0;
		if (gDeltaDataFileInfo.file_info[findex].file_size > sizeof(AcdbDeltaFileHeader) + sizeof(uint32_t))
		{
			int ret = ar_fseek(gDeltaDataFileInfo.file_info[findex].file_handle, offset, AR_FSEEK_BEGIN);
			if (ret != 0)
			{
				ACDB_ERR("AcdbDeltaDataCmdGetDeltaVersion() failed, file seek to %d was unsuccessful", offset);
				return AR_EFAILED;
			}
			ret = ar_fread(gDeltaDataFileInfo.file_info[findex].file_handle, &delta_major, sizeof(uint32_t), &bytes_read);
			if (ret != 0)
			{
				ACDB_ERR("AcdbDeltaDataCmdGetDeltaVersion() failed, delta_major read was unsuccessful");
				return AR_EFAILED;
			}
			offset += sizeof(uint32_t); // curDeltafilemajor
			ret = ar_fseek(gDeltaDataFileInfo.file_info[findex].file_handle, offset, AR_FSEEK_BEGIN);
			if (ret != 0)
			{
				ACDB_ERR("AcdbDeltaDataCmdGetDeltaVersion() failed, file seek to %d was unsuccessful", offset);
				return AR_EFAILED;
			}
			ret = ar_fread(gDeltaDataFileInfo.file_info[findex].file_handle, &delta_minor, sizeof(uint32_t), &bytes_read);
			if (ret != 0)
			{
				ACDB_ERR("AcdbDeltaDataCmdGetDeltaVersion() failed, delta_minor read was unsuccessful");
				return AR_EFAILED;
			}
			offset += sizeof(uint32_t); // curDeltafileminor
		}
	}

	return result;
}

int32_t AcdbDeltaDataCmdSave(void)
{
	int32_t status = AR_EOK;
    ar_fhandle *fhandle = NULL;
	uint32_t fsize = 0;
	uint32_t fdata_size = 0; ;
    uint32_t findex = 0;
    uint32_t acdb_findex = 0;
	LinkedList *p_map_list = NULL;
	LinkedList map_list = { 0 };
	LinkedListNode *cur_node = NULL;
	AcdbFileManPathInfo file_name_info = { 0 };
	AcdbDeltaKVToSubgraphDataMap *map = NULL;
    AcdbInitUtilDeltaInfo delta_info = { 0 };

    if (findex >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, findex, gDeltaDataFileInfo.file_count)
            return AR_EBADPARAM;
    }

    fhandle = &gDeltaDataFileInfo.file_info[findex].file_handle;
    acdb_findex = gDeltaDataFileInfo.file_info[findex].acdb_file_index;
	memset(&file_name_info.path[0], 0, MAX_FILENAME_LENGTH);
	file_name_info.path_len = 0;
	status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_FILE_NAME, 
		(uint8_t *)&acdb_findex, sizeof(uint32_t),
		(uint8_t *)&file_name_info, sizeof(file_name_info));

	//Close and delete old delta file
	status = ar_fclose(*fhandle);
	if (AR_EOK != status) return status;

	status = AcdbInitUtilDeleteDeltaFileData(
		&file_name_info.path[0], file_name_info.path_len,
		&gDeltaDataFileInfo.file_info[findex].delta_file_path);

	if (AR_EOK != status) return status;

    if (file_name_info.path_len >= MAX_FILENAME_LENGTH)
    {
        ACDB_ERR("Error[%d]: The file path length "
            "%d is larger than the max file legnth of %d bytes",
            AR_EBADPARAM, file_name_info.path_len, MAX_FILENAME_LENGTH)
            return AR_EBADPARAM;
    }

	//Create new delta file and set ar_fhandle to gDeltaDataFileInfo at findex
	//Check to see if file exists otherwise create it
    delta_info.file_access_io_flag = ACDB_FUNC_IO_IN;
    delta_info.file_access = AR_FOPEN_WRITE_ONLY;
	status = AcdbInitUtilGetDeltaInfo(
		&file_name_info.path[0], file_name_info.path_len,
		&gDeltaDataFileInfo.file_info[findex].delta_file_path, &delta_info);
	if (AR_EOK != status)
	{
		ACDB_ERR_MSG_1("Unable to create a new delta acdb file", status);
        goto end;
	}

    fsize = delta_info.file_size;
    delta_info.file_access = AR_FOPEN_READ_ONLY_WRITE;

	status = AcdbInitUtilOpenDeltaFile(&delta_info, fhandle);
	if (AR_EOK != status) goto end;

	gDeltaDataFileInfo.file_info[findex].file_handle = *fhandle;

	status = WriteDeltaFileHeader(
		gDeltaDataFileInfo.file_info[findex].file_handle,
		&gDeltaDataFileInfo.file_info[findex].file_info, 0, 0);

	if (AR_EOK != status) goto end;

	p_map_list = &map_list;

	status = acdb_heap_ioctl(
		ACDB_HEAP_CMD_GET_MAP_LIST, NULL, (uint8_t*)&p_map_list, sizeof(uint32_t));
	if (AR_EOK != status) goto end;

	cur_node = p_map_list->p_head;

	while (cur_node != NULL)
	{
        map = ((AcdbDeltaKVToSubgraphDataMap*)cur_node->p_struct);
        status = WriteMapChunk(*fhandle, map);
        if (AR_EOK != status) break;
		cur_node = cur_node->p_next;
	}

	status = ar_fseek(*fhandle, 0, AR_FSEEK_BEGIN);

	if (AR_EOK != status) goto end;

	fsize = (uint32_t)ar_fsize(*fhandle);
	fdata_size = fsize - sizeof(AcdbDeltaFileHeader) + sizeof(uint32_t);

	status = WriteDeltaFileHeader(
		gDeltaDataFileInfo.file_info[findex].file_handle,
		&gDeltaDataFileInfo.file_info[findex].file_info, fdata_size, p_map_list->length);

	if (AR_EOK != status) goto end;

	gDeltaDataFileInfo.file_info[findex].is_updated = TRUE;
	gDeltaDataFileInfo.file_info[findex].file_size = fsize;

end:
    AcdbListClear(p_map_list);
    ACDB_FREE(delta_info.path);
    p_map_list = NULL;

	return status;
}

int32_t AcdbDeltaDataCmdReset(void)
{
	int32_t result = AR_EOK;
	uint32_t idx = 0;


	for (idx = 0; idx < gDeltaDataFileInfo.file_count; idx++)
	{
		//result = AcdbDeltaDataCmdSave(idx);

		//if (AR_EOK != result)
		//{
		//	ACDB_ERR_MSG_1("Failed to save delta file", result);
		//	return result;
		//}

		result = acdb_heap_ioctl(ACDB_HEAP_CMD_CLEAR, NULL, NULL, 0);

		memset(&gDeltaDataFileInfo.file_info[idx].file_info, 0, sizeof(AcdbDeltaFileInfo));
		gDeltaDataFileInfo.file_info[idx].exists = FALSE;
		gDeltaDataFileInfo.file_info[idx].file_size = 0;
		if (gDeltaDataFileInfo.file_info[idx].file_handle != NULL)
		{
			int ret = ar_fclose(gDeltaDataFileInfo.file_info[idx].file_handle);
			if (ret != 0)
			{
				ACDB_ERR("AcdbDeltaDataCmdReset() failed, delta file close was unsuccessful %d", ret);
				return AR_EFAILED;
			}
			gDeltaDataFileInfo.file_info[idx].file_handle = NULL;
		}
	}
	gDeltaDataFileInfo.file_count = 0;
	return result;
}

int32_t AcdbDeltaGetMapChunkInfo(uint32_t findex, uint32_t *foffset, AcdbDeltaMapChunkInfo* chunk_info)
{
    if (findex >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, findex, gDeltaDataFileInfo.file_count)
            return AR_EBADPARAM;
    }

	return AcdbDeltaParserGetMapChunkInfo(
		gDeltaDataFileInfo.file_info[findex].file_handle,
		foffset,
		chunk_info);
}

int32_t AcdbDeltaInitHeap(uint32_t findex)
{
    int32_t status = AR_EOK;
    uint32_t foffset = sizeof(AcdbDeltaFileHeader);
    AcdbDeltaKVToSubgraphDataMap *map = NULL;

    if (findex >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, findex, gDeltaDataFileInfo.file_count)
            return AR_EBADPARAM;
    }

    while (foffset < gDeltaDataFileInfo.file_info[findex].file_size)
    {
        map = ACDB_MALLOC(AcdbDeltaKVToSubgraphDataMap, 1);

        if (IsNull(map))
        {
            ACDB_ERR("Error[%d]: Failed to allocate memory "
                "for Key Vector to Subgraph Map.", status);
            status = AR_ENOMEMORY;
            break;
        }

        ACDB_CLEAR_BUFFER(*map);

        status = ReadMapChunk(
            gDeltaDataFileInfo.file_info[findex].file_handle,
            &foffset, map);

        if (AR_EOK != status && !IsNull(map))
        {
            ACDB_ERR("Error[%d]: Failed to read map "
                "from the *.acdbdelta file.", status);
            free_map(map);
            break;
        }

        status = acdb_heap_ioctl(
            ACDB_HEAP_CMD_ADD_MAP,
            (uint8_t*)map,
            NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to add map to the heap.", status);
            break;
        }
    }

    if (AR_EOK != status)
    {
        status = acdb_heap_ioctl(ACDB_HEAP_CMD_CLEAR, NULL, NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to clear the heap.", status);
        }
    }

    return status;
}

int32_t AcdbDeltaUpdateHeap(uint32_t findex)
{
    int32_t status = AR_EOK;
    uint32_t foffset = sizeof(AcdbDeltaFileHeader);
    AcdbDeltaKVToSubgraphDataMap *map = NULL;

    if (findex >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, findex, gDeltaDataFileInfo.file_count)
            return AR_EBADPARAM;
    }

    while (foffset < gDeltaDataFileInfo.file_info[findex].file_size)
    {
        map = ACDB_MALLOC(AcdbDeltaKVToSubgraphDataMap, 1);

        if (IsNull(map))
        {
            ACDB_ERR("Error[%d]: Failed to allocate memory "
                "for Key Vector to Subgraph Map.", status);
            status = AR_ENOMEMORY;
            break;
        }

        ACDB_CLEAR_BUFFER(*map);

        status = ReadMapChunk(
            gDeltaDataFileInfo.file_info[findex].file_handle,
            &foffset, map);

        if (AR_EOK != status && !IsNull(map))
        {
            ACDB_ERR("Error[%d]: Failed to read map "
                "from the *.acdbdelta file.", status);
            free_map(map);
            break;
        }

        status = DataProcSetMapToHeap(map);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to set map to the heap.", status);
            break;
        }
    }

    if (AR_EOK != status)
    {
        status = acdb_heap_ioctl(ACDB_HEAP_CMD_CLEAR, NULL, NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to clear the heap.", status);
        }
    }

    return status;
}

int32_t AcdbDeltaDeleteFile(uint32_t findex)
{
    int32_t status = AR_EOK;
    ar_fhandle *fhandle = NULL;
    AcdbFileManPathInfo file_name_info = { 0 };

    if (findex >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, findex, gDeltaDataFileInfo.file_count)
            return AR_EBADPARAM;
    }

    fhandle = &gDeltaDataFileInfo.file_info[findex].file_handle;

    memset(&file_name_info.path[0], 0, MAX_FILENAME_LENGTH);
    file_name_info.path_len = 0;
    status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_FILE_NAME,
        (uint8_t *)&findex, sizeof(uint32_t),
        (uint8_t *)&file_name_info, sizeof(file_name_info));

    //Close and delete old delta file
    status = ar_fclose(*fhandle);
    if (AR_EOK != status)
    {
        ACDB_ERR("Error[%d]: Failed to close delta file", status);
        return status;
    }

    status = AcdbInitUtilDeleteDeltaFileData(
        file_name_info.path, file_name_info.path_len,
        &gDeltaDataFileInfo.file_info[findex].delta_file_path);

    if (AR_EOK != status) return status;

    gDeltaDataFileInfo.file_info[findex].exists = FALSE;
    gDeltaDataFileInfo.file_info[findex].is_updated = FALSE;

    return status;
}

int32_t AcdbDeltaDataSwapDelta(AcdbDeltaDataSwapInfo *swap_info)
{
    int32_t status = AR_EOK;
    AcdbCmdDeltaFileInfo *delta_finfo = NULL;
    AcdbInitUtilDeltaInfo delta_info = { 0 };
    AcdbFileManPathInfo file_name_info = { 0 };

    if (IsNull(swap_info))
    {
        ACDB_ERR("Error[%d]: The swap info input parameter is null",
            AR_EBADPARAM)
            return AR_EBADPARAM;
    }

    if (swap_info->file_index >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, swap_info->file_index, gDeltaDataFileInfo.file_count)
        return AR_EBADPARAM;
    }

    delta_finfo = &gDeltaDataFileInfo.file_info[swap_info->file_index];
    delta_finfo->delta_file_path.fileNameLen = swap_info->path_info.path_len;

    /* Close the previous delta file and open/create the new file */
    status = ar_fclose(delta_finfo->file_handle);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Failed to close %s ", status,
            &delta_finfo->delta_file_path.fileName[0]);
        return status;
    }

    status = ar_strcpy(
        &delta_finfo->delta_file_path.fileName[0], MAX_FILENAME_LENGTH,
        &swap_info->path_info.path[0], MAX_FILENAME_LENGTH);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Failed update writable delta path info", status);
        return status;
    }

    status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_FILE_NAME,
        (uint8_t *)&delta_finfo->acdb_file_index, sizeof(uint32_t),
        (uint8_t *)&file_name_info, sizeof(file_name_info));

    //Open/Create rw_delta file
    delta_info.file_access_io_flag = ACDB_FUNC_IO_IN;
    delta_info.file_access = swap_info->file_access;

    status = AcdbInitUtilGetDeltaInfo(
        &file_name_info.path[0], file_name_info.path_len,
        &delta_finfo->delta_file_path, &delta_info);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: "
            "Failed to get read/write delta file info", status);
        return status;
    }

    status = AcdbInitUtilOpenDeltaFile(
        &delta_info, &delta_finfo->file_handle);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: "
            "Failed to open read/write delta file", status);
        return status;
    }

    delta_finfo->file_size = delta_info.file_size;

    return status;
}

int32_t AcdbDeltaDataIsFileAtPath(AcdbDeltaDataSwapInfo *swap_info)
{
    int32_t status = AR_EOK;
    AcdbCmdDeltaFileInfo *delta_finfo = NULL;
    AcdbInitUtilDeltaInfo delta_info = { 0 };
    AcdbFileManPathInfo file_name_info = { 0 };
    AcdbFile delta_path = { 0 };

    if (IsNull(swap_info))
    {
        ACDB_ERR("Error[%d]: The swap info input parameter is null",
            AR_EBADPARAM)
            return AR_EBADPARAM;
    }

    if (swap_info->file_index >= gDeltaDataFileInfo.file_count)
    {
        ACDB_ERR("Error[%d]: "
            "The file index %d is larger than the total file count %d",
            AR_EBADPARAM, swap_info->file_index, gDeltaDataFileInfo.file_count)
            return AR_EBADPARAM;
    }

    delta_finfo = &gDeltaDataFileInfo.file_info[swap_info->file_index];
    delta_info.file_access_io_flag = ACDB_FUNC_IO_IN;
    delta_info.file_access = swap_info->file_access;
    delta_path.fileNameLen = swap_info->path_info.path_len;

    status = ar_strcpy(
        &delta_path.fileName[0], MAX_FILENAME_LENGTH,
        &swap_info->path_info.path[0], MAX_FILENAME_LENGTH);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Failed copy temp path string", status);
        return status;
    }

    status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_FILE_NAME,
        (uint8_t *)&delta_finfo->acdb_file_index, sizeof(uint32_t),
        (uint8_t *)&file_name_info, sizeof(file_name_info));
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: "
            "Failed to get acdb file name at file index %d",
            status, delta_finfo->acdb_file_index);
        return status;
    }

    status = AcdbInitUtilGetDeltaInfo(
        &file_name_info.path[0], file_name_info.path_len,
        &delta_path, &delta_info);
    if (AR_ENOTEXIST == status)
    {
        ACDB_ERR("Error[%d]: "
            "The delta file %s does not exist", status, delta_info.path);
        return status;
    }
    else if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: "
            "Failed to open read/write delta file", status);
        return status;
    }

    return status;
}

/* ---------------------------------------------------------------------------
* Public Functions
*--------------------------------------------------------------------------- */

int32_t acdb_delta_data_is_persist_enabled(void)
{
    return (int32_t)g_delta_persist_state;
}

int32_t acdb_delta_data_ioctl(uint32_t cmd_id,
	uint8_t *req, uint32_t req_size,
	uint8_t *rsp, uint32_t rsp_size)
{
	int32_t status = AR_EOK;

    switch (cmd_id)
    {
    case ACDB_DELTA_DATA_CMD_INIT:
    {
        if (req == NULL || req_size != sizeof(AcdbCmdDeltaFileInfo))
        {
            return AR_EBADPARAM;
        }
        status = AcdbDeltaDataCmdInit((AcdbCmdDeltaFileInfo *)req);
    }
    break;
    case ACDB_DELTA_DATA_CMD_ENABLE_PERSISTENCE:
    {
        if (req == NULL || req_size != sizeof(AcdbDeltaPersistState))
        {
            return AR_EBADPARAM;
        }

        g_delta_persist_state = *(AcdbDeltaPersistState*)req;
    }
    break;
    case ACDB_DELTA_DATA_CMD_RESET:
    {
        status = AcdbDeltaDataCmdReset();
    }
    break;
    case ACDB_DELTA_DATA_CMD_CLEAR_FILE_BUFFER:
    {
        status = AcdbDeltaDataClearFileInfo();
    }
    break;
    case ACDB_DELTA_DATA_CMD_GET_FILE_VERSION:
    {
        uint32_t delta_major = 0;
        uint32_t delta_minor = 0;
        uint32_t *pDeltafile_index = NULL;
        AcdbDeltaFileVersion *deltaVersion =
            ACDB_MALLOC(AcdbDeltaFileVersion, 1);

        if (deltaVersion == NULL)
        {
            ACDB_FREE(deltaVersion);
            return AR_ENOMEMORY;
        }

        if (IsNull(req) || req_size != sizeof(uint32_t))
        {
            ACDB_FREE(deltaVersion);
            return AR_EBADPARAM;
        }

        pDeltafile_index = (uint32_t *)req;
        status = AcdbDeltaDataCmdGetDeltaVersion(
            *pDeltafile_index, &delta_major, &delta_minor);
        deltaVersion->major = delta_major;
        deltaVersion->minor = delta_minor;

        ACDB_MEM_CPY_SAFE(rsp, sizeof(AcdbDeltaFileVersion),
            deltaVersion, sizeof(AcdbDeltaFileVersion));
        ACDB_FREE(deltaVersion);
        status = AR_EOK;
    }
    break;
    case ACDB_DELTA_DATA_CMD_INIT_HEAP:
    {
        if (IsNull(req) || req_size != sizeof(uint32_t))
        {
            return AR_EBADPARAM;
        }

        status = AcdbDeltaInitHeap((uint32_t)*req);
    }
    break;
    case ACDB_DELTA_DATA_CMD_UPDATE_HEAP:
    {
        if (IsNull(req) || req_size != sizeof(uint32_t))
        {
            return AR_EBADPARAM;
        }

        status = AcdbDeltaUpdateHeap((uint32_t)*req);
    }
    break;
    case ACDB_DELTA_DATA_CMD_SET_DATA:
    {
        status = AR_ENOTIMPL;
        //status = AcdbDeltaDataWriteFile(0, 1, (AcdbDeltaCkvToSubgraphDataMap*)pInput);
    }
    break;
    case ACDB_DELTA_DATA_CMD_GET_DATA:
    {
        //status = AcdbDeltaGetCalData();
        status = AR_ENOTIMPL;
    }
    break;
    case ACDB_DELTA_DATA_CMD_SAVE:
    {
        status = AcdbDeltaDataCmdSave();
    }
    break;
    case ACDB_DELTA_DATA_CMD_DELETE_ALL_FILES:
    {
        status = AcdbDeltaDeleteFile(0);
    }
    break;
    case ACDB_DELTA_DATA_CMD_GET_FILE_COUNT:
    {
        if (IsNull(rsp) || rsp_size != sizeof(uint32_t))
        {
            return AR_EBADPARAM;
        }

        *(uint32_t*)rsp = gDeltaDataFileInfo.file_count;
    }
    break;
    case ACDB_DELTA_DATA_CMD_SWAP_DELTA:
    {
        if (IsNull(req) || req_size != sizeof(AcdbDeltaDataSwapInfo))
        {
            return AR_EBADPARAM;
        }

        status = AcdbDeltaDataSwapDelta((AcdbDeltaDataSwapInfo*)req);
    }
    break;
    case ACDB_DELTA_DATA_CMD_IS_FILE_AT_PATH:
    {
        if (IsNull(req) || req_size != sizeof(AcdbDeltaDataSwapInfo))
        {
            return AR_EBADPARAM;
        }

        status = AcdbDeltaDataIsFileAtPath((AcdbDeltaDataSwapInfo*)req);
    }
    break;
	default:
		status = AR_EUNSUPPORTED;
		ACDB_ERR("Unsupported command %08X", cmd_id);
	}

	return status;
}
