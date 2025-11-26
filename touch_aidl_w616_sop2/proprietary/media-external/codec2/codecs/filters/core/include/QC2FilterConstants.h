/*
 **************************************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_FILTER_CONSTANTS_H_
#define _QC2_FILTER_CONSTANTS_H_

//DRUNWAL: TODO: These should later be queried via IDeviceQuery interface

// INPUT PORT RESOLUTION
constexpr uint32_t FILTER_MIN_INPUT_WIDTH = 64;
constexpr uint32_t FILTER_MAX_INPUT_WIDTH = 4096;
constexpr uint32_t FILTER_MIN_INPUT_HEIGHT = 64;
constexpr uint32_t FILTER_MAX_INPUT_HEIGHT = 2160;
constexpr uint32_t FILTER_DEFAULT_INPUT_WIDTH = 1920;
constexpr uint32_t FILTER_DEFAULT_INPUT_STRIDE = 1920;
constexpr uint32_t FILTER_DEFAULT_INPUT_HEIGHT = 1080;
constexpr uint32_t FILTER_DEFAULT_INPUT_SCANLINES = 1920;

// OUTPUT PORT RESOLUTION
constexpr uint32_t FILTER_MIN_OUTPUT_WIDTH = FILTER_MIN_INPUT_WIDTH;
constexpr uint32_t FILTER_MAX_OUTPUT_WIDTH = FILTER_MAX_INPUT_WIDTH;
constexpr uint32_t FILTER_MIN_OUTPUT_HEIGHT = FILTER_MIN_INPUT_HEIGHT;
constexpr uint32_t FILTER_MAX_OUTPUT_HEIGHT = FILTER_MAX_INPUT_HEIGHT;
constexpr uint32_t FILTER_DEFAULT_OUTPUT_WIDTH = FILTER_DEFAULT_INPUT_WIDTH;
constexpr uint32_t FILTER_DEFAULT_OUTPUT_STRIDE = FILTER_DEFAULT_INPUT_STRIDE;
constexpr uint32_t FILTER_DEFAULT_OUTPUT_HEIGHT = FILTER_DEFAULT_INPUT_HEIGHT;
constexpr uint32_t FILTER_DEFAULT_OUTPUT_SCANLINES = FILTER_DEFAULT_INPUT_SCANLINES;

// DELAY
constexpr uint32_t FILTER_INPUT_DELAY = 1;    // Input frames needed to start processing
constexpr uint32_t FILTER_OUTPUT_DELAY = 1;  // Output frames needed to start producing output buffers
constexpr uint32_t FILTER_PROCESSING_DELAY = 0;
#endif // _QC2_FILTER_CONSTANTS_H_
