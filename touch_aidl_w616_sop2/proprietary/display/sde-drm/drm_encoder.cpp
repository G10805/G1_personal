/*
 * Copyright (c) 2018-2019, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#include <stdint.h>
#include <stdlib.h>
#include <drm.h>
#include <drm_logger.h>
#include <errno.h>
#ifdef __MIN_ANDROID_VER_T__
#include <display/drm/sde_drm.h>
#else
#include <drm/sde_drm.h>
#endif
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "drm_encoder.h"
#include "drm_utils.h"

namespace sde_drm {

using std::distance;
using std::make_pair;
using std::map;
using std::pair;
using std::set;
using std::unique_ptr;

#define __CLASS__ "DRMEncoderManager"

DRMEncoderManager::~DRMEncoderManager() {}

void DRMEncoderManager::Init(drmModeRes *resource) {
  for (int i = 0; i < resource->count_encoders; i++) {
    unique_ptr<DRMEncoder> encoder(new DRMEncoder(fd_));
    drmModeEncoder *libdrm_encoder = drmModeGetEncoder(fd_, resource->encoders[i]);
    if (!libdrm_encoder) {
      DRM_LOGE("Critical error: drmModeGetEncoder() failed for encoder %d.", resource->encoders[i]);
      continue;
    }
    encoder->InitAndParse(libdrm_encoder);
    encoder_pool_.push_back(std::make_pair(resource->encoders[i], std::move(encoder)));
  }

  // TODO(user): Remove call when driver reporting of encoders is consistent across all use cases
  InsertSecondaryDSI();
}

/*
 * This API is a workaround for maintaining appropriate HW port numbers for displays on platforms
 * that can dynamically change between single panel and secondary panel uses cases. It is required
 * that the port # for a pixel stream remains consistent, so userspace must account for the
 * possiblility of the secondary panel use case, even during single panel use case.

 * Driver rearchitecture for encoder creation is under discussion for future chipsets to avoid the
 * need of this function.
*/
void DRMEncoderManager::InsertSecondaryDSI() {
  auto first_dsi_iter = encoder_pool_.end();
  bool second_dsi_found = false;
  bool first_dsi_found = false;

  for (auto encoder = encoder_pool_.begin(); encoder != encoder_pool_.end(); encoder++) {
    std::unique_ptr<DRMEncoder> &encoder_ptr = encoder->second;
    uint32_t encoder_type;
    encoder_ptr->GetType(&encoder_type);
    if (encoder_type == DRM_MODE_ENCODER_DSI) {
      if (!first_dsi_found) {
        first_dsi_iter = encoder;
        first_dsi_found = true;
      } else if (first_dsi_found) {
        // Secondary panel use case is active, below logic is skipped
        second_dsi_found = true;
        break;
      }
    }
  }

  if (first_dsi_found && !second_dsi_found) {
    // Single panel use case is active, inject new DSI encoder
    auto enc_iter = first_dsi_iter + 1;
    unique_ptr<DRMEncoder> sec_dsi_enc(new DRMEncoder(fd_, 0, DRM_MODE_ENCODER_DSI));
    encoder_pool_.insert(enc_iter, std::make_pair(0, std::move(sec_dsi_enc)));
    DRM_LOGI("Userspace has inserted secondary panel DSI encoder!");
  } else {
    DRM_LOGI("Userspace did not need to insert secondary panel DSI encoder, it is present.");
  }
}

void DRMEncoderManager::DumpByID(uint32_t id) {
  encoder_pool_.at(id).second->Dump();
}

void DRMEncoderManager::DumpAll() {
  for (auto &encoder : encoder_pool_) {
    encoder.second->Dump();
  }
}

int DRMEncoderManager::GetEncoderInfo(uint32_t encoder_id, DRMEncoderInfo *info) {
  int ret = -ENODEV;
  auto iter = std::find_if(encoder_pool_.begin(), encoder_pool_.end(),
    [&](std::pair<uint32_t, std::unique_ptr<DRMEncoder>> &pair) {
      return pair.first == encoder_id;
    });

  if (iter != encoder_pool_.end()) {
    iter->second->GetInfo(info);
    ret = 0;
  }
  return ret;
}

int DRMEncoderManager::GetEncoderList(std::vector<uint32_t> *encoder_ids) {
  if (!encoder_ids) {
    return -EINVAL;
  }
  encoder_ids->clear();
  for (auto &encoder : encoder_pool_) {
    encoder_ids->push_back(encoder.first);
  }
  return 0;
}

