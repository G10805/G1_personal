/**
*=============================================================================
* \file acdb_data_proc.c
*
* \brief
*      Processes data for the ACDB SW commands.
*
* \copyright
*  Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/

#include "acdb_data_proc.h"
#include "acdb_parser.h"
#include "acdb_heap.h"
#include "acdb_common.h"

ar_heap_info glb_ar_heap_info;
/**
* \brief AcdbBlobToCalData
*		Copies one module calibration from AcdbBlob to AcdbDeltaModuleCalData
*		at some offset in AcdbBlob.
* \param[in/out] caldata: Calibration data in the format of delta data that
*		must be freed in acdb_heap. calblob is copied to caldata
* \param[int] calblob: Module calibration blob passed in from the client
* \param[in/out] blob_offset: Offset within the blob
*/
int32_t AcdbBlobToCalData(bool_t ignore_error_code, 
    AcdbDeltaModuleCalData *caldata, const AcdbBlob calblob, 
    uint32_t *blob_offset)
{
    if (*blob_offset > calblob.buf_size)
    {
        return AR_EBADPARAM;
    }
	
	ACDB_MEM_CPY_SAFE(&caldata->module_iid, sizeof(uint32_t), 
        (uint8_t*)calblob.buf + *blob_offset, sizeof(uint32_t));
	*blob_offset += sizeof(uint32_t);

    if (*blob_offset > calblob.buf_size)
    {
        ACDB_ERR("Unable to parse past module iid. "
            "Blob offset %d is greater than the blob size %d", 
            *blob_offset, calblob.buf_size);
        return AR_EFAILED;
    }

	ACDB_MEM_CPY_SAFE(&caldata->param_id, sizeof(uint32_t), 
        (uint8_t*)calblob.buf + *blob_offset, sizeof(uint32_t));
	*blob_offset += sizeof(uint32_t);

    if (*blob_offset > calblob.buf_size)
    {
        ACDB_ERR("Unable to parse past parameter id. "
            "Blob offset %d is greater than the blob size %d",
            *blob_offset, calblob.buf_size);
        return AR_EFAILED;
    }

	ACDB_MEM_CPY_SAFE(&caldata->param_size, sizeof(uint32_t), 
        (uint8_t*)calblob.buf + *blob_offset, sizeof(uint32_t));
    *blob_offset += sizeof(uint32_t);

    if (*blob_offset > calblob.buf_size)
    {
        ACDB_ERR("Unable to parse past parameter size. "
            "Blob offset %d is greater than the blob size %d",
            *blob_offset, calblob.buf_size);
        return AR_EFAILED;
    }

    if (!ignore_error_code)
        *blob_offset += sizeof(uint32_t);

    if (*blob_offset > calblob.buf_size)
    {
        ACDB_ERR("Unable to parse past error code. "
            "Blob offset %d is greater than the blob size %d",
            *blob_offset, calblob.buf_size);
        return AR_EFAILED;
    }

	caldata->param_payload = ACDB_MALLOC(uint8_t, caldata->param_size);
		
	if (!IsNull(caldata->param_payload))
	{
		ACDB_MEM_CPY_SAFE(caldata->param_payload, (size_t)caldata->param_size, 
            (uint8_t*)calblob.buf + *blob_offset, (size_t)caldata->param_size);
		*blob_offset += caldata->param_size;
        *blob_offset += ACDB_PADDING_8_BYTE(caldata->param_size);
	}
    else
    {
        ACDB_ERR("Error[%d]: Unable to allocate %d bytes for "
            "iid 0x%x pid 0x%x", AR_ENOMEMORY, 
            caldata->param_size, caldata->module_iid, caldata->param_id);
        return AR_ENOMEMORY;
    }
	
    return AR_EOK;
}

void CalDataToAcdbBlob(AcdbDeltaModuleCalData *caldata, AcdbBlob calblob, uint32_t *blob_offset)
{
    if (*blob_offset < calblob.buf_size)
    {
        ACDB_MEM_CPY_SAFE(calblob.buf + *blob_offset, sizeof(uint32_t),
            &caldata->module_iid, sizeof(uint32_t));
        *blob_offset += sizeof(uint32_t);

        ACDB_MEM_CPY_SAFE(calblob.buf + *blob_offset, sizeof(uint32_t),
            &caldata->param_id, sizeof(uint32_t));
        *blob_offset += sizeof(uint32_t);

        ACDB_MEM_CPY_SAFE(calblob.buf + *blob_offset, sizeof(uint32_t),
            &caldata->param_size, sizeof(uint32_t));
        *blob_offset += sizeof(uint32_t);

        if (!IsNull(caldata->param_payload))
        {
            ACDB_MEM_CPY_SAFE((uint8_t*)calblob.buf + *blob_offset, (size_t)caldata->param_size,
                caldata->param_payload, (size_t)caldata->param_size);
            *blob_offset += caldata->param_size;
        }
    }
}

void FreeCalData(AcdbDeltaModuleCalData **caldata)
{
	ACDB_FREE((*caldata)->param_payload);
	ACDB_FREE(*caldata);
	*caldata = NULL;
}

void FreeSubgraphIIDMap(SubgraphModuleIIDMap* sg_iid_map)
{
    if (IsNull(sg_iid_map)) return;

    if (!IsNull(sg_iid_map) &&
        !IsNull(sg_iid_map->proc_info))
    {
        ACDB_FREE(sg_iid_map->proc_info);
        sg_iid_map->proc_info = NULL;
    }

    ACDB_FREE(sg_iid_map);
}

/**
* \brief
* Retrieve the mapping between a subgraph and its modules from the ACDB DATA files
* Caller is responsible for freeing memory to sg_iid_map
*
* \param[in] subgraph_id: The subgraph to search for
* \param[out] sg_iid_map: The map parsed from the data files
* \return 0 on Success, non-zero on failure
*/
int32_t GetSubgraphIIDMap(uint32_t subgraph_id, SubgraphModuleIIDMap **sg_iid_map)
{
	//Acdb File Manager will handle selecting the correct ACDB file to read from
	//ar_fhandle fhandle = NULL;

	uint32_t status = AR_EOK;
	uint32_t offset = 0;
    ChunkInfo ci = { ACDB_CHUNKID_SG_IID_MAP, 0, 0 };
	uint32_t subgraph_count = 0;
	uint32_t found_count = 0;
    //Header: <Subgraph ID, Processor Count, Size>
    size_t sz_subgraph_obj_header = 3 * sizeof(uint32_t);

    if (IsNull(sg_iid_map))
    {
        ACDB_ERR("Error[%d]: Subgraph Instance ID Map is null",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    status = ACDB_GET_CHUNK_INFO(&ci);
    if (AR_EOK != status) return status;

    offset = ci.chunk_offset;
    status = FileManReadBuffer(&subgraph_count, sizeof(uint32_t), &offset);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Failed to read subgraph count", status);
        return status;
    }

	for (uint32_t i = 0; i < subgraph_count; i++)
	{
        if (IsNull(*sg_iid_map))
        {
            *sg_iid_map = ACDB_MALLOC(SubgraphModuleIIDMap, 1);
            if (*sg_iid_map == NULL)
                return AR_ENOMEMORY;
            memset(*sg_iid_map, 0, sizeof(SubgraphModuleIIDMap));
        }

        status = FileManReadBuffer(*sg_iid_map, sz_subgraph_obj_header, &offset);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to read subgraph count", status);
            goto end;
        }

		if (subgraph_id == (*sg_iid_map)->subgraph_id)
		{
            if ((*sg_iid_map)->size <= 0) continue;

            (*sg_iid_map)->proc_info = ACDB_MALLOC(
                ProcIIDMap, (*sg_iid_map)->size);

            if (IsNull((*sg_iid_map)->proc_info))
            {
                status = AR_ENOMEMORY;
                goto end;
            }

            //ACDB_DBG("Processor info Size(%d)", (*sg_iid_map)->size);

            status = FileManReadBuffer((*sg_iid_map)->proc_info,
                (*sg_iid_map)->size, &offset);
            if (AR_FAILED(status))
            {
                ACDB_ERR("Error[%d]: Failed to read processor info", status);
                goto end;
            }

			found_count++;
			break;
		}
		else
		{
			offset += (*sg_iid_map)->size;
		}
	}

    end:
    status = (0 == found_count)? AR_ENOTEXIST: status;

    if (AR_FAILED(status))
    {
        FreeSubgraphIIDMap(*sg_iid_map);
        *sg_iid_map = NULL;
    }

	return status;
}

