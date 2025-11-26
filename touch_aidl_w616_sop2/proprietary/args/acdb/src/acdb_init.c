/**
*=============================================================================
* \file acdb_init.c
*
* \brief
*		Handles the initialization and de-initialization of ACDB SW.
*
* \copyright
*  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/
#include "acdb_init.h"
#include "acdb_init_utility.h"
#include "acdb_parser.h"
#include "acdb_delta_parser.h"
#include "acdb_common.h"

/* ---------------------------------------------------------------------------
* Global Data Definitions
*--------------------------------------------------------------------------- */

const char* WKSP_FILE_NAME_EXT = ".qwsp";
const char* ACDB_FILE_NAME_EXT = ".acdb";

int32_t AcdbInitLoadAcdbFile(AcdbCmdFileInfo *acdb_cmd_finfo, AcdbFileInfo *acdb_finfo)
{
	int32_t result = ACDB_INIT_SUCCESS;
	ar_fhandle *fhandle = &acdb_cmd_finfo->file_handle;

	if (ACDB_FIND_STR(acdb_cmd_finfo->chFileName, WKSP_FILE_NAME_EXT) != NULL)
	{
        /* The workspace file is only opened when ARCT(or other ACDB client)
         * connects and querries for file information
         */
        acdb_cmd_finfo->file_type = ACDB_FILE_TYPE_WORKSPACE;
        acdb_finfo->file_type = ACDB_FILE_TYPE_WORKSPACE;
	}
	else if (ACDB_FIND_STR(acdb_cmd_finfo->chFileName, ACDB_FILE_NAME_EXT) != NULL)
	{
        acdb_cmd_finfo->file_type = ACDB_FILE_TYPE_DATABASE;

        result = AcdbInitUtilGetFileData(
            acdb_cmd_finfo->chFileName,
            fhandle,
            &acdb_cmd_finfo->file_size,
            &acdb_cmd_finfo->file_buffer
        );

        if (result != ACDB_PARSE_SUCCESS || fhandle == NULL)
            return ACDB_INIT_FAILURE;

		if (ACDB_PARSE_SUCCESS != IsAcdbFileValid(*fhandle,
            acdb_cmd_finfo->file_size))
			return ACDB_INIT_FAILURE;

		acdb_finfo->file_type = ACDB_FILE_TYPE_DATABASE;
		acdb_finfo->major = 1;
		acdb_finfo->minor = 0;
		acdb_finfo->revision = 0;
		acdb_finfo->cplInfo = 0;

        AcdbLogFileVersion(
            acdb_finfo->major,
            acdb_finfo->minor,
            acdb_finfo->revision,
            acdb_finfo->cplInfo);
	}
    else
    {
        //Provided file is not an acdb or workspace file
        ACDB_ERR("%s is not a *.qwsp or *.acdb file.",
            acdb_cmd_finfo->chFileName);
        return ACDB_INIT_FAILURE;
    }

	fhandle = NULL;
	return result;
}

int32_t AcdbInitLoadDeltaFile(AcdbCmdFileInfo *acdb_cmd_finfo,
    AcdbFile *delta_fpath, AcdbCmdDeltaFileInfo *delta_finfo)
{
    int32_t status = AR_EOK;
    AcdbInitUtilDeltaInfo delta_info = { 0 };
    ar_fhandle *fhandle = &delta_finfo->file_handle;

    delta_finfo->delta_file_path.fileNameLen = (uint32_t)ar_strlen(delta_fpath->fileName,
        sizeof(delta_fpath->fileName));

    status = ACDB_STR_CPY_SAFE(
        delta_finfo->delta_file_path.fileName,
        MAX_FILENAME_LENGTH,
        delta_fpath->fileName,
        delta_fpath->fileNameLen);
    if (AR_FAILED(status))
    {
        fhandle = NULL;
        ACDB_ERR("Error[%d]: Failed to copy delta file path string", status);
        return status;
    }

    delta_info.file_access_io_flag = ACDB_FUNC_IO_OUT;
    status = AcdbInitUtilGetDeltaInfo(
        &acdb_cmd_finfo->chFileName[0],
        acdb_cmd_finfo->filename_len,
        delta_fpath, &delta_info);
    if (AR_FAILED(status))
    {
        fhandle = NULL;
        delta_finfo->exists = FALSE;
        goto end;
    }

    status = AcdbInitUtilOpenDeltaFile(&delta_info, fhandle);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to open the delta file %s",
            status, delta_info.path);
        goto end;
    }

    /* Validate the delta file only if it exists. Otherwise bypass
     * validation. */
    if (delta_info.file_size > 0 &&
        delta_info.file_access != AR_FOPEN_WRITE_ONLY)
    {
        if (ACDB_PARSE_SUCCESS != IsAcdbDeltaFileValid(
            *fhandle, delta_info.file_size))
        {
            status = AR_EFAILED;
            goto end;
        }

        status = AcdbDeltaParserGetSwVersion(*fhandle, delta_info.file_size,
            &delta_finfo->file_info);
        if (AR_FAILED(status))
        {
            goto end;
        }

        AcdbLogDeltaFileVersion(
            delta_finfo->file_info.major,
            delta_finfo->file_info.minor,
            delta_finfo->file_info.revision,
            delta_finfo->file_info.cplInfo
        );
    }

    delta_finfo->exists = TRUE;
    delta_finfo->file_size = delta_info.file_size;
    delta_finfo->acdb_file_index = acdb_cmd_finfo->file_index;
    
end:
    ACDB_FREE(delta_info.path);

    return status;
}

bool_t AcdbInitDoSwVersionsMatch(AcdbFileInfo *acdb_finfo, AcdbDeltaFileInfo *acdb_delta_finfo)
{
	if ((acdb_finfo->major == acdb_delta_finfo->major) &&
		(acdb_finfo->minor == acdb_delta_finfo->minor) &&
		(acdb_finfo->revision == acdb_delta_finfo->revision) &&
		(acdb_finfo->cplInfo  == INF || acdb_finfo->cplInfo == acdb_delta_finfo->cplInfo))
	{
		return TRUE;
	}
	return FALSE;
}
