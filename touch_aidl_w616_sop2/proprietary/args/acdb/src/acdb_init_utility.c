/**
*=============================================================================
* \file acdb_init_utility.c
*
* \brief
*		Contains utility fucntions for ACDB SW initialiation. This inclues
*		initializaing the ACDB Data and ACDB Delta Data files.
*
* \copyright
*  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*=============================================================================
*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_init_utility.h"
#include "acdb_common.h"
#include "ar_osal_error.h"
#include "ar_osal_file_io.h"

void AcdbLogSwVersion(uint32_t major, uint32_t minor, uint32_t revision, uint32_t cpl)
{
	ACDB_PKT_LOG_DATA("ACDB_SW_MAJOR", &(major), sizeof(major));
	ACDB_PKT_LOG_DATA("ACDB_SW_MINOR", &(minor), sizeof(minor));
	ACDB_PKT_LOG_DATA("ACDB_SW_REVISION", &(revision), sizeof(revision));

	if (cpl != INF)
	{
		ACDB_PKT_LOG_DATA("ACDB_SW_CPL", &(cpl), sizeof(cpl));
        ACDB_VERBOSE("ACDB SW Version %u.%u.%u.%u", major, minor, revision, cpl);
	}
	else
	{
        ACDB_VERBOSE("ACDB SW Version %u.%u.%u", major, minor, revision);
	}
}

void AcdbLogDeltaSwVersion(uint32_t major, uint32_t minor, uint32_t revision, uint32_t cpl)
{
	ACDB_PKT_LOG_DATA("ACDB_DELTA_SW_MAJOR", &(major), sizeof(major));
	ACDB_PKT_LOG_DATA("ACDB_DELTA_SW_MINOR", &(minor), sizeof(minor));
	ACDB_PKT_LOG_DATA("ACDB_DELTA_SW_REVISION", &(revision), sizeof(revision));

	if (cpl != INF)
	{
		ACDB_PKT_LOG_DATA("ACDB_DELTA_SW_CPL", &(cpl), sizeof(cpl));
        ACDB_VERBOSE("ACDB Delta SW Version %u.%u.%u.%u", major, minor, revision, cpl);
	}
	else
	{
        ACDB_VERBOSE("ACDB Delta SW Version %u.%u.%u", major, minor, revision);
	}
}

void AcdbLogFileVersion(uint32_t major, uint32_t minor, uint32_t revision, uint32_t cpl)
{
	ACDB_PKT_LOG_DATA("ACDB_FILE_MAJOR", &(major), sizeof(major));
	ACDB_PKT_LOG_DATA("ACDB_FILE_MINOR", &(minor), sizeof(minor));
	ACDB_PKT_LOG_DATA("ACDB_FILE_REVISION", &(revision), sizeof(revision));

	if (cpl != INF)
	{
		ACDB_PKT_LOG_DATA("ACDB_FILE_CPL", &(cpl), sizeof(cpl));
        ACDB_VERBOSE("ACDB File Version %u.%u.%u.%u", major, minor, revision, cpl);
	}
	else
	{
        ACDB_VERBOSE("ACDB File Version %u.%u.%u", major, minor, revision);
	}
}

void AcdbLogDeltaFileVersion(uint32_t major, uint32_t minor, uint32_t revision, uint32_t cpl)
{
	ACDB_PKT_LOG_DATA("ACDB_DELTA_FILE_MAJOR", &(major), sizeof(major));
	ACDB_PKT_LOG_DATA("ACDB_DELTA_FILE_MINOR", &(minor), sizeof(minor));
	ACDB_PKT_LOG_DATA("ACDB_DELTA_FILE_REVISION", &(revision), sizeof(revision));

	if (cpl != INF)
	{
		ACDB_PKT_LOG_DATA("ACDB_DELTA_FILE_CPL", &(cpl), sizeof(cpl));
        ACDB_VERBOSE("ACDB Delta File Version %u.%u.%u.%u", major, minor, revision, cpl);
	}
	else
	{
        ACDB_VERBOSE("ACDB Delta File Version %u.%u.%u", major, minor, revision);
	}
}

int32_t AcdbInitUtilGetFileData(const char* fname, ar_fhandle* fhandle, uint32_t *fsize, void** fbuffer)
{
    int32_t status = AR_EOK;
    size_t bytes_read = 0;

    if (fname == NULL || fbuffer == NULL)
    {
        ACDB_ERR("Error[%d]: The file name provided is null", AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    status = ar_fopen(fhandle, fname, AR_FOPEN_READ_ONLY);
    if (AR_FAILED(status) || fhandle == NULL)
    {
        ACDB_ERR("Error[%d]: Unable to open file: %s", status, fname);
        return status;
    }

    status = ar_fseek(*fhandle, 0, AR_FSEEK_BEGIN);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to fseek file: %s", status, fname);
        return AR_EFAILED;
    }

    *fsize = (uint32_t)ar_fsize(*fhandle);
    if (*fsize != 0)
    {
        *fbuffer = (void*)ACDB_MALLOC(uint8_t, *fsize);
        if (IsNull(*fbuffer))
        {
            ACDB_ERR("Error[%d]: Not enough memory to "
                "allocate for file %s", AR_ENOMEMORY, fname);
            return AR_ENOMEMORY;
        }
    }
    else
    {
        ACDB_ERR("Error[%d]: The file %s is empty", status, fname);
        return AR_EFAILED;
    }

    status = ar_fread(*fhandle, *fbuffer, *fsize, &bytes_read);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to read file: %s", status, fname);
        goto end;
    }

    if (bytes_read != *fsize)
    {
        status = AR_EBADPARAM;
        ACDB_ERR("Error[%d]: File size does not match "
            "the number of bytes read: %s", status, fname);
    }

end:
    if (AR_FAILED(status))
    {
        *fsize = 0;
        ACDB_FREE(*fbuffer);
    }

    return status;
}

char *GetDeltaFileName(const char* fname, size_t fname_len,
    AcdbFile* delta_fpath, uint32_t *delta_fpath_length)
{
	char* delta_fname = NULL;
    char* delta_fname_ext = "delta";
    size_t ext_size = 6;
    size_t slash_size = 0;
	size_t delta_fname_size = 0;
	size_t remaining_size = 0;
    uint32_t delta_dir_length = 0;

	if (fname == NULL || fname_len == 0)
	{
		return NULL;
	}

    fname_len = ar_strlen(fname,
        MAX_FILENAME_LENGTH);

	if (delta_fpath == NULL)
	{
		//Delta file path is null, so use acdb file path and concatenate "delta"
		delta_fname_size = fname_len + ar_strlen(delta_fname_ext, ext_size) + 1;
		delta_fname = ACDB_MALLOC(char, delta_fname_size);

		if (IsNull(delta_fname)) return NULL;

		if (delta_fname != NULL)
		{
			ACDB_STR_CPY_SAFE(delta_fname, delta_fname_size, fname, fname_len);
			ACDB_STR_CAT_SAFE(delta_fname, delta_fname_size, delta_fname_ext,
                ar_strlen(delta_fname_ext, ext_size));
		}
	}
	else
	{
		//Delta file path is provided, so form the full path
		//Ex. ..\acdb_data + \ + acdb_data.acdb + delta
		uint32_t acdb_file_name_len = 0;
		uint32_t offset = 0;
		int32_t char_index = (int32_t)fname_len;
		char *slash = NULL;

		/* Detect windows\linux style path prefixes
		Absolute Paths
			Ex. C:\acdb_data\ar_acdb.acdb
			Ex. /acdb_data/ar_acdb.acdb
		Relative paths
			Ex. ..\acdb_data\ar_acdb.acdb
			Ex. ../acdb_data/ar_acdb.acdb
		*/
		//if (!((fname[0] == '.' && fname[1] == '.') ||
		//	(fname[1] == ':' && fname[2] == '\\') ||
		//	fname[0] == '/'))
		//{
		//	char_index = 0;
		//}

		while (--char_index > 0)
		{
			if (fname[char_index] == '\\' ||
				fname[char_index] == '/')
			{
                if (fname[char_index] == '/')
                {
                    slash = "/";
                    slash_size = 2;
                }

                if (fname[char_index] == '\\')
                {
                    slash = "\\";
                    slash_size = 3;
                }

				char_index++;
				break;
			}
		}

		acdb_file_name_len = (uint32_t)fname_len - char_index;
        delta_dir_length = (uint32_t)ar_strlen(delta_fpath->fileName,
            sizeof(delta_fpath->fileName));

		if (IsNull(slash))
		{
			delta_fname_size =
                delta_dir_length
				+ (size_t)acdb_file_name_len
				+ ar_strlen(delta_fname_ext, ext_size) + 1UL;
		}
		else
		{
			delta_fname_size =
                delta_dir_length
                + ar_strlen(slash, slash_size)
                + acdb_file_name_len
                + (ar_strlen(delta_fname_ext, ext_size) + 1);
		}

		delta_fname = ACDB_MALLOC(char, delta_fname_size);//account for \0

		if (IsNull(delta_fname)) return NULL;

		//delta file path: ..\acdb_data
		ACDB_STR_CPY_SAFE(delta_fname + offset, delta_fname_size,
            delta_fpath->fileName, delta_dir_length);
		offset += delta_dir_length;
		remaining_size = delta_fname_size - delta_dir_length;

		//slash
		if (!IsNull(slash))
		{
			ACDB_STR_CPY_SAFE(delta_fname + offset, remaining_size,
                slash, ar_strlen(slash, slash_size));
			offset += (uint32_t)ar_strlen(slash, slash_size);
			remaining_size = delta_fname_size
                - delta_dir_length - ar_strlen(slash, slash_size);
		}

		//acdb_data.acdbdelta
		ACDB_STR_CPY_SAFE(delta_fname + offset, remaining_size,
            fname + char_index, acdb_file_name_len);
		offset += acdb_file_name_len;
		if (IsNull(slash))
		{
			remaining_size = delta_fname_size
                - delta_dir_length - acdb_file_name_len;
		}
		else
		{
			remaining_size = delta_fname_size
                - delta_dir_length
                - ar_strlen(slash, slash_size) - acdb_file_name_len;
		}

        ACDB_STR_CAT_SAFE(delta_fname, delta_fname_size,
            delta_fname_ext, ar_strlen(delta_fname_ext, ext_size));

		uint32_t delta_file_name_len =
            (uint32_t)ar_strlen(delta_fname, delta_fname_size);
		if ((uint32_t)delta_fname_size != delta_file_name_len + 1UL)
		{
			ACDB_FREE(delta_fname);
			delta_fname = NULL;
			return delta_fname;
		}
	}

    *delta_fpath_length = (uint32_t)delta_fname_size;

	return delta_fname;
}