/**
*	\brief DoesSubgraphContainModule
*		Checks if a module is contained in a subgraph
*	\param[in] subgraph_id: The subgraph to search for
*	\param[in] module_iid: The module instance to check for
*	\param[in] status: ar error status
*	\return TRUE if found, FALSE otherwise
*/
bool_t DoesSubgraphContainModule(uint32_t subgraph_id, uint32_t module_iid, int32_t *status)
{
    SubgraphModuleIIDMap *sg_iid_map = NULL;

    bool_t found = FALSE;

    *status = GetSubgraphIIDMap(subgraph_id, &sg_iid_map);

    if (AR_EOK != *status) return found;

    for (uint32_t i = 0; i < (sg_iid_map)->proc_count; i++)
    {
        for (uint32_t j = 0; j < (sg_iid_map)->proc_info[i].iid_mid_count; j++)
        {
            if (module_iid == (sg_iid_map)->proc_info[i].iid_mid_list[j].mid_iid)
            {
                found = TRUE;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }

    FreeSubgraphIIDMap(sg_iid_map);

    return found;
}

int32_t IsPidGlobalyPersistent(uint32_t pid)
{
	//Acdb File Manager will handle selecting the correct ACDB file to read from
	//ar_fhandle fhandle = NULL;

	uint32_t status = AR_EOK;
	uint32_t offset = 0;
	uint32_t cal_id_count = 0;
	uint32_t search_index = 0;
	uint32_t *glb_persist_pid_map = NULL;
	ChunkInfo ci = { ACDB_CHUNKID_GLB_PID_PERSIST_MAP, 0, 0 };
	//Header: <Cal ID, PID, Cal Data offset in datapool>
	size_t sz_cal_id_obj_header = 3 * sizeof(uint32_t);

	CalibrationIdMap cal_id_obj;

	cal_id_obj.cal_id = 0;
	cal_id_obj.param_id = pid;
	cal_id_obj.cal_data_offset = 0;

	//ACDB_CLEAR_BUFFER(glb_buf_1);

	status = AcdbGetChunkInfo(ci.chunk_id, &ci.chunk_offset, &ci.chunk_size);
	if (AR_EOK != status) return status;

    offset = ci.chunk_offset;
    status = FileManReadBuffer(&cal_id_count, sizeof(uint32_t), &offset);
	if (AR_EOK != status) return status;

    if (cal_id_count == 0)
    {
        ACDB_ERR("Cannot determine global persistence of Parameter(%x). "
            "The number of Calibration Identifiers is zero.", pid);
        return AR_ENOTEXIST;
    }

    glb_persist_pid_map = ACDB_MALLOC(uint32_t, 3 * cal_id_count);

    status = FileManReadBuffer(glb_persist_pid_map, cal_id_count * sz_cal_id_obj_header, &offset);
    if (AR_EOK != status) goto end;

	if (AR_EOK != AcdbDataBinarySearch2(
		glb_persist_pid_map/*glb_buf_1*/, cal_id_count * sz_cal_id_obj_header,
		&cal_id_obj, 1, sizeof(CalibrationIdMap)/sizeof(uint32_t),
		&search_index))
	{
		status =  AR_ENOTEXIST;
	}

end:

	ACDB_FREE(glb_persist_pid_map);

	return status;
}

int32_t IsPidPersistent(uint32_t pid)
{
	int32_t status = AR_EOK;
	uint32_t num_pids = 0;
	uint32_t offset = 0;
	uint32_t search_index = 0;
	uint32_t *persist_pid_map = NULL;
	ChunkInfo ci = { ACDB_CHUNKID_PIDPERSIST, 0, 0 };

    status = ACDB_GET_CHUNK_INFO(&ci);
	if (AR_EOK != status) return status;

    offset = ci.chunk_offset;
    status = FileManGetFilePointer1(
        (void**)&persist_pid_map, sizeof(uint32_t), &offset);
    if (AR_EOK != status) return status;

    num_pids = *persist_pid_map;
    if (num_pids == 0)
    {
        return AR_ENOTEXIST;
    }

    status = FileManGetFilePointer1(
        (void**)&persist_pid_map, num_pids * sizeof(uint32_t), &offset);
    if (AR_EOK != status) return status;

	if (AR_EOK != AcdbDataBinarySearch2(
		persist_pid_map, num_pids * sizeof(uint32_t),
		&pid, 1, 1,
		&search_index))
	{
		status =  AR_ENOTEXIST;
	}

	return status;
}

int32_t DataProcGetPersistenceType(uint32_t pid, AcdbDataPersistanceType *persistence_type)
{
    int32_t status = AR_EOK;

    if (IsNull(persistence_type))
        return AR_EBADPARAM;

    status = IsPidPersistent(pid);
    if (AR_SUCCEEDED(status))
    {
        *persistence_type = ACDB_DATA_PERSISTENT;
        return status;
    }

    //PID not found in Persistent Map so its either Non-persist or global
    if(status == AR_ENOTEXIST)
    {
        status = IsPidGlobalyPersistent(pid);
        if (AR_SUCCEEDED(status))
        {
            *persistence_type = ACDB_DATA_GLOBALY_PERSISTENT;
            return status;
        }
        else if (status == AR_ENOTEXIST)
        {
            *persistence_type = ACDB_DATA_NON_PERSISTENT;
            status = AR_EOK;
            return status;
        }
    }

    *persistence_type = ACDB_DATA_UNKNOWN;
    ACDB_ERR("Error[%d] - Unable to determine the persistence type of Parameter(0x%x)", status, pid);
    return status;
}

/**
* \brief ConvertSetCalDataReqToMap
*		Converts the input request into a format usable by the
*		delta file manager.
* \param[in] req_key_vector: The key vector passed in by the client. The
* request key vector can be AcdbGraphKeyVector or AcdbModuleTag
* \param[in] key_vector_type: The type of key vector used in the requeset(CKV, TKV, etc...)
* \param[in] param_data: The subgraph param data to set to the heap
* \return a map pointer on success, Null on failure
*/
AcdbDeltaKVToSubgraphDataMap *ConvertSetCalDataReqToMap(
	void *req_key_vector, KeyVectorType key_vector_type,
    AcdbSubgraphParamData* param_data, int32_t *status)
{
	AcdbBlob calblob;
	uint32_t blob_offset = 0;
	uint32_t subgraph_id = 0;
    uint32_t num_sg_ids = 0;
    uint32_t *sg_id_list = NULL;
	LinkedListNode *node = NULL;
	AcdbDeltaSubgraphData *subgraph_data = NULL;
    AcdbGraphKeyVector *key_vector = NULL;
    AcdbDeltaKVToSubgraphDataMap* map = NULL;

    if (IsNull(status))
        return NULL;

    *status = AR_EOK;
    calblob.buf = NULL;
    calblob.buf_size = 0;
    map = ACDB_MALLOC(AcdbDeltaKVToSubgraphDataMap, 1);

	if (IsNull(map))
	{
		*status = AR_ENOMEMORY;
		return NULL;
	}

	ACDB_CLEAR_BUFFER(*map);
    map->map_size = 0;
    map->key_vector_type = key_vector_type;
    switch (key_vector_type)
    {
        case TAG_KEY_VECTOR:
            map->key_vector_data = ACDB_MALLOC(AcdbModuleTag, 1);
            if (IsNull((AcdbModuleTag*)map->key_vector_data))
            {
                *status = AR_ENOMEMORY;
                ACDB_FREE(map);
                return NULL;
            }

            ((AcdbModuleTag*)map->key_vector_data)->tag_id =
                ((AcdbModuleTag*)req_key_vector)->tag_id;
            key_vector = &((AcdbModuleTag*)map->key_vector_data)->tag_key_vector;

            key_vector->num_keys =
                ((AcdbModuleTag*)req_key_vector)->tag_key_vector.num_keys;

            key_vector->graph_key_vector =
                ACDB_MALLOC(AcdbKeyValuePair,
                    key_vector->num_keys);

            if (IsNull(key_vector->graph_key_vector))
            {
                *status = AR_ENOMEMORY;
                ACDB_FREE(map);
                return NULL;
            }

            ACDB_MEM_CPY_SAFE(
                key_vector->graph_key_vector,
                key_vector->num_keys * sizeof(AcdbKeyValuePair),
                ((AcdbModuleTag*)req_key_vector)->tag_key_vector.graph_key_vector,
                key_vector->num_keys * sizeof(AcdbKeyValuePair));

            break;
        case CAL_KEY_VECTOR:
            key_vector = ACDB_MALLOC(AcdbGraphKeyVector, 1);
            if (IsNull(key_vector))
            {
                *status = AR_ENOMEMORY;
                ACDB_FREE(map);
                return NULL;
            }

            key_vector->graph_key_vector = NULL;
            map->key_vector_data = key_vector;

            key_vector->num_keys =
                ((AcdbGraphKeyVector*)req_key_vector)->num_keys;

            if (key_vector->num_keys)
            {
                key_vector->graph_key_vector =
                    ACDB_MALLOC(AcdbKeyValuePair,
                        key_vector->num_keys);

                if (IsNull(key_vector->graph_key_vector))
                {
                    *status = AR_ENOMEMORY;
                    ACDB_FREE(key_vector);
                    ACDB_FREE(map);
                    return NULL;
                }

                ACDB_MEM_CPY_SAFE(
                    key_vector->graph_key_vector,
                    key_vector->num_keys * sizeof(AcdbKeyValuePair),
                    ((AcdbGraphKeyVector*)req_key_vector)->graph_key_vector,
                    key_vector->num_keys * sizeof(AcdbKeyValuePair));
            }
            break;
    default:
        break;
    }

    num_sg_ids = param_data->subgraph_id_list.count;
    sg_id_list = param_data->subgraph_id_list.list;

    calblob.buf_size = param_data->data_size;
    calblob.buf = param_data->data;

    map->num_subgraphs = num_sg_ids;
	ACDB_CLEAR_BUFFER(map->subgraph_data_list);

	for (uint32_t i = 0; i < num_sg_ids; i++)
	{
		subgraph_id = sg_id_list[i];

		subgraph_data = ACDB_MALLOC(AcdbDeltaSubgraphData, 1);

		if (IsNull(subgraph_data))
		{
			*status = AR_ENOMEMORY;
			break;
		}

		ACDB_CLEAR_BUFFER(subgraph_data->non_global_data);
		ACDB_CLEAR_BUFFER(subgraph_data->global_data);

		node = AcdbListCreateNode(subgraph_data);
		if (IsNull(node))
		{
			*status = AR_ENOMEMORY;
			break;
		}

		AcdbListAppend(&map->subgraph_data_list, node);

		subgraph_data->subgraph_id = subgraph_id;
		subgraph_data->subgraph_data_size = 0;

		blob_offset = 0;

		//Parse Cal Blob and map the subgraph to its modules
		while (blob_offset < calblob.buf_size)
		{
            if (blob_offset > calblob.buf_size)
            {
                ACDB_ERR("Error[%d]: cal data blob offset %d is larger "
                    "than blob size %d", *status, blob_offset, calblob.buf_size);
                *status = AR_EFAILED;
                break;
            }

			AcdbDeltaModuleCalData *caldata = 
				ACDB_MALLOC(AcdbDeltaModuleCalData, 1);

			if (IsNull(caldata))
			{
				*status = AR_ENOMEMORY;
				break;
			}

            *status = AcdbBlobToCalData(param_data->ignore_error_code,
                caldata, calblob, &blob_offset);
            if (AR_FAILED(*status))
            {
                *status = AR_ENOMEMORY;
                break;
            }

			if (TRUE == DoesSubgraphContainModule(subgraph_id,
				caldata->module_iid, status) && AR_EOK == *status)
			{
				LinkedListNode *caldata_node = AcdbListCreateNode(caldata);

                //Non Global(Persistent and Non-Persistent)
                subgraph_data->non_global_data.data_size +=
                    3 * sizeof(uint32_t) + caldata->param_size;

                AcdbListAppend(
                    &subgraph_data->non_global_data.cal_data_list,
                    caldata_node);

                //todo: uncomment this functionality when global persist is supported
				//if (AR_EOK == IsPidGlobalyPersistent(caldata->param_id))
				//{
				//	//Global (Globaly Persistent)
				//	subgraph_data->global_data.data_size +=
				//		3 * sizeof(uint32_t) + caldata->param_size;

				//	AcdbListAppend(
				//		&subgraph_data->global_data.cal_data_list,
				//		caldata_node);
				//}
				//else
				//{
					//Non Global(Persistent and Non-Persistent)
					//subgraph_data->non_global_data.data_size +=
					//	3 * sizeof(uint32_t) + caldata->param_size;

					//AcdbListAppend(
					//	&subgraph_data->non_global_data.cal_data_list,
					//	caldata_node);
				//}
			}
			else
			{
                //ACDB_DBG("Subgraph(0x%x) does not contain Module Instance(0x%x).", subgraph_id, caldata->module_iid);
				FreeCalData(&caldata);
			}
		}

        if (AR_FAILED(*status))
        {
            ACDB_ERR("Error[%d]: Unable to create delta data "
                "for subgraph 0x%x", *status, subgraph_id);
            break;
        }

		subgraph_data->subgraph_data_size =
			subgraph_data->non_global_data.data_size +
			subgraph_data->global_data.data_size;

		map->map_size += subgraph_data->subgraph_data_size;
	}

	if (AR_EOK != *status)
	{
		free_map(map);
		map = NULL;
	}
	else
	{
		map->map_size += map->num_subgraphs * sizeof(uint32_t);
	}

	subgraph_data = NULL;
	node = NULL;

	return map;
}

int32_t UpdateHeap(AcdbDeltaKVToSubgraphDataMap *req_map)
{
    int32_t status = AR_EOK;
    AcdbDeltaKVToSubgraphDataMap *heap_map = NULL;
    AcdbGraphKeyVector* key_vector = NULL;

    if (IsNull(req_map))
    {
        ACDB_ERR("Failed to convert Set Data request to map");
        return status;
    }

    switch (req_map->key_vector_type)
    {
    case TAG_KEY_VECTOR:
        key_vector =
            &((AcdbModuleTag*)req_map->key_vector_data)->tag_key_vector;

        if (IsNull(key_vector))
        {
            ACDB_ERR("The request Tag Key Vector is null");
            return AR_EBADPARAM;
        }
        break;
    case CAL_KEY_VECTOR:
        key_vector =
            (AcdbGraphKeyVector*)req_map->key_vector_data;

        if (IsNull(key_vector))
        {
            ACDB_ERR("The request Calibration Key Vector is null");
            return AR_EBADPARAM;
        }
        break;
    default:
        break;
    }

    if (IsNull(key_vector))
    {
        ACDB_ERR("The request Key Vector is null");
        return AR_EBADPARAM;
    }

    status = acdb_heap_ioctl(
        ACDB_HEAP_CMD_GET_MAP,
        (uint8_t*)key_vector,
        (uint8_t*)&heap_map, sizeof(uint32_t));

    key_vector = NULL;

    if (AR_ENOTEXIST == status)
    {
        /**
        * Scenario 1 - Add New CKV-to-Subgraph Map to Heap
        * KV is not in the heap, so skip the comparison and write it to the heap
        */
        status = acdb_heap_ioctl(
            ACDB_HEAP_CMD_ADD_MAP,
            (uint8_t*)req_map,
            NULL, 0);
    }
    else
    {
        /**
        * Scenario 2 & 3
        * Update the subgraph data list for an existing map as well as update
        * module calibration for an existing subgraph.
        * Heap subgraph list is being modified. Any subgraph entry found is
        * moved to the end of the heap maps subgraph list
        */
        LinkedListNode *heap_sg_data_node = heap_map->subgraph_data_list.p_head;
        LinkedListNode *req_sg_data_node = req_map->subgraph_data_list.p_head;
        AcdbDeltaSubgraphData *heap_sg_data = NULL;
        AcdbDeltaSubgraphData *req_sg_data = NULL;
        LinkedListNode *heap_prev = NULL;
        LinkedListNode *req_prev = NULL;
        LinkedListNode *dont_care = NULL;
        bool_t is_new_sg = FALSE;

        while (!IsNull(req_sg_data_node))
        {
            req_sg_data = (AcdbDeltaSubgraphData*)req_sg_data_node->p_struct;

            if (IsNull(heap_sg_data_node))
            {
                status = AR_EFAILED;
                break;
            }

            do
            {
                if (heap_sg_data_node == dont_care)
                {
                    is_new_sg = TRUE;
                    break;
                }

                heap_sg_data = (AcdbDeltaSubgraphData*)heap_sg_data_node->p_struct;

                if ((heap_sg_data->subgraph_id == req_sg_data->subgraph_id) &&
                    req_sg_data->subgraph_data_size == 0)
                {
                    break;
                }
                else if (heap_sg_data->subgraph_id == req_sg_data->subgraph_id)
                {
                    //Perform Calibration Data Diff
                    is_new_sg = FALSE;
                    AcdbDeltaModuleCalData *heap_caldata = NULL;
                    AcdbDeltaModuleCalData *req_caldata = NULL;
                    LinkedListNode *heap_caldata_node = NULL;
                    LinkedListNode *req_caldata_node = NULL;
                    LinkedListNode *req_caldata_prev = NULL;
                    bool_t found = FALSE;

                    heap_caldata_node = heap_sg_data->non_global_data.cal_data_list.p_head;
                    req_caldata_node = req_sg_data->non_global_data.cal_data_list.p_head;

                    do
                    {
                        found = FALSE;
                        req_caldata = (AcdbDeltaModuleCalData*)req_caldata_node->p_struct;
                        do
                        {
                            heap_caldata = (AcdbDeltaModuleCalData*)heap_caldata_node->p_struct;

                            // Compare <Instance ID, Parameter ID>
                            if (0 == ACDB_MEM_CMP(heap_caldata, req_caldata, 2 * sizeof(uint32_t)))
                            {
                                /* Replace the old parameter data with then new parameter data by
                                * subtracting the old parameters size and freeing its data */

                                // Remove old param data from heap map
                                heap_map->map_size -= heap_caldata->param_size;
                                heap_sg_data->non_global_data.data_size -= heap_caldata->param_size;

                                //Add request maps data size
                                heap_map->map_size += req_caldata->param_size;
                                heap_sg_data->non_global_data.data_size += req_caldata->param_size;

                                ACDB_FREE(heap_caldata->param_payload);
                                heap_caldata->param_payload = req_caldata->param_payload;
                                heap_caldata->param_size = req_caldata->param_size;

                                req_caldata->param_payload = NULL;
                                req_caldata->param_size = 0;

                                found = TRUE;
                                break;
                            }

                        } while (TRUE != AcdbListSetNext(&heap_caldata_node));

                        if (!found)
                        {
                            AcdbListRemove(&req_sg_data->non_global_data.cal_data_list, req_caldata_prev, req_caldata_node);
                            AcdbListAppend(&heap_sg_data->non_global_data.cal_data_list, req_caldata_node);

                            //Update Map Size and Subgraph data size
                            heap_sg_data->non_global_data.data_size += req_sg_data->non_global_data.data_size;
                            heap_map->map_size += req_sg_data->non_global_data.data_size;

                            if (IsNull(req_prev))
                            {
                                //If the head is removed and added to the new list,
                                //reset the request subgraph node to the new head
                                req_caldata_node = req_sg_data->non_global_data.cal_data_list.p_head;
                            }
                            else
                            {
                                req_caldata_node = req_caldata_prev;
                            }
                        }

                        heap_caldata_node = heap_sg_data->non_global_data.cal_data_list.p_head;
                        req_caldata_prev = req_caldata_node;
                    } while (TRUE != AcdbListSetNext(&req_caldata_node));

                    if (dont_care == NULL)
                    {
                        dont_care = heap_sg_data_node;
                    }

                    AcdbListMoveToEnd(&heap_map->subgraph_data_list, &heap_sg_data_node, &heap_prev);
                    break;
                }
                else
                {
                    is_new_sg = TRUE;
                }

                heap_prev = heap_sg_data_node;

            } while (TRUE != AcdbListSetNext(&heap_sg_data_node));

            //Add any subgraphs that were not found in the heap here
            if (is_new_sg)
            {
                heap_map->map_size += sizeof(req_sg_data->subgraph_id) + req_sg_data->subgraph_data_size;
                heap_map->num_subgraphs++;

                AcdbListRemove(&req_map->subgraph_data_list, req_prev, req_sg_data_node);
                AcdbListAppend(&heap_map->subgraph_data_list, req_sg_data_node);
                is_new_sg = FALSE;

                if (dont_care == NULL)
                {
                    dont_care = req_sg_data_node;
                }
                if (IsNull(req_prev))
                {
                    //If the head is removed and added to the new list,
                    //reset the request subgraph node to the new head
                    req_sg_data_node = req_map->subgraph_data_list.p_head;
                    heap_sg_data_node = heap_map->subgraph_data_list.p_head;
                    continue;
                }
                else
                {
                    req_sg_data_node = req_prev;
                }
            }

            req_prev = req_sg_data_node;
            heap_sg_data_node = heap_map->subgraph_data_list.p_head;
            req_sg_data_node = req_sg_data_node->p_next;
        }

        //Free Request Map
        free_map(req_map);
        req_map = NULL;
    }

    return status;
}

int32_t DataProcSetMapToHeap(AcdbDeltaKVToSubgraphDataMap *map)
{
    int32_t status = AR_EOK;

    status = UpdateHeap(map);

    return status;
}


int32_t SetSubraphData(void *req, KeyVectorType key_vector_type,
    AcdbSubgraphParamData *subgraph_param_data)
{
	int32_t status = AR_EOK;
    AcdbDeltaKVToSubgraphDataMap *req_map = NULL;
    AcdbGraphKeyVector* key_vector = NULL;

    switch (key_vector_type)
    {
    case TAG_KEY_VECTOR:
        key_vector = &((AcdbModuleTag*)req)->tag_key_vector;

        if (IsNull(key_vector))
        {
            ACDB_ERR("The request Tag Key Vector is null");
            return AR_EBADPARAM;
        }
        break;
    case CAL_KEY_VECTOR:
        key_vector = ((AcdbGraphKeyVector*)req);

        if (IsNull(key_vector))
        {
            ACDB_ERR("The request Calibration Key Vector is null");
            return AR_EBADPARAM;
        }
        break;
    default:
        break;
    }

    if (IsNull(key_vector))
    {
        ACDB_ERR("The request Key Vector is null");
        return AR_EBADPARAM;
    }

	status = AcdbSort2(key_vector->num_keys * sizeof(AcdbKeyValuePair),
        key_vector->graph_key_vector, sizeof(AcdbKeyValuePair), 0);

	req_map = ConvertSetCalDataReqToMap(req, key_vector_type,
        subgraph_param_data, &status);

    if (IsNull(req_map) || AR_FAILED(status))
    {
        ACDB_ERR("Failed to convert Set Data request to map");
        return status;
    }

    status = UpdateHeap(req_map);

	return status;
}

int32_t DataProcSetCalData(AcdbGraphKeyVector* cal_key_vector,
    AcdbSubgraphParamData* subgraph_param_data)
{
    return SetSubraphData(cal_key_vector, CAL_KEY_VECTOR,
        subgraph_param_data);
}

int32_t DataProcSetTagData(AcdbModuleTag* module_tag,
    AcdbSubgraphParamData* subgraph_param_data)
{
    return SetSubraphData(module_tag, TAG_KEY_VECTOR,
        subgraph_param_data);
}

int32_t GetSubgraphCalData(
    AcdbDataPersistanceType persist_type, AcdbDataClient client,
    AcdbGraphKeyVector *key_vector,
    uint32_t subgraph_id, uint32_t module_iid, uint32_t param_id,
    bool_t is_offloaded_param, uint32_t *blob_offset, AcdbBlob *blob)
{
	int32_t status = AR_EOK;
	int32_t error_code = 0;
    uint32_t padded_param_size = 0;
    uint32_t tmp_param_size = 0;
    uint32_t caldata_size = 0;
	uint32_t padding = 0;
    uint32_t tmp_blob_offset = 0;
	bool_t found = FALSE;
    AcdbDeltaKVToSubgraphDataMap *heap_map = NULL;
	LinkedListNode *sg_data_node = NULL;
	AcdbDeltaSubgraphData *sg_data = NULL;
	status = acdb_heap_ioctl(
		ACDB_HEAP_CMD_GET_MAP,
		(uint8_t*)key_vector,
		(uint8_t*)&heap_map, sizeof(uint32_t));

	if (AR_EOK != status) return status;

	sg_data_node = heap_map->subgraph_data_list.p_head;

    //Resolve persist type if its unknown
    if (persist_type == ACDB_DATA_UNKNOWN)
    {
        /* TODO: replace this check with DataProcGetPersistenceType(...)
         * when global persistence is supported */
        persist_type = IsPidPersistent(param_id) == AR_EOK ?
            ACDB_DATA_PERSISTENT : ACDB_DATA_NON_PERSISTENT;
    }

	do
	{
		sg_data = (AcdbDeltaSubgraphData*)sg_data_node->p_struct;

		if (subgraph_id == sg_data->subgraph_id)
		{
			LinkedListNode *cur_node = NULL;
			AcdbDeltaModuleCalData *caldata = NULL;

			switch (persist_type)
			{
			case ACDB_DATA_GLOBALY_PERSISTENT:
			{
				cur_node = sg_data->global_data.cal_data_list.p_head;
			}
			break;
			case ACDB_DATA_PERSISTENT:
			case ACDB_DATA_NON_PERSISTENT:
			{
				cur_node = sg_data->non_global_data.cal_data_list.p_head;
			}
			break;
			default:
				return AR_EBADPARAM;
			}

			while (cur_node != NULL)
			{
				caldata = (AcdbDeltaModuleCalData*)cur_node->p_struct;

				if (caldata->module_iid != module_iid ||
					caldata->param_id != param_id)
				{
					status = AR_ENOTEXIST;
					cur_node = cur_node->p_next;
					continue;
				}

				//Collect only persistent data
				if (ACDB_DATA_PERSISTENT == persist_type &&
					AR_EOK != IsPidPersistent(caldata->param_id))
				{
					cur_node = cur_node->p_next;
				}

				//Collect only non-persistent data
				if (ACDB_DATA_NON_PERSISTENT == persist_type &&
					AR_EOK == IsPidPersistent(caldata->param_id))
				{
					cur_node = cur_node->p_next;
				}

				//Collect only globaly persistent data
				if (ACDB_DATA_GLOBALY_PERSISTENT == persist_type &&
					AR_EOK != IsPidGlobalyPersistent(caldata->param_id))
				{
					cur_node = cur_node->p_next;
				}


                status = AR_EOK;
                //accumulate size

                /*
                When ARCT requests for calibration from the heap, padding is not requried.
                When ACDB Clients requests for calibration data, padding is requried since the
                DSP requires 8 byte alignment.
                */
                switch (client)
                {
                case ACDB_DATA_ACDB_CLIENT:

                    if (is_offloaded_param)
                    {
                        AcdbPayload offloaded_caldata = { 0 };

                        if (blob->buf == NULL)
                            tmp_blob_offset = blob->buf_size;
                        else
                            tmp_blob_offset = *blob_offset;

                        tmp_blob_offset += sizeof(AcdbDspModuleHeader);
                        offloaded_caldata.size = caldata->param_size;
                        offloaded_caldata.payload = caldata->param_payload;

                        status = GetOffloadedParameterData( &offloaded_caldata,
                            &tmp_param_size, blob, &tmp_blob_offset);

                        padded_param_size = tmp_param_size;
                        padding = 0;

                        if (blob->buf != NULL)
                        {
                            tmp_param_size = caldata->param_size;
                            caldata->param_size = padded_param_size;
                        }
                    }
                    else
                    {
                        padded_param_size = ACDB_ALIGN_8_BYTE(
                            caldata->param_size);
                        padding = padded_param_size
                            - caldata->param_size;
                    }
                    break;

                case ACDB_DATA_ATS_CLIENT:
                    padded_param_size = caldata->param_size;
                    padding = 0;
                    break;
                default:
                    break;
                }

                caldata_size = sizeof(AcdbDspModuleHeader)
                    + padded_param_size;

				//AcdbDeltaModuleCalData to AcdbBlob
				if (blob->buf != NULL)
				{
                    if (client == ACDB_DATA_ATS_CLIENT)
                    {
                        blob->buf_size += caldata_size;
                    }

                    //Copy MID, PID, Param Size
                    if (blob->buf_size < (*blob_offset +
                        sizeof(AcdbDspModuleHeader) - sizeof(uint32_t)))
                        return AR_ENEEDMORE;

                    ACDB_MEM_CPY_SAFE( blob->buf + *blob_offset,
                        sizeof(AcdbDspModuleHeader) - sizeof(uint32_t),
                        caldata, sizeof(AcdbDspModuleHeader) - sizeof(uint32_t));
                    *blob_offset += sizeof(AcdbDspModuleHeader)
                        - sizeof(uint32_t);

                    //Insert error code
                    if (blob->buf_size < (*blob_offset + sizeof(error_code)))
                        return AR_ENEEDMORE;

                    ACDB_MEM_CPY_SAFE(
                        blob->buf + *blob_offset, sizeof(error_code),
                        &error_code, sizeof(error_code));
                    *blob_offset += sizeof(error_code);

                    if (is_offloaded_param && client == ACDB_DATA_ACDB_CLIENT)
                    {
                        caldata->param_size = tmp_param_size;
                        *blob_offset = tmp_blob_offset;
                        return status;
                    }

                    //Payload
                    if (blob->buf_size < (*blob_offset + caldata->param_size))
                        return AR_ENEEDMORE;

                    ACDB_MEM_CPY_SAFE(
                        blob->buf + *blob_offset, caldata->param_size,
                        caldata->param_payload, caldata->param_size);
                    *blob_offset += caldata->param_size;

					//Add Padding
					if (padding != 0)
					{
						memset((uint8_t*)blob->buf + *blob_offset, 0, padding);
						*blob_offset += padding;
					}
				}
                else
                {
                    //Moudle IID, Param ID, Param Size, Error Code, Payload
                    blob->buf_size += caldata_size;
                    *blob_offset += caldata_size;
                }

				found = TRUE;
				break;
			}
		}

		if (TRUE == found) break;

	} while (TRUE != AcdbListSetNext(&sg_data_node));

    if (!found) status = AR_ENOTEXIST;

	return status;
}

int32_t DataProcGetHeapInfo(AcdbBufferContext *rsp)
{
    int32_t status = AR_EOK;
    status = acdb_heap_ioctl(ACDB_HEAP_CMD_GET_HEAP_INFO,
        NULL,
        (uint8_t*)rsp, sizeof(AcdbBufferContext));

    if (AR_FAILED(status))
    {
        ACDB_ERR("Failed to get key vector and map info from the heap")
    }
    return status;
}

int32_t WriteCalDataToBuffer(LinkedListNode *caldata_head, AcdbBlob* blob, uint32_t *blob_offset)
{
    int32_t status = AR_EOK;
    AcdbDeltaModuleCalData *caldata = NULL;
    LinkedListNode *cur_node = caldata_head;
    uint32_t offset = *blob_offset;

    if (IsNull(caldata_head)) return AR_ENOTEXIST;

    while (!IsNull(cur_node))
    {
        caldata = (AcdbDeltaModuleCalData*)cur_node->p_struct;

        status = AR_EOK;

        CalDataToAcdbBlob(caldata, *blob, &offset);

        cur_node = cur_node->p_next;
    }

    *blob_offset = offset;
    cur_node = NULL;
    return status;
}

int32_t DataProcGetHeapData(AcdbGraphKeyVector *key_vector, AcdbBufferContext* rsp, uint32_t *blob_offset)
{
    int32_t status = AR_EOK;
    AcdbDeltaKVToSubgraphDataMap *heap_map = NULL;
    LinkedListNode *sg_data_node = NULL;
    AcdbDeltaSubgraphData *sg_data = NULL;
    AcdbGraphKeyVector *heap_map_key_vector = NULL;
    AcdbBlob blob = { 0 };
    uint32_t offset = *blob_offset;
    blob.buf = rsp->buf;
    blob.buf_size = rsp->size;

    status = acdb_heap_ioctl(
        ACDB_HEAP_CMD_GET_MAP,
        (uint8_t*)key_vector,
        (uint8_t*)&heap_map, sizeof(uint32_t));

    if (AR_ENOTEXIST == status)
    {
        ACDB_ERR("No heap entry found for the provided key vector.")
        return status;
    }

    heap_map_key_vector = get_key_vector_from_map(heap_map);
    if (IsNull(heap_map_key_vector))
    {
        ACDB_ERR("Error[%d]: Map retrieved from heap is null",
            AR_EBADPARAM);
            return AR_EBADPARAM;
    }

    //Write Heap map to buffer

    //Write Key Vector
    ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(heap_map_key_vector->num_keys),
        &heap_map_key_vector->num_keys,
        sizeof(heap_map_key_vector->num_keys));
    offset += sizeof(heap_map_key_vector->num_keys);

    uint32_t sz_key_vector = heap_map_key_vector->num_keys * sizeof(AcdbKeyValuePair);
    ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sz_key_vector,
        heap_map_key_vector->graph_key_vector,
        sz_key_vector);
    offset += sz_key_vector;

    //Write Map Size
    ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(heap_map->map_size),
        &heap_map->map_size, sizeof(heap_map->map_size));
    offset += sizeof(heap_map->map_size);

    //Write Number of Subgraphs Data Objs
    ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(heap_map->num_subgraphs),
        &heap_map->num_subgraphs, sizeof(heap_map->num_subgraphs));
    offset += sizeof(heap_map->num_subgraphs);

    //Write Subgraph Data Objs
    sg_data_node = heap_map->subgraph_data_list.p_head;

    do
    {
        sg_data = (AcdbDeltaSubgraphData*)sg_data_node->p_struct;

        //Write Subgraph ID
        ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(sg_data->subgraph_id),
            &sg_data->subgraph_id, sizeof(sg_data->subgraph_id));
        offset += sizeof(sg_data->subgraph_id);

        //Write Subgraph Data Size
        ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(sg_data->subgraph_data_size),
            &sg_data->subgraph_data_size, sizeof(sg_data->subgraph_data_size));
        offset += sizeof(sg_data->subgraph_data_size);

        //Write Calibration Data (Same code for Global and Non Global Data)
        ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(sg_data->non_global_data.data_size),
            &sg_data->non_global_data.data_size, sizeof(sg_data->non_global_data.data_size));
        offset += sizeof(sg_data->non_global_data.data_size);

		ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(sg_data->non_global_data.cal_data_list.length),
			&sg_data->non_global_data.cal_data_list.length, sizeof(sg_data->non_global_data.cal_data_list.length));
		offset += sizeof(sg_data->non_global_data.cal_data_list.length);

        //blob.buf = &rsp->buf[offset];

        status = WriteCalDataToBuffer(
            sg_data->non_global_data.cal_data_list.p_head,
            &blob, &offset);

        if (AR_SUCCEEDED(status))
        {
            *blob_offset += offset;
        }
        else
        {
            ACDB_DBG("There is no non-global subgraph calibration data in the heap entry");
            status = AR_EOK;
        }

        ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(sg_data->global_data.data_size),
            &sg_data->global_data.data_size, sizeof(sg_data->global_data.data_size));
        offset += sizeof(sg_data->global_data.data_size);

		ACDB_MEM_CPY_SAFE(&rsp->buf[offset], sizeof(sg_data->global_data.cal_data_list.length),
			&sg_data->global_data.cal_data_list.length, sizeof(sg_data->global_data.cal_data_list.length));
		offset += sizeof(sg_data->global_data.cal_data_list.length);

        status = WriteCalDataToBuffer(
            sg_data->global_data.cal_data_list.p_head,
            &blob, &offset);

        if (AR_SUCCEEDED(status))
        {
            *blob_offset += offset;
        }
        else
        {
            ACDB_DBG("There is no global subgraph calibration data in the heap entry");
            status = AR_EOK;
        }

    } while (TRUE != AcdbListSetNext(&sg_data_node));

    *blob_offset = offset;

    return status;
}

