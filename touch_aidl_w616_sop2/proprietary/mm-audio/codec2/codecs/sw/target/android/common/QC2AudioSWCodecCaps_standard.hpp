/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include "QC2AudioSwCodec.h"
#include "QC2CodecCapsHelper.h"
#include "QC2CodecConfig.h"
#include "QC2Constants.h"

#undef LOG_TAG
#define LOG_TAG "SwStandardCaps"

namespace qc2audio {

//--------------------------------------------------------------------------------------------------
//------------------------ v2 ----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
using C2R = C2SettingResultsBuilder;

//--- C2ComponentKindSetting -----------------------------------------------------------------------
class KindHelper : public QC2ParamCapsHelper {
 public:
    KindHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        auto kindType = (mKind == ComponentKind::KIND_ENCODER) ?
                C2Component::KIND_ENCODER : C2Component::KIND_DECODER;
        mParam = std::make_shared<T>(kindType);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({kindType})};
        mName = C2_PARAMKEY_COMPONENT_KIND;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2ComponentKindSetting;
    std::shared_ptr<T> mParam;
};

//--- C2ComponentDomainSetting ---------------------------------------------------------------------
class DomainHelper : public QC2ParamCapsHelper {
 public:
    DomainHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        auto domain = C2Component::DOMAIN_AUDIO;
        mParam = std::make_shared<T>(domain);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({domain})};
        mName = C2_PARAMKEY_COMPONENT_DOMAIN;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2ComponentDomainSetting;
    std::shared_ptr<T> mParam;
};

//--- C2PortBlockPoolsTuning::output ---------------------------------------------------------------
class BlockPoolOutputHelper : public QC2ParamCapsHelper {
 public:
    BlockPoolOutputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2BlockPool::local_id_t outputPoolIds[] = { C2BlockPool::BASIC_LINEAR };
        mParam = T::AllocShared(outputPoolIds);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, m.values[0]).any(),
                C2F(mParam, m.values).any()};
        mName = C2_PARAMKEY_OUTPUT_BLOCK_POOLS;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(SetterHelper<T>::NonStrictValuesWithNoDeps);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2PortBlockPoolsTuning::output;
    std::shared_ptr<T> mParam;
};

//--- C2StreamBufferTypeSetting::input -------------------------------------------------------------
class BufferTypeInputHelper : public QC2ParamCapsHelper {
 public:
    BufferTypeInputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2BufferData::type_t bufferType = C2BufferData::LINEAR;
        mParam = std::make_shared<T>(0u, bufferType);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_INPUT_STREAM_BUFFER_TYPE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamBufferTypeSetting::input;
    std::shared_ptr<T> mParam;
};

//--- C2StreamBufferTypeSetting::output ------------------------------------------------------------
class BufferTypeOutputHelper : public QC2ParamCapsHelper {
 public:
    BufferTypeOutputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2BufferData::type_t bufferType = C2BufferData::LINEAR;
        mParam = std::make_shared<T>(0u, bufferType);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_OUTPUT_STREAM_BUFFER_TYPE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamBufferTypeSetting::output;
    std::shared_ptr<T> mParam;
};

//--- C2StreamMaxBufferSizeInfo::input -------------------------------------------------------------
class MaxBufferSizeInputHelper : public QC2ParamCapsHelper {
 public:
    MaxBufferSizeInputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder()) {
            return;
        }
        uint32_t maxBufferSize = 0;
        if (isALAC()) {
            maxBufferSize = std::get<ALAC_MAX_INPUT_BUF_SIZE>(AlacDecMinMaxList);
        } else if (isAPE()) {
            maxBufferSize = std::get<APE_MAX_INPUT_BUF_SIZE>(ApeDecMinMaxList);
        } else if (isEVRC()) {
            maxBufferSize =
                QC2SpeechCodecConfig::getMaxInputBufferSize(SpeechCodecType::EVRC);
        } else if (isQCELP()) {
            maxBufferSize =
                QC2SpeechCodecConfig::getMaxInputBufferSize(SpeechCodecType::QCELP);
        } else if (isFLAC()) {
            maxBufferSize = std::get<FLAC_MAX_INPUT_BUF_SIZE>(FlacDecMinMaxList);
        } else if (isDSD()) {
            maxBufferSize = std::get<DSD_MAX_INPUT_BUF_SIZE>(DsdDecMinMaxList);
        } else {
            maxBufferSize = 4096 * 2 * 2; // framelength * channels * bytes per sample
        }
        mParam = std::make_shared<T>(0u, maxBufferSize);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

