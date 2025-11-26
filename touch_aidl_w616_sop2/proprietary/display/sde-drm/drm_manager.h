/*
 * Copyright (c) 2017-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __DRM_MANAGER_H__
#define __DRM_MANAGER_H__

#include <drm_interface.h>
#include <mutex>
#include "drm_dpps_mgr_intf.h"

namespace sde_drm {

class DRMAtomicReqInterface;
class DRMPlaneManager;
class DRMPlane;
class DRMConnectorManager;
class DRMEncoderManager;
class DRMConnnector;
class DRMCrtcManager;
class DRMCrtc;

class DRMManager : public DRMManagerInterface {
 public:
  virtual ~DRMManager();
  virtual int RegisterDisplay(DRMDisplayType disp_type, DRMDisplayToken *token);
  virtual int RegisterDisplay(int32_t display_id, DRMDisplayToken *token);
  virtual void UnregisterDisplay(DRMDisplayToken *token);
  virtual void GetPlanesInfo(DRMPlanesInfo *info);
  virtual int GetCrtcInfo(uint32_t crtc_id, DRMCrtcInfo *info);
  virtual int GetConnectorInfo(uint32_t conn_id, DRMConnectorInfo *info);
  virtual int GetConnectorsInfo(DRMConnectorsInfo *infos);
  virtual int GetEncoderInfo(uint32_t encoder_id, DRMEncoderInfo *info);
  virtual int GetEncodersInfo(DRMEncodersInfo *infos);
  virtual void GetCrtcPPInfo(uint32_t crtc_id, DRMPPFeatureInfo *info);
  virtual int CreateAtomicReq(const DRMDisplayToken &token, DRMAtomicReqInterface **intf);
  virtual int DestroyAtomicReq(DRMAtomicReqInterface *intf);
  virtual int SetScalerLUT(const DRMScalerLUTInfo &lut_info);
  virtual int UnsetScalerLUT();
  virtual void GetDppsFeatureInfo(DRMDppsFeatureInfo *info);
  virtual void GetPlanesStateInfo(DRMPlanesStateInfo *info, bool update);
  virtual int HandoffPlane(uint32_t plane_id);

  DRMPlaneManager *GetPlaneMgr();
  DRMConnectorManager *GetConnectorMgr();
  DRMEncoderManager *GetEncoderMgr();
  DRMCrtcManager *GetCrtcMgr();
  DRMDppsManagerIntf *GetDppsMgrIntf();

  static DRMManager *GetInstance(int fd);
  static void Destroy();

 private:
  int Init(int drm_fd);

  int fd_ = -1;
  DRMPlaneManager *plane_mgr_ = {};
  DRMConnectorManager *conn_mgr_ = {};
  DRMEncoderManager *encoder_mgr_ = {};
  DRMCrtcManager *crtc_mgr_ = {};
  DRMDppsManagerIntf *dpps_mgr_intf_ = {};

  static std::map<int, DRMManager*> s_drm_instance;
  static std::mutex s_lock;
  static int s_count_;
};

}  // namespace sde_drm
#endif  // __DRM_MANAGER_H__
