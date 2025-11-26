/*
 * Copyright (c) 2021, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef ANDROID_SYSTEM_LISTENSOUNDMODEL_V1_0_LISTENSOUNDMODEL_H
#define ANDROID_SYSTEM_LISTENSOUNDMODEL_V1_0_LISTENSOUNDMODEL_H

#include <vendor/qti/hardware/ListenSoundModel/1.0/IListenSoundModel.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vector>

using namespace android;

namespace vendor {
namespace qti {
namespace hardware {
namespace ListenSoundModel {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

class LsmClientDeathRecipient;

struct ListenSoundModel : public IListenSoundModel {
    public:

    Return<void> ipc_lsm_findKeywordEndPosition(const hidl_memory& pKeywordModel,
        int32_t modelSize, const keywordId_t_s& keywordId,
        const hidl_memory& pUserRecording, int32_t recSize,
        ipc_lsm_findKeywordEndPosition_cb _hidl_cb) override;

    Return<void> ipc_lsm_verifyUserRecording(const hidl_memory& pKeywordModel,
        int32_t modelSize, const keywordId_t_s& keywordId,
        const hidl_vec<ListenEpdParams>& pEpdParameter, const hidl_memory& pUserRecording,
        int32_t recSize, ipc_lsm_verifyUserRecording_cb _hidl_cb) override;

    Return<void> ipc_lsm_checkUserRecording(const hidl_memory& pLanguageModel,
        int32_t langModelSize, const hidl_vec<ListenEpdParams>& pEpdParameter,
        const hidl_memory& pUserRecording, int32_t recSize, uint32_t maxPhonemeLength,
        ipc_lsm_checkUserRecording_cb _hidl_cb) override;

    Return<void> ipc_lsm_checkRecordingsQuality(const hidl_memory& pLanguageModel,
        int32_t langModelSize, const hidl_vec<ListenEpdParams>& pEpdParameter,
        uint32_t numUserRecording, const hidl_vec<hidl_memory>& pUserRecordings,
        const hidl_vec<int32_t>& recsSize, ipc_lsm_checkRecordingsQuality_cb _hidl_cb) override;

    Return<void> ipc_lsm_tuneUserDefinedKeywordModelThreshold(
        const hidl_memory& pUserDefinedKeyword, int32_t modelSize,
        const keywordId_t_s& keywordId, const hidl_memory& pUserRecording,
        int32_t recSize, ipc_lsm_tuneUserDefinedKeywordModelThreshold_cb _hidl_cb) override;

    Return<void> ipc_lsm_getUserDefinedKeywordSize(const hidl_memory& pUserDefinedKeyword,
        int32_t modelSize, const keywordId_t_s& keywordId,
        const userId_t_s& userId, const hidl_vec<ListenEpdParams>& pEpdParameter,
        uint32_t numUserRecording, const hidl_vec<hidl_memory>& pUserRecordings,
        const hidl_vec<int32_t>& recsSize, const hidl_memory& pLanguageModel,
        int32_t langModelSize, ipc_lsm_getUserDefinedKeywordSize_cb _hidl_cb) override;

    Return<void> ipc_lsm_getUserDefinedKeywordApproxSize(const keywordId_t_s& keywordId,
        const userId_t_s& userId, const hidl_memory& pLanguageModel,
        int32_t langModelSize, uint32_t maxPhonemeLength,
        ipc_lsm_getUserDefinedKeywordApproxSize_cb _hidl_cb) override;

    Return<void> ipc_lsm_createUserDefinedKeywordModel(const hidl_memory& pUserDefinedKeyword,
         int32_t modelSize, const keywordId_t_s& keywordId,
         const userId_t_s& userId, const hidl_vec<ListenEpdParams>& pEpdParameter,
         uint32_t numUserRecording, const hidl_vec<hidl_memory>& pUserRecordings,
         const hidl_vec<int32_t>& recsSize, const hidl_memory& pLanguageModel,
         int32_t langModelSize, int32_t outModelSize,
         ipc_lsm_createUserDefinedKeywordModel_cb _hidl_cb) override;

    Return<void> ipc_lsm_getStrippedUserKeywordModelSize(const hidl_memory& pModel,
         int32_t modelSize,
         ipc_lsm_getStrippedUserKeywordModelSize_cb _hidl_cb) override;

    Return<void> ipc_lsm_stripUserKeywordModel(const hidl_memory& pModel,
         int32_t modelSize, ipc_lsm_stripUserKeywordModel_cb _hidl_cb) override;

    Return<void> ipc_lsm_getUserKeywordModelSize(const hidl_memory& pKeywordModel,
        int32_t modelSize, const keywordId_t_s& keywordId,  const userId_t_s& userId,
        ipc_lsm_getUserKeywordModelSize_cb _hidl_cb) override;

    Return<void> ipc_lsm_createUserKeywordModel(const hidl_memory& pUserDefinedKeyword,
        int32_t modelSize, const keywordId_t_s& keywordId,  const userId_t_s& userId,
        const hidl_vec<ListenEpdParams>& pEpdParameter, uint32_t numUserRecording,
        const hidl_vec<hidl_memory>& pUserRecordings,
        const hidl_vec<int32_t>& recsSize, int32_t outModelSize,
        ipc_lsm_createUserKeywordModel_cb _hidl_cb) override;

    Return<void> ipc_lsm_getSizeAfterDeleting(const hidl_memory& pInputModel,
        int32_t modelSize, const keywordId_t_s& keywordId,  const userId_t_s& userId,
        ipc_lsm_getSizeAfterDeleting_cb _hidl_cb) override;

    Return<void> ipc_lsm_deleteFromModel(const hidl_memory& pInputModel,
        int32_t modelSize, const keywordId_t_s& keywordId,
        const userId_t_s& userId, int32_t resModelSize,
        ipc_lsm_deleteFromModel_cb _hidl_cb) override;

    Return<void> ipc_lsm_getMergedModelSize(uint16_t numModels,
        const hidl_vec<hidl_memory>& pModels, const hidl_vec<int32_t>& modelsSize,
        ipc_lsm_getMergedModelSize_cb _hidl_cb) override;

    Return<void> ipc_lsm_mergeModels(uint16_t numModels, const hidl_vec<hidl_memory>& pModels,
        const hidl_vec<int32_t>& modelsSize, int32_t mergedModelSize,
        ipc_lsm_mergeModels_cb _hidl_cb) override;

    Return<void> ipc_lsm_parseFromBigSoundModel(const hidl_memory& pSM3p0Model,
        int32_t modelSize, ipc_lsm_parseFromBigSoundModel_cb _hidl_cb) override;

    Return<void> ipc_lsm_parseDetectionEventData(const hidl_memory& pUserKeywordModel,
        int32_t modelSize, const hidl_vec<ListenEventPayload>& pEventPayload,
        ipc_lsm_parseDetectionEventData_cb _hidl_cb) override;

    Return<void> ipc_lsm_querySoundModel(const hidl_memory& pSoundModel,
        int32_t modelSize, ipc_lsm_querySoundModel_cb _hidl_cb) override;

    Return<void> ipc_lsm_getSoundModelHeader(const hidl_memory& pSoundModel,
        int32_t modelSize, ipc_lsm_getSoundModelHeader_cb _hidl_cb) override;

    Return<uint32_t> ipc_lsm_releaseSoundModelHeader(
        const hidl_vec<ListenSoundModelHeader>& pListenSoundModelHeader) override;

    Return<void> ipc_lsm_getKeywordPhrases(const hidl_memory& pSoundModel,
        int32_t modelSize, uint16_t numInpKeywords,
        ipc_lsm_getKeywordPhrases_cb _hidl_cb) override;

    Return<void> ipc_lsm_getUserNames(const hidl_memory& pSoundModel,
        int32_t modelSize, uint16_t numInpUsers,
        ipc_lsm_getUserNames_cb _hidl_cb) override;

    Return<uint32_t> ipc_lsm_loadConfParams(const hidl_vec<uint8_t>& pConfData,
        uint32_t confDataSize) override;

    Return<void> ipc_lsm_getBinaryModelSize(const hidl_memory& pListenModel,
        int32_t modelSize,
        ipc_lsm_getBinaryModelSize_cb _hidl_cb) override;

    Return<void> ipc_lsm_getSortedKeywordStatesUserKeywordModelSize(
        const hidl_memory& pListenModel, int32_t modelSize,
        ipc_lsm_getSortedKeywordStatesUserKeywordModelSize_cb _hidl_cb) override;

    Return<void> ipc_lsm_sortKeywordStatesOfUserKeywordModel(
        const hidl_memory& pInputModel, int32_t modelSize, int32_t outModelSize,
        ipc_lsm_sortKeywordStatesOfUserKeywordModel_cb _hidl_cb) override;

    Return<void> ipc_lsm_verifyUserRecordingExt(
        const hidl_memory& pKeywordModel, int32_t modelSize, const keywordId_t_s& keywordId,
        const hidl_vec<ListenEpdParams>& pEpdParameter,
        const hidl_memory& pUserRecording, int32_t recSize, uint32_t isNoisySample,
        ipc_lsm_verifyUserRecordingExt_cb __hidl_cb) override;

    Return<void> ipc_lsm_smlInit(uint32_t configId, const hidl_vec<int8_t>& pConfig,
        uint32_t configStructSize, const hidl_vec<int8_t>& pStaticMemory,
        uint32_t staticMemorySize, ipc_lsm_smlInit_cb _hidl_cb) override;

    Return<uint32_t> ipc_lsm_smlSet(const hidl_vec<ListenSmlModel>& pSmlModel,
        uint32_t configId, const hidl_vec<int8_t>& pConfig, uint32_t configSize) override;

    Return<void> ipc_lsm_smlGet(const hidl_vec<ListenSmlModel>& pSmlModel,
        uint32_t configId, const hidl_vec<int8_t>& pConfig, uint32_t configSize,
        ipc_lsm_smlGet_cb _hidl_cb) override;

    Return<void> ipc_lsm_smlProcess(const hidl_vec<ListenSmlModel>& pSmlModel,
        const hidl_vec<int8_t>& pInputData, uint32_t inputDataSize,
        uint32_t inputDataId, ipc_lsm_smlProcess_cb _hidl_cb) override;

    sp<LsmClientDeathRecipient> client_death_recipient_;
};

class LsmClientDeathRecipient : public android::hardware::hidl_death_recipient
{
    public:
        LsmClientDeathRecipient(const sp<ListenSoundModel> LsmInstance)
        :lsm_instance_(LsmInstance){}
        void serviceDied(uint64_t cookie,
         const android::wp<::android::hidl::base::V1_0::IBase>& who) override ;
    private:
       sp<ListenSoundModel> lsm_instance_;
};

}
}
}
}
}
}

#endif

