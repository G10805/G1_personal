/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DRM_DPPS_MGR_IMP_H__
#define __DRM_DPPS_MGR_IMP_H__

#include <map>
#include <string>
#include <vector>
#include "drm_interface.h"
#include "drm_property.h"
#include "drm_dpps_mgr_intf.h"

namespace sde_drm {

struct DRMDppsPropInfo {
  /* params to be set in Init */
  uint32_t version;
  DRMProperty prop_enum;
  uint32_t prop_id;
  bool is_event;
  /* params to be set in CacheDppsFeature */
  uint32_t obj_id;
  uint64_t value;
};

class DRMDppsManagerImp : public DRMDppsManagerIntf {
 public:
  ~DRMDppsManagerImp() {}
  void Init(int fd, drmModeRes *res);
  void CacheDppsFeature(uint32_t obj_id, va_list args);
  void CommitDppsFeatures(drmModeAtomicReq *req, const DRMDisplayToken &tok);
  void GetDppsFeatureInfo(DRMDppsFeatureInfo *info);

 private:
  int GetDrmResources(drmModeRes *res);
  int InitCrtcProps();
  int InitConnProps();

  struct DRMDppsPropInfo dpps_feature_[kDppsFeaturesMax];
  std::vector<struct DRMDppsPropInfo> dpps_dirty_prop_;
  std::vector<struct DRMDppsPropInfo> dpps_dirty_event_;
  /* key is the prop name, value is prop_id */
  std::map<std::string, uint32_t> dpps_prop_info_map_ = {};
  DRMPropertyManager prop_mgr_{};
  int conn_id_ = -1;
  int crtc_id_ = -1;
  int drm_fd_ = -1;
};

class DRMDppsManagerDummyImp : public DRMDppsManagerIntf {
 public:
  ~DRMDppsManagerDummyImp() {}
  void Init(int fd, drmModeRes *res) {}
  void CacheDppsFeature(uint32_t obj_id, va_list args) {}
  void CommitDppsFeatures(drmModeAtomicReq *req, const DRMDisplayToken &tok) {}
  void GetDppsFeatureInfo(DRMDppsFeatureInfo *info) {}
};
}  // namespace sde_drm
#endif  // __DRM_DPPS_MGR_IMP_H__
