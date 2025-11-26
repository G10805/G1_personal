/* ===========================================================================
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
=========================================================================== */
#include "config_port_metadata.h"
#include "android-base/strings.h"

#include "json/json.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <assert.h>

bool config_port_metadata::initialize(const char* configFileName)
{
  bool complete = true;
  const std::string EMPTY_STRING = "";

  // Set up a stream to read in the input file
  std::ifstream configStream(configFileName);

  // Parse the stream into JSON objects
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  std::string errorMessage;
  Json::Value rootNode;
  bool parseOk = Json::parseFromStream(builder, configStream, &rootNode, &errorMessage);
  if (!parseOk) {
      HAL_LOGE("Failed to read configuration file %s\n", configFileName);
      HAL_LOGE("%s\n", errorMessage.c_str());
      return false;
  }

  // Read camera information
    Json::Value cameraArray = rootNode["cameras"];
    if (!cameraArray.isArray()) {
        HAL_LOGE("Invalid configuration format -- we expect an array of cameras\n");
        return false;
    }

    mCameras.reserve(cameraArray.size());
    for (auto&& node: cameraArray) {
        // Get data from the configuration file
        Json::Value nameNode = node.get("input_id", "MISSING");
        const char *input_id = nameNode.asCString();

        Json::Value lensFacingNode = node.get("lens_facing", EMPTY_STRING);
        const char *lens_facing = lensFacingNode.asCString();

        Json::Value poseTranslationNode = node.get("pose_translation", EMPTY_STRING);
        std::vector<float> lens_pose_translation;
        for (int index = 0; index < poseTranslationNode.size(); ++index) {
        lens_pose_translation.push_back(poseTranslationNode[index].asFloat());
        }

        Json::Value poseRotationNode = node.get("pose_rotation", EMPTY_STRING);
        std::vector<float> lens_pose_rotation;
        for (int index = 0; index < poseRotationNode.size(); ++index) {
        lens_pose_rotation.push_back(poseRotationNode[index].asFloat());
        }

        Json::Value intrinsicCalibrationNode = node.get("intrinsic_calibration", EMPTY_STRING);
        std::vector<float> lens_intrinsic_calibration;
        for (int index = 0; index < intrinsicCalibrationNode.size(); ++index) {
        lens_intrinsic_calibration.push_back(intrinsicCalibrationNode[index].asFloat());
        }

        Json::Value poseReferenceNode = node.get("pose_reference", EMPTY_STRING);
        const char *lens_pose_reference = poseReferenceNode.asCString();

        Json::Value reqAvlCapNode = node.get("req_avl_cap", EMPTY_STRING);

        std::vector<std::string> req_avl_cap;
        for ( int index = 0; index < reqAvlCapNode.size(); ++index )
        {
            const char *reqAvlCap = reqAvlCapNode[index].asCString();
            if ((reqAvlCapNode[index] != NULL) && (reqAvlCap != NULL))
                req_avl_cap.push_back(reqAvlCap);
        }

#if defined(ANDROID_T_ABOVE)
        Json::Value locationNode = node.get("location", EMPTY_STRING);
        const char *automotive_location = locationNode.asCString();
        bool isLensFacingOther = false;
        bool isLocationOther = false;

        Json::Value facingNode = node.get("facing", EMPTY_STRING);
        const char *automotive_lens_facing = facingNode.asCString();

        if (automotive_lens_facing != NULL) {
            isLensFacingOther = android::base::EndsWith(automotive_lens_facing, "_OTHER");
        }
        if (automotive_location != NULL) {
            isLocationOther = android::base::EndsWith(automotive_location, "_OTHER");
        }

        if (isLensFacingOther || isLocationOther) {
          if (lens_intrinsic_calibration.size() != 5 || lens_pose_translation.size() != 3 || lens_pose_rotation.size() != 4 ||
              !lens_pose_reference || strcmp(lens_pose_reference, "AUTOMOTIVE") != 0) {
            HAL_LOGE("Unable to fetch lens_pose properties from Config file");
            complete = false;
            break;
          }
        }
        if (!input_id || !lens_facing || req_avl_cap.empty() || !automotive_location || !automotive_lens_facing || (strcmp(input_id, "MISSING") == 0))
#else
        if (!input_id || !lens_facing || req_avl_cap.empty() || (strcmp(input_id, "MISSING") == 0))
#endif
        {
          HAL_LOGE("Unable to fetch Static Metadata info from Config file");
          complete = false;
          break;
        }

        // Store the camera info
        CameraInfo info;
        info.input_id    = input_id;
        info.lens_facing = lens_facing;
        info.req_avl_cap = req_avl_cap;
#if defined(ANDROID_T_ABOVE)
        info.automotive_location    = automotive_location;
        info.automotive_lens_facing = automotive_lens_facing;
        info.lens_pose_reference = lens_pose_reference;
        info.lens_pose_rotation = lens_pose_rotation;
        info.lens_pose_translation = lens_pose_translation;
        info.lens_intrinsic_calibration = lens_intrinsic_calibration;
#endif
        mCameras.emplace_back(info);
    }

  // If we got this far, we were successful as long as we found all our child fields
  return complete;
}

int config_port_metadata::camera_V4l2nodeToCamId(const char *v4l2_node)
{
  auto iter = v4l2NodeToPortID.find(v4l2_node);
  if (iter == v4l2NodeToPortID.end()) {
    HAL_LOGE("ERROR: video node %s does not exist",v4l2_node);
    return -1;
  }
  port_idx = iter->second;
  device_idx++;
  return 0;
}

