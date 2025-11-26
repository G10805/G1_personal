/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

//#include "QC2AudioCodec.h"
#include "QC2CodecCapsHelper.h"
#include "QC2Constants.h"
#include "QC2CodecConfig.h"

#undef LOG_TAG
#define LOG_TAG "StandardCaps"

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
        uint32_t maxBufferSize = 8192 * 8 * 4; // framelength * channels * bytes per sample;
        if (isAac() || isAacAdts()) {
            if (isEncoder()) {
                maxBufferSize = std::get<AAC_MAX_INPUT_BUF_SIZE>(AacEncMinMaxList[AAC_DEFAULT]);
            } else {
                maxBufferSize = std::get<AAC_MAX_INPUT_BUF_SIZE>(AacDecMinMaxList[AAC_DEFAULT]);
            }
        } else if (isALAC()) {
            maxBufferSize = std::get<ALAC_MAX_INPUT_BUF_SIZE>(AlacDecMinMaxList);
        } else if (isAPE()) {
            maxBufferSize = std::get<APE_MAX_INPUT_BUF_SIZE>(ApeDecMinMaxList);
        } else if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            maxBufferSize = std::get<WMA_MAX_INPUT_BUF_SIZE>(WmaMinMaxList[WMA_DEFAULT]);
        } else if (isEVRC()) {
            maxBufferSize = QC2EvrcEncConfig::getMaxInputBufferSize();
        } else if (isQCELP()) {
            maxBufferSize = QC2QcelpEncConfig::getMaxInputBufferSize();
        } else if (isAmrNb() && isEncoder()) {
            maxBufferSize = std::get<AMR_MAX_ENC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRNB]);
        } else if (isAmrWb() && isEncoder()) {
            maxBufferSize = std::get<AMR_MAX_ENC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRWB]);
        } else if (isAmrWbPlus() && isDecoder()) {
            maxBufferSize = std::get<AMR_MAX_DEC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRWBPlus]);
        } else {
            // Parameter not supported for other formats
            return;
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
class OutputSamplingRateHelper : public QC2ParamCapsHelper {
 public:
    OutputSamplingRateHelper(const std::string& codecName,
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
        mDependencies = {};
        auto sampleRate = 48000;
        auto sampleRates = defaultSampleRateRange;
        if (isAac() || isAacAdts()) {
            sampleRate = 44100;
            sampleRates = std::get<AAC_SAMPLE_RATE_RANGE>(AacDecMinMaxList[AAC_DEFAULT]);
        } else if(isAmrWbPlus()) {
            sampleRate = 16000;
            sampleRates = std::get<AMR_SAMPLE_RATE_RANGE>(AmrMinMaxList[AMRWBPlus]);
        } else if (isALAC()) {
            sampleRate = AlacDefaults::ALACDefaultSampleRate;
            sampleRates = std::get<ALAC_SAMPLE_RATE_RANGE>(AlacDecMinMaxList);
            mDependencies = {C2StreamChannelCountInfo::output::PARAM_TYPE};
        } else if (isAPE()) {
            sampleRate = ApeDefaults::APEDefaultSampleRate;
            sampleRates = std::get<APE_SAMPLE_RATE_RANGE>(ApeDecMinMaxList);
        } else if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            sampleRate = WmaStdDefaults::kWmaStdDefaultSampleRate;
            sampleRates = std::get<WMA_SAMPLE_RATE_RANGE>(WmaMinMaxList[WMA_DEFAULT]);
            mDependencies = {C2AudioWmaVersionInfo::input::PARAM_TYPE};
        } else {
            // Parameter not supported for other formats
            return;
        }
        mParam = std::make_shared<T>(0, sampleRate);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        uint32_t minRate = std::get<MIN_SAMPLE_RATE_IDX>(sampleRates);
        uint32_t maxRate = std::get<MAX_SAMPLE_RATE_IDX>(sampleRates);
        mRange = {C2F(mParam, value).inRange(minRate, maxRate)};
        mName = C2_PARAMKEY_SAMPLE_RATE;
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
        if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            auto wmaVersion = getDepRef<C2AudioWmaVersionInfo::input>(deps);
            if (!wmaVersion) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(WmaOutputSamplingRateSetter, *wmaVersion);
        } else if (isALAC()) {
            auto chCount = getDepRef<C2StreamChannelCountInfo::output>(deps);
            if (!chCount) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(AlacOutputSamplingRateSetter, *chCount);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamSampleRateInfo::output;
    std::shared_ptr<T> mParam;

    static C2R WmaOutputSamplingRateSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2AudioWmaVersionInfo::input>& wmaVersion) {
        (void)mayBlock;
        uint32_t version = wmaVersion.v.value;
        if (WmaVersionSet.find(version) == WmaVersionSet.end()) {
            return C2R::BadState();
        }
        auto sampleRateRange = std::get<WMA_SAMPLE_RATE_RANGE>(WmaMinMaxList[version]);
        if (me.v.value < std::get<MIN_SAMPLE_RATE_IDX>(sampleRateRange) &&
                me.v.value > std::get<MAX_SAMPLE_RATE_IDX>(sampleRateRange)) {
            return C2R::BadState();
        }
        return C2R::Ok();
    }

    static C2R AlacOutputSamplingRateSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2StreamChannelCountInfo::output>& chCount) {
        (void)mayBlock;
        uint32_t channels = chCount.v.value;
        auto sampleRateRange = std::get<ALAC_SAMPLE_RATE_RANGE>(AlacDecMinMaxList);
        if (channels > 2 && (me.v.value < std::get<MIN_SAMPLE_RATE_IDX>(sampleRateRange) ||
                me.v.value > sAlacMaxMultiChSampleRate)) {
            return C2R::BadState();
        } else if (channels <= 2 && (me.v.value < std::get<MIN_SAMPLE_RATE_IDX>(sampleRateRange) ||
                me.v.value > std::get<MAX_SAMPLE_RATE_IDX>(sampleRateRange))) {
            return C2R::BadState();
        }
        return C2R::Ok();
    }
};


