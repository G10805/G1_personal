/*
 *******************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#define __C2_GENERATE_GLOBAL_VARS__

#include <unordered_map>
#include "QC2Codec.h"
#include "QC2FilterCapsHelper.h"
#include "QC2FilterConstants.h"

#undef LOG_TAG
#define LOG_TAG "QC2FilterCapsHelper"

namespace qc2 {

#define filterMime "MimeType::RAW"
using C2R = C2SettingResultsBuilder;
class QC2Filter;

//------------------------------------------------------------------------------
// C2ComponentKindSetting
//------------------------------------------------------------------------------
class KindHelper : public QC2ParamCapsHelper {
 public:
    KindHelper(const std::string& codecName, const std::string& inputMime,
               const std::string& outputMime, uint32_t variant, ComponentKind kind,
               const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        auto kindType = C2Component::KIND_OTHER;
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

//------------------------------------------------------------------------------
// C2ComponentDomainSetting
//------------------------------------------------------------------------------
class DomainHelper : public QC2ParamCapsHelper {
 public:
    DomainHelper(const std::string& codecName, const std::string& inputMime,
                 const std::string& outputMime, uint32_t variant, ComponentKind kind,
                 const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        auto domain = C2Component::DOMAIN_VIDEO;
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

//------------------------------------------------------------------------------
// C2SecureModeTuning
//------------------------------------------------------------------------------
class SecureModeHelper : public QC2ParamCapsHelper {
 public:
    SecureModeHelper(const std::string& codecName, const std::string& inputMime,
                     const std::string& outputMime, uint32_t variant, ComponentKind kind,
                     const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        auto sec = isSecure() ? C2Config::secure_mode_t::SM_READ_PROTECTED :
                C2Config::secure_mode_t::SM_UNPROTECTED;
        mParam = std::make_shared<T>(sec);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({sec})};
        mName = C2_PARAMKEY_SECURE_MODE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = isSecure() ? platform_flags_t::DEFAULT_DIFFERENT : 0;
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
    using T = C2SecureModeTuning;
    std::shared_ptr<T> mParam;
};

//------------------------------------------------------------------------------
// C2PortBlockPoolsTuning::output
//------------------------------------------------------------------------------
class BlockPoolOutputHelper : public QC2ParamCapsHelper {
 public:
    BlockPoolOutputHelper(const std::string& codecName, const std::string& inputMime,
                          const std::string& outputMime, uint32_t variant, ComponentKind kind,
                          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        // For KIND_FILTER I/O are Graphic Buffers
        C2BlockPool::local_id_t outputPoolIds[] = {C2BlockPool::BASIC_GRAPHIC};
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

//------------------------------------------------------------------------------
// C2StreamPixelFormatInfo::Input
//------------------------------------------------------------------------------
class PixelFormatInputHelper : public QC2ParamCapsHelper {
 public:
    PixelFormatInputHelper(const std::string& codecName, const std::string& inputMime,
                           const std::string& outputMime, uint32_t variant, ComponentKind kind,
                           const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        // DRUNWAL: This list can also be expanded and kept in constants ?
        uint32_t nDefault = PixFormat::VENUS_NV12_UBWC;
        std::vector<uint32_t> values = {
                PixFormat::VENUS_NV12,
                PixFormat::VENUS_NV12_UBWC};

        mParam = std::make_shared<T>(0u);
        RETURN_IF_NULL(mParam);
        mParam->value = nDefault;
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({values})};
        mName = C2_PARAMKEY_PIXEL_FORMAT;
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
    using T = C2StreamPixelFormatInfo::input;
    std::shared_ptr<T> mParam;
// DRUNWAL: Need setters for this ?
};

//------------------------------------------------------------------------------
// C2StreamPixelFormatInfo::output
//------------------------------------------------------------------------------
class PixelFormatOutputHelper : public QC2ParamCapsHelper {
 public:
    PixelFormatOutputHelper(const std::string& codecName, const std::string& inputMime,
                            const std::string& outputMime, uint32_t variant, ComponentKind kind,
                            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        uint32_t nDefault = PixFormat::VENUS_NV12_UBWC;
        // DRUNWAL: This list should be expanded and refer'd from Constants ?
        std::vector<uint32_t> values = {
                PixFormat::VENUS_NV12,
                PixFormat::VENUS_NV12_UBWC,
                PixFormat::VENUS_TP10,
                PixFormat::VENUS_P010};

        mParam = std::make_shared<T>(0u);
        RETURN_IF_NULL(mParam);
        mParam->value = nDefault;
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({values})};
        mName = C2_PARAMKEY_PIXEL_FORMAT;
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
          .withSetter(PixelFormatOutputSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPixelFormatInfo::output;
    std::shared_ptr<T> mParam;

    static C2R PixelFormatOutputSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
            C2InterfaceHelper::C2P<T> &me) {
        (void)mayBlock;
        // DRUNWAL: Fix this setter!
        if (me.v.value == PixFormat::OPAQUE) {
            me.set().value = oldMe.v.value;
        }
        QLOGD("%s: Setting pixel format as %u", __func__, me.v.value);
        return C2R::Ok();
    }
};

//------------------------------------------------------------------------------
// C2StreamBufferTypeSetting::input
//------------------------------------------------------------------------------
class BufferTypeInputHelper : public QC2ParamCapsHelper {
 public:
    BufferTypeInputHelper(const std::string& codecName, const std::string& inputMime,
                          const std::string& outputMime, uint32_t variant, ComponentKind kind,
                          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2BufferData::type_t bufferType = C2BufferData::GRAPHIC;

        mParam = std::make_shared<T>(0u, bufferType);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({C2BufferData::GRAPHIC})};
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

//------------------------------------------------------------------------------
// C2StreamBufferTypeSetting::output
//------------------------------------------------------------------------------
class BufferTypeOutputHelper : public QC2ParamCapsHelper {
 public:
    BufferTypeOutputHelper(const std::string& codecName, const std::string& inputMime,
                           const std::string& outputMime, uint32_t variant, ComponentKind kind,
                           const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        C2BufferData::type_t bufferType = C2BufferData::GRAPHIC;
        mParam = std::make_shared<T>(0u, bufferType);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({C2BufferData::GRAPHIC})};
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

//------------------------------------------------------------------------------
// C2PortMediaTypeSetting::input
//------------------------------------------------------------------------------
class MediaTypeInputHelper : public QC2ParamCapsHelper {
 public:
    MediaTypeInputHelper(const std::string& codecName, const std::string& inputMime,
                         const std::string& outputMime, uint32_t variant, ComponentKind kind,
                         const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2String mimeType;

        mimeType = MimeType::RAW;
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

//------------------------------------------------------------------------------
// C2PortMediaTypeSetting::output
//------------------------------------------------------------------------------
class MediaTypeOutputHelper : public QC2ParamCapsHelper {
 public:
    MediaTypeOutputHelper(const std::string& codecName, const std::string& inputMime,
                          const std::string& outputMime, uint32_t variant, ComponentKind kind,
                          const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2String mimeType;

        mimeType = MimeType::RAW;
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

    // DRUNWAL: Fix the setter.
    static C2R MediaTypeOutputSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me) {
        QLOGD("MediaTypeOutputSetter called %s", me.v.m.value);
        (void)me;
        (void)mayBlock;
        C2R res = C2R::Ok();
        return res;
    }
};

//------------------------------------------------------------------------------
// C2StreamPictureSizeInfo::output
//------------------------------------------------------------------------------
class PictureSizeOutputHelper : public QC2ParamCapsHelper {
 public:
    PictureSizeOutputHelper(const std::string& codecName, const std::string& inputMime,
                            const std::string& outputMime, uint32_t variant, ComponentKind kind,
                            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u, FILTER_DEFAULT_OUTPUT_WIDTH,
                                         FILTER_DEFAULT_OUTPUT_HEIGHT);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, width).inRange(FILTER_MIN_OUTPUT_WIDTH, FILTER_MAX_OUTPUT_WIDTH, 1),
                  C2F(mParam, height).inRange(FILTER_MIN_OUTPUT_HEIGHT,
                                              FILTER_MAX_OUTPUT_HEIGHT, 1)};
        mName = C2_PARAMKEY_PICTURE_SIZE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(std::shared_ptr<C2Param>* param,
                        const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
                        std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .withSetter(PictureSizeOutputSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPictureSizeInfo::output;
    std::shared_ptr<T> mParam;

    static C2R PictureSizeOutputSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
                                       C2InterfaceHelper::C2P<T>& me) {
        (void)mayBlock;
        C2R res = C2R::Ok();

        QLOGD("%s: Setting resolution %ux%u", __func__, me.v.width, me.v.height);
        if (!me.F(me.v.width).supportsAtAll(me.v.width)) {
            QLOGE("requested width is not supported");
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.width)));
            me.set().width = oldMe.v.width;
        }
        if (!me.F(me.v.height).supportsAtAll(me.v.height)) {
            QLOGE("requested height is not supported");
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.height)));
            me.set().height = oldMe.v.height;
        }
        return res;
    }
};

//------------------------------------------------------------------------------
// C2StreamPictureSizeInfo::input
//------------------------------------------------------------------------------
class PictureSizeInputHelper : public QC2ParamCapsHelper {
 public:
    PictureSizeInputHelper(const std::string& codecName, const std::string& inputMime,
                           const std::string& outputMime, uint32_t variant, ComponentKind kind,
                           const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        mParam = std::make_shared<T>(0u, FILTER_DEFAULT_INPUT_WIDTH,
                                         FILTER_DEFAULT_INPUT_HEIGHT);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, width).inRange(FILTER_MIN_INPUT_WIDTH, FILTER_MAX_INPUT_WIDTH, 1),
                  C2F(mParam, height).inRange(FILTER_MIN_INPUT_HEIGHT, FILTER_MAX_INPUT_HEIGHT, 1)};
        mName = C2_PARAMKEY_PICTURE_SIZE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(std::shared_ptr<C2Param>* param,
                        const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
                        std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }
        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .withSetter(PictureSizeInputSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPictureSizeInfo::input;
    std::shared_ptr<T> mParam;

    static C2R PictureSizeInputSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
                                      C2InterfaceHelper::C2P<T>& me) {
        (void)mayBlock;
        C2R res = C2R::Ok();

        QLOGD("%s: Setting resolution %ux%u", __func__, me.v.width, me.v.height);
        if (!me.F(me.v.width).supportsAtAll(me.v.width)) {
            QLOGE("requested width is not supported");
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.width)));
            me.set().width = oldMe.v.width;
        }
        if (!me.F(me.v.height).supportsAtAll(me.v.height)) {
            QLOGE("requested height is not supported");
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.height)));
            me.set().height = oldMe.v.height;
        }
        return res;
    }
};

//------------------------------------------------------------------------------
// C2StreamMaxPictureSizeTuning::output
//------------------------------------------------------------------------------
class MaxPictureSizeOutputHelper : public QC2ParamCapsHelper {
 public:
    MaxPictureSizeOutputHelper(const std::string& codecName, const std::string& inputMime,
                               const std::string& outputMime, uint32_t variant, ComponentKind kind,
                               const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        mParam = std::make_shared<T>(0u, FILTER_MAX_INPUT_WIDTH,
                                         FILTER_MAX_INPUT_HEIGHT);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, width).inRange(FILTER_MIN_INPUT_WIDTH, FILTER_MAX_INPUT_WIDTH, 1),
                  C2F(mParam, height).inRange(FILTER_MIN_INPUT_HEIGHT, FILTER_MAX_INPUT_HEIGHT, 1)};

        mName = C2_PARAMKEY_MAX_PICTURE_SIZE;
        mDependencies = {C2StreamPictureSizeInfo::output::PARAM_TYPE};
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
        auto sizeT = getDepRef<C2StreamPictureSizeInfo::output>(deps);
        if (!sizeT) {
            return;
        }

        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .withSetter(MaxPictureSizeOutputSetter, *sizeT);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamMaxPictureSizeTuning::output;
    std::shared_ptr<T> mParam;

    static C2R MaxPictureSizeOutputSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me,
            const C2InterfaceHelper::C2P<C2StreamPictureSizeInfo::output>& size) {
        QLOGD("MaxPictureSizeSetter called %ux%u", me.v.width, me.v.height);
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (size.v.width > me.v.width || size.v.height > me.v.height) {
            QLOGE("max size must equal to/greater than output size");
            res = C2R::BadState();
        }
        return res;
    }
};

//------------------------------------------------------------------------------
// C2StreamMaxBufferSizeInfo::input
//------------------------------------------------------------------------------
class MaxBufferSizeInputHelper : public QC2ParamCapsHelper {
 public:
    MaxBufferSizeInputHelper(const std::string& codecName, const std::string& inputMime,
                             const std::string& outputMime, uint32_t variant, ComponentKind kind,
                             const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        uint32_t bufferSize = 0;
        bufferSize = (FILTER_MAX_INPUT_WIDTH * FILTER_MAX_INPUT_HEIGHT * 3) / 4;

        mParam = std::make_shared<T>(0u, bufferSize);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).any()};
        mName = C2_PARAMKEY_INPUT_MAX_BUFFER_SIZE;
        mDependencies = {C2StreamMaxPictureSizeTuning::output::PARAM_TYPE,
                C2PortMediaTypeSetting::input::PARAM_TYPE,
                C2StreamProfileLevelInfo::input::PARAM_TYPE};
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

        auto maxSizeT = getDepRef<C2StreamMaxPictureSizeTuning::output>(deps);
        auto codecTypeT = getDepRef<C2PortMediaTypeSetting::input>(deps);
        auto profileLevelT = getDepRef<C2StreamProfileLevelInfo::input>(deps);
        if (!maxSizeT || !codecTypeT || !profileLevelT) {
            return;
        }

        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .calculatedAs(MaxInputBufferSizeGetter, *maxSizeT, *codecTypeT, *profileLevelT);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    static C2R MaxInputBufferSizeGetter(bool mayBlock,
            C2InterfaceHelper::C2P<C2StreamMaxBufferSizeInfo::input>& me,
            const C2InterfaceHelper::C2P<C2StreamMaxPictureSizeTuning::output>& maxSize,
            const C2InterfaceHelper::C2P<C2PortMediaTypeSetting::input>& codecType,
            const C2InterfaceHelper::C2P<C2StreamProfileLevelInfo::input>& profile) {
        (void)mayBlock;
        (void)profile;
        C2R res = C2R::Ok();
        C2String codec = codecType.v.m.value;
        if (maxSize.v.width <= FILTER_MAX_INPUT_WIDTH) {
             me.set().value = maxSize.v.width * maxSize.v.height * 3 / 4;
        }
        QLOGD("MaxInputBufferSizeGetter: buffer size updated to %u", me.v.value);
        return res;
    }
    using T = C2StreamMaxBufferSizeInfo::input;
    std::shared_ptr<T> mParam;
};

//------------------------------------------------------------------------------
// C2PortActualDelayTuning::input
//------------------------------------------------------------------------------
class ActualInputDelayHelper : public QC2ParamCapsHelper {
 public:
    ActualInputDelayHelper(const std::string& codecName, const std::string& inputMime,
                           const std::string& outputMime, uint32_t variant, ComponentKind kind,
                           const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(FILTER_INPUT_DELAY);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0u, FILTER_INPUT_DELAY)};
        mName = C2_PARAMKEY_INPUT_DELAY;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(std::shared_ptr<C2Param>* param,
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

//------------------------------------------------------------------------------
// C2PortActualDelayTuning::output
//------------------------------------------------------------------------------
class ActualOutputDelayHelper : public QC2ParamCapsHelper {
 public:
    ActualOutputDelayHelper(const std::string& codecName, const std::string& inputMime,
                            const std::string& outputMime, uint32_t variant, ComponentKind kind,
                            const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        std::string systemCodename =
                OS::SystemProperty::Get("vendor.media.system.build_codename", "11");
        // Start from lower output delay only if system is not Android-R (codename 11)
        uint32_t outputDelay = systemCodename.compare("11") ? 1u : 10u;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(outputDelay);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0u, 10u/*FILTER_OUTPUT_DELAY*/)};
        mName = C2_PARAMKEY_OUTPUT_DELAY;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = 0;
        mSupported = true;
    }

    void getParamHelper(std::shared_ptr<C2Param>* param,
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

ParamCapsHelperFactories QC2FilterCapsHelper::mSFCapsFactories = {
    CapsHelperFactory<KindHelper>,
    CapsHelperFactory<DomainHelper>,
    CapsHelperFactory<SecureModeHelper>,
    CapsHelperFactory<MediaTypeInputHelper>,
    CapsHelperFactory<MediaTypeOutputHelper>,
    CapsHelperFactory<ActualInputDelayHelper>,
    CapsHelperFactory<ActualOutputDelayHelper>,
};

};  // namespace qc2