int32_t GetPartitionInfo(AcdbTableInfo *table_info, uint32_t scratch_buf_size,
    uint32_t* num_partitions, uint32_t *partition_size)
{
    if (IsNull(table_info) || IsNull(num_partitions) || IsNull(partition_size))
    {
        ACDB_ERR("Either table info, number of partitions, or partition size is null");
        return AR_EBADPARAM;
    }

    if (table_info->table_size > scratch_buf_size)
    {
        //Determine partition size which is the max readable size based on the scratch buffer
        *partition_size = (uint32_t)(
            scratch_buf_size - (scratch_buf_size % table_info->table_entry_size)
            );
        *num_partitions = (uint32_t)AcdbCeil(table_info->table_size, *partition_size);
    }
    else
    {
        *num_partitions = 1;
        *partition_size = table_info->table_size;
    }

    return AR_EOK;
}

int32_t AcdbPartitionBinarySearch(AcdbTableInfo *table_info,
    AcdbPartitionSearchInfo *part_info)
{
    int32_t status = AR_EOK;
    uint32_t entry_index = 0;
    uint32_t actual_index = 0;
    uint32_t part_offset = 0;
    uint32_t file_offset = 0;
    Partition part = { 0 };
    uint32_t num_parts = 0;
    uint32_t partition_size = 0;

    if (IsNull(table_info) || IsNull(part_info) || table_info->table_size <= 0)
    {
        return AR_EBADPARAM;
    }

    status = GetPartitionInfo(table_info, part_info->scratch_space.buf_size,
        &num_parts, &partition_size);

    part.offset = table_info->table_offset;
    part.size = partition_size;

    for (uint32_t j = 0; j < num_parts; j++)
    {
        file_offset = part.offset;
        status = FileManReadBuffer(part_info->scratch_space.buf,
            part.size, &file_offset);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Unable to read Partition[%d][size: %d bytes, offset: 0x%x]",
                j, part.offset, part.size);
            return AR_EFAILED;
        }

        part.offset += partition_size;

        //Binary Search
        if (SEARCH_ERROR == AcdbDataBinarySearch2(
            part_info->scratch_space.buf,
            part.size,
            part_info->table_entry_struct,
            part_info->num_search_keys,
            part_info->num_structure_elements,
            &entry_index))
        {
            //Nothing found in Partition X so continue
            status = AR_ENOTEXIST;
            continue;
        }
        else
        {
            //Found entry
            part_offset = j * part.size;
            actual_index = part_offset / sizeof(uint32_t) + entry_index;
            actual_index /= part_info->num_structure_elements;
            part_info->entry_index = actual_index;
            part_info->entry_offset = entry_index * sizeof(uint32_t);

            ACDB_MEM_CPY_SAFE(
                part_info->table_entry_struct,
                table_info->table_entry_size,
                ((uint8_t*)part_info->scratch_space.buf) + part_info->entry_offset,
                table_info->table_entry_size);

            status = AR_EOK;
            break;
        }
    }

    return status;
}