//--- C2StreamSampleRateInfo::input ------------------------------------------------------------
class InputSamplingRateHelper : public QC2ParamCapsHelper {
 public:
    InputSamplingRateHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder()) {
            return;
        }
        mDependencies = {};
        auto sampleRate = 48000;
        auto sampleRates = defaultSampleRateRange;
        if (isAmrNb()) {
            sampleRate = 8000;
            sampleRates = std::get<AMR_SAMPLE_RATE_RANGE>(AmrMinMaxList[AMRNB]);
        } else if (isAmrWb()) {
            sampleRate = 16000;
            sampleRates = std::get<AMR_SAMPLE_RATE_RANGE>(AmrMinMaxList[AMRWB]);
        } else if (isAac() || isAacAdts()) {
            sampleRate = AacDefaults::AacDefaultSampleRate;
            sampleRates = std::get<AAC_SAMPLE_RATE_RANGE>(AacEncMinMaxList[AAC_DEFAULT]);
            mDependencies = {C2StreamProfileLevelInfo::output::PARAM_TYPE};
        } else if (isEVRC()) {
          sampleRate = SpeechDefaults::SpeechDefaultSampleRate;
          sampleRates = SpeechSampleRateRange;
        } else if (isQCELP()) {
          sampleRate = SpeechDefaults::SpeechDefaultSampleRate;
          sampleRates = SpeechSampleRateRange;
        } else {
            // Parameter not supported for other formats
            return;
        }
        mParam = std::make_shared<T>(0, sampleRate);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        uint32_t minRate = std::get<MIN_SAMPLE_RATE_IDX>(sampleRates);
        uint32_t maxRate = std::get<MAX_SAMPLE_RATE_IDX>(sampleRates);
        mRange = {C2F(mParam, value).inRange(minRate, maxRate)};
        mName = C2_PARAMKEY_SAMPLE_RATE;
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
        if (isAac() || isAacAdts()) {
            auto profileType = getDepRef<C2StreamProfileLevelInfo::output>(deps);
            if (!profileType) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(AacInputSamplingRateSetter, *profileType);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::EmptySetter);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamSampleRateInfo::input;
    std::shared_ptr<T> mParam;

    static C2R AacInputSamplingRateSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2StreamProfileLevelInfo::output>& profileType) {
        (void)mayBlock;
        uint32_t profile = profileType.v.profile;
        if (C2ProfileToAacProfileIndex.find(profile) == C2ProfileToAacProfileIndex.end()) {
            return C2R::BadState();
        }
        uint32_t profileIndex = C2ProfileToAacProfileIndex.at(profile);
        auto sampleRateRange = std::get<AAC_SAMPLE_RATE_RANGE>(AacEncMinMaxList[profileIndex]);
        if (me.v.value < std::get<MIN_SAMPLE_RATE_IDX>(sampleRateRange) ||
                me.v.value > std::get<MAX_SAMPLE_RATE_IDX>(sampleRateRange)) {
            return C2R::BadState();
        }
        return C2R::Ok();
    }
};

