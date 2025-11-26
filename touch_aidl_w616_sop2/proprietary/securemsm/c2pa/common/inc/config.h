/*========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  =========================================================================*/

#pragma once
#include <map>

#define STRINGIFY(tag) #tag

enum C2PACborTag
{
    //Enroll tag
    FEATURE_LICENSE = 0,
    CLOUD_CREDENTIALS,
    SIGN_TERM_TEE,
    SIGN_TERM_NON_TEE,
    TSA_TERM,

    //General media tag
    MEDIA_TYPE = 1<<6,
    MEDIA_IS_SECURE,
    INPUT_MEDIA_FILE,
    OUTPUT_MEDIA_FILE,
    DATA_FILE,
    VALIDATION_REPORT,
    LOCATION_PERMISSION,
    THUMBNAIL_MAX_SIZE,
    THUMBNAIL_QUALITY,

    //Thumbnail tags
    THUMBNAIL_ARRAY = 1<<7,
    THUMBNAIL_URI,
    THUMBNAIL_ID,
    THUMBNAIL_MANIFEST_URI,
    THUMBNAIL_INSTANCE_ID,
    THUMBNAIL_TITLE,
    THUMBNAIL_FORMAT,
    THUMBNAIL_FILE,

    //Image media sign tag
    IMAGE_TYPE = 1<<8,
    IMAGE_WIDTH,
    IMAGE_HEIGHT,
    IMAGE_STRIDE,
    IMAGE_COMPRESSION,
};

const std::map<int64_t, std::string> tagMap = {
    //Enroll tag map
    {FEATURE_LICENSE, "FEATURE_LICENSE"},
    {CLOUD_CREDENTIALS, "CLOUD_CREDENTIALS"},
    {SIGN_TERM_TEE, "SIGN_TERM_TEE"},
    {SIGN_TERM_NON_TEE, "SIGN_TERM_NON_TEE"},
    {TSA_TERM, "TSA_TERM"},

    //General media tag map
    {MEDIA_TYPE, "MEDIA_TYPE"},
    {MEDIA_IS_SECURE, "MEDIA_IS_SECURE"},
    {INPUT_MEDIA_FILE, "INPUT_MEDIA_FILE"},
    {OUTPUT_MEDIA_FILE, "OUTPUT_MEDIA_FILE"},
    {DATA_FILE, "DATA_FILE"},
    {VALIDATION_REPORT, "VALIDATION_REPORT"},
    {LOCATION_PERMISSION, "LOCATION_PERMISSION"},
    {THUMBNAIL_MAX_SIZE, "THUMBNAIL_MAX_SIZE"},
    {THUMBNAIL_QUALITY, "THUMBNAIL_QUALITY"},

    //Thumbnail tag map
    {THUMBNAIL_ARRAY, "THUMBNAIL_ARRAY"},
    {THUMBNAIL_URI, "THUMBNAIL_URI"},
    {THUMBNAIL_ID, "THUMBNAIL_ID"},
    {THUMBNAIL_MANIFEST_URI, "THUMBNAIL_MANIFEST_URI"},
    {THUMBNAIL_INSTANCE_ID, "THUMBNAIL_INSTANCE_ID"},
    {THUMBNAIL_TITLE, "THUMBNAIL_TITLE"},
    {THUMBNAIL_FORMAT, "THUMBNAIL_FORMAT"},
    {THUMBNAIL_FILE, "THUMBNAIL_FILE"},

    //Image media sign tag map
    {IMAGE_TYPE, "IMAGE_TYPE"},
    {IMAGE_WIDTH, "IMAGE_WIDTH"},
    {IMAGE_HEIGHT, "IMAGE_HEIGHT"},
    {IMAGE_STRIDE, "IMAGE_STRIDE"},
    {IMAGE_COMPRESSION, "IMAGE_COMPRESSION"}
};
