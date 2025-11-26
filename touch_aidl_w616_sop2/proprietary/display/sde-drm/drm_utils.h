/*
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DRM_UTILS_H__
#define __DRM_UTILS_H__

#include <stdint.h>
#include <stdlib.h>
#include <xf86drmMode.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sde_drm {

enum struct DRMStatus {
  BUSY,
  FREE,
};

void ParseFormats(const std::string &line, std::vector<std::pair<uint32_t, uint64_t>> *formats);
void Tokenize(const std::string &str, std::vector<std::string> *tokens, char delim);
void AddProperty(drmModeAtomicReqPtr req, uint32_t object_id, uint32_t property_id, uint64_t value,
                 bool cache, std::unordered_map<uint32_t, uint64_t> *prop_val_map);

}  // namespace sde_drm

#endif  // __DRM_UTILS_H__