//--- C2StreamChannelCountInfo::output ------------------------------------------------------------
class OutputChannelCountHelper : public QC2ParamCapsHelper {
 public:
    OutputChannelCountHelper(const std::string& codecName,
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
        mDependencies = {};
        auto channelCount = 2;
        auto maxChannels = ChannelHelper::MAX_NUM_CHANNELS;
        if (isAac() || isAacAdts()) {
            channelCount = maxChannels = std::get<AAC_MAX_CHANNELS>(AacDecMinMaxList[AAC_DEFAULT]);
        } else if(isAmrWbPlus()) {
            channelCount = 1;
            maxChannels = std::get<AMR_MAX_CHANNELS>(AmrMinMaxList[AMRWBPlus]);
        } else if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            channelCount = maxChannels = std::get<WMA_MAX_CHANNELS>(WmaMinMaxList[WMA_DEFAULT]);
            mDependencies = {C2AudioWmaVersionInfo::input::PARAM_TYPE};
        } else if (isALAC()) {
            channelCount = maxChannels = std::get<ALAC_MAX_CHANNELS>(AlacDecMinMaxList);
        } else if (isAPE()) {
            channelCount = maxChannels = std::get<APE_MAX_CHANNELS>(ApeDecMinMaxList);
        } else {
            // Parameter not supported for other formats
            return;
        }
        mParam = std::make_shared<T>(0, channelCount);
        RETURN_IF_NULL(mParam);
        mRange = {C2F(mParam, value).inRange(1, maxChannels)};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_CHANNEL_COUNT;
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
        if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            auto wmaVersion = getDepRef<C2AudioWmaVersionInfo::input>(deps);
            if (!wmaVersion) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(WmaOutputChannelCountSetter, *wmaVersion);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamChannelCountInfo::output;
    std::shared_ptr<T> mParam;

    static C2R WmaOutputChannelCountSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2AudioWmaVersionInfo::input>& wmaVersion) {
        (void)mayBlock;
        uint32_t version = wmaVersion.v.value;
        if (WmaVersionSet.find(version) == WmaVersionSet.end()) {
            return C2R::BadState();
        }
        auto maxChannelCount = std::get<WMA_MAX_CHANNELS>(WmaMinMaxList[version]);
        if (me.v.value < 1 || me.v.value > maxChannelCount) {
            return C2R::BadState();
        }
        return C2R::Ok();
    }
};


//--- C2StreamMaxChannelCountInfo::input ------------------------------------------------------------
class OutputMaxChannelCountHelper : public QC2ParamCapsHelper {
 public:
    OutputMaxChannelCountHelper(const std::string& codecName,
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
        mDependencies = {};
        auto maxChannels = ChannelHelper::MAX_NUM_CHANNELS;
        if (isAac() || isAacAdts()) {
            maxChannels = std::get<AAC_MAX_CHANNELS>(AacDecMinMaxList[AAC_DEFAULT]);
        } else if(isAmrWbPlus()) {
            maxChannels = std::get<AMR_MAX_CHANNELS>(AmrMinMaxList[AMRWBPlus]);
        } else if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            maxChannels = std::get<WMA_MAX_CHANNELS>(WmaMinMaxList[WMA_DEFAULT]);
            mDependencies = {C2AudioWmaVersionInfo::input::PARAM_TYPE};
        } else if (isALAC()) {
            maxChannels = std::get<ALAC_MAX_CHANNELS>(AlacDecMinMaxList);
        } else if (isAPE()) {
            maxChannels = std::get<APE_MAX_CHANNELS>(ApeDecMinMaxList);
        } else {
            // Parameter not supported for other formats
            return;
        }
        mParam = std::make_shared<T>(0, maxChannels);
        RETURN_IF_NULL(mParam);
        mRange = {C2F(mParam, value).inRange(1, maxChannels)};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_MAX_CHANNEL_COUNT;
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
        if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            auto wmaVersion = getDepRef<C2AudioWmaVersionInfo::input>(deps);
            if (!wmaVersion) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(WmaOutputMaxChannelCountSetter, *wmaVersion);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::StrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamMaxChannelCountInfo::input;
    std::shared_ptr<T> mParam;

    static C2R WmaOutputMaxChannelCountSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2AudioWmaVersionInfo::input>& wmaVersion) {
        (void)mayBlock;
        uint32_t version = wmaVersion.v.value;
        if (WmaVersionSet.find(version) == WmaVersionSet.end()) {
            return C2R::BadState();
        }
        auto maxChannelCount = std::get<WMA_MAX_CHANNELS>(WmaMinMaxList[version]);
        if (me.v.value < 1 || me.v.value > maxChannelCount) {
            return C2R::BadState();
        }
        return C2R::Ok();
    }
};


