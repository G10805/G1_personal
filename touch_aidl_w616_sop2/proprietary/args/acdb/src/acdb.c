/**
*=============================================================================
* \file acdb.c
*
* \brief
*      Contains the implementation of the public interface to the Audio
*      Calibration Database (ACDB) module.
*
* \copyright
*  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/

#include "acdb.h"
#include "acdb_file_mgr.h"
#include "acdb_delta_file_mgr.h"
#include "acdb_init.h"
#include "acdb_init_utility.h"
#include "acdb_command.h"
#include "acdb_common.h"
#include "acdb_utility.h"
#include "ar_osal_error.h"
#include "ar_osal_file_io.h"
#include "ar_osal_mutex.h"


/* ---------------------------------------------------------------------------
* Global Data Definitions
*--------------------------------------------------------------------------- */

/**< The lock for incoming ACDB client commands.(e.g ATS online
commands from QACT and acdb_ioctl commands from GSL */
extern ar_osal_mutex_t acdb_client_lock;
ar_osal_mutex_t acdb_client_lock;

int32_t acdb_cmd_set_temp_path(AcdbSetTempPathReq *req);

/* ----------------------------------------------------------------------------
* Public Function Definitions
*--------------------------------------------------------------------------- */

int32_t acdb_init(AcdbDataFiles *acdb_data_files, AcdbFile* delta_file_path)
{
	int32_t status = AR_EOK;
	uint32_t i = 0;
    uint32_t remaining_file_slots = 0;
    uint32_t num_data_files = 0;
	bool_t should_free_delta_file = FALSE;
	AcdbFileInfo acdb_finfo;
	AcdbCmdFileInfo *p_acdb_cmd_finfo = NULL;
	AcdbCmdDeltaFileInfo *p_acdb_cmd_delta_finfo = NULL;

	ACDB_PKT_LOG_INIT();

    status = AcdbARHeapInit();
    if (AR_FAILED(status))
    {
        return status;
    }

    AcdbLogSwVersion(
        ACDB_SOFTWARE_VERSION_MAJOR,
        ACDB_SOFTWARE_VERSION_MINOR,
        ACDB_SOFTWARE_VERSION_REVISION,
        ACDB_SOFTWARE_VERSION_CPLINFO);

	if (IsNull(acdb_data_files) || acdb_data_files->num_files <= 0 ||
        acdb_data_files->num_files > MAX_ACDB_FILE_COUNT)
	{
		ACDB_ERR_MSG_1("Files do not exist or the file count is more than 20.", AR_EBADPARAM);
		return AR_EBADPARAM;
	}

	p_acdb_cmd_finfo = ACDB_MALLOC(AcdbCmdFileInfo, acdb_data_files->num_files);
	if (IsNull(p_acdb_cmd_finfo))
	{
		ACDB_ERR_MSG_1("Insufficient memory for acdb file allocation.", AR_ENOMEMORY);
		return AR_ENOMEMORY;
	}

	p_acdb_cmd_delta_finfo = ACDB_MALLOC(AcdbCmdDeltaFileInfo, acdb_data_files->num_files);
	if (IsNull(p_acdb_cmd_delta_finfo))
	{
		ACDB_ERR_MSG_1("Insufficient memory for delta acdb file allocation.", AR_ENOMEMORY);
		status =  AR_ENOMEMORY;
		goto end;
	}

	for (i = 0; i < acdb_data_files->num_files; i++)
	{
		ACDB_CLEAR_BUFFER(p_acdb_cmd_finfo[i]);
		ACDB_CLEAR_BUFFER(p_acdb_cmd_delta_finfo[i]);

		//Copy filename to AcdbCmdFileInfo
        (p_acdb_cmd_finfo + i)->filename_len =
            acdb_data_files->acdbFiles[i].fileNameLen;
        (p_acdb_cmd_finfo + i)->file_index = i;

		status = ACDB_STR_CPY_SAFE(
			(p_acdb_cmd_finfo + i)->chFileName,
			MAX_FILENAME_LENGTH,
			acdb_data_files->acdbFiles[i].fileName,
			acdb_data_files->acdbFiles[i].fileNameLen);

		if (AR_EOK != status)
		{
			ACDB_ERR_MSG_1("Copying filename failed.", status);
			goto end;
		}

		status = AcdbInitLoadAcdbFile(&p_acdb_cmd_finfo[i], &acdb_finfo);
		if(AR_FAILED(status))
		{
			// If even one file is not valid then acdb_init should return failure.
			// So we should ACDB_FREE the memory allocated for any of the files.

			uint32_t idx = 0;
			for (idx = 0; idx < i + 1; idx++)
			{
				status = ar_fclose(p_acdb_cmd_finfo[idx].file_handle);
				if (AR_EOK != status)
				{
					ACDB_ERR_MSG_1("Could not close the acdb file.", status);
					status =  AR_EFAILED;
					goto end;
				}

				p_acdb_cmd_finfo[idx].file_handle = NULL;

				if (NULL != p_acdb_cmd_delta_finfo[idx].file_handle)
				{
					status = ar_fclose(p_acdb_cmd_delta_finfo[idx].file_handle);
					if (AR_EOK != status)
					{
						ACDB_ERR_MSG_1("Could not close the acdb delta file.", status);
						status = AR_EFAILED;
						goto end;
					}

					p_acdb_cmd_delta_finfo[idx].file_handle = NULL;
				}
			}

			ACDB_ERR_MSG_1("Acdb initialization failed.", AR_EFAILED);
			status = AR_EFAILED;
			goto end;
		}

        if (p_acdb_cmd_finfo[i].file_type == ACDB_FILE_TYPE_WORKSPACE) continue;

        num_data_files++;

		/* Load delta file */
        if (!IsNull(delta_file_path))
        {
            status = AcdbInitLoadDeltaFile(&p_acdb_cmd_finfo[i],
                delta_file_path, &p_acdb_cmd_delta_finfo[i]);

            should_free_delta_file = ACDB_INIT_SUCCESS != status ? TRUE : FALSE;

            if ((p_acdb_cmd_delta_finfo + i)->file_size == 0)
            {
                /* Delta File is empty, so copy the set acdb version info
                 * to delta files acdb version info */
                ACDB_MEM_CPY_SAFE(&(p_acdb_cmd_delta_finfo + i)->file_info,
					sizeof(acdb_finfo),
                    &acdb_finfo, sizeof(acdb_finfo));
            }

            if (TRUE == (p_acdb_cmd_delta_finfo + i)->exists &&
                FALSE == AcdbInitDoSwVersionsMatch(
                    &acdb_finfo, &(p_acdb_cmd_delta_finfo + i)->file_info))
            {
                should_free_delta_file = TRUE;
            }

            if (should_free_delta_file == TRUE)
            {
                // ignore loading this delta acdb file
                if ((p_acdb_cmd_delta_finfo + i)->file_handle != NULL)
                {
                    status = ar_fclose((p_acdb_cmd_delta_finfo + i)->file_handle);
                    if (AR_EOK != status)
                    {
                        ACDB_ERR_MSG_1("Could not close the acdb delta file.", status);
						status = AR_EFAILED;
						goto end;
                    }

                    (p_acdb_cmd_delta_finfo + i)->file_handle = NULL;
                }

                (p_acdb_cmd_delta_finfo + i)->exists = FALSE;
                (p_acdb_cmd_delta_finfo + i)->file_size = 0;
                (p_acdb_cmd_delta_finfo + i)->is_updated = FALSE;
            }
        }
	}

    if (num_data_files == 0)
    {
        ACDB_ERR("Error[%d]: No *.acdb files were found.", AR_ENORESOURCE);
		status = AR_ENORESOURCE;
		goto end;
    }

    if (!IsNull(delta_file_path))
    {
        /* The delta file path serves two purposes:
        *  1. to store the delta files
        *  2. to store temporary files generated by ATS clients (e.g ARCT)
        * The delta file path will be set as the temporary file path managed
        * by the File Manager
        */
        status = acdb_file_man_ioctl(ACDB_FILE_MAN_SET_TEMP_PATH_INFO,
            (uint8_t *)delta_file_path, sizeof(AcdbFileManPathInfo), NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Unable to set temporary file path using "
                "the delta file path. Skipping..");
            status = AR_EOK;
        }
    }
    else
    {
        ACDB_ERR("Error[%d]: Unable to set temporary file path using "
            "the delta file path. No path was provided. Skipping..");
    }

	// Now that all the acbd files requested for initialization
	// are valid, check if acdb sw can host this many files first
	// and then update the file manger with each files information
	status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_AVAILABLE_FILE_SLOTS,
        NULL, 0, (uint8_t *)&remaining_file_slots, sizeof(uint32_t));

	if (AR_EOK != status || remaining_file_slots < acdb_data_files->num_files)
	{
		for (uint32_t idx = 0; idx < acdb_data_files->num_files; idx++)
		{
			int ret = ar_fclose((p_acdb_cmd_finfo + idx)->file_handle);
			if (ret != 0)
			{
				ACDB_ERR_MSG_1("Could not close the acdb file.", ret);
				status = ret;
				goto end;
			}
			(p_acdb_cmd_finfo + idx)->file_handle = NULL;
			if ((p_acdb_cmd_delta_finfo + idx)->file_handle != NULL)
			{
				ret = ar_fclose((p_acdb_cmd_delta_finfo + idx)->file_handle);
				if (ret != 0)
				{
					ACDB_ERR_MSG_1("Could not close the acdb delta file.", ret);
					status = ret;
					goto end;
				}
				(p_acdb_cmd_delta_finfo + idx)->file_handle = NULL;
			}
		}

		ACDB_ERR_MSG_1("Could not count the number of avalible file slots.", status);
		status = AR_EFAILED;
		goto end;
	}

	for (i = 0; i < acdb_data_files->num_files; i++)
	{
		status = acdb_file_man_ioctl(ACDB_FILE_MAN_SET_FILE_INFO,
            (uint8_t *)&p_acdb_cmd_finfo[i], sizeof(AcdbCmdFileInfo), NULL, 0);
		if (AR_EOK != status)
		{
			// This failure should never occur,
			// if so ACDB_FREE up the rest of the acdb files memory which was created for safety
			while (i < acdb_data_files->num_files)
			{
				int ret = ar_fclose((p_acdb_cmd_finfo + i)->file_handle);
				if (ret != 0)
				{
					ACDB_ERR_MSG_1("Could not close the acdb file.", ret);
					status = ret;
					goto end;
				}
				(p_acdb_cmd_finfo + i)->file_handle = NULL;
				if ((p_acdb_cmd_delta_finfo + i)->file_handle != NULL)
				{
					ret = ar_fclose((p_acdb_cmd_delta_finfo + i)->file_handle);
					if (ret != 0)
					{
						ACDB_ERR_MSG_1("Could not close the acdb delta file.", ret);
						status = ret;
						goto end;
					}
					(p_acdb_cmd_delta_finfo + i)->file_handle = NULL;
				}
				i++;
			}

			ACDB_ERR_MSG_1("Could not set acdb data.", status);
			status = AR_EFAILED;
			goto end;
		}
	}

    /* Initialzed the delta file manager and load the contents of the delta
     * file into the heap */

    if (!IsNull(delta_file_path) && !should_free_delta_file)
    {
		uint32_t delta_status = 0;
		uint32_t findex = 0;
        for (i = 0; i < acdb_data_files->num_files; i++)
        {
			if ((p_acdb_cmd_finfo + i)->file_type == ACDB_FILE_TYPE_WORKSPACE)
				continue;

            (p_acdb_cmd_delta_finfo + i)->file_index = findex;
            AcdbCmdDeltaFileInfo *p_delta_finfo = (p_acdb_cmd_delta_finfo + i);
            delta_status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_INIT,
                (uint8_t *)p_delta_finfo, sizeof(AcdbCmdDeltaFileInfo),
                NULL, 0);
            if (AR_EOK != delta_status)
            {
                // remove from memory.
                if (NULL != (p_acdb_cmd_delta_finfo + i)->file_handle)
                {
                    int ret = ar_fclose(
                        (p_acdb_cmd_delta_finfo + i)->file_handle);
                    if (ret != 0)
                    {
                        ACDB_ERR_MSG_1("Could not close the acdb delta file.",
                            ret);
						status = ret;
						goto end;
                    }
                    (p_acdb_cmd_delta_finfo + i)->file_handle = NULL;
                }
                ACDB_ERR_MSG_1("Failed to initialize delta acdb files.",
                    delta_status);
				status = delta_status;
				goto end;
            }

            delta_status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_INIT_HEAP,
                (uint8_t*)&findex, sizeof(uint32_t), NULL, 0);
            if (AR_EOK != delta_status)
            {
                ACDB_ERR_MSG_1("Heap initialization failed.", delta_status);
                status = delta_status;
            }
			findex++;
        }
    }

	if (!acdb_client_lock)
	{
		status = ar_osal_mutex_create(&acdb_client_lock);
		if (status)
		{
			ACDB_ERR("failed to create mutex: %d", status);
			goto end;
		}
	}

