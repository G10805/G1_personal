/*
 * Copyright (c) 2017-2019, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#ifndef __DRM_CONNECTOR_H__
#define __DRM_CONNECTOR_H__

#include <drm/msm_drm.h>
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
#include <mutex>
#include <set>
#include <vector>

#include "drm_pp_manager.h"
#include "drm_property.h"
#include "drm_utils.h"

namespace sde_drm {

class DRMConnector {
 public:
  explicit DRMConnector(int fd) : fd_(fd) {}
  ~DRMConnector();
  void InitAndParse(drmModeConnector *conn);
  void Lock() { status_ = DRMStatus::BUSY; }
  void Unlock() { status_ = DRMStatus::FREE; }
  DRMStatus GetStatus() { return status_; }
  int GetInfo(DRMConnectorInfo *info);
  void GetType(uint32_t *conn_type) { *conn_type = drm_connector_->connector_type; }
  void Perform(DRMOps code, drmModeAtomicReq *req, va_list args);
  int IsConnected() { return (DRM_MODE_CONNECTED == drm_connector_->connection); }
  int GetPossibleEncoders(std::set<uint32_t> *possible_encoders);
  void SetSkipConnectorReload(bool skip_reload) { skip_connector_reload_ = skip_reload; }
  void Dump();

 private:
  void ParseProperties();
  void ParseCapabilities(uint64_t blob_id, DRMConnectorInfo *info);
  void ParseCapabilities(uint64_t blob_id, drm_panel_hdr_properties *hdr_info);
  void ParseModeProperties(uint64_t blob_id, DRMConnectorInfo *info);
  void ParseCapabilities(uint64_t blob_id, drm_msm_ext_hdr_properties *hdr_info);
  void ParseCapabilities(uint64_t blob_id, std::vector<uint8_t> *edid);
  void SetROI(drmModeAtomicReq *req, uint32_t obj_id, uint32_t num_roi, DRMRect *conn_rois);

  int fd_ = -1;
  drmModeConnector *drm_connector_ = {};
  DRMPropertyManager prop_mgr_{};
  bool skip_connector_reload_ = false;  // Usually set to true for new TV/pluggable displays.
  DRMStatus status_ = DRMStatus::FREE;
  std::unique_ptr<DRMPPManager> pp_mgr_{};
};

class DRMConnectorManager {
 public:
  explicit DRMConnectorManager(int fd) : fd_(fd) {}
  void Init(drmModeRes *res);
  void Update();
  void DeInit() {}
  void DumpAll();
  void DumpByID(uint32_t id);
  int Reserve(DRMDisplayType disp_type, DRMDisplayToken *token);
  int Reserve(uint32_t conn_id, DRMDisplayToken *token);
  void Free(DRMDisplayToken *token);
  void Perform(DRMOps code, uint32_t obj_id, drmModeAtomicReq *req, va_list args);
  int GetConnectorInfo(uint32_t conn_id, DRMConnectorInfo *info);
  void GetConnectorList(std::vector<uint32_t> *conn_ids);
  int GetPossibleEncoders(uint32_t connector_id, std::set<uint32_t> *possible_encoders);
  ~DRMConnectorManager() {}

 private:
  int fd_ = -1;
  std::mutex lock_;
  // Map of connector id to DRMConnector *
  std::map<uint32_t, std::unique_ptr<DRMConnector>> connector_pool_{};
};

}  // namespace sde_drm

#endif  // __DRM_CONNECTOR_H__