//--- C2StreamChannelCountInfo::input ------------------------------------------------------------
class InputChannelCountHelper : public QC2ParamCapsHelper {
 public:
    InputChannelCountHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder()) {
            return;
        }
        mDependencies = {};
        auto channelCount = 2;
        auto maxChannels = ChannelHelper::MAX_NUM_CHANNELS;
        if (isAmrNb()) {
            channelCount = maxChannels = std::get<AMR_MAX_CHANNELS>(AmrMinMaxList[AMRNB]);
        } else if (isAmrWb()) {
            channelCount = maxChannels = std::get<AMR_MAX_CHANNELS>(AmrMinMaxList[AMRWB]);
        } else if (isAac() || isAacAdts()) {
            channelCount = maxChannels = std::get<AAC_MAX_CHANNELS>(AacEncMinMaxList[AAC_DEFAULT]);
        } else if (isEVRC()) {
            channelCount =  SpeechDefaults::SpeechDefaultChannels;
            maxChannels = MAX_SPEECH_CHANNELS;
        } else if (isQCELP()) {
            channelCount =  SpeechDefaults::SpeechDefaultChannels;
            maxChannels = MAX_SPEECH_CHANNELS;
        } else {
            // Parameter not supported for other formats
            return;
        }
        mParam = std::make_shared<T>(0, channelCount);
        RETURN_IF_NULL(mParam);
        mRange = {C2F(mParam, value).inRange(1, maxChannels)};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_CHANNEL_COUNT;
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
    using T = C2StreamChannelCountInfo::input;
    std::shared_ptr<T> mParam;
};

//--- C2StreamPcmEncodingInfo::output ------------------------------------------------------------
class OutputBitsPerSampleHelper : public QC2ParamCapsHelper {
 public:
    OutputBitsPerSampleHelper(const std::string& codecName,
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
        mDependencies = {};
        auto bitsPerSample = C2Config::PCM_16;
        auto bitsPerSampleSet = defaultBitsPerSampleSet;

        if (isAac() || isAacAdts()) {
            bitsPerSampleSet = std::get<AAC_BITS_PER_SAMPLE_SET>(AacDecMinMaxList[AAC_DEFAULT]);
        } else if (isAmrWbPlus()) {
            bitsPerSampleSet = std::get<AMR_BITS_PER_SAMPLE_SET>(AmrMinMaxList[AMRWBPlus]);
        } else if (isALAC()) {
            bitsPerSampleSet = std::get<ALAC_BITS_PER_SAMPLE_SET>(AlacDecMinMaxList);
        } else if (isAPE()) {
            bitsPerSampleSet = std::get<APE_BITS_PER_SAMPLE_SET>(ApeDecMinMaxList);
        } else if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            bitsPerSampleSet = std::get<WMA_BITS_PER_SAMPLE_SET>(WmaMinMaxList[WMA_DEFAULT]);
            mDependencies = {C2AudioWmaVersionInfo::input::PARAM_TYPE};
        } else {
            return;
        }

        auto convertToC2PcmEncCfg = [](uint32_t& n) { n = sBitsPerSampletoPcmEncoding.at(n); };
        std::for_each(bitsPerSampleSet.begin(), bitsPerSampleSet.end(), convertToC2PcmEncCfg);
        mParam = std::make_shared<T>(0, bitsPerSample);
        RETURN_IF_NULL(mParam);
        mRange = {C2F(mParam, value).oneOf(bitsPerSampleSet)};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_PCM_ENCODING;
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
        if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            auto wmaVersion = getDepRef<C2AudioWmaVersionInfo::input>(deps);
            if (!wmaVersion) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(WmaOutputBitsPerSampleSetter, *wmaVersion);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::NonStrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPcmEncodingInfo::output;
    std::shared_ptr<T> mParam;

    static C2R WmaOutputBitsPerSampleSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2AudioWmaVersionInfo::input>& wmaVersion) {
        (void)mayBlock;
        uint32_t version = wmaVersion.v.value;
        if (WmaVersionSet.find(version) == WmaVersionSet.end()) {
            return C2R::BadState();
        }
        auto bitsPerSampleRange = std::get<WMA_BITS_PER_SAMPLE_SET>(WmaMinMaxList[version]);
        if (std::find(bitsPerSampleRange.begin(), bitsPerSampleRange.end(),
                me.v.value) == bitsPerSampleRange.end()) {
            return C2R::BadState();
        }
        return C2R::Ok();
    }
};