end:

	ACDB_FREE(p_acdb_cmd_finfo);
	ACDB_FREE(p_acdb_cmd_delta_finfo);
	return status;
}

int32_t acdbCmdIsPersistenceSupported(__UNUSED uint32_t *resp)
{
    __UNREFERENCED_PARAM(resp);
    /* This function should not be used. Use the acdb_iotcl with
     * the ACDB_CMD_ENABLE_PERSISTENCE command*/
	return AR_ENOTIMPL;
}

int32_t acdb_flash_init(uint32_t* acdbdata_base_addr)
{
	*acdbdata_base_addr = 0x0;
	int32_t status = AR_EOK;
	return status;
}

int32_t acdb_deinit()
{
	int32_t status = AR_EOK;

	status = acdb_delta_data_ioctl(
        ACDB_DELTA_DATA_CMD_RESET, NULL, 0, NULL, 0);


    status = acdb_file_man_ioctl(
        ACDB_FILE_MAN_RESET, NULL, 0, NULL, 0);
	if (acdb_client_lock)
	{
		int32_t mutex_status = ar_osal_mutex_destroy(acdb_client_lock);
		if (mutex_status)
		{
			ACDB_ERR("failed to destroy mutex: %d", status);
			return mutex_status;
		}
		acdb_client_lock = NULL;
	}

	ACDB_PKT_LOG_DEINIT();

    status = AcdbARHeapDeinit();
    if (AR_FAILED(status))
    {
        return status;
    }

	return status;
}

