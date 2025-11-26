#ifndef QCARCAM_METADATA_H_
#define QCARCAM_METADATA_H_

/**************************************************************************************************
@file
    qcarcam_metadata.h

@brief
    QCarCam API - QTI Automotive Imaging System Proprietary API for metadata

Copyright (c) 2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

**************************************************************************************************/
#include <stdint.h>
#include "camera_vendor_tags.h"


/** @brief QCarCam Vendor supported metadata tags */
typedef enum
{
    QCARCAM_METADATA_TAG_SATURATION_LEVEL = 0,
    QCARCAM_METADATA_TAG_CONTRAST_LEVEL,
    QCARCAM_METADATA_TAG_SHARPNESS_STRENGTH,
    QCARCAM_METADATA_TAG_ICA_LDC_TRANSFORM_MODE,
} QCarCamMetadataTagId_e;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Queries camera_metadata tag ID from the QCarCamMetadataTagId_e
 *
 * @param     qccId  QCarCamMetadataTagId_e tag ID.
 * @param     pTagId pointer to tagId that will be filled with camera_metadata tag ID on success
 *
 * @return QCARCAM_RET_OK only if successful; check QCarCamRet_e otherwise.
 */
QCarCamRet_e QCarCamGetMetaDataTagId(QCarCamMetadataTagId_e qccId, uint32_t *pTagId);

/**
 * @brief     Fills vendor tag operations for camera_metadata operations
 *
 * @param     pVendorTagOps   Pointer to camera_metadata vendor tag operations structure to be filled
 *
 * @return QCARCAM_RET_OK only if successful; check QCarCamRet_e otherwise.
 */
QCarCamRet_e QCarCamMetadataGetVendorOps(vendor_tag_ops_t* pVendorTagOps);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* QCARCAM_METADATA_H_ */
