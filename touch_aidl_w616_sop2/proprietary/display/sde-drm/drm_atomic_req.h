/*
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DRM_ATOMIC_REQ_H__
#define __DRM_ATOMIC_REQ_H__

#include <drm_interface.h>
#include <drm_utils.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <vector>

namespace sde_drm {

class DRMManager;

class DRMAtomicReq : public DRMAtomicReqInterface {
 public:
  DRMAtomicReq(int fd, DRMManager *drm_manager);
  virtual ~DRMAtomicReq();
  virtual int Perform(DRMOps op_code, uint32_t obj_id, ...);
  virtual int Commit(bool synchronous, bool retain_planes);
  virtual int Validate();
  int Init(const DRMDisplayToken &tok);

 private:
  drmModeAtomicReq *drm_atomic_req_ = {};
  DRMManager *drm_mgr_ = {};
  int fd_ = -1;
  DRMDisplayToken token_ = {};
};

}  // namespace sde_drm
#endif  // __DRM_ATOMIC_REQ_H__