//--- C2StreamPcmEncodingInfo::input ------------------------------------------------------------
class InputBitsPerSampleHelper : public QC2ParamCapsHelper {
 public:
    InputBitsPerSampleHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder()) {
            return;
        }
        mDependencies = {};
        auto bitsPerSample = C2Config::PCM_16;
        auto bitsPerSampleSet = defaultBitsPerSampleSet;
        if (isAmrNb()) {
            bitsPerSampleSet = std::get<AMR_BITS_PER_SAMPLE_SET>(AmrMinMaxList[AMRNB]);
        } else if (isAmrWb()) {
            bitsPerSampleSet = std::get<AMR_BITS_PER_SAMPLE_SET>(AmrMinMaxList[AMRWB]);
        } else if (isAac() || isAacAdts()) {
            bitsPerSampleSet = std::get<AAC_BITS_PER_SAMPLE_SET>(AacEncMinMaxList[AAC_DEFAULT]);
            mDependencies = {C2StreamProfileLevelInfo::output::PARAM_TYPE};
        } else {
            // Parameter not supported for other formats
            return;
        }
        auto convertToC2PcmEncCfg = [](uint32_t& n) { n = sBitsPerSampletoPcmEncoding.at(n); };
        std::for_each(bitsPerSampleSet.begin(), bitsPerSampleSet.end(), convertToC2PcmEncCfg);
        mParam = std::make_shared<T>(0, bitsPerSample);
        RETURN_IF_NULL(mParam);
        mRange = {C2F(mParam, value).oneOf(bitsPerSampleSet)};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_PCM_ENCODING;
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
        if (isAac() || isAacAdts()) {
            auto profileType = getDepRef<C2StreamProfileLevelInfo::output>(deps);
            if (!profileType) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(AacInputBitsPerSampleSetter, *profileType);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::NonStrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPcmEncodingInfo::input;
    std::shared_ptr<T> mParam;

    static C2R AacInputBitsPerSampleSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2StreamProfileLevelInfo::output>& profileType) {
        (void)mayBlock;
        uint32_t profile = profileType.v.profile;
        if (C2ProfileToAacProfileIndex.find(profile) == C2ProfileToAacProfileIndex.end()) {
            return C2R::BadState();
        }
        uint32_t profileIndex = C2ProfileToAacProfileIndex.at(profile);
        auto bitsPerSampleRange = std::get<AAC_BITS_PER_SAMPLE_SET>(AacEncMinMaxList[profileIndex]);
        if (std::find(bitsPerSampleRange.begin(), bitsPerSampleRange.end(),
                me.v.value) == bitsPerSampleRange.end()) {
            return C2R::BadState();
        }
        return C2R::Ok();
    }
};

