/******************************************************************************
#  Copyright (c) 2019,2021 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef __ANDROID_AUDIO_MODULE_H_
#define __ANDROID_AUDIO_MODULE_H_

#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <interfaces/audio/QcRilGetAudioParamSyncMessage.h>
#include <interfaces/audio/QcRilSetAudioParamSyncMessage.h>
#include "hidl_impl/qti_audio_service_base.h"

namespace ril {
namespace modules {

class AndroidAudioModule : public Module {
 public:
  AndroidAudioModule();
  ~AndroidAudioModule();
  void init();

 private:
  ::android::sp<QtiAudioServiceBase> mAudioService = nullptr;

  void handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg);
  void initAndroidAudio(qcril_instance_id_e_type instance_id);
  void handleQcrilGetAudioParameters(std::shared_ptr<QcRilGetAudioParamSyncMessage> msg);
  void handleQcrilSetAudioParameters(std::shared_ptr<QcRilSetAudioParamSyncMessage> msg);
  android::String8 convertRilAudioParamtersToAndroid(QcRilSetAudioParams params);
  uint32_t convertCallStateToAudioNum(qcril::interfaces::AudioCallState call_state);
  qcril::interfaces::AudioCallState convertAudioNumToCallState(uint32_t call_state);
};

}  // namespace modules
}  // namespace ril
#endif  // __ANDROID_AUDIO_MODULE_H_
