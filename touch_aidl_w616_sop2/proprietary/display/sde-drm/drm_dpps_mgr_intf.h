/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DRM_DPPS_MGR_INTF_H__
#define __DRM_DPPS_MGR_INTF_H__

namespace sde_drm {

class DRMDppsManagerIntf {
 public:
  virtual ~DRMDppsManagerIntf() {}
  virtual void Init(int fd, drmModeRes *res) = 0;
  virtual void CacheDppsFeature(uint32_t obj_id, va_list args) = 0;
  virtual void CommitDppsFeatures(drmModeAtomicReq *req, const DRMDisplayToken &tok) = 0;
  virtual void GetDppsFeatureInfo(DRMDppsFeatureInfo *info) = 0;
};

extern "C" DRMDppsManagerIntf *GetDppsManagerIntf();
}  // namespace sde_drm
#endif  // __DRM_DPPS_MGR_INTF_H__