//--- C2StreamBitrateInfo::input ------------------------------------------------------------
class InputBitRateHelper : public QC2ParamCapsHelper {
 public:
    InputBitRateHelper(const std::string& codecName,
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
        mDependencies = {};
        auto bitRateInfo = 2048;
        auto minBitRate = std::get<AMR_MIN_BIT_RATE>(AmrMinMaxList[AMRNB]);
        auto maxBitRate = std::get<AMR_MAX_BIT_RATE>(AmrMinMaxList[AMRNB]);
        if (isAac() || isAacAdts()) {
            bitRateInfo = AacDecDefaults::kMinBitrate;
            minBitRate = std::get<AAC_MIN_BITRATE>(AacDecMinMaxList[AAC_DEFAULT]);
            maxBitRate = std::get<AAC_MAX_BITRATE>(AacDecMinMaxList[AAC_DEFAULT]);
        } else if (isAmrWbPlus()) {
            bitRateInfo = AmrDefaults::AmrWbPlusDefaultBitRate;
            minBitRate = std::get<AMR_MIN_BIT_RATE>(AmrMinMaxList[AMRWBPlus]);
            maxBitRate = std::get<AMR_MAX_BIT_RATE>(AmrMinMaxList[AMRWBPlus]);
        } else if (isALAC()) {
            bitRateInfo = AlacDefaults::ALACDefaultAvgBitRate;
            minBitRate = std::get<ALAC_MIN_BIT_RATE>(AlacDecMinMaxList);
            maxBitRate = std::get<ALAC_MAX_BIT_RATE>(AlacDecMinMaxList);
        } else if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            bitRateInfo = WmaStdDefaults::kWmaStdDefaultAvgBytesPerSecond * 8;
            minBitRate = std::get<WMA_MIN_BIT_RATE>(WmaMinMaxList[WMA_STD]);
            maxBitRate = std::get<WMA_MAX_BIT_RATE>(WmaMinMaxList[WMA_DEFAULT]);
            mDependencies = {C2AudioWmaVersionInfo::input::PARAM_TYPE};
        } else {
            // Parameter not supported for other formats
            return;
        }
        mParam = std::make_shared<T>(0, bitRateInfo);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(minBitRate, maxBitRate)};
        mName = C2_PARAMKEY_BITRATE;
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
        if (isWmaStd() || isWmaPro() || isWmaLossless()) {
            auto wmaVersion = getDepRef<C2AudioWmaVersionInfo::input>(deps);
            if (!wmaVersion) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(WmaInputBitRateSetter, *wmaVersion);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::NonStrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamBitrateInfo::input;
    std::shared_ptr<T> mParam;

    static C2R WmaInputBitRateSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2AudioWmaVersionInfo::input>& wmaVersion) {
        (void)mayBlock;
        uint32_t version = wmaVersion.v.value;
        if (WmaVersionSet.find(version) == WmaVersionSet.end()) {
            return C2R::BadState();
        }
        if (me.v.value < std::get<WMA_MIN_BIT_RATE>(WmaMinMaxList[version]) ||
                me.v.value > std::get<WMA_MAX_BIT_RATE>(WmaMinMaxList[version])) {
            return C2R::BadState();
        }
        QLOGV("Setting bitRate = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2StreamBitrateInfo::output ------------------------------------------------------------
class OutputBitRateHelper : public QC2ParamCapsHelper {
 public:
    OutputBitRateHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder()) {
            return;
        }
        mDependencies = {};
        auto bitRateInfo = 8000;
        auto minBitRate = 8000;
        auto maxBitRate = 8000;
        if (isAmrNb()) {
            bitRateInfo = AmrDefaults::AmrNbDefaultBitRate;
            minBitRate = std::get<AMR_MIN_BIT_RATE>(AmrMinMaxList[AMRNB]);
            maxBitRate = std::get<AMR_MAX_BIT_RATE>(AmrMinMaxList[AMRNB]);
        } else if(isAmrWb()) {
            bitRateInfo = AmrDefaults::AmrWbDefaultBitRate;
            minBitRate = std::get<AMR_MIN_BIT_RATE>(AmrMinMaxList[AMRWB]);
            maxBitRate = std::get<AMR_MAX_BIT_RATE>(AmrMinMaxList[AMRWB]);
        } else if (isAac() || isAacAdts()) {
            std::tuple<uint32_t, uint32_t> aacBitRateRange;
            if (QC2AacEncConfig::getAacBitRateRange(C2Config::profile_t::PROFILE_AAC_LC,
                        AacDefaults::AacDefaultSampleRate, AacDefaults::AacDefaultNumChannels,
                        aacBitRateRange) == QC2_OK) {
                minBitRate = std::get<AAC_MIN_BITRATE>(aacBitRateRange);
                maxBitRate = std::get<AAC_MAX_BITRATE>(aacBitRateRange);
            } else {
                QLOGW("%s: failed to fetch bitrate range, setting random defaults", __func__);
                minBitRate = 8000;
                maxBitRate = 960000;
            }
            mDependencies = {C2StreamProfileLevelInfo::output::PARAM_TYPE,
                    C2StreamSampleRateInfo::input::PARAM_TYPE,
                    C2StreamChannelCountInfo::input::PARAM_TYPE};
        } else {
            // Parameter not supported for other formats
            return;
        }
        mParam = std::make_shared<T>(0, bitRateInfo);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(minBitRate, maxBitRate)};
        mName = C2_PARAMKEY_BITRATE;
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
        if (isAac() || isAacAdts()) {
            auto profileType = getDepRef<C2StreamProfileLevelInfo::output>(deps);
            auto sampleRate = getDepRef<C2StreamSampleRateInfo::input>(deps);
            auto channelCount = getDepRef<C2StreamChannelCountInfo::input>(deps);
            if (!profileType || !sampleRate || !channelCount) {
                return;
            }
            pb.withDefault(getTempCopy(mParam))
            .withFields({mRange.begin(), mRange.end()})
            .withSetter(AacOutputBitRateSetter, *profileType, *sampleRate, *channelCount);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::NonStrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamBitrateInfo::output;
    std::shared_ptr<T> mParam;

    static C2R AacOutputBitRateSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
                const C2InterfaceHelper::C2P<C2StreamProfileLevelInfo::output>& profileType,
                const C2InterfaceHelper::C2P<C2StreamSampleRateInfo::input>& sampleRate,
                const C2InterfaceHelper::C2P<C2StreamChannelCountInfo::input>& channelCount) {
        (void)mayBlock;
        // ToDo: Add a comment
        uint32_t profile = profileType.v.profile;
        if (C2ProfileToAacProfileIndex.find(profile) == C2ProfileToAacProfileIndex.end()) {
            return C2R::BadState();
        }
        std::tuple<uint32_t, uint32_t> aacBitRateRange;
        if (QC2AacEncConfig::getAacBitRateRange(profile, sampleRate.v.value,
                    channelCount.v.value, aacBitRateRange) != QC2_OK) {
            return C2R::BadState();
        }
        if (me.v.value < std::get<AAC_MIN_BITRATE>(aacBitRateRange) ||
                me.v.value > std::get<AAC_MAX_BITRATE>(aacBitRateRange)) {
            return C2R::BadState();
        }
        QLOGV("Setting bitRate = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};


//--- C2StreamAacFormatInfo::input ---------------------------------------------------------------
class InputAacFormatInfoHelper : public QC2ParamCapsHelper {
 public:
    InputAacFormatInfoHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || (!isAac() && !isAacAdts())) {
            return;
        }
        auto defaultFormat = C2Config::aac_packaging_t::AAC_PACKAGING_RAW;
        std::vector<uint32_t> formats;
        mParam = std::make_shared<T>(
                    0u, static_cast<C2Config::aac_packaging_t>(defaultFormat));
        RETURN_IF_NULL(mParam);
        auto addSupportedFormats = [&](const std::pair<uint32_t, uint16_t>& it)
                                      { formats.push_back(it.first); };
        std::for_each(C2PackagingTypeToAacFormatFlag.begin(), C2PackagingTypeToAacFormatFlag.end(),
                      addSupportedFormats);
        mRange = {C2F(mParam, value).oneOf({formats})};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_AAC_PACKAGING;
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
    using T = C2StreamAacFormatInfo::input;
    std::shared_ptr<T> mParam;
};

//--- C2StreamProfileLevelInfo::input ---------------------------------------------------------------
class InputAacProfileLevelHelper : public QC2ParamCapsHelper {
 public:
    InputAacProfileLevelHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || (!isAac() && !isAacAdts())) {
            return;
        }
        auto defaultProfile = C2Config::profile_t::PROFILE_UNUSED;
        auto defaultLevel = C2Config::level_t::LEVEL_UNUSED;
        std::vector<uint32_t> profiles = {C2Config::profile_t::PROFILE_UNUSED};
        std::vector<uint32_t> levels = {C2Config::level_t::LEVEL_UNUSED};
        mParam = std::make_shared<T>(0u, static_cast<C2Config::profile_t>(defaultProfile),
                static_cast<C2Config::level_t>(defaultLevel));
        RETURN_IF_NULL(mParam);
        auto addSupportedProfiles = [&](const std::pair<uint32_t, uint16_t>& it)
                                      { profiles.push_back(it.first); };
        std::for_each(C2ProfileToAacEncodingMode.begin(), C2ProfileToAacEncodingMode.end(),
                      addSupportedProfiles);
        mRange = {C2F(mParam, profile).oneOf({profiles}),
                C2F(mParam, level).oneOf({levels})};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_PROFILE_LEVEL;
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
            .withSetter(SetterHelper<T>::EmptySetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamProfileLevelInfo::input;
    std::shared_ptr<T> mParam;
};

//--- C2StreamProfileLevelInfo::output ---------------------------------------------------------
class ProfileLevelOutputHelper : public QC2ParamCapsHelper {
 public:
    ProfileLevelOutputHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder() || (!isAac() && !isAacAdts())) {
            return;
        }
        auto defaultProfile = C2Config::profile_t::PROFILE_UNUSED;
        auto defaultLevel = C2Config::level_t::LEVEL_UNUSED;
        std::vector<uint32_t> profiles = sSupportedHwAacEncoderProfiles;
        std::vector<uint32_t> levels = {C2Config::level_t::LEVEL_UNUSED};
        mParam = std::make_shared<T>(0u, static_cast<C2Config::profile_t>(defaultProfile),
                static_cast<C2Config::level_t>(defaultLevel));
        RETURN_IF_NULL(mParam);
        mRange = {C2F(mParam, profile).oneOf({profiles}),
                C2F(mParam, level).oneOf({levels})};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_PROFILE_LEVEL;
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
          .withSetter(ProfileLevelSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamProfileLevelInfo::output;
    std::shared_ptr<T> mParam;

    static C2R ProfileLevelSetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting profile = %" PRIu32 " ,level = %" PRIu32 "", me.v.profile, me.v.level);
        return C2R::Ok();
    }
};

//--- C2StreamAacFormatInfo::output ---------------------------------------------------------
class OutputStreamAacFormatHelper : public QC2ParamCapsHelper {
 public:
    OutputStreamAacFormatHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder() || (!isAac() && !isAacAdts())) {
            return;
        }
        auto defaultFormat = C2Config::aac_packaging_t::AAC_PACKAGING_RAW;
        std::vector<uint32_t> formats;
        mParam = std::make_shared<T>(
                    0u, static_cast<C2Config::aac_packaging_t>(defaultFormat));
        RETURN_IF_NULL(mParam);
        auto addSupportedFormats = [&](const std::pair<uint32_t, uint16_t>& it)
                                      { formats.push_back(it.first); };
        std::for_each(C2PackagingTypeToAacFormatFlag.begin(), C2PackagingTypeToAacFormatFlag.end(),
                      addSupportedFormats);
        mRange = {C2F(mParam, value).oneOf({formats})};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_AAC_PACKAGING;
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
    using T = C2StreamAacFormatInfo::output;
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

//--- C2PortActualDelayTuning::input -------------------------------------------------------
class ActualInputDelayHelper : public QC2ParamCapsHelper {
 public:
    ActualInputDelayHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant,
            ComponentKind kind, const std::shared_ptr<IDeviceQuery> device)
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
            uint32_t variant,
            ComponentKind kind, const std::shared_ptr<IDeviceQuery> device)
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