int32_t AcdbTableBinarySearch(
    AcdbTableInfo *table_info, AcdbTableSearchInfo *part_info)
{
    int32_t status = AR_EOK;
    uint32_t entry_index = 0;

    void *table_ptr = NULL;
    if (IsNull(table_info) || IsNull(part_info) || table_info->table_size <= 0)
    {
        return AR_EBADPARAM;
    }

    status = FileManGetFilePointer2(&table_ptr, table_info->table_offset);

    if (SEARCH_ERROR == AcdbDataBinarySearch2(
        table_ptr,
        table_info->table_size,
        part_info->table_entry_struct,
        part_info->num_search_keys,
        part_info->num_structure_elements,
        &entry_index))
    {
        //Nothing found in the table
        return AR_ENOTEXIST;
    }

    part_info->entry_index = entry_index;
    part_info->entry_offset = entry_index * sizeof(uint32_t);

    ACDB_MEM_CPY_SAFE(
        part_info->table_entry_struct,
        table_info->table_entry_size,
        ((uint8_t*)table_ptr) + part_info->entry_offset,
        table_info->table_entry_size);

    return status;
}

int32_t GetAcdbProperty(AcdbProperty *prop)
{
    int32_t status = AR_EOK;
    uint32_t entry_count = 0;
    uint32_t offset = 0;
    ChunkInfo ci_gprop = { 0 };
    ChunkInfo ci_data_pool = { 0 };
    AcdbTableInfo table_info = { 0 };
    AcdbTableSearchInfo search_info = { 0 };
    AcdbPropertyEntry entry = { 0 };

    ci_gprop.chunk_id = ACDB_CHUNKID_GLOBAL_PROP;
    ci_data_pool.chunk_id = ACDB_CHUNKID_DATAPOOL;
    status = ACDB_GET_CHUNK_INFO(&ci_gprop, &ci_data_pool);
    if (AR_FAILED(status))
    {
        ACDB_DBG("Error[%d]: Failed to read one or more chunks", status);
        return status;
    }

    offset = ci_gprop.chunk_offset;
    status = FileManReadBuffer(&entry_count, sizeof(uint32_t), &offset);
    if (AR_FAILED(status))
    {
        ACDB_DBG("Error[%d]: Unable to get property table entry count",
            status);
        return status;
    }

    entry.property_id = prop->property_id;
    //Setup Table Information (skip table entry count)
    table_info.table_offset = offset;
    table_info.table_size = ci_gprop.chunk_size - sizeof(uint32_t);
    table_info.table_entry_size = sizeof(AcdbPropertyEntry);

    //Setup Search Information
    search_info.num_search_keys = 1;
    search_info.num_structure_elements =
        (sizeof(AcdbPropertyEntry) / sizeof(uint32_t));
    search_info.table_entry_struct = &entry;

    status = AcdbTableBinarySearch(&table_info, &search_info);

    if (AR_FAILED(status) && status == AR_ENOTEXIST)
    {
        ACDB_ERR("Error[%d]: Property(0x%x) not found.",
            status, prop->property_id);
        return status;
    }
    else if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: An error occured while searching "
            "for Property(0x%x).", status, prop->property_id);
        return status;
    }

    offset = ci_data_pool.chunk_offset + entry.offset_property_data;
    status = FileManReadBuffer(&prop->size, sizeof(uint32_t), &offset);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to get property size for Property(0x%x)",
            status, prop->property_id);
        return status;
    }

    status = FileManGetFilePointer2((void*)&prop->property_data, offset);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to get property data for Property(0x%x)",
            status, prop->property_id);
        return status;
    }

    return status;
}

