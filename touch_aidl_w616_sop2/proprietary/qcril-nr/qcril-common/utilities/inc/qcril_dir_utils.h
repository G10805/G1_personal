/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************

  @file    qcril_file_utils.h
  @brief   Provides interface to communicate with files

  DESCRIPTION

******************************************************************************/

#ifndef QCRIL_DIR_UTILS_H
#define QCRIL_DIR_UTILS_H

#include <list>
#include <string>
using std::string;
using std::list;

typedef struct dirList {
    char *dir_name;
    struct dirList *next;
} qcril_other_dirlist;

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
);


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
);

void qmi_ril_retrieve_directory_list(const char *path,const char *subStr, qcril_other_dirlist **dir_list);
void qmi_ril_free_directory_list(qcril_other_dirlist *dir_list);

#endif /* QCRIL_DIR_UTILS_H */