//--- C2StreamAudioFrameSizeInfo::input ------------------------------------------------------------
class EncoderFrameSizeHelper : public QC2ParamCapsHelper {
 public:
    EncoderFrameSizeHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder()) {
            return;
        }
        mDependencies = {};
        auto frameSize = 0;
        if (isAac() || isAacAdts()) {
            frameSize = std::get<AAC_MIN_FRAME_SIZE>(AacEncMinMaxList[AAC_DEFAULT]);
            mDependencies = {C2StreamProfileLevelInfo::output::PARAM_TYPE};
        } else if (isAmrNb()) {
            frameSize = std::get<AMR_MIN_FRAME_SIZE>(AmrMinMaxList[AMRNB]);
        } else if (isAmrWb()) {
            frameSize = std::get<AMR_MIN_FRAME_SIZE>(AmrMinMaxList[AMRWB]);
        } else if (isEVRC()) {
            frameSize = QC2EvrcEncConfig::getMinFrameSize();
        } else if (isQCELP()) {
            frameSize = QC2QcelpEncConfig::getMinFrameSize();
        } else {
            return;
        }
        mParam = std::make_shared<T>(0, frameSize);
        RETURN_IF_NULL(mParam);
        mRange = {C2F(mParam, value).any()};
        mDefault = C2Param::Copy(*mParam);
        mName = C2_PARAMKEY_AUDIO_FRAME_SIZE;
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
        if (isAac() || isAacAdts()) {
            auto profileType = getDepRef<C2StreamProfileLevelInfo::output>(deps);
            if (!profileType) { return; }
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(AacEncoderFrameSizeGetter, *profileType);
        } else {
            pb.withDefault(getTempCopy(mParam))
                .withFields({mRange.begin(), mRange.end()})
                .withSetter(SetterHelper<T>::NonStrictValueWithNoDeps);
        }
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamAudioFrameSizeInfo::input;
    std::shared_ptr<T> mParam;

    static C2R AacEncoderFrameSizeGetter(bool mayBlock, C2InterfaceHelper::C2P<T> &me,
            const C2InterfaceHelper::C2P<C2StreamProfileLevelInfo::output>& profileType) {
        (void)mayBlock;
        uint32_t profile = profileType.v.profile;
        if (C2ProfileToAacProfileIndex.find(profile) == C2ProfileToAacProfileIndex.end()) {
            return C2R::BadState();
        }
        uint32_t profileIndex = C2ProfileToAacProfileIndex.at(profile);
        // ToDo: Align frame size?
        if (profileIndex == AAC_LC || profileIndex == AAC_HE ||
                profileIndex == AAC_HE_PS) {
            me.set().value = std::get<AAC_MIN_FRAME_SIZE>(AacEncMinMaxList[profileIndex]);
            QLOGV("%s: Setting frame size %u", __func__, me.set().value);
        }
        return C2R::Ok();
    }
};

class ApiFeaturesHelper : public QC2ParamCapsHelper {
 public:
    ApiFeaturesHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        //Add support for same input buffer when 64 bit alignment not needed by hw
        auto values = API_REFLECTION | API_VALUES | API_CURRENT_VALUES | API_DEPENDENCY /*| API_SAME_INPUT_BUFFER*/;
        mParam = std::make_shared<T>(static_cast<C2Config::api_feature_t>(values));
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({values})};
        mName = C2_PARAMKEY_API_FEATURES;
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
    using T = C2ApiFeaturesSetting;
    std::shared_ptr<T> mParam;
};

};