int32_t IsOffloadedParam(uint32_t param_id, AcdbUintList *offloaded_param_list)
{
    if (IsNull(offloaded_param_list))
        return AR_ENOTEXIST;

    for (uint32_t i = 0; i < offloaded_param_list->count; i++)
    {
        if (param_id == offloaded_param_list->list[i])
            return AR_EOK;
    }

    return AR_ENOTEXIST;
}

int32_t GetOffloadedParamList(AcdbUintList *offloaded_param_list)
{
    int32_t status = AR_EOK;
    AcdbProperty prop = { 0 };

    prop.property_id = ACDB_GPROP_OFFLOADED_PARAM_INFO;
    status = GetAcdbProperty(&prop);
    if (AR_FAILED(status))
    {
        offloaded_param_list->count = 0;
        offloaded_param_list->list = NULL;
        return status;
    }

    offloaded_param_list->count = *(uint32_t*)prop.property_data;
    offloaded_param_list->list = ((uint32_t*)prop.property_data + 1);

    return status;
}

int32_t GetOffloadedParameterData(
    AcdbPayload *caldata, uint32_t *paramSize,
    AcdbBlob *rsp, uint32_t *blob_offset)
{
    int32_t status = AR_EOK;
    uint32_t offset = 0;
    AcdbOffloadedParamHeader header = { 0 };
    size_t data_size = 0;
    uint32_t padding = 0;
    uint32_t path_length = 0;
    char_t* path = NULL;
    bool_t is_relative_path = FALSE;

    if (IsNull(caldata) || IsNull(paramSize) || IsNull(rsp) || IsNull(blob_offset))
    {
        ACDB_ERR("Error[%d]: One or more input parameters is null",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    if (caldata->size <= 0 || caldata->payload == NULL)
    {
        ACDB_ERR("Error[%d]: The input calibration data is null or empty",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    //Parse AcdbOffloadedParamHeader from parameter payload
    ACDB_MEM_CPY_SAFE(&header, sizeof(header), caldata->payload, sizeof(header));
    offset += sizeof(header);

    //Parse payload: Should contain [path length, path]
    path_length = *(uint32_t*)(caldata->payload + offset);
    if (path_length > 0)
    {
        path = (char_t*)caldata->payload + offset + sizeof(uint32_t);

        //Check for relative path or full path
        if(path_length > 2 && !(
            (path[0] == '/') ||
            (path[1] == ':' && path[2] == '\\')
            ))
        {
            /* Relative Path: Merge acdb directory path with offloaded file name
            * Ex. "../path/to/acdb/" + "offloaded_file.bin"
            */
            char_t *tmp_path = NULL;
            AcdbFileManPathInfo *path_info = NULL;
            uint32_t tmp_path_length = 0;
            is_relative_path = TRUE;

            status = AcdbFileManGetAcdbDirectoryPath(&path_info);
            if (AR_FAILED(status))
            {
                ACDB_ERR("Error[%d]: Unable to get acdb directory path to"
                    " merge with relative path %s", status, path);
                return status;
            }

            tmp_path_length = path_length + path_info->path_len;
            tmp_path = ACDB_MALLOC(char_t, tmp_path_length);

            if (IsNull(tmp_path))
            {
                return AR_ENOMEMORY;
            }

            ACDB_STR_CPY_SAFE(tmp_path, tmp_path_length,
                path_info->path, path_info->path_len);
            ACDB_STR_CPY_SAFE(tmp_path + path_info->path_len,
                path_length, path, path_length);

            path = tmp_path;
            path_length = tmp_path_length;
            tmp_path = NULL;
        }
    }

    if (path != NULL && path[0] != '\0' && path_length > 0)
    {
        status = FileManReadFileToBuffer(
            path, &data_size, NULL, rsp->buf_size);
        if (AR_FAILED(status))
        {
            status = AR_EOK;
            ACDB_ERR("Error[%d]: Unable to read file size for"
                " offloaded parameter. Does the file exist?"
                " The data section will be zeroed out", AR_ENOTEXIST);
            data_size = padding = 0;
        }

        //perform alignment before writing data
        if ((*blob_offset + offset) % header.alignment && data_size > 0)
        {
            //padding here is for offloaded data front-padding
            padding = header.alignment
                - (*blob_offset + offset) % header.alignment;
            header.data_offset = padding;

            if (!IsNull(rsp->buf) && rsp->buf_size > 0)
            {
                if (offset > rsp->buf_size)
                {
                    status = AR_ENEEDMORE;
                    ACDB_ERR("Error[%d]: Buffer is not large enough "
                        "to set offloaded parameter padding.\n", status);
                    goto end;
                }
                ar_mem_set((void*)((uint8_t*)rsp->buf + offset + *blob_offset),
                    0, padding);
            }
        }
    }
    else
    {
        status = AR_EOK;
        if(IsNull(rsp->buf))
            ACDB_DBG(
                "Warning[%d]: No file data to load for offloaded parameter."
                "Does the file exist? The data section will be zeroed out."
                , status);
        data_size = padding = 0;
    }

    //Update Parameter Size
    *paramSize = (uint32_t)sizeof(header) + padding + (uint32_t)data_size;
    header.data_size = (uint32_t)data_size;

    //8 byte aligned padding
    padding = ACDB_PADDING_8_BYTE(*paramSize);
    *paramSize = ACDB_ALIGN_8_BYTE(*paramSize);

    if (IsNull(rsp->buf) || rsp->buf_size == 0)
        goto end;

    //Write Data
    if (sizeof(header) > rsp->buf_size)
    {
        status = AR_ENEEDMORE;
        ACDB_ERR("Error[%d]: Buffer is not large enough "
            "to copy offloaded parameter header.\n", status);
        goto end;
    }

    offset = 0;
    //Write Header
    ACDB_MEM_CPY_SAFE(rsp->buf + offset + *blob_offset, sizeof(header),
        &header, sizeof(header));
    offset += sizeof(header);
    *blob_offset += sizeof(header);

    if ((uint32_t)data_size > rsp->buf_size - offset)
    {
        status = AR_ENEEDMORE;
        ACDB_ERR("Error[%d]: Buffer is not large enough "
            "to copy offloaded parameter data.\n", status);
        goto end;
    }

    if (path != NULL && path[0] != '\0' && data_size > 0)
    {
        status = FileManReadFileToBuffer(path, &data_size,
            (uint8_t*)rsp->buf + *blob_offset + header.data_offset,
            rsp->buf_size);
        if ((uint32_t)data_size > 0UL && AR_SUCCEEDED(status))
        {
            *blob_offset += header.data_offset + (uint32_t)data_size;
        }
    }
    else
    {
        *blob_offset += header.data_offset + (uint32_t)data_size;
    }

    //Write 8 byte aligned padding
    if (padding)
    {
        ar_mem_set((void*)((uint8_t*)rsp->buf + *blob_offset), 0, padding);
        *blob_offset += padding;
    }

end:
    if (is_relative_path)
    {
        if (path) ACDB_FREE(path);
    }

    return status;
}

int32_t DataProcGetHwAccelSubgraphInfo(AcdbHwAccelSubgraphInfo* info)
{
    int32_t status = AR_EOK;
    uint32_t offset = 0;
    uint32_t num_subgraphs = 0;
    ChunkInfo ci_hasi = { 0 };
    AcdbHwAccelSubgraphInfo *subgraph_info = NULL;

    ci_hasi.chunk_id = ACDB_CHUNKID_HW_ACCEL_SUBGRAPH_INFO;
    status = ACDB_GET_CHUNK_INFO(&ci_hasi);
    if (AR_FAILED(status))
    {
        return status;
    }

    //Todo: make this a linear search for now. add a search function that
    //determines how to search based on the number of elements

    offset = ci_hasi.chunk_offset;

    status = FileManReadBuffer(&num_subgraphs, sizeof(uint32_t), &offset);
    if (AR_FAILED(status))
    {
        ACDB_DBG("Error[%d]: Unable to read HW Accel "
            "Subgraph Info subgraph count", status);
        return status;
    }

    status = FileManGetFilePointer2((void**)&subgraph_info, offset);
    if (AR_FAILED(status))
    {
        ACDB_DBG("Error[%d]: Unable to get HW Accel Subgraph Info",
            status);
        return status;
    }

    for (uint32_t i = 0; i < num_subgraphs; i++)
    {
        if (subgraph_info[i].subgraph_id == info->subgraph_id)
        {
            info->subgraph_id = subgraph_info[i].subgraph_id;
            info->mem_type = subgraph_info[i].mem_type;
            info->module_list_offset = subgraph_info[i].module_list_offset;
            return AR_EOK;
        }
    }

    return AR_ENOTEXIST;
}

int32_t DataProcIsHwAccelParam(uint32_t module_list_offset,
    uint32_t module_iid, uint32_t param_id)
{
    int32_t status = AR_EOK;
    uint32_t num_params = AR_EOK;
    uint32_t offset = AR_EOK;
    AcdbMiidPidPair *module_param_list = NULL;
    ChunkInfo ci_data_pool = { 0 };

    ci_data_pool.chunk_id = ACDB_CHUNKID_DATAPOOL;
    status = ACDB_GET_CHUNK_INFO(&ci_data_pool);
    if (AR_FAILED(status))
    {
        return status;
    }

    /* Skip 4 byte data size */
    offset = ci_data_pool.chunk_offset
        + module_list_offset + sizeof(uint32_t);

    status = FileManReadBuffer(&num_params, sizeof(uint32_t), &offset);
    if (AR_FAILED(status))
    {
        ACDB_DBG("Error[%d]: Unable to read HW Accel "
            "Subgraph Info subgraph count", status);
        return status;
    }

    status = FileManGetFilePointer2((void**)&module_param_list, offset);
    if (AR_FAILED(status))
    {
        ACDB_DBG("Error[%d]: Unable to get HW Accel Subgraph Info",
            status);
        return status;
    }

    for (uint32_t i = 0; i < num_params; i++)
    {
        if (module_iid == module_param_list[i].module_iid &&
            param_id == module_param_list[i].parameter_id)
            return AR_EOK;
    }

    return AR_ENOTEXIST;
}