int32_t acdb_ioctl(uint32_t cmd_id,
	const void *cmd_struct,
	uint32_t cmd_struct_size,
	void *rsp_struct,
	uint32_t rsp_struct_size)
{
	int32_t status = AR_EOK;

	ACDB_PKT_LOG_DATA("ACDB_IOCTL_CMD_ID", &cmd_id, sizeof(cmd_id));

	ACDB_MUTEX_LOCK(acdb_client_lock);

	switch (cmd_id) {
	case ACDB_CMD_GET_GRAPH:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGraphKeyVector) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbGraphKeyVector *req = (AcdbGraphKeyVector *)cmd_struct;
			AcdbGetGraphRsp *rsp = (AcdbGetGraphRsp *)rsp_struct;

			if (req->num_keys == 0 || req->num_keys >= ACDB_MAX_KEY_COUNT)
			{
				status = AR_EBADPARAM;
			}
			else
				status = AcdbCmdGetGraph(req, rsp, rsp_struct_size);
		}
		break;
	case ACDB_CMD_GET_SUBGRAPH_DATA:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSgIdGraphKeyVector) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbSgIdGraphKeyVector *req = (AcdbSgIdGraphKeyVector*)cmd_struct;
			AcdbGetSubgraphDataRsp *rsp = (AcdbGetSubgraphDataRsp*)rsp_struct;
			if (req->sg_ids == NULL ||
				req->graph_key_vector.graph_key_vector == NULL ||
				req->graph_key_vector.num_keys >= ACDB_MAX_KEY_COUNT)
			{
				status = AR_EBADPARAM;
			}
			else
			{
				status = AcdbCmdGetSubgraphData(req, rsp, rsp_struct_size);
			}
		}
		break;
	case ACDB_CMD_GET_SUBGRAPH_CONNECTIONS:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSubGraphList) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbSubGraphList *req = (AcdbSubGraphList*)cmd_struct;
			AcdbBlob *rsp = (AcdbBlob*)rsp_struct;
			if ((req->num_subgraphs == 0) || (req->subgraphs == NULL))
			{
				status = AR_EBADPARAM;
			}
			else
			{
				status = AcdbCmdGetSubgraphConn(req, rsp, rsp_struct_size);
			}
		}
		break;
	case ACDB_CMD_GET_SUBGRAPH_CALIBRATION_DATA_NONPERSIST:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSgIdCalKeyVector) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbSgIdCalKeyVector *req = (AcdbSgIdCalKeyVector*)cmd_struct;
			AcdbBlob *rsp = (AcdbBlob*)rsp_struct;
			if ((req->num_sg_ids == 0) || (req->sg_ids == NULL))
			{
				status = AR_EBADPARAM;
			}
			else
			{
				status = AcdbCmdGetSubgraphCalDataNonPersist(req, rsp, rsp_struct_size);
			}
		}
		break;
	case ACDB_CMD_GET_SUBGRAPH_CALIBRATION_DATA_PERSIST:
		if (cmd_struct == NULL ||
			cmd_struct_size != sizeof(AcdbSgIdCalKeyVector) ||
			rsp_struct == NULL ||
			rsp_struct_size != sizeof(AcdbSgIdPersistCalData))
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbSgIdCalKeyVector *req = (AcdbSgIdCalKeyVector*)cmd_struct;
			AcdbSgIdPersistCalData *rsp = (AcdbSgIdPersistCalData*)rsp_struct;
			if ((req->num_sg_ids == 0) || (req->sg_ids == NULL))
			{
				status = AR_EBADPARAM;
			}
			else
			{
				status = AcdbCmdGetSubgraphCalDataPersist(req, rsp);
			}
		}
		break;
	case ACDB_CMD_GET_SUBGRAPH_GLB_PSIST_IDENTIFIERS:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSgIdCalKeyVector) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			//todo: Commenting out to suppress warnings. Uncomment when this api is supported
			// AcdbSgIdCalKeyVector *req = (AcdbSgIdCalKeyVector*)cmd_struct;
			// AcdbGlbPsistIdentifierList  *rsp = (AcdbGlbPsistIdentifierList*)rsp_struct;
			// if ((req->num_sg_ids == 0) || (req->sg_ids == NULL))
			// {
			// 	status = AR_EBADPARAM;
			// }
			// else
			// {
			// 	status = AcdbCmdGetSubgraphGlbPersistIds(req, rsp, rsp_struct_size);
			// }

			status = AR_ENOTEXIST;

		}
		break;
	case ACDB_CMD_GET_SUBGRAPH_GLB_PSIST_CALDATA:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGlbPersistCalDataCmdType) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			//todo: Commenting out to suppress warnings. Uncomment when this api is supported
			// AcdbGlbPersistCalDataCmdType *req = (AcdbGlbPersistCalDataCmdType*)cmd_struct;
			// AcdbBlob  *rsp = (AcdbBlob*)rsp_struct;
			// if (req->cal_Id == 0)
			// {
			// 	status = AR_EBADPARAM;
			// }
			// else
			// {
			// 	status = AcdbCmdGetSubgraphGlbPersistCalData(req, rsp, rsp_struct_size);
			// }

			status = AR_ENOTEXIST;

		}
		break;
	case ACDB_CMD_GET_MODULE_TAG_DATA:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSgIdModuleTag) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbSgIdModuleTag *req = (AcdbSgIdModuleTag*)cmd_struct;
			AcdbBlob  *rsp = (AcdbBlob*)rsp_struct;
		    status = AcdbCmdGetModuleTagData(req, rsp, rsp_struct_size);

		}
		break;
	case ACDB_CMD_GET_TAGGED_MODULES:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGetTaggedModulesReq) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbGetTaggedModulesReq *req = (AcdbGetTaggedModulesReq*)cmd_struct;
			AcdbGetTaggedModulesRsp  *rsp = (AcdbGetTaggedModulesRsp*)rsp_struct;
		    status = AcdbCmdGetTaggedModules(req, rsp, rsp_struct_size);
		}
		break;
	case ACDB_CMD_GET_DRIVER_DATA:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbDriverData) ||
            rsp_struct == NULL || rsp_struct_size == 0)
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbDriverData *req = (AcdbDriverData*)cmd_struct;
            AcdbBlob  *rsp = (AcdbBlob*)rsp_struct;

            status = AcdbCmdGetDriverData(req, rsp, rsp_struct_size);
        }
		break;
	case ACDB_CMD_SET_CAL_DATA:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSetCalDataReq))
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbSetCalDataReq *req = (AcdbSetCalDataReq*)cmd_struct;
			status = AcdbCmdSetCalData(req);
		}
		break;
	case ACDB_CMD_ENABLE_PERSISTANCE:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(uint8_t))
		{
			status = AR_EBADPARAM;
		}
		else
		{
			uint8_t *req = (uint8_t*)cmd_struct;
            AcdbDeltaPersistState state = *req == 1 ?
                ACDB_DELTA_DATA_PERSIST_STATE_ENABLED :
                ACDB_DELTA_DATA_PERSIST_STATE_DISABLED;

            status = acdb_delta_data_ioctl(
                ACDB_DELTA_DATA_CMD_ENABLE_PERSISTENCE,
                (uint8_t*)&state, sizeof(AcdbDeltaPersistState), NULL, 0);
		}
		break;
	case ACDB_CMD_GET_AMDB_REGISTRATION_DATA:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbAmdbProcID) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbAmdbProcID *req = (AcdbAmdbProcID*)cmd_struct;
			AcdbBlob *rsp = (AcdbBlob*)rsp_struct;

			status = AcdbCmdGetAmdbRegData(req, rsp, rsp_struct_size);

		}
		break;
	case ACDB_CMD_GET_AMDB_DEREGISTRATION_DATA:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbAmdbProcID) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbAmdbProcID *req = (AcdbAmdbProcID*)cmd_struct;
			AcdbBlob *rsp = (AcdbBlob*)rsp_struct;

			status = AcdbCmdGetAmdbDeRegData(req, rsp, rsp_struct_size);

		}
		break;
	case ACDB_CMD_GET_SUBGRAPH_PROCIDS:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbCmdGetSubgraphProcIdsReq) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbCmdGetSubgraphProcIdsReq *req = (AcdbCmdGetSubgraphProcIdsReq*)cmd_struct;
			AcdbCmdGetSubgraphProcIdsRsp *rsp = (AcdbCmdGetSubgraphProcIdsRsp*)rsp_struct;
			if (req->num_sg_ids == 0)
			{
				status = AR_EBADPARAM;
			}
			else
			{
				status = AcdbCmdGetSubgraphProcIds(req, rsp, rsp_struct_size);
			}
		}
		break;
	case ACDB_CMD_GET_AMDB_BOOTUP_LOAD_MODULES:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbAmdbProcID) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbAmdbProcID *req = (AcdbAmdbProcID*)cmd_struct;
			AcdbBlob *rsp = (AcdbBlob*)rsp_struct;

			status = AcdbCmdGetAmdbBootupLoadModules(req, rsp, rsp_struct_size);

		}
		break;
	case ACDB_CMD_GET_TAGS_FROM_GKV:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbCmdGetTagsFromGkvReq) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbCmdGetTagsFromGkvReq *req = (AcdbCmdGetTagsFromGkvReq*)cmd_struct;
			AcdbCmdGetTagsFromGkvRsp *rsp = (AcdbCmdGetTagsFromGkvRsp*)rsp_struct;
			if (req->graph_key_vector->num_keys == 0 ||
				req->graph_key_vector->num_keys >= ACDB_MAX_KEY_COUNT)
			{
				status = AR_EBADPARAM;
			}
			else
			{
				status = AcdbCmdGetTagsFromGkv(req, rsp, rsp_struct_size);
			}
		}
		break;
    case ACDB_CMD_GET_GRAPH_CAL_KVS:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGraphKeyVector) ||
            rsp_struct == NULL || rsp_struct_size == 0)
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbGraphKeyVector *req = (AcdbGraphKeyVector*)cmd_struct;
            AcdbKeyVectorList *rsp = (AcdbKeyVectorList*)rsp_struct;
            if (req->num_keys == 0 || req->num_keys >= ACDB_MAX_KEY_COUNT)
            {
                status = AR_EBADPARAM;
            }
            else
            {
                status = AcdbCmdGetGraphCalKeyVectors(
                    req, rsp, rsp_struct_size);
            }
        }
        break;
    case ACDB_CMD_GET_SUPPORTED_GKVS:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbUintList) ||
            rsp_struct == NULL || rsp_struct_size == 0)
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbUintList *req = (AcdbUintList*)cmd_struct;
            AcdbKeyVectorList *rsp = (AcdbKeyVectorList*)rsp_struct;

            status = AcdbCmdGetSupportedGraphKeyVectors(
                req, rsp, rsp_struct_size);
        }
        break;
    case ACDB_CMD_GET_DRIVER_MODULE_KVS:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(uint32_t) ||
            rsp_struct == NULL || rsp_struct_size == 0)
        {
            status = AR_EBADPARAM;
        }
        else
        {
            uint32_t *req = (uint32_t*)cmd_struct;
            AcdbKeyVectorList *rsp = (AcdbKeyVectorList*)rsp_struct;

            status = AcdbCmdGetDriverModuleKeyVectors(
                *req, rsp, rsp_struct_size);
        }
        break;
    case ACDB_CMD_GET_GRAPH_TAG_KVS:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGraphKeyVector) ||
            rsp_struct == NULL || rsp_struct_size == 0)
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbGraphKeyVector *req = (AcdbGraphKeyVector*)cmd_struct;
            AcdbTagKeyVectorList *rsp = (AcdbTagKeyVectorList*)rsp_struct;
            if (req->num_keys == 0 || req->num_keys >= ACDB_MAX_KEY_COUNT)
            {
                status = AR_EBADPARAM;
            }
            else
            {
                status = AcdbCmdGetGraphTagKeyVectors(
                    req, rsp, rsp_struct_size);
            }
        }
        break;
    case ACDB_CMD_GET_CAL_DATA:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGetCalDataReq) ||
            rsp_struct == NULL || rsp_struct_size == 0)
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbGetCalDataReq *req = (AcdbGetCalDataReq*)cmd_struct;
            AcdbBlob *rsp = (AcdbBlob*)rsp_struct;
            status = AcdbCmdGetCalData(req, rsp);
        }
        break;
    case ACDB_CMD_GET_TAG_DATA:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGetTagDataReq) ||
            rsp_struct == NULL || rsp_struct_size == 0)
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbGetTagDataReq *req = (AcdbGetTagDataReq*)cmd_struct;
            AcdbBlob *rsp = (AcdbBlob*)rsp_struct;
            status = AcdbCmdGetTagData(req, rsp);
        }
        break;
    case ACDB_CMD_SET_TAG_DATA:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSetTagDataReq))
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbSetTagDataReq *req = (AcdbSetTagDataReq*)cmd_struct;
            status = AcdbCmdSetTagData(req);
        }
        break;
    case ACDB_CMD_SET_TEMP_PATH:
        if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbSetTempPathReq))
        {
            status = AR_EBADPARAM;
        }
        else
        {
            AcdbSetTempPathReq *req = (AcdbSetTempPathReq*)cmd_struct;
            status = acdb_cmd_set_temp_path(req);
        }
        break;
	case ACDB_CMD_GET_HW_ACCEL_SUBGRAPH_INFO:
		if (cmd_struct == NULL ||
			cmd_struct_size != sizeof(AcdbHwAccelSubgraphInfoReq) ||
			rsp_struct == NULL ||
			rsp_struct_size != sizeof(AcdbHwAccelSubgraphInfoRsp))
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbHwAccelSubgraphInfoReq* req =
				(AcdbHwAccelSubgraphInfoReq*)cmd_struct;
			AcdbHwAccelSubgraphInfoRsp* rsp =
				(AcdbHwAccelSubgraphInfoRsp*)rsp_struct;
			status = AcdbCmdGetHwAccelInfo(req, rsp);
		}
		break;
	case ACDB_CMD_GET_PROC_SUBGRAPH_CAL_DATA_PERSIST:
		if (cmd_struct == NULL  ||
			cmd_struct_size != sizeof(AcdbProcSubgraphPersistCalReq) ||
			rsp_struct == NULL ||
			rsp_struct_size != sizeof(AcdbSgIdPersistCalData))
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbProcSubgraphPersistCalReq* req =
				(AcdbProcSubgraphPersistCalReq*)cmd_struct;
			AcdbSgIdPersistCalData* rsp =
				(AcdbSgIdPersistCalData*)rsp_struct;
			if (req->num_subgraphs == 0)
			{
				status = AR_EBADPARAM;
			}
			else
			{
				status = AcdbCmdGetProcSubgraphCalDataPersist(req, rsp);
			}
		}
		break;
	case ACDB_CMD_GET_GRAPH_ALIAS:
		if (cmd_struct == NULL || cmd_struct_size != sizeof(AcdbGraphKeyVector) ||
			rsp_struct == NULL || rsp_struct_size == 0)
		{
			status = AR_EBADPARAM;
		}
		else
		{
			AcdbGraphKeyVector* req = (AcdbGraphKeyVector*)cmd_struct;
			AcdbString* rsp = (AcdbString*)rsp_struct;

			if (req->num_keys == 0 || req->num_keys >= ACDB_MAX_KEY_COUNT)
			{
				status = AR_EBADPARAM;
			}
			else
				status = AcdbCmdGetGraphAlias(req, rsp);
		}
		break;
	default:
		status = AR_ENOTEXIST;
		ACDB_ERR("Error[%d]: Received unsupported command request"
            " with Command ID[%08X]", cmd_id);
		break;
	}

	ACDB_MUTEX_UNLOCK(acdb_client_lock);

	return status;
}

