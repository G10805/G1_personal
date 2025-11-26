/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DRM_ENCODER_H__
#define __DRM_ENCODER_H__

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "drm_interface.h"
#include "drm_utils.h"

namespace sde_drm {

class DRMEncoder {
 public:
  explicit DRMEncoder(int fd) : fd_(fd) {}
  DRMEncoder(int fd, uint32_t id, uint32_t type) : fd_(fd), fake_id_(id), fake_type_(type) {}
  void InitAndParse(drmModeEncoder *encoder);
  DRMStatus GetStatus() { return status_; }
  void GetInfo(DRMEncoderInfo *info);
  void GetType(uint32_t *encoder_type) {
    drm_encoder_ ? *encoder_type = drm_encoder_->encoder_type : *encoder_type = fake_type_;
  }
  void GetId(uint32_t *encoder_id) {
    drm_encoder_ ? *encoder_id = drm_encoder_->encoder_id : *encoder_id = fake_id_;
  }
  int GetPossibleCrtcIndices(std::set<uint32_t> *possible_crtc_indices);
  void Dump();
  void Lock();
  void Unlock();
  ~DRMEncoder();

 private:
  int fd_ = -1;
  drmModeEncoder *drm_encoder_ = {};
  DRMStatus status_ = DRMStatus::FREE;
  DRMEncoderInfo encoder_info_ = {};

  // Userspace injected data, only used for creating object not reported by the driver
  uint32_t fake_id_;
  uint32_t fake_type_;
};

class DRMEncoderManager {
 public:
  explicit DRMEncoderManager(int fd) : fd_(fd) {}
  ~DRMEncoderManager();
  void Init(drmModeRes *res);
  void InsertSecondaryDSI();
  void DeInit() {}
  void DumpAll();
  void DumpByID(uint32_t id);
  int Reserve(const std::set<uint32_t> &possible_encoders, DRMDisplayToken *token);
  void Free(DRMDisplayToken *token);
  int GetEncoderInfo(uint32_t encoder_id, DRMEncoderInfo *info);
  int GetEncoderList(std::vector<uint32_t> *encoder_ids);
  int GetPossibleCrtcIndices(uint32_t encoder_id, std::set<uint32_t> *possible_crtc_indices);

 private:
  int fd_ = -1;
  std::vector<std::pair<uint32_t, std::unique_ptr<DRMEncoder>>> encoder_pool_{};
  int GetDisplayTypeCode(uint32_t encoder_type);
};

}  // namespace sde_drm

#endif  // __DRM_ENCODER_H__