private:
    using T = C2StreamMaxBufferSizeInfo::input;
    std::shared_ptr<T> mParam;
};

//--- C2PortActualDelayTuning::input -------------------------------------------------------
class ActualInputDelayHelper : public QC2ParamCapsHelper {
 public:
    ActualInputDelayHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).equalTo(0u)};
        mName = C2_PARAMKEY_INPUT_DELAY;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2PortActualDelayTuning::input;
    std::shared_ptr<T> mParam;
};

//--- C2PortActualDelayTuning::output -------------------------------------------------------
class ActualOutputDelayHelper : public QC2ParamCapsHelper {
 public:
    ActualOutputDelayHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).equalTo(0u)};
        mName = C2_PARAMKEY_OUTPUT_DELAY;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2PortActualDelayTuning::output;
    std::shared_ptr<T> mParam;
};

//--- C2PortMediaTypeSetting::input ----------------------------------------------------------------
class MediaTypeInputHelper : public QC2ParamCapsHelper {
 public:
    MediaTypeInputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2String mimeType;
        if (mKind == ComponentKind::KIND_ENCODER) {
            mimeType = MimeType::RAW;
        } else {
            mimeType = kind == KIND_DECODER ? inputMime : outputMime;
        }
        mParam = GetSharedString<T>(mimeType);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, m.value).any()};
        mName = C2_PARAMKEY_INPUT_MEDIA_TYPE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2PortMediaTypeSetting::input;
    std::shared_ptr<T> mParam;
};

//--- C2PortMediaTypeSetting::output ---------------------------------------------------------------
class MediaTypeOutputHelper : public QC2ParamCapsHelper {
 public:
    MediaTypeOutputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2String mimeType;
        if (mKind != ComponentKind::KIND_ENCODER) {
            mimeType = MimeType::RAW;
        } else {
            mimeType = kind == KIND_DECODER ? inputMime : outputMime;
        }
        mParam = GetSharedString<T>(mimeType);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, m.value).any()};
        mName = C2_PARAMKEY_OUTPUT_MEDIA_TYPE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2PortMediaTypeSetting::output;
    std::shared_ptr<T> mParam;

    static C2R MediaTypeOutputSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me) {
        QLOGD("MediaTypeOutputSetter called %s", me.v.m.value);
        (void)me;
        (void)mayBlock;
        C2R res = C2R::Ok();
        return res;
    }
};

//--- C2StreamSampleRateInfo::output ------------------------------------------------------------
class SamplingRateOutputHelper : public QC2ParamCapsHelper {
 public:
    SamplingRateOutputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder()) {
            return;
        }
        auto sampleRate = 48000;
        auto sampleRateRange = defaultSampleRateRange;
        mFlags = 0;
        if (isALAC()) {
            sampleRate = AlacDefaults::ALACDefaultSampleRate;
            sampleRateRange = std::get<ALAC_SAMPLE_RATE_RANGE>(AlacDecMinMaxList);
        } else if (isAPE()) {
            sampleRate = ApeDefaults::APEDefaultSampleRate;
            sampleRateRange = std::get<APE_SAMPLE_RATE_RANGE>(ApeDecMinMaxList);
        } else if (isEVRC() || isQCELP()) {
            sampleRate = SpeechDefaults::SpeechDefaultSampleRate;
            sampleRateRange = SpeechSampleRateRange;
        } else if (isFLAC()) {
            sampleRate = FlacDefaults::FLACDefaultSampleRate;
            sampleRateRange = std::get<FLAC_SAMPLE_RATE_RANGE>(FlacDecMinMaxList);
        } else if (isDSD()) {
            // since DSD output sample rate is given by CSIM decoder
            mFlags = FRAME_SYNC;
            sampleRate = DsdDefaults::DSDDefaultOutputSampleRate;
            sampleRateRange = std::get<DSD_OUTPUT_SAMPLE_RATE_RANGE>(DsdDecMinMaxList);
        } else {
            // Param not supported
            return;
        }
        mParam = std::make_shared<T>(0, sampleRate);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        uint32_t minRate = std::get<MIN_SAMPLE_RATE_IDX>(sampleRateRange);
        uint32_t maxRate = std::get<MAX_SAMPLE_RATE_IDX>(sampleRateRange);
        mRange = {C2F(mParam, value).inRange(minRate, maxRate)};
        mName = C2_PARAMKEY_SAMPLE_RATE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamSampleRateInfo::output;
    std::shared_ptr<T> mParam;
};

