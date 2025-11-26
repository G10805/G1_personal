#ifndef QCXMETADATA_H
#define QCXMETADATA_H

/**================================================================================================

 @file
 qcxmetadata.h

 @brief
 QCX Meta data defintions

 Copyright (c) 2022 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

 ================================================================================================**/


#include "qcarcam_types.h"
#include "qcarcam_metadata.h"

#define QCARCAM_MAX_METADATA_TAGS 100

typedef struct CMMetadata_t
{
    const char* pSectionName;     ///< Pointer to the section name for the vendor tag.
    const char* pTagName;         ///< Pointer to the name of the vendor tag.
    QCarCamMetadataTagId_e  qccId;           ///< Unique identifier for the QCarCamMetadata tag.
    uint32_t     tagId;           ///< Unique identifier for the vendor tag.
    int          tagType;         ///< Type of the vendor tag.
} CMMetadata_t;

typedef struct CMMetadataMsg_t
{
    uint32_t     tagId;           ///< Unique identifier for the vendor tag.
    int          tagType;         ///< Type of the vendor tag.
} CMMetadataMsg_t;

#endif //QCXMETADATA_H
