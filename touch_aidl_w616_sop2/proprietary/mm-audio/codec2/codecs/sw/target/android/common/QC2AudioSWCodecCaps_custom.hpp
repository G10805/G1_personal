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

#undef LOG_TAG
#define LOG_TAG "SwCustomCaps"

namespace qc2audio {

//--- C2AudioFrameLengthInfo::input ------------------------------------------------------------
class FrameLengthHelper : public QC2ParamCapsHelper {
 public:
    FrameLengthHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isALAC()) return;
        auto frameLength = 4096;
        mParam = std::make_shared<T>(0u, frameLength);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({4096, 8192})};
        mName = QC2_PARAMKEY_AUDIO_FRAME_LENGTH;
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
            .withSetter(FrameLengthSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioFrameLengthInfo::input;
    std::shared_ptr<T> mParam;

    static C2R FrameLengthSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting frameLength = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioPbTuningParam::input ------------------------------------------------------------
class PbTuningHelper : public QC2ParamCapsHelper {
 public:
    PbTuningHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isALAC()) return;
        auto pB = 40;
        mParam = std::make_shared<T>(0u, pB);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0, 255)};
        mName = QC2_PARAMKEY_AUDIO_PB_TUNING;
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
            .withSetter(PbTuningSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioPbTuningParam::input;
    std::shared_ptr<T> mParam;

    static C2R PbTuningSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting pb tuning = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioMbTuningParam::input ------------------------------------------------------------
class MbTuningHelper : public QC2ParamCapsHelper {
 public:
    MbTuningHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isALAC()) return;
        auto mB = 10;
        mParam = std::make_shared<T>(0u, mB);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0, 255)};
        mName = QC2_PARAMKEY_AUDIO_MB_TUNING;
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
            .withSetter(MbTuningSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMbTuningParam::input;
    std::shared_ptr<T> mParam;

    static C2R MbTuningSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting mb tuning = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioKbTuningParam::input ------------------------------------------------------------
class KbTuningHelper : public QC2ParamCapsHelper {
 public:
    KbTuningHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isALAC()) return;
        auto kB = 14;
        mParam = std::make_shared<T>(0u, kB);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0, 255)};
        mName = QC2_PARAMKEY_AUDIO_KB_TUNING;
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
            .withSetter(KbTuningSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioKbTuningParam::input;
    std::shared_ptr<T> mParam;

    static C2R KbTuningSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting mb tuning = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioMinFrameBytesInfo::input ------------------------------------------------------------
class MinFrameBytesHelper : public QC2ParamCapsHelper {
 public:
    MinFrameBytesHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isFLAC()) return;
        auto minFrameBytes = 0;
        mParam = std::make_shared<T>(0u, minFrameBytes);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_MIN_FRAME_BYTES;
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
            .withSetter(MinFrameBytesSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMinFrameBytesInfo::input;
    std::shared_ptr<T> mParam;

    static C2R MinFrameBytesSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting min frame bytes = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioMaxFrameBytesInfo::input ------------------------------------------------------------
class MaxFrameBytesHelper : public QC2ParamCapsHelper {
 public:
    MaxFrameBytesHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isALAC() || !isFLAC()) return;
        auto maxFrameBytes = isALAC() ? 9530 : 0;
        mParam = std::make_shared<T>(0u, maxFrameBytes);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_MAX_FRAME_BYTES;
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
            .withSetter(MaxFrameBytesSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMaxFrameBytesInfo::input;
    std::shared_ptr<T> mParam;

    static C2R MaxFrameBytesSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting max frame bytes = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioMaxRunInfo::input ------------------------------------------------------------
class MaxRunHelper : public QC2ParamCapsHelper {
 public:
    MaxRunHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isALAC()) return;
        auto maxRun = 255;
        mParam = std::make_shared<T>(0u, maxRun);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0, 255)};
        mName = QC2_PARAMKEY_AUDIO_MAX_RUN;
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
            .withSetter(MaxRunSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMaxRunInfo::input;
    std::shared_ptr<T> mParam;

    static C2R MaxRunSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting max run = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioCompressionLevelInfo::input ------------------------------------------------------------
class CompressionLevelHelper : public QC2ParamCapsHelper {
 public:
    CompressionLevelHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isAPE()) return;
        auto compressionLevel = APE_COMPRESSION_LEVEL_NORMAL;
        mParam = std::make_shared<T>(0u, compressionLevel);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({APE_COMPRESSION_LEVEL_FAST,
                                            APE_COMPRESSION_LEVEL_NORMAL,
                                            APE_COMPRESSION_LEVEL_HIGH,
                                            APE_COMPRESSION_LEVEL_EXTRA_HIGH,
                                            APE_COMPRESSION_LEVEL_INSANE})};
        mName = QC2_PARAMKEY_AUDIO_COMPRESSION_LEVEL;
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
            .withSetter(CompressionLevelSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioCompressionLevelInfo::input;
    std::shared_ptr<T> mParam;

    static C2R CompressionLevelSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting compression level = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioFormatFlagsInfo::input ------------------------------------------------------------
class FormatFlagsHelper : public QC2ParamCapsHelper {
 public:
    FormatFlagsHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isAPE()) return;
        auto formatFlags = 0;
        mParam = std::make_shared<T>(0u, formatFlags);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_FORMAT_FLAGS;
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
            .withSetter(FormatFlagsSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioFormatFlagsInfo::input;
    std::shared_ptr<T> mParam;

    static C2R FormatFlagsSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting format flags = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioBlocksPerFrameInfo::input ------------------------------------------------------------
class BlocksPerFrameHelper : public QC2ParamCapsHelper {
 public:
    BlocksPerFrameHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isAPE()) return;
        auto blocksPerFrame = 0;
        mParam = std::make_shared<T>(0u, blocksPerFrame);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_BLOCKS_PER_FRAME;
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
            .withSetter(BlocksPerFrameSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioBlocksPerFrameInfo::input;
    std::shared_ptr<T> mParam;

    static C2R BlocksPerFrameSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting blocks per frame = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioFinalFrameBlocksInfo::input ------------------------------------------------------------
class FinalFrameBlocksHelper : public QC2ParamCapsHelper {
 public:
    FinalFrameBlocksHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isAPE()) return;
        auto finalFrameBlocks = 0;
        mParam = std::make_shared<T>(0u, finalFrameBlocks);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_FINAL_FRAME_BLOCKS;
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
            .withSetter(FinalFrameBlocksSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioFinalFrameBlocksInfo::input;
    std::shared_ptr<T> mParam;

    static C2R FinalFrameBlocksSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting final frame blocks = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioTotalFramesInfo::input ------------------------------------------------------------
class TotalFramesHelper : public QC2ParamCapsHelper {
 public:
    TotalFramesHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isAPE()) return;
        auto totalFrames = 0;
        mParam = std::make_shared<T>(0u, totalFrames);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_TOTAL_FRAMES;
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
            .withSetter(TotalFramesSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioTotalFramesInfo::input;
    std::shared_ptr<T> mParam;

    static C2R TotalFramesSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting total frames = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioSeekTablePresentInfo::input ------------------------------------------------------------
class SeekTableHelper : public QC2ParamCapsHelper {
 public:
    SeekTableHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isAPE()) return;
        auto seekTablePresent = 0;
        mParam = std::make_shared<T>(0u, seekTablePresent);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({0, 1})};
        mName = QC2_PARAMKEY_AUDIO_SEEK_TABLE;
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
            .withSetter(SeekTableSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioSeekTablePresentInfo::input;
    std::shared_ptr<T> mParam;

    static C2R SeekTableSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting seek table present = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioMinBlockSizeInfo::input ------------------------------------------------------------
class MinBlockSizeHelper : public QC2ParamCapsHelper {
 public:
    MinBlockSizeHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isFLAC()) return;
        auto minBlockSize = 16;
        mParam = std::make_shared<T>(0u, minBlockSize);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_MIN_BLOCK_SIZE;
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
            .withSetter(MinBlockSizeSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMinBlockSizeInfo::input;
    std::shared_ptr<T> mParam;

    static C2R MinBlockSizeSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting min block size = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioMaxBlockSizeInfo::input ------------------------------------------------------------
class MaxBlockSizeHelper : public QC2ParamCapsHelper {
 public:
    MaxBlockSizeHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (!isFLAC()) return;
        auto maxBlockSize = 16;
        mParam = std::make_shared<T>(0u, maxBlockSize);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = QC2_PARAMKEY_AUDIO_MAX_BLOCK_SIZE;
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
            .withSetter(MaxBlockSizeSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMaxBlockSizeInfo::input;
    std::shared_ptr<T> mParam;

    static C2R MaxBlockSizeSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting max block size = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioNumOutSamplesPerChannelInfo::output ------------------------------------------------------------
class NumOutSamplesPerChannelHelper : public QC2ParamCapsHelper {
 public:
    NumOutSamplesPerChannelHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant, ComponentKind kind,
            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isDSD()) {
            return;
        }
        auto numOutSamplesPerChannel = 3528;
        auto numOutSamplesPerChannelSet = defaultBitsPerSampleSet;
        mParam = std::make_shared<T>(0, numOutSamplesPerChannel);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        if (isDSD()) {
            numOutSamplesPerChannelSet =
                std::get<DSD_NUM_OUT_SAMPLES_PER_CHANNEL_SET>(DsdDecMinMaxList);
        }  else {
            return;
        }
        mRange = {C2F(mParam, value).oneOf(numOutSamplesPerChannelSet)};
        mName = QC2_PARAMKEY_AUDIO_NUM_OUT_SAMPLES_PER_CHANNEL;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_READ_ONLY ;
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
    using T = C2AudioNumOutSamplesPerChannelInfo::output;
    std::shared_ptr<T> mParam;
};


}; // namespace qc2audio
