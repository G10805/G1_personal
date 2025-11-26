/* ===========================================================================
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#ifndef CONFIG_PORT_METADATA_H
#define CONFIG_PORT_METADATA_H

#include <cerrno>
#include <string>
#include <vector>
#include <map>
#include <system/camera_metadata.h>
#include <system/graphics-base.h>
#include "common.h"

#define CAMERA_METADATA_ENUM_STRING_MAX_SIZE 29
#define CAMERA_CONFIG_JSON_FILE "/vendor/bin/config_v4l2_camera_hal.json"

class config_port_metadata {
public:
    struct CameraInfo {
        // Indicates the physical port of the camera device
        std::string input_id = "";
        // Direction the camera faces.
        std::string lens_facing = "";
        // List of capabilities that this camera device advertises as fully supporting.
        std::vector<std::string> req_avl_cap;
#if defined(ANDROID_T_ABOVE)
        // The camera device location in the vehicle body frame
        std::string automotive_location = "";
        // The camera device facing direction w.r.t the vehicle body frame
        std::string automotive_lens_facing = "";
#endif
    };

    bool initialize(const char* configFileName);

    int camera_V4l2nodeToCamId(const char *v4l2_node);

    const std::vector<CameraInfo>& getCameras() const   { return mCameras; };
    bool cameraConfigFound = false;
    int device_idx = -1; //Takes Incrementing values 0,1,2,3,4,5,6,7 .....12,13,14,15
    int port_idx = -1;   //Takes Actual Input Port ID values 0,1,2,3,4,5,8,9,12,13,14,15,16,17,18,19

private:
    // Camera information
    std::vector<CameraInfo> mCameras;
    std::map<std::string, int> v4l2NodeToPortID {
    {"/dev/video51", 0},
    {"/dev/video52", 1},
    {"/dev/video53", 2},
    {"/dev/video54", 3},
    {"/dev/video55", 4},
    {"/dev/video56", 5},
    {"/dev/video57", 8},
    {"/dev/video58", 9},
    {"/dev/video59", 12},
    {"/dev/video60", 13},
    {"/dev/video61", 14},
    {"/dev/video62", 15},
    {"/dev/video63", 16},
    {"/dev/video64", 17},
    {"/dev/video65", 18},
    {"/dev/video66", 19}};
};

#endif // CONFIG_PORT_METADATA_H
