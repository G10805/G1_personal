/*
 * Copyright (c) 2017-2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#ifndef __DRM_PLANE_H__
#define __DRM_PLANE_H__

#ifdef __MIN_ANDROID_VER_T__
#include <display/drm/sde_drm.h>
#else
#include <drm/sde_drm.h>
#endif
#include <drm_interface.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <mutex>

#include "drm_pp_manager.h"
#include "drm_property.h"

namespace sde_drm {

class DRMPlaneManager;

enum DRMPlaneLutState {
  kInactive,  //  Lut is not in use, default
  kActive,    //  Lut is in use
  kDirty,     //  Plane was unset while being LUT was active, mark LUT as dirty
              //  to make sure it's cleared the next time plane is used
};

class DRMPlane {
 public:
  explicit DRMPlane(int fd, uint32_t priority);
  ~DRMPlane();
  void InitAndParse(drmModePlane *plane);
  void GetId(uint32_t *id) { *id = drm_plane_->plane_id; }
  void GetType(DRMPlaneType *type) { *type = plane_type_info_.type; }
  void GetPriority(uint32_t *priority) { *priority = priority_; }
  void GetAssignedCrtc(uint32_t *crtc_id) { *crtc_id = assigned_crtc_id_; }
  void GetRequestedCrtc(uint32_t *crtc_id) { *crtc_id = requested_crtc_id_; }
  void SetAssignedCrtc(uint32_t crtc_id) { assigned_crtc_id_ = crtc_id; }
  void SetRequestedCrtc(uint32_t crtc_id) { requested_crtc_id_ = crtc_id; }
  bool SetScalerConfig(drmModeAtomicReq *req, uint64_t handle);
  bool SetCscConfig(drmModeAtomicReq *req, DRMCscType csc_type);
  bool ConfigureScalerLUT(drmModeAtomicReq *req, uint32_t dir_lut_blob_id, uint32_t cir_lut_blob_id,
                          uint32_t sep_lut_blob_id);
  const DRMPlaneTypeInfo &GetPlaneTypeInfo() { return plane_type_info_; }
  void SetDecimation(drmModeAtomicReq *req, uint32_t prop_id, uint32_t prop_value);
  void SetExclRect(drmModeAtomicReq *req, DRMRect rect);
  void Perform(DRMOps code, drmModeAtomicReq *req, va_list args);
  void Dump();
  void SetMultiRectMode(drmModeAtomicReq *req, DRMMultiRectMode drm_multirect_mode);
  void Unset(bool is_commit, drmModeAtomicReq *req);
  void PostValidate(uint32_t crtc_id, bool success);
  void PostCommit(uint32_t crtc_id, bool success);
  bool SetDgmCscConfig(drmModeAtomicReq *req, uint64_t handle);
  void UpdatePPLutFeatureInuse(DRMPPFeatureInfo *data);
  void ResetColorLUTs(bool is_commit, drmModeAtomicReq *req);
  void ResetColorLUTState(DRMTonemapLutType lut_type, bool is_commit, drmModeAtomicReq *req);
  void ResetColorLUT(DRMPPFeatureID id, drmModeAtomicReq *req);
  DRMPlaneStateInfo GetStateInfo(void);
  bool Handoff(drmModeAtomicReq *req);

 private:
  typedef std::map<DRMProperty, std::tuple<uint64_t, drmModePropertyRes *>> PropertyMap;
  void ParseProperties();
  void GetTypeInfo(const PropertyMap &props);
  void GetRotatorCaps(const PropertyMap &props);
  void PerformWrapper(DRMOps code, drmModeAtomicReq *req, ...);

  int fd_ = -1;
  uint32_t priority_ = 0;
  drmModePlane *drm_plane_ = {};
  DRMPlaneTypeInfo plane_type_info_{};
  uint32_t assigned_crtc_id_ = 0;
  uint32_t requested_crtc_id_ = 0;
  DRMPropertyManager prop_mgr_{};
  bool has_excl_rect_ = false;
  drm_clip_rect excl_rect_copy_ = {};
  std::unique_ptr<DRMPPManager> pp_mgr_{};
  std::unordered_map<uint32_t, uint64_t> tmp_prop_val_map_{};
  std::unordered_map<uint32_t, uint64_t> committed_prop_val_map_{};

  // Only applicable to planes that have scaler
  sde_drm_scaler_v2 scaler_v2_config_copy_ = {};
  sde_drm_csc_v1 csc_config_copy_ = {};
  bool is_lut_configured_ = false;

  bool dgm_csc_in_use_ = false;
  // Tone-mapping lut properties
  DRMPlaneLutState dgm_1d_lut_igc_state_ = kInactive;
  DRMPlaneLutState dgm_1d_lut_gc_state_ = kInactive;
  DRMPlaneLutState vig_1d_lut_igc_state_ = kInactive;
  DRMPlaneLutState vig_3d_lut_gamut_state_ = kInactive;

  bool has_handoff_ = false;
};

class DRMPlaneManager {
 public:
  explicit DRMPlaneManager(int fd);
  void Init();
  void DeInit() {}
  void GetPlanesInfo(DRMPlanesInfo *info);
  void DumpAll();
  void DumpByID(uint32_t id);
  void Perform(DRMOps code, uint32_t obj_id, drmModeAtomicReq *req, va_list args);
  void UnsetUnusedResources(uint32_t crtc_id, bool is_commit, drmModeAtomicReq *req);
  void ResetColorLutsOnUsedPlanes(uint32_t crtc_id, bool is_commit, drmModeAtomicReq *req);
  void RetainPlanes(uint32_t crtc_id);
  void SetScalerLUT(const DRMScalerLUTInfo &lut_info);
  void UnsetScalerLUT();
  void PostValidate(uint32_t crtc_id, bool success);
  void PostCommit(uint32_t crtc_id, bool success);
  void GetPlanesStateInfo(DRMPlanesStateInfo *info, bool update);
  int HandoffPlane(uint32_t obj_id);

 private:
  void Perform(DRMOps code, drmModeAtomicReq *req, uint32_t obj_id, ...);

  int fd_ = -1;
  // Map of plane id to DRMPlane *
  std::map<uint32_t, std::unique_ptr<DRMPlane>> plane_pool_{};
  // Global Scaler LUT blobs
  uint32_t dir_lut_blob_id_ = 0;
  uint32_t cir_lut_blob_id_ = 0;
  uint32_t sep_lut_blob_id_ = 0;
  std::mutex lock_;
};

}  // namespace sde_drm

#endif  // __DRM_PLANE_H__