/* ----------------------------------------------------------------------------
* Private Function Definitions
*--------------------------------------------------------------------------- */

int32_t acdb_cmd_set_temp_path(AcdbSetTempPathReq *req)
{
    int32_t status = AR_EOK;
    bool_t rw_delta_exists = FALSE;
    uint32_t num_files = 0;
    /**< read-only directory */
    AcdbFileManPathInfo ro_dir = { 0 };
    AcdbDeltaDataSwapInfo swap_finfo = { 0 };

    if (IsNull(req))
    {
        return AR_EBADPARAM;
    }

    if (req->path_length == 0)
    {
        ACDB_ERR("Error[%d]: The directory path length is zero ",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    if (req->path_length > MAX_FILENAME_LENGTH)
    {
        ACDB_ERR("Error[%d]: The directory path length %d "
            "is greater than the max path length %d",
            AR_EBADPARAM, req->path_length, MAX_FILENAME_LENGTH);
        return AR_EBADPARAM;
    }

    swap_finfo.file_access = AR_FOPEN_READ_ONLY_WRITE;
    swap_finfo.path_info.path_len = req->path_length;

    status = ar_strcpy(
        &swap_finfo.path_info.path[0], MAX_FILENAME_LENGTH,
        &req->path[0], MAX_FILENAME_LENGTH);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Failed to copy writeable path", status);
        return status;
    }

    /* Get read-only path */
    status = acdb_file_man_ioctl(ACDB_FILE_MAN_GET_TEMP_PATH_INFO,
        NULL, 0, (uint8_t*)&ro_dir, sizeof(AcdbFileManPathInfo));
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to get readonly path", status);
        return status;
    }

    /* Set the read/write path which will serve as the new temp directory */
    status = acdb_file_man_ioctl(ACDB_FILE_MAN_SET_TEMP_PATH_INFO,
        (uint8_t*)&swap_finfo.path_info, sizeof(AcdbFileManPathInfo), NULL, 0);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to set writable path", status);
        return status;
    }

    status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_GET_FILE_COUNT,
        NULL, 0, (uint8_t*)&num_files, sizeof(uint32_t));
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Failed to get total delta file count",
            status);
        return status;
    }

    for (uint32_t findex = 0; findex < num_files; findex++)
    {
        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_IS_FILE_AT_PATH,
            (uint8_t*)&swap_finfo, sizeof(AcdbDeltaDataSwapInfo),
            NULL, 0);
        if (AR_ENOTEXIST == status)
        {
            rw_delta_exists = FALSE;
        }
        else if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: An error occured while trying to verify"
                " the existance of the following path %s", status,
                &swap_finfo.path_info.path[0]);
            return status;
        }
        else
        {
            rw_delta_exists = TRUE;
        }

        if (rw_delta_exists)
        {
            //do not reload ro_delta since the rw_delta will already contains the latest changes
            //this will occur if the device is rebooted and the rw delta file still exists on target and this api is called again
            //this is the scenario where both the ro and rw files exist. the ro should be loaded first, then load the rw file
            /*
            The ro_delta is already loaded during init. Load the rw_delta*/

            swap_finfo.file_access = AR_FOPEN_READ_ONLY_WRITE;
            swap_finfo.file_index = findex;

            /* Swap the ro_delta for the rw_delta in the delta file manager */
            status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SWAP_DELTA,
                (uint8_t*)&swap_finfo, sizeof(AcdbDeltaDataSwapInfo), NULL, 0);
            if (AR_FAILED(status))
            {
                ACDB_ERR("Error[%d]: Failed to swap read-only "
                    "with read/write delta file", status);
                return status;
            }

            /* load the rw_delta onto the heap */
            status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_UPDATE_HEAP,
                (uint8_t*)&findex, sizeof(uint32_t), NULL, 0);
            if (AR_FAILED(status))
            {
                ACDB_ERR("Error[%d]: Failed to update heap with "
                    "data from the read/write delta", status);
                return status;
            }
            continue;
        }

        /* Scenario 1
        *   AML is already initialized.
        *   The ro_delta is already loaded onto the heap and its possible that other data has already been set to the heap by QACT or a GSL client through the Set Cal Data APIs
        *   The rw_delta does not exist
        * 1. Get the temp path that is set during acdb_init. This is the ro_delta path that we need to remember
        * 2. Update the delta file manager with the rw_delta directory as the new delta file path (this will replace the ro_delta file info in the delta file manager)
        * 3. save heap to the rw_delta file
        * 4. Update the delta file manager with the ro_delta file path to load the ro_delta file contents
        * 5. load the ro_delta to the heap
        * 6. Again, Update the delta file manager with the rw_delta directory as the new delta file path
        * 7. load the rw_delta on top
        *
        * After all these steps, the location of the ro_delta is forgoten.
        */

        /* 1. Get file info of the ro_delta file set during acdb_init */

        /* Update the delta file info by changing the file path to the rw_delta file
        * Create a new delta file
        */
        swap_finfo.file_access = AR_FOPEN_WRITE_ONLY;
        swap_finfo.file_index = findex;

        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SWAP_DELTA,
            (uint8_t*)&swap_finfo, sizeof(AcdbDeltaDataSwapInfo), NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to swap read-only "
                "with read/write delta file", status);
            return status;
        }

        /* Save the heap to the rw_delta file */
        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SAVE,
            NULL, 0, NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to save delta file", status);
        }

        /* Update the delta file manager with the ro_delta file path to load the ro_delta file contents */

        swap_finfo.file_access = AR_FOPEN_READ_ONLY;
        swap_finfo.file_index = findex;
        swap_finfo.path_info.path_len = ro_dir.path_len;
        status = ar_strcpy(
            swap_finfo.path_info.path, MAX_FILENAME_LENGTH,
            &ro_dir.path[0], MAX_FILENAME_LENGTH);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to copy read-only path string",
                status);
            return status;
        }

        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SWAP_DELTA,
            (uint8_t*)&swap_finfo, sizeof(AcdbDeltaDataSwapInfo), NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to swap read/write "
                "with read-only delta file", status);
            return status;
        }

        /* load the ro_delta to the heap */
        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_INIT_HEAP,
            (uint8_t*)&findex, sizeof(uint32_t), NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to save delta file", status);
        }

        /* Again, Update the delta file manager with the rw_delta directory as the new delta file path */
        swap_finfo.file_access = AR_FOPEN_READ_ONLY_WRITE;
        swap_finfo.file_index = findex;
        swap_finfo.path_info.path_len = req->path_length;
        status = ar_strcpy(
            &swap_finfo.path_info.path[0], MAX_FILENAME_LENGTH,
            &req->path[0], MAX_FILENAME_LENGTH);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to copy read-only path string",
                status);
            return status;
        }

        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_SWAP_DELTA,
            (uint8_t*)&swap_finfo, sizeof(AcdbDeltaDataSwapInfo), NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to swap read/write "
                "with read-only delta file", status);
            return status;
        }

        /* load the rw_delta on top */

        status = acdb_delta_data_ioctl(ACDB_DELTA_DATA_CMD_UPDATE_HEAP,
            (uint8_t*)&findex, sizeof(uint32_t), NULL, 0);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: Failed to update heap with delta data", status);
        }
    }

    return status;
}
