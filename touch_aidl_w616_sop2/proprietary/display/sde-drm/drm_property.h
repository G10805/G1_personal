/*
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DRM_PROPERTY_H__
#define __DRM_PROPERTY_H__

#include <stdint.h>
#include <string>

namespace sde_drm {

enum struct DRMProperty {
  INVALID,
  TYPE,
  FB_ID,
  CRTC_ID,
  CRTC_X,
  CRTC_Y,
  CRTC_W,
  CRTC_H,
  SRC_X,
  SRC_Y,
  SRC_W,
  SRC_H,
  ZPOS,
  ALPHA,
  EXCL_RECT,
  H_DECIMATE,
  V_DECIMATE,
  INPUT_FENCE,
  ROTATION,
  BLEND_OP,
  SRC_CONFIG,
  SCALER_V1,
  SCALER_V2,
  CSC_V1,
  CAPABILITIES,
  MODE_PROPERTIES,
  LUT_ED,
  LUT_CIR,
  LUT_SEP,
  ROTATOR_CAPS_V1,
  ROT_DST_X,
  ROT_DST_Y,
  ROT_DST_W,
  ROT_DST_H,
  FB_TRANSLATION_MODE,
  ACTIVE,
  MODE_ID,
  OUTPUT_FENCE_OFFSET,
  OUTPUT_FENCE,
  ROI_V1,
  CORE_CLK,
  CORE_AB,
  CORE_IB,
  LLCC_AB,
  LLCC_IB,
  DRAM_AB,
  DRAM_IB,
  ROT_PREFILL_BW,
  ROT_CLK,
  SECURITY_LEVEL,
  DIM_STAGES_V1,
  IDLE_TIME,
  RETIRE_FENCE,
  DST_X,
  DST_Y,
  DST_W,
  DST_H,
  LP,
  HDR_PROPERTIES,
  DEST_SCALER,
  DS_LUT_ED,
  DS_LUT_CIR,
  DS_LUT_SEP,
  SDE_DSPP_GAMUT_V3,
  SDE_DSPP_GAMUT_V4,
  SDE_DSPP_GAMUT_V5,
  SDE_DSPP_GC_V1,
  SDE_DSPP_GC_V2,
  SDE_DSPP_IGC_V2,
  SDE_DSPP_IGC_V3,
  SDE_DSPP_IGC_V4,
  SDE_DSPP_PCC_V3,
  SDE_DSPP_PCC_V4,
  SDE_DSPP_PCC_V5,
  SDE_DSPP_PA_HSIC_V1,
  SDE_DSPP_PA_HSIC_V2,
  SDE_DSPP_PA_SIXZONE_V1,
  SDE_DSPP_PA_SIXZONE_V2,
  SDE_DSPP_PA_MEMCOL_SKIN_V1,
  SDE_DSPP_PA_MEMCOL_SKIN_V2,
  SDE_DSPP_PA_MEMCOL_SKY_V1,
  SDE_DSPP_PA_MEMCOL_SKY_V2,
  SDE_DSPP_PA_MEMCOL_FOLIAGE_V1,
  SDE_DSPP_PA_MEMCOL_FOLIAGE_V2,
  SDE_DSPP_PA_MEMCOL_PROT_V1,
  SDE_DSPP_PA_MEMCOL_PROT_V2,
  AUTOREFRESH,
  EXT_HDR_PROPERTIES,
  HDR_METADATA,
  MULTIRECT_MODE,
  ROT_FB_ID,
  SDE_DSPP_PA_DITHER_V1,
  SDE_DSPP_PA_DITHER_V2,
  SDE_PP_DITHER_V1,
  SDE_PP_DITHER_V2,
  INVERSE_PMA,
  CSC_DMA_V1,
  SDE_DGM_1D_LUT_IGC_V5,
  SDE_DGM_1D_LUT_GC_V5,
  SDE_VIG_1D_LUT_IGC_V5,
  SDE_VIG_1D_LUT_IGC_V6,
  SDE_VIG_3D_LUT_GAMUT_V5,
  SDE_VIG_3D_LUT_GAMUT_V6,
  SDE_DSPP_AD4_MODE,
  SDE_DSPP_AD4_INIT,
  SDE_DSPP_AD4_CFG,
  SDE_DSPP_AD4_INPUT,
  SDE_DSPP_AD4_BACKLIGHT,
  SDE_DSPP_AD4_ROI,
  SDE_DSPP_AD4_ASSERTIVENESS,
  SDE_DSPP_AD4_STRENGTH,
  SDE_DSPP_ABA_HIST_CTRL,
  SDE_DSPP_ABA_HIST_IRQ,
  SDE_DSPP_ABA_LUT,
  SDE_DSPP_AD4_BL_SCALE,
  SDE_DSPP_BL_SCALE,
  CAPTURE_MODE,
  QSYNC_MODE,
  IDLE_PC_STATE,
  TOPOLOGY_CONTROL,
  EDID,
  HANDOFF,
  EPT,

  // Insert above
  MAX
};

struct DRMPropertyManager {
  DRMProperty GetPropertyEnum(const std::string &name) const;

  void SetPropertyId(DRMProperty prop_enum, uint32_t prop_id) {
    properties_[(uint32_t)prop_enum] = prop_id;
  }

  uint32_t GetPropertyId(DRMProperty prop_enum) const { return properties_[(uint32_t)prop_enum]; }

  bool IsPropertyAvailable(DRMProperty prop_enum) const {
    return !!properties_[(uint32_t)prop_enum];
  }

 private:
  uint32_t properties_[(uint32_t)DRMProperty::MAX]{};
};

}  // namespace sde_drm

#endif  // __DRM_PROPERTY_H__