int32_t AcdbInitUtilDeleteDeltaFileData(
    const char* pFilename, uint32_t fileNameLen, AcdbFile* deltaFilePath)
{
	int32_t result = ACDB_UTILITY_INIT_SUCCESS;
    int32_t delta_result = AR_EOK;
	char *pDeltaFileName = NULL;
    uint32_t delta_fpath_length = 0;

	pDeltaFileName = GetDeltaFileName(
        pFilename, fileNameLen, deltaFilePath, &delta_fpath_length);
	if (pDeltaFileName == NULL)
	{
		ACDB_ERR("AcdbInitUtilDeleteDeltaFileData() failed, delta filename is null");
		return ACDB_UTILITY_INIT_FAILURE;
	}
	else
	{
		delta_result = ar_fdelete(pDeltaFileName);
		if (delta_result != 0)
		{
			ACDB_ERR("Error[%d]: unable to delete delta acdb file %d", delta_result);
			result = ACDB_UTILITY_INIT_FAILURE;
		}

		ACDB_FREE(pDeltaFileName);
	}

	return result;
}

int32_t AcdbInitUtilGetDeltaInfo(
    const char* acdb_file_path, uint32_t acdb_file_path_length,
    AcdbFile* delta_path, AcdbInitUtilDeltaInfo *info)
{
    int32_t status = AR_EOK;
    ar_fhandle* fhandle = NULL;
    char_t *delta_file_path = NULL;
    uint32_t delta_file_path_length = 0;

    if (IsNull(info))
    {
        ACDB_ERR("Error[%d]: The info input parameters is null",
            AR_EBADPARAM);

        return AR_EBADPARAM;
    }

    if (info->file_access_io_flag == ACDB_FUNC_IO_UNUSED)
    {
        ACDB_ERR("Error[%d]: The file access IO flag is not set",
            AR_EBADPARAM);

        return AR_EBADPARAM;
    }

    if (IsNull(acdb_file_path) || acdb_file_path_length == 0)
    {
        ACDB_ERR("Error[%d]: The input file name is null or empty",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    delta_file_path = GetDeltaFileName(
        acdb_file_path, (size_t)acdb_file_path_length,
        delta_path, &delta_file_path_length);
    if (delta_file_path == NULL)
    {
        ACDB_ERR("Error[%d]: The delta file path is null", AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    info->file_size = 0;
    info->path_length = delta_file_path_length;
    info->path = delta_file_path;

    /* Try opening the delta file with RW, then RO to determine the
    * access type */
    fhandle = ACDB_MALLOC(ar_fhandle, 1);
    if (IsNull(fhandle))
    {
        ACDB_ERR("Error[%d]: Unable to allocate memory for the file handle",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    if (info->file_access_io_flag == ACDB_FUNC_IO_OUT)
    {
        info->file_access = AR_FOPEN_READ_ONLY_WRITE;
        status = ar_fopen(fhandle, delta_file_path, info->file_access);
        if (status == AR_ENOTEXIST)
        {
            ACDB_DBG(
                "Delta Acdb file does not exist. Creating new file...");
            info->file_access = AR_FOPEN_WRITE_ONLY;
            status = ar_fopen(fhandle, delta_file_path, info->file_access);
            if (status == AR_ENOTEXIST)
            {
                ACDB_ERR("Error[%d]: The %s file path does not exist",
                    status, delta_file_path);
            }
        }

        if (AR_FAILED(status))
        {
            info->file_access = AR_FOPEN_READ_ONLY;
            status = ar_fopen(fhandle, delta_file_path, info->file_access);
            if (status == AR_ENOTEXIST)
            {
                ACDB_ERR("Error[%d]: The %s file does not exist",
                    status, delta_file_path);
                goto end;
            }
        }
    }
    else if (info->file_access_io_flag == ACDB_FUNC_IO_IN)
    {
        status = ar_fopen(fhandle, delta_file_path, info->file_access);
        if (status == AR_ENOTEXIST)
        {
            ACDB_ERR("Error[%d]: The %s file path does not exist",
                status, delta_file_path);
            goto end;
        }
    }

    if (!IsNull(fhandle) && AR_SUCCEEDED(status))
    {
        info->file_size = (uint32_t)ar_fsize(*fhandle);
        status = ar_fclose(*fhandle);
        if (AR_FAILED(status))
        {
            ACDB_ERR("Error[%d]: "
                "Unable to close %s", status, delta_file_path);
            goto end;
        }
    }

end:
    ACDB_FREE(fhandle);

    return status;
}

int32_t AcdbInitUtilOpenDeltaFile(
    AcdbInitUtilDeltaInfo *info, ar_fhandle* delta_fhandle)
{
    int32_t status = AR_EOK;

    if (IsNull(info))
    {
        ACDB_ERR("Error[%d]: One or more input parameters is null",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    if (IsNull(info->path) || info->path_length == 0)
    {
        ACDB_ERR("Error[%d]: The input file name is null or empty",
            AR_EBADPARAM);
        return AR_EBADPARAM;
    }

    status = ar_fopen(delta_fhandle, info->path, info->file_access);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to open delta acdb file", status);
        return status;
    }

    status = ar_fseek(*delta_fhandle, 0, AR_FSEEK_BEGIN);
    if (AR_FAILED(status))
    {
        ACDB_ERR("Error[%d]: Unable to seek file", status);
        return status;
    }

    return status;
}