int DRMEncoderManager::Reserve(const set<uint32_t> &possible_encoders, DRMDisplayToken *token) {
  int ret = -ENODEV;
  for (auto encoder = encoder_pool_.begin(); encoder != encoder_pool_.end(); encoder++) {
    const uint32_t &encoder_id = encoder->first;
    if ((encoder->second)->GetStatus() == DRMStatus::FREE) {
      // If encoder is found in the set, the encoder is a candidate for selection and the encoder
      // type for display's connector and encoder-this-iteration are already matched implicitly
      if (possible_encoders.find(encoder_id) != possible_encoders.end()) {
        encoder->second->Lock();
        token->encoder_id = encoder_id;
        int encoder_index = static_cast<int>(distance(encoder_pool_.begin(), encoder) + 1);  // 1-indexed port
        // Port id format.
        // Bit 7   --> Display type 0: Pluggable 1: BuiltIn X:Virtual.
        // Bit 6   --> Pluggable: 0 for TMDS encoder, 1 for DPMST encoder.
        //             Builtin Or Virtual: X
        // Bit 5-0 --> Encoder index.
        uint32_t encoder_type;
        encoder->second->GetType(&encoder_type);
        token->hw_port = static_cast<uint8_t>(GetDisplayTypeCode(encoder_type) | encoder_index);
        ret = 0;
        break;
      }
    }
  }
  return ret;
}

int DRMEncoderManager::GetDisplayTypeCode(uint32_t encoder_type) {
  int disp_info = 0x2;
  switch (encoder_type) {
    case DRM_MODE_ENCODER_TMDS:
      disp_info = 0x0;
      break;
    case DRM_MODE_ENCODER_DPMST:
      disp_info = 0x1;
      break;
    default:
      break;
  }

  return (disp_info << 6);
}

void DRMEncoderManager::Free(DRMDisplayToken *token) {
  auto iter = std::find_if(encoder_pool_.begin(), encoder_pool_.end(),
    [&](std::pair<uint32_t, std::unique_ptr<DRMEncoder>> &pair) {
      return pair.first == token->encoder_id;
    });
  if (iter != encoder_pool_.end()) {
    iter->second->Unlock();
  } else {
    DRM_LOGW("Failed! encoder_id %u not found! Cleaning up token anyway...", token->encoder_id);
  }
  token->encoder_id = 0;
  token->hw_port = 0;
}

int DRMEncoderManager::GetPossibleCrtcIndices(uint32_t encoder_id,
                                  std::set<uint32_t> *possible_crtc_indices) {
  auto iter = std::find_if(encoder_pool_.begin(), encoder_pool_.end(),
    [&](std::pair<uint32_t, std::unique_ptr<DRMEncoder>> &pair) {
      return pair.first == encoder_id;
    });

  if (iter != encoder_pool_.end()) {
    return iter->second->GetPossibleCrtcIndices(possible_crtc_indices);
  }

  return -ENODEV;
}

// ==============================================================================================//

#undef __CLASS__
#define __CLASS__ "DRMEncoder"

DRMEncoder::~DRMEncoder() {
  if (drm_encoder_) {
    drmModeFreeEncoder(drm_encoder_);
  }
}

void DRMEncoder::GetInfo(DRMEncoderInfo *info) {
  *info = encoder_info_;
}

void DRMEncoder::Lock() {
  status_ = DRMStatus::BUSY;
}

void DRMEncoder::Unlock() {
  status_ = DRMStatus::FREE;
}

void DRMEncoder::InitAndParse(drmModeEncoder *encoder) {
  drm_encoder_ = encoder;
  encoder_info_.type = drm_encoder_->encoder_type;
}

int DRMEncoder::GetPossibleCrtcIndices(std::set<uint32_t> *possible_crtc_indices) {
  if (!possible_crtc_indices) {
    return -EINVAL;
  }

  (*possible_crtc_indices).clear();
  std::bitset<32> possible_crtcs = drm_encoder_->possible_crtcs;
  for (uint32_t i = 0; i < possible_crtcs.size(); i++) {
    if (possible_crtcs[i]) {
      (*possible_crtc_indices).insert(i);
    }
  }

  return 0;
}

void DRMEncoder::Dump() {
  if (drm_encoder_) {
    DRM_LOGI(
        "[driver-reported] id: %u encoder_type: %u crtc id: %u possible_crtcs: %u"
        "possible_clones: %u fd = %d",
        drm_encoder_->encoder_id, drm_encoder_->encoder_type, drm_encoder_->crtc_id,
        drm_encoder_->possible_crtcs, drm_encoder_->possible_clones, fd_);
  } else {
    DRM_LOGI("[userspace-only] id: %u encoder_type: %u fd = %d ", fake_id_, fake_type_, fd_);
  }
}

}  // namespace sde_drm
