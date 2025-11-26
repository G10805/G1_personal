/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************

  @file    qcril_file_utils.c
  @brief   Provides interface to communicate with files

  DESCRIPTION

******************************************************************************/
#include "qcril_dir_utils.h"
#include "qcril_memory_management.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "utils_internal.h"
#include "utils_common.h"

/*===========================================================================

  FUNCTION qcril_dir_retrieve_all_files

===========================================================================*/
/*!
    @brief
    retrieve all file names under certain directory

    @return
    0 for success
*/
/*=========================================================================*/
int qcril_dir_retrieve_all_files
(
    const char  *dir,
    list<string> &file_names,
    list<string> &dir_names
)
{
    DIR           *dir_desc;
    struct dirent *entry;
    int           root_dir_name_len = 0;
    int           ret               = UTILS_FAILURE;

    do {
        if (!dir)
        {
            break;
        }

        root_dir_name_len = strlen(dir);
        dir_desc = opendir(dir);

        if (dir_desc)
        {
            while ((entry = readdir(dir_desc)))
            {
                if (strcmp(entry->d_name, "..") && strcmp(entry->d_name,"."))
                {
                    if (entry->d_type == DT_DIR)
                    {
                        string dir_entry;
                        dir_entry.append(dir);
                        dir_entry.append(entry->d_name);
                        dir_entry.append("/");
                        dir_names.push_back(dir_entry);
                        UTIL_LOG_INFO("retrieved dir: %s", dir_entry.c_str());
                    }
                    else
                    {
                        string file_entry;
                        file_entry.append(dir);
                        file_entry.append(entry->d_name);
                        file_names.push_back(file_entry);
                        UTIL_LOG_INFO("retrieved file: %s", file_entry.c_str());
                    }
                }
            }

            closedir (dir_desc);
            ret = UTILS_SUCCESS;
        }
        else
        {
            char errstr[200];
            strerror_r(errno, errstr, sizeof(errstr));
            UTIL_LOG_DEBUG("QCRIL_ERROR:IO: Failed to open directory %s with error: %d (%s)\n",
                    dir, errno, errstr);
        }

    }while (0);

    return ret;
};

/*===========================================================================

  FUNCTION qcril_dir_retrieve_all_files_recursively

===========================================================================*/
/*!
    @brief
    retrieve all files under current dir recursively

    @return
    0 for success
*/
/*=========================================================================*/
int qcril_dir_retrieve_all_files_recursively
(
    const char  *dir,
    list<string> &file_names
)
{
    int ret = UTILS_FAILURE;

    list<string> dir_names;

    do {
        if (!dir)
        {
            break;
        }

        dir_names.push_back(dir);

        while (!dir_names.empty())
        {
            string dir_index = dir_names.front();
            UTIL_LOG_DEBUG("processing dir %s ",dir_index.c_str());
            ret = qcril_dir_retrieve_all_files(dir_index.c_str(),
                                         file_names,
                                         dir_names);
            dir_names.pop_front();

            UTIL_LOG_DEBUG("cur_num_of_dirs %d", dir_names.size());
            UTIL_LOG_DEBUG("cur_num_of_files %d", file_names.size());
        }
    } while (0);

    return ret;
}

//===========================================================================
//qmi_ril_retrieve_directory_list
//===========================================================================
void qmi_ril_retrieve_directory_list(const char *path, const char *subStr, qcril_other_dirlist **dir_list)
{
  DIR                  *base_dir = NULL;
  int                  tmp_len   = 0;
  qcril_other_dirlist  *dir_trav = NULL;
  struct dirent        *dir;

  if(path == NULL)
  {
    return;
  }

  base_dir = opendir(path);
  if (base_dir)
  {
    while ((dir = readdir(base_dir)) != NULL)
    {
      UTIL_LOG_INFO("Dir: %s\n", dir->d_name);
      if(((dir->d_type == DT_DIR) || (dir->d_type == DT_LNK)) &&
         ((NULL == subStr) || ((NULL != subStr) && (!strncmp(dir->d_name, subStr, strlen(subStr))))))
      {
          if(dir_trav)
          {
            dir_trav->next = (struct dirList *)qcril_malloc(sizeof(*(dir_trav->next)));
            dir_trav = dir_trav->next;
          }
          else
          {
            *dir_list = (qcril_other_dirlist *)qcril_malloc(sizeof(**dir_list));
            dir_trav = *dir_list;
          }

          if( dir_trav != NULL )
          {
            tmp_len = strlen(dir->d_name) + 1;
            dir_trav->dir_name = (char*)qcril_malloc(tmp_len);
            if(dir_trav->dir_name != NULL)
            {
              strlcpy(dir_trav->dir_name, dir->d_name, tmp_len);
              UTIL_LOG_INFO("Matched dir %s\n", dir_trav->dir_name);
            }
            else
            {
              UTIL_LOG_ERROR("Failed to allocate memory");
              break;
            }
          }
          else
          {
            UTIL_LOG_ERROR("Failed to allocate memory");
            break;
          }
      }
    }

    closedir(base_dir);
  }
  else
  {
    UTIL_LOG_ERROR("Failed to open directory %s", path);
    UTIL_LOG_INFO("QCRIL_ERROR:IO: Failed to open directory %s", path);
  }

  return;
}

//===========================================================================
//qmi_ril_free_directory_list
//===========================================================================
void qmi_ril_free_directory_list(qcril_other_dirlist *dir_list)
{
  qcril_other_dirlist *dir_trav = dir_list;

  while(dir_list != NULL)
  {
    if(dir_list->dir_name != NULL) qcril_free(dir_list->dir_name);
    dir_trav = dir_list->next;
    qcril_free(dir_list);
    dir_list = dir_trav;
  }

  return;
}