//--- C2StreamSampleRateInfo::input
// -------------------------------------------------------------
class SamplingRateInputHelper : public QC2ParamCapsHelper {
   public:
    SamplingRateInputHelper(const std::string& codecName,
                            const std::string& inputMime,
                            const std::string& outputMime, uint32_t variant,
                            ComponentKind kind,
                            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime,
                             variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX,
                            C2StructDescriptor(static_cast<T*>(nullptr)));
        // this param is used by encoder typically.
        // since DSD decoder (works like downsampler but not limited to) is an
        // exception here,because DSD has large input sample rates
        // e.g: 2.8 MHz we lower to 176.4k
        if (isDecoder()) {
            if (!isDSD()) {
                return;
            }
        }
        auto sampleRate = 48000;
        auto sampleRateRange = defaultSampleRateRange;
        mParam = nullptr;
        if (isDSD()) {
            sampleRate = DsdDefaults::DSDDefaultInputSampleRate;
            sampleRateRange = std::get<DSD_INPUT_SAMPLE_RATE_RANGE>(DsdDecMinMaxList);
            mParam = std::make_shared<T>(0, sampleRate);
        }

        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        uint32_t minRate = std::get<MIN_SAMPLE_RATE_IDX>(sampleRateRange);
        uint32_t maxRate = std::get<MAX_SAMPLE_RATE_IDX>(sampleRateRange);
        mRange = {C2F(mParam, value).inRange(minRate, maxRate)};
        mName = C2_PARAMKEY_CODED_SAMPLE_RATE;
        mDependencies = {};
        mAttributes =
            C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

   private:
    using T = C2StreamSampleRateInfo::input;
    std::shared_ptr<T> mParam;
};

//--- C2StreamChannelCountInfo::output ------------------------------------------------------------
class ChannelCountOutputHelper : public QC2ParamCapsHelper {
 public:
    ChannelCountOutputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder()) {
            return;
        }
        auto channelCount = 2;
        uint32_t maxChannels = 2;
        mParam = std::make_shared<T>(0, channelCount);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        if (isALAC()) {
            maxChannels = std::get<ALAC_MAX_CHANNELS>(AlacDecMinMaxList);
        } else if (isAPE()) {
            maxChannels = std::get<APE_MAX_CHANNELS>(ApeDecMinMaxList);
        } else if (isEVRC() || isQCELP()) {
            maxChannels = MAX_SPEECH_CHANNELS;
        } else if (isFLAC()) {
            maxChannels = std::get<FLAC_MAX_CHANNELS>(FlacDecMinMaxList);
        } else if (isDSD()) {
            maxChannels = std::get<DSD_MAX_OUTPUT_CHANNELS>(DsdDecMinMaxList);
        } else {
            // Param not supported
            return;
        }
        mRange = {C2F(mParam, value).inRange(1, maxChannels)};
        mName = C2_PARAMKEY_CHANNEL_COUNT;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamChannelCountInfo::output;
    std::shared_ptr<T> mParam;
};

