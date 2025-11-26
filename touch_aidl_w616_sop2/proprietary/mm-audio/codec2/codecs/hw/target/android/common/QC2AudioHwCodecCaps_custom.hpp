/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "QC2AudioHwCodec.h"
#include "QC2CodecCapsHelper.h"
#include "QC2CodecConfig.h"

#undef LOG_TAG
#define LOG_TAG "CustomCaps"

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
        if (isEncoder() || !isALAC()) return;
        auto maxFrameBytes = 9530;
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
        if (isEncoder() || !isAPE()) return;
        auto compressionLevel = APE_COMPRESSION_LEVEL_NORMAL;
        mParam = std::make_shared<T>(0u, compressionLevel);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({APE_COMPRESSION_LEVEL_FAST,
                                            APE_COMPRESSION_LEVEL_NORMAL,
                                            APE_COMPRESSION_LEVEL_HIGH})};
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
        if (isEncoder() || !isAPE()) return;
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
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || !isAPE()) return;
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
        if (isEncoder() || !isAPE()) return;
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
        if (isEncoder() || !isAPE()) return;
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
        if (isEncoder() || !isAPE()) return;
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

//--- C2AudioMinVocoderRateInfo::input ------------------------------------------------------------
class MinVocoderRateHelper : public QC2ParamCapsHelper {
 public:
    MinVocoderRateHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder() || (!isEVRC() && !isQCELP())) return;
        auto minVocoderRate = VocoderRates::VOCODER_FULL_RATE;
        mParam = std::make_shared<T>(0u, minVocoderRate);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({VOCODER_ONE_EIGHTH_RATE,
                        VOCODER_ONE_FOURTH_RATE, VOCODER_HALF_RATE, VOCODER_FULL_RATE})};
        mName = QC2_PARAMKEY_AUDIO_MIN_VOCODER_RATE;
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
            .withSetter(MinVocoderRateSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMinVocoderRateInfo::input;
    std::shared_ptr<T> mParam;

    static C2R MinVocoderRateSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting min vocoder rate = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioMaxVocoderRateInfo::input ------------------------------------------------------------
class MaxVocoderRateHelper : public QC2ParamCapsHelper {
 public:
    MaxVocoderRateHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder() || (!isEVRC() && !isQCELP())) return;
        auto maxVocoderRate = VocoderRates::VOCODER_FULL_RATE;
        mParam = std::make_shared<T>(0u, maxVocoderRate);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({VOCODER_ONE_EIGHTH_RATE,
                        VOCODER_ONE_FOURTH_RATE, VOCODER_HALF_RATE, VOCODER_FULL_RATE})};
        mName = QC2_PARAMKEY_AUDIO_MAX_VOCODER_RATE;
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
            .withSetter(MaxVocoderRateSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioMaxVocoderRateInfo::input;
    std::shared_ptr<T> mParam;

    static C2R MaxVocoderRateSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting max vocoder rate = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioWmaVersionInfo::input -------------------------------------------------------
class WmaVersionHelper : public QC2ParamCapsHelper {
 public:
    WmaVersionHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || (!isWmaStd() && !isWmaPro() && !isWmaLossless())) {
            return;
        }
        auto wmaVersion = WMA_DEFAULT;
        mParam = std::make_shared<T>(0u, wmaVersion);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({WMA_STD, WMA_PRO, WMA_LOSSLESS})};
        mName = QC2_PARAMKEY_AUDIO_WMA_VERSION;
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
            .withSetter(WmaVersionSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2AudioWmaVersionInfo::input;
    std::shared_ptr<T> mParam;

    static C2R WmaVersionSetter(
            bool mayBlock, C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        QLOGD("Setting wma version = %" PRIu32 "", me.v.value);
        return C2R::Ok();
    }
};

//--- C2AudioWmaConfig::input -------------------------------------------------------
class WmaConfigHelper : public QC2ParamCapsHelper {
 public:
    WmaConfigHelper(const std::string& codecName,
          const std::string& inputMime, const std::string& outputMime,
          uint32_t variant, ComponentKind kind,
          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(kind == KIND_DECODER ? inputMime : outputMime, variant, kind) {
        (void)codecName;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isEncoder() || (!isWmaStd() && !isWmaPro() && !isWmaLossless())) return;
        mParam = std::make_shared<T>();
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        std::vector<uint32_t> bitsPerSampleSet = std::get<WMA_BITS_PER_SAMPLE_SET>(WmaMinMaxList[WMA_PRO]);
        std::vector<uint32_t> formatTagSet = { WMA_V9_STD, WMA_V9_PRO, WMA_V10_PRO,
                                                WMA_V9_LOSSLESS, WMA_V10_LOSSLESS };
        mRange = {C2F(mParam, bitsPerSample).oneOf(bitsPerSampleSet),
                  C2F(mParam, blkAlign).inRange(0, 65535),
                  C2F(mParam, channelMask).inRange(0, 65535),
                  C2F(mParam, formatTag).oneOf(formatTagSet),
                  C2F(mParam, encOptions).inRange(0, 65535),
                  C2F(mParam, advEncOption).inRange(0, 65535),
                  C2F(mParam, advEncOptions2).inRange(0, 65535)};
        mName = QC2_PARAMKEY_AUDIO_WMA_CONFIG;
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
    using T = C2AudioWmaConfig::input;
    std::shared_ptr<T> mParam;
};

};
