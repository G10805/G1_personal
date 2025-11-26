/*
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DRM_PP_MANAGER_H__
#define __DRM_PP_MANAGER_H__

#include <limits>
#include "drm_interface.h"
#include "drm_property.h"
#include "drm_utils.h"

namespace sde_drm {

struct DRMPPPropInfo {
  DRMProperty prop_enum;
  uint32_t version = std::numeric_limits<uint32_t>::max();
  uint32_t prop_id;
  uint32_t blob_id;
};

class DRMPPManager {
 public:
  explicit DRMPPManager(int fd);
  ~DRMPPManager();
  void Init(const DRMPropertyManager &pm, uint32_t object_type);
  void DeInit() {}
  void GetPPInfo(DRMPPFeatureInfo *info);
  void SetPPFeature(drmModeAtomicReq *req, uint32_t obj_id, const DRMPPFeatureInfo &feature);

 private:
  int SetPPBlobProperty(drmModeAtomicReq *req, uint32_t obj_id, struct DRMPPPropInfo *prop_info,
                        const DRMPPFeatureInfo &feature);

  int fd_ = -1;
  uint32_t object_type_ = std::numeric_limits<uint32_t>::max();
  DRMPPPropInfo pp_prop_map_[kPPFeaturesMax] = {};
};

}  // namespace sde_drm
#endif  // __DRM_PP_MANAGER_H__