//--- C2StreamPcmEncodingInfo::output ------------------------------------------------------------
class BitsPerSampleOutputHelper : public QC2ParamCapsHelper {
 public:
    BitsPerSampleOutputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder()) {
            return;
        }
        auto bitsPerSample = C2Config::PCM_16;
        auto bitsPerSampleSet = defaultBitsPerSampleSet;
        mParam = std::make_shared<T>(0, bitsPerSample);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        if (isALAC()) {
            bitsPerSampleSet = std::get<ALAC_BITS_PER_SAMPLE_SET>(AlacDecMinMaxList);
        } else if (isAPE()) {
            bitsPerSampleSet = std::get<APE_BITS_PER_SAMPLE_SET>(ApeDecMinMaxList);
        } else if (isFLAC()) {
            bitsPerSampleSet = std::get<FLAC_BITS_PER_SAMPLE_SET>(FlacDecMinMaxList);
        } else if (isDSD()) {
            bitsPerSampleSet = std::get<DSD_OUTPUT_BITS_PER_SAMPLE_SET>(DsdDecMinMaxList);
        } else {
            return;
        }
        auto convertToC2PcmEncCfg = [](uint32_t& n) { n = sBitsPerSampletoPcmEncoding.at(n); };
        std::for_each(bitsPerSampleSet.begin(), bitsPerSampleSet.end(), convertToC2PcmEncCfg);
        mRange = {C2F(mParam, value).oneOf(bitsPerSampleSet)};
        mName = C2_PARAMKEY_PCM_ENCODING;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPcmEncodingInfo::output;
    std::shared_ptr<T> mParam;
};

//--- C2StreamBitrateInfo::input ------------------------------------------------------------
class BitRateInputHelper : public QC2ParamCapsHelper {
 public:
    BitRateInputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder()) {
            return;
        }
        auto bitRateInfo = 2048;
        auto minBitRate = 8000;
        auto maxBitrate = 8000;
        mParam = std::make_shared<T>(0, bitRateInfo);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        if (isALAC()) {
            bitRateInfo = AlacDefaults::ALACDefaultAvgBitRate;
            minBitRate = std::get<ALAC_MIN_BIT_RATE>(AlacDecMinMaxList);
            maxBitrate = std::get<ALAC_MAX_BIT_RATE>(AlacDecMinMaxList);
        } else {
            // Parameter not supported for other formats
            return;
        }
        mRange = {C2F(mParam, value).inRange(minBitRate, maxBitrate)};
        mName = C2_PARAMKEY_BITRATE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(SetterHelper<T>::NonStrictValueWithNoDeps);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamBitrateInfo::input;
    std::shared_ptr<T> mParam;
};

//--- C2PortAllocatorsTuning::output ---------------------------------------------------------------
class AllocatorsOutputHelper : public QC2ParamCapsHelper {
 public:
    AllocatorsOutputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2Allocator::id_t rawAllocator = C2AllocatorStore::DEFAULT_LINEAR;
        C2Allocator::id_t outputAllocators[1] = { rawAllocator };
        C2PortAllocatorsTuning::output::AllocShared(outputAllocators);
        mParam = T::AllocShared(outputAllocators);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_OUTPUT_ALLOCATORS;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2PortAllocatorsTuning::output;
    std::shared_ptr<T> mParam;
};

//--- C2PortAllocatorsTuning::input ---------------------------------------------------------------
class AllocatorsInputHelper : public QC2ParamCapsHelper {
 public:
    AllocatorsInputHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2Allocator::id_t codedAllocator = C2AllocatorStore::DEFAULT_LINEAR;
        C2Allocator::id_t inputAllocators[1] = { codedAllocator };
        C2PortAllocatorsTuning::input::AllocShared(inputAllocators);
        mParam = T::AllocShared(inputAllocators);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_INPUT_ALLOCATORS;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withConstValue(getTempCopy(mParam));
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2PortAllocatorsTuning::input;
    std::shared_ptr<T> mParam;
};

}; // namsespace qc2audio
