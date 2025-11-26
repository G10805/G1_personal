/*
 **************************************************************************************************
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#define LOG_TAG "MockFilter"
#include "QC2CodecPlugin.h"
#include "QC2Config.h"
#include "QC2MockFilter.h"

namespace qc2 {

const char* const gFilterMime = MimeType::RAW;

constexpr const char * kInputSequenceIndex = "mock-filter-sequence-index"; // uint64_t

namespace {
//--- C2VideoMockSizeMultiplier::output -----------------------------------------------------------
class MockSizeMultiplierHelper : public QC2ParamCapsHelper {
 public:
    MockSizeMultiplierHelper(__attribute__((unused)) const std::string& codecName,
                             __attribute__((unused)) const std::string& inputMime,
                             __attribute__((unused)) const std::string& outputMime,
                              uint32_t variant, ComponentKind kind,
                            __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        mParam = std::make_shared<T>(1);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(1, 4, 1)};
        mName = QC2_PARAMKEY_MOCK_RESIZE;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_PERSISTENT;
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
          .withSetter(MockResizeSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2VideoMockSizeMultiplier::output;
    std::shared_ptr<T> mParam;

    static C2R MockResizeSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
            C2InterfaceHelper::C2P<T>& me) {
        QLOGD("MockResizeSetter called %u", me.v.value);
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (!me.F(me.v.value).supportsAtAll(me.v.value)) {
            QLOGE("requested width is not supported");
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.value)));
            me.set().value = oldMe.v.value;
        }
        return res;
    }
};

//--- C2VideoMockFrameMultiplier::output ----------------------------------------------------------
class MockFrameMultiplierHelper : public QC2ParamCapsHelper {
 public:
    MockFrameMultiplierHelper(__attribute__((unused)) const std::string& codecName,
                              __attribute__((unused)) const std::string& inputMime,
                              __attribute__((unused)) const std::string& outputMime,
                               uint32_t variant, ComponentKind kind,
                          __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        (void)codecName;
        (void)
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        mParam = std::make_shared<T>(1);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(1, 4, 1)};
        mName = QC2_PARAMKEY_MOCK_FRC;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_PERSISTENT;
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
          .withSetter(MockFRCSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2VideoMockFrameMultiplier::output;
    std::shared_ptr<T> mParam;

    static C2R MockFRCSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
            C2InterfaceHelper::C2P<T>& me) {
        QLOGD("MockFRCSetter called %u", me.v.value);
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (!me.F(me.v.value).supportsAtAll(me.v.value)) {
            QLOGE("requested width is not supported");
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.value)));
            me.set().value = oldMe.v.value;
        }
        return res;
    }
};

//--- C2VideoMockColorEnhance::output -------------------------------------------------------------
class MockColorEnhanceHelper : public QC2ParamCapsHelper {
 public:
    MockColorEnhanceHelper(__attribute__((unused)) const std::string& codecName,
                           __attribute__((unused)) const std::string& inputMime,
                           __attribute__((unused)) const std::string& outputMime,
                             uint32_t variant, ComponentKind kind,
                           __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        mParam = std::make_shared<T>(C2_FALSE);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({C2_TRUE, C2_FALSE})};
        mName = QC2_PARAMKEY_MOCK_COLORENH;
        mDependencies = {};
        mAttributes = C2ParamDescriptor::IS_PERSISTENT;
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
          .withSetter(MockColorEnhSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2VideoMockColorEnhance::output;
    std::shared_ptr<T> mParam;

    static C2R MockColorEnhSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
            C2InterfaceHelper::C2P<T>& me) {
        (void)oldMe;
        QLOGD("MockColorEnhSetter called %u", me.v.value);
        (void)mayBlock;
        C2R res = C2R::Ok();
        return res;
    }
};

//--- C2PortActualDelayTuning::input -------------------------------------------------------
class ActualInputDelayHelper : public QC2ParamCapsHelper {
 public:
    ActualInputDelayHelper(__attribute__((unused)) const std::string& codecName,
                           __attribute__((unused)) const std::string& inputMime,
                           __attribute__((unused)) const std::string& outputMime,
                            uint32_t variant, ComponentKind kind,
                           __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(5u);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).equalTo(5u)};
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
    ActualOutputDelayHelper(__attribute__((unused)) const std::string& codecName,
                            __attribute__((unused)) const std::string& inputMime,
                            __attribute__((unused)) const std::string& outputMime,
                             uint32_t variant, ComponentKind kind,
                            __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        uint32_t worstOutputDelay = 6u;
        mParam = std::make_shared<T>(worstOutputDelay);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0u, worstOutputDelay)};
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


//--- C2MockFilterEnable -------------------------------------------------------
class MockFilterEnableHelper : public QC2ParamCapsHelper {
 public:
    MockFilterEnableHelper(__attribute__((unused)) const std::string& codecName,
                           __attribute__((unused)) const std::string& inputMime,
                           __attribute__((unused)) const std::string& outputMime,
                            uint32_t variant, ComponentKind kind,
                           __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(C2_FALSE);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(C2_FALSE, C2_TRUE)};

        mName = QC2_PARAMKEY_MOCK_ENABLED;
        mDependencies = {C2VideoMockFrameMultiplier::output::PARAM_TYPE,
                         C2VideoMockSizeMultiplier::output::PARAM_TYPE,
                         C2VideoMockColorEnhance::output::PARAM_TYPE};
        mAttributes = C2ParamDescriptor::IS_REQUIRED | C2ParamDescriptor::IS_PERSISTENT;
        mFlags = platform_flags_t::DEFAULT_DIFFERENT;
        mSupported = true;
    }

    void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const {
        if (deps.size() != mDependencies.size()) {
            return;
        }

        auto mockFrc = getDepRef<C2VideoMockFrameMultiplier::output>(deps);
        if (!mockFrc) {
            return;
        }

        auto mockResize = getDepRef<C2VideoMockSizeMultiplier::output>(deps);
        if (!mockResize) {
            return;
        }

        auto mockColor = getDepRef<C2VideoMockColorEnhance::output>(deps);
        if (!mockColor) {
            return;
        }

        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .withSetter(MockFilterEnableSetter, *mockFrc, *mockResize, *mockColor);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2VideoMockFilterEnabled;
    std::shared_ptr<T> mParam;

    static C2R MockFilterEnableSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me,
                         const C2InterfaceHelper::C2P<C2VideoMockFrameMultiplier::output>& mfrc,
                         const C2InterfaceHelper::C2P<C2VideoMockSizeMultiplier::output>& mResize,
                         const C2InterfaceHelper::C2P<C2VideoMockColorEnhance::output>& mColor) {
        (void)mayBlock;

        if (mfrc.v.value > 1 ||
            mResize.v.value > 1 ||
            mColor.v.value == C2_TRUE) {
            me.set().value = C2_TRUE;
        } else {
            me.set().value = C2_FALSE;
        }
        QLOGD("C2VideoMockFilterEnabled =%u", me.v.value);
        return C2R::Ok();
    }
};

//--- C2ComponentKindSetting -----------------------------------------------------------------------
class KindHelper : public QC2ParamCapsHelper {
 public:
    KindHelper(__attribute__((unused)) const std::string& codecName,
               __attribute__((unused)) const std::string& inputMime,
               __attribute__((unused)) const std::string& outputMime,
                 uint32_t variant, ComponentKind kind,
               __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
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


//--- C2ComponentDomainSetting ---------------------------------------------------------------------
class DomainHelper : public QC2ParamCapsHelper {
 public:
    DomainHelper(__attribute__((unused)) const std::string& codecName,
                 __attribute__((unused)) const std::string& inputMime,
                 __attribute__((unused)) const std::string& outputMime,
                  uint32_t variant, ComponentKind kind,
                 __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
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

//--- C2PortBlockPoolsTuning::output ---------------------------------------------------------------
class BlockPoolOutputHelper : public QC2ParamCapsHelper {
 public:
    BlockPoolOutputHelper(__attribute__((unused)) const std::string& codecName,
                          __attribute__((unused)) const std::string& inputMime,
                          __attribute__((unused)) const std::string& outputMime,
                           uint32_t variant, ComponentKind kind,
                          __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
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

//--- C2StreamPixelFormatInfo::input ---------------------------------------------------------------
class PixelFormatInputHelper : public QC2ParamCapsHelper {
 public:
    PixelFormatInputHelper(__attribute__((unused)) const std::string& codecName,
                           __attribute__((unused)) const std::string& inputMime,
                           __attribute__((unused)) const std::string& outputMime,
                           uint32_t variant, ComponentKind kind,
                           __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder()) {
            return;
        }
        mParam = std::make_shared<T>(0u);
        RETURN_IF_NULL(mParam);
        mParam->value = PixFormat::VENUS_NV12_UBWC;
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({
                PixFormat::VENUS_NV12,
                PixFormat::VENUS_NV12_UBWC,
                PixFormat::VENUS_TP10,
                PixFormat::VENUS_P010
        })};
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
};

//--- C2StreamPixelFormatInfo::output --------------------------------------------------------------
class PixelFormatOutputHelper : public QC2ParamCapsHelper {
 public:
    PixelFormatOutputHelper(__attribute__((unused)) const std::string& codecName,
                            __attribute__((unused)) const std::string& inputMime,
                            __attribute__((unused)) const std::string& outputMime,
                             uint32_t variant, ComponentKind kind,
                            __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u);
        RETURN_IF_NULL(mParam);
        mParam->value = PixFormat::VENUS_NV12_UBWC;
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({
                PixFormat::VENUS_NV12,
                PixFormat::VENUS_NV12_UBWC,
                PixFormat::VENUS_TP10,
                PixFormat::VENUS_P010
        })};
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
        if (me.v.value == PixFormat::OPAQUE) {
            me.set().value = oldMe.v.value;
        }
        QLOGD("%s: Setting pixel format as %u", __func__, me.v.value);
        return C2R::Ok();
    }
};

//--- C2StreamPictureSizeInfo::output --------------------------------------------------------------
class PictureSizeOutputHelper : public QC2ParamCapsHelper {
 public:
    PictureSizeOutputHelper(__attribute__((unused)) const std::string& codecName,
                            __attribute__((unused)) const std::string& inputMime,
                            __attribute__((unused)) const std::string& outputMime,
                             uint32_t variant, ComponentKind kind,
                           __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u, 1920, 1080);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, width).inRange(64, 4096, 2),
                  C2F(mParam, height).inRange(64, 4096, 2)};
        mName = C2_PARAMKEY_PICTURE_SIZE;
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
          .withSetter(PictureSizeOutputSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPictureSizeInfo::output;
    std::shared_ptr<T> mParam;

    static C2R PictureSizeOutputSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
            C2InterfaceHelper::C2P<T>& me) {
        QLOGD("PictureSizeSetter called %ux%u", me.v.width, me.v.height);
        (void)mayBlock;
        C2R res = C2R::Ok();
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

//--- C2StreamPictureSizeInfo::input ---------------------------------------------------------------
class PictureSizeInputHelper : public QC2ParamCapsHelper {
 public:
    PictureSizeInputHelper(__attribute__((unused)) const std::string& codecName,
                           __attribute__((unused)) const std::string& inputMime,
                           __attribute__((unused)) const std::string& outputMime,
                            uint32_t variant, ComponentKind kind,
                           __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u, 1920, 1080);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, width).inRange(64, 4096, 2),
                  C2F(mParam, height).inRange(64, 4096, 2)};
        mName = C2_PARAMKEY_PICTURE_SIZE;
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
          .withSetter(PictureSizeInputSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamPictureSizeInfo::input;
    std::shared_ptr<T> mParam;

    static C2R PictureSizeInputSetter(bool mayBlock, const C2InterfaceHelper::C2P<T>& oldMe,
            C2InterfaceHelper::C2P<T>& me) {
        QLOGD("PictureSizeSetter called %ux%u", me.v.width, me.v.height);
        (void)mayBlock;
        C2R res = C2R::Ok();
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

//--- C2StreamMaxPictureSizeTuning::output ---------------------------------------------------------
class MaxPictureSizeOutputHelper : public QC2ParamCapsHelper {
 public:
    MaxPictureSizeOutputHelper(__attribute__((unused)) const std::string& codecName,
                               __attribute__((unused)) const std::string& inputMime,
                               __attribute__((unused)) const std::string& outputMime,
                                uint32_t variant, ComponentKind kind,
                               __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u, 4096, 2160);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, width).inRange(64, 4096, 2),
                  C2F(mParam, height).inRange(64, 4096, 2)};
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

//--- C2StreamPictureSizeInfo::input ---------------------------------------------------------------
class MaxBufferSizeInputHelper : public QC2ParamCapsHelper {
 public:
    MaxBufferSizeInputHelper(__attribute__((unused)) const std::string& codecName,
                             __attribute__((unused)) const std::string& inputMime,
                             __attribute__((unused)) const std::string& outputMime,
                             uint32_t variant, ComponentKind kind,
                             __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));

        uint32_t bufferSize = 4096*2160*3/4;

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
        if (!maxSizeT) {
            return;
        }

        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .calculatedAs(MaxInputBufferSizeGetter, *maxSizeT);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    static C2R MaxInputBufferSizeGetter(bool mayBlock,
            C2InterfaceHelper::C2P<C2StreamMaxBufferSizeInfo::input>& me,
            const C2InterfaceHelper::C2P<C2StreamMaxPictureSizeTuning::output>& maxSize) {
        QLOGD("MaxInputBufferSizeGetter called");
        (void)mayBlock;
        C2R res = C2R::Ok();
        me.set().value = maxSize.v.width * maxSize.v.height * 3 / 4;
        QLOGD("Getter: buffer size updated to %u", me.v.value);
        return res;
    }
    using T = C2StreamMaxBufferSizeInfo::input;
    std::shared_ptr<T> mParam;
};

//--- C2StreamFrameRateInfo::output ---------------------------------------------------------
class FrameRateOutputHelper : public QC2ParamCapsHelper {
 public:
    FrameRateOutputHelper(__attribute__((unused)) const std::string& codecName,
                          __attribute__((unused)) const std::string& inputMime,
                          __attribute__((unused)) const std::string& outputMime,
                          uint32_t variant, ComponentKind kind,
                          __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        if (isDecoder()) {
            return;
        }
        mParam = std::make_shared<T>(0u, 30);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(1, 480, 1)};
        mName = C2_PARAMKEY_FRAME_RATE;
        mDependencies = {C2StreamPictureSizeInfo::input::PARAM_TYPE};
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
        auto sizeT = getDepRef<C2StreamPictureSizeInfo::input>(deps);
        if (!sizeT) {
            return;
        }

        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .withSetter(FrameRateOutputSetter, *sizeT);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamFrameRateInfo::output;
    std::shared_ptr<T> mParam;

    static C2R FrameRateOutputSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me,
            const C2InterfaceHelper::C2P<C2StreamPictureSizeInfo::input>& size) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        // TODO(SK): Put the actual MB check
        if (size.v.width * size.v.height * me.v.value > 4096*2160*60 || me.v.value > 480) {
            QLOGE("cannot support this framerate");
            res = C2R::BadState();
        }
        QLOGD("FrameRateOutputSetter wxh %ux%u, framerate = %f",
                size.v.width, size.v.height, me.v.value);
        return res;
    }
};

//--- C2StreamFrameRateInfo::input ---------------------------------------------------------
class FrameRateInputHelper : public QC2ParamCapsHelper {
 public:
    FrameRateInputHelper(__attribute__((unused)) const std::string& codecName,
                         __attribute__((unused)) const std::string& inputMime,
                         __attribute__((unused)) const std::string& outputMime,
                         uint32_t variant, ComponentKind kind,
                         __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(0u, 30);
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(0, 480, 1)};
        mName = C2_PARAMKEY_FRAME_RATE;
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
          .withSetter(FrameRateInputSetter, *sizeT);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

 private:
    using T = C2StreamFrameRateInfo::input;
    std::shared_ptr<T> mParam;

    static C2R FrameRateInputSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me,
            const C2InterfaceHelper::C2P<C2StreamPictureSizeInfo::output>& size) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        // TODO(SK): Put the actual MB check
        if (size.v.width * size.v.height * me.v.value > 4096*2160*60 || me.v.value > 480) {
            QLOGE("cannot support this framerate");
            res = C2R::BadState();
        }
        QLOGD("FrameRateOutputSetter wxh %ux%u, framerate = %f",
                size.v.width, size.v.height, me.v.value);
        return res;
    }
};

//--- C2StreamBufferTypeSetting::input -------------------------------------------------------------
class BufferTypeInputHelper : public QC2ParamCapsHelper {
 public:
    BufferTypeInputHelper(__attribute__((unused)) const std::string& codecName,
                          __attribute__((unused)) const std::string& inputMime,
                          __attribute__((unused)) const std::string& outputMime,
                           uint32_t variant, ComponentKind kind,
                          __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
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

//--- C2StreamBufferTypeSetting::output ------------------------------------------------------------
class BufferTypeOutputHelper : public QC2ParamCapsHelper {
 public:
    BufferTypeOutputHelper(__attribute__((unused)) const std::string& codecName,
                           __attribute__((unused)) const std::string& inputMime,
                          __attribute__((unused)) const std::string& outputMime,
                           uint32_t variant, ComponentKind kind,
                          __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
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

//--- C2PortMediaTypeSetting::input ----------------------------------------------------------------
class MediaTypeInputHelper : public QC2ParamCapsHelper {
 public:
    MediaTypeInputHelper(__attribute__((unused)) const std::string& codecName,
                         __attribute__((unused)) const std::string& inputMime,
                         __attribute__((unused)) const std::string& outputMime,
                         uint32_t variant, ComponentKind kind,
                         __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        C2String mimeType = MimeType::RAW;
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
    MediaTypeOutputHelper(__attribute__((unused)) const std::string& codecName,
                          __attribute__((unused)) const std::string& inputMime,
                          __attribute__((unused)) const std::string& outputMime,
                           uint32_t variant, ComponentKind kind,
                          __attribute__((unused)) const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(gFilterMime, variant, kind) {
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

    static C2R MediaTypeOutputSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me) {
        QLOGD("MediaTypeOutputSetter called %s", me.v.m.value);
        (void)me;
        (void)mayBlock;
        C2R res = C2R::Ok();
        return res;
    }
};



ParamCapsHelperFactories mCapsFactories = {
    CapsHelperFactory<KindHelper>,
    CapsHelperFactory<DomainHelper>,
    CapsHelperFactory<BlockPoolOutputHelper>,
    CapsHelperFactory<PixelFormatInputHelper>,
    CapsHelperFactory<PixelFormatOutputHelper>,
    CapsHelperFactory<BufferTypeInputHelper>,
    CapsHelperFactory<BufferTypeOutputHelper>,
    CapsHelperFactory<MediaTypeInputHelper>,
    CapsHelperFactory<MediaTypeOutputHelper>,
    CapsHelperFactory<PictureSizeOutputHelper>,
    CapsHelperFactory<PictureSizeInputHelper>,
    CapsHelperFactory<MaxPictureSizeOutputHelper>,
    CapsHelperFactory<MaxBufferSizeInputHelper>,
    CapsHelperFactory<FrameRateOutputHelper>,
    CapsHelperFactory<FrameRateInputHelper>,
    CapsHelperFactory<MockFrameMultiplierHelper>,
    CapsHelperFactory<MockSizeMultiplierHelper>,
    CapsHelperFactory<MockColorEnhanceHelper>,
    CapsHelperFactory<MockFilterEnableHelper>,
    CapsHelperFactory<ActualInputDelayHelper>,
    CapsHelperFactory<ActualOutputDelayHelper>
};

}


//-------------------------------------------------------------------------------------------------
// Mock Codec Plugin Impl
//-------------------------------------------------------------------------------------------------
using ENTRY = QC2CodecEntry;

static QC2CodecEntry::PipelinedParamIndices sPipelineIds;

static std::vector<QC2CodecEntry> sMockCodecEntries = {
    ENTRY({"mock.c2.qti.filter", KIND_FILTER, MimeType::RAW, MimeType::RAW, "filter",
         QC2CodecVariant::CAN_PIPELINE, sPipelineIds}),
};

class QC2MockFilterFactory : public QC2CodecFactory {
 public:
    virtual ~QC2MockFilterFactory() = default;

    QC2MockFilterFactory(std::string name, ComponentKind kind, std::string mime, bool secure)
        : mName(name), mKind(kind), mCompressedMime(mime), mSecure(secure) {
    }

    QC2Status createCodec(
            uint32_t id,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec) override {
        (void)upstreamMime;
        (void)downstreamMime;
        (void)id;
        if (codec == nullptr) {
            return QC2_BAD_ARG;
        }


        *codec = std::make_unique<QC2MockFilter>(mName, mCompressedMime, mKind, mSecure, listener);
        return QC2_OK;
    }

    virtual std::string name() {
        return "QC2MockFilterFactory" + mName;
    }

 private:
    std::string mName;
    ComponentKind mKind;
    std::string mCompressedMime;
    bool mSecure = false;
};


struct QC2MockFilterPlugin : public QC2CodecPlugin {
    virtual ~QC2MockFilterPlugin() = default;

    std::string name() override {
        return "qc2_mock_filter";
    }

    uint32_t version() override {
        return 0;
    }

    std::vector<QC2CodecPlugin::Entry> getSupportedCodecs() override;
};

std::vector<QC2CodecPlugin::Entry> QC2MockFilterPlugin::getSupportedCodecs() {
    QLOGI("getSupportedCodecs");
    std::vector<QC2CodecPlugin::Entry> entries;
    for (const auto& v : sMockCodecEntries) {
        std::unique_ptr<QC2CodecFactory> factory;
        factory = std::unique_ptr<QC2MockFilterFactory>(
                    new QC2MockFilterFactory(v.mName, v.mKind,
                           v.mInputMime, v.mVariant & QC2CodecVariant::SECURE));
        entries.emplace_back(std::make_tuple(v.mName, v, std::move(factory)));
    }

    return entries;
}

extern "C" QC2CodecPlugin * QC2Codec_GetPlugin() {
    return new QC2MockFilterPlugin;
}


//-------------------------------------------------------------------------------------------------
// Mock implementation
//-------------------------------------------------------------------------------------------------
QC2MockFilter::QC2MockFilter(
        const std::string& name,
        const std::string& mime,
        ComponentKind kind,
        bool canBeSecure,
        std::shared_ptr<EventHandler> listener)
    : QC2Codec(name, 1, mime, mime, kind, canBeSecure, listener),
      mName(name),
      mThread(nullptr) {
    mWidth = 1280;
    mHeight = 720;
    mPixFmt = PixFormat::VENUS_NV12;
    mLatency = 2;
    mOutputCounter = 0;

    mStopRequest.store(false);

    mKind = KIND_FILTER;

    QLOGI("QC2MockFilter() : %s ", mName.c_str());
}

QC2MockFilter::~QC2MockFilter() {
    if (mThread) {
        stop();
    }
}

QC2Status QC2MockFilter::init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) {
    if (mFailInit) {
        QLOGE("Return init failure");
        return QC2_BAD_STATE;
    }
    mBufPoolProvider = std::move(bufPoolProvider);
    QLOGI("init()");
    return QC2_OK;
}

QC2Status QC2MockFilter::getCapsHelpers(
        std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers) const {
    if (helpers) {
        for (auto& f : mCapsFactories) {
            auto b = f(mName, gFilterMime, "", QC2CodecVariant::CAN_PIPELINE, mKind, nullptr);
            if (b->isSupported()) {
                helpers->emplace(b->getIndex(), b);
                QLOGD("%s registered", DebugString::C2Param(b->getIndex()).c_str());
            }
        }
    } else {
        return QC2_BAD_ARG;
    }
    return QC2_OK;
}

QC2Status QC2MockFilter::configure(const C2Param& param) {
    QLOGI("configure: core-index=%x baseIndex=%x", param.coreIndex().coreIndex(), param.index());
    switch (param.coreIndex().typeIndex()) {
    case kParamIndexPictureSize:
    {
        C2StreamPictureSizeInfo::output o;
        if (param.index() == o.index()) {
            C2StreamPictureSizeInfo::output *outSize = (C2StreamPictureSizeInfo::output*)&param;
            QLOGI("Configure o/p resolution = %u x %u", outSize->width, outSize->height);
            mWidth = outSize->width;
            mHeight = outSize->height;
            if (mGraphicPool) {
                mGraphicPool->setResolution(mWidth, mHeight);
                mGraphicPool->setFormat(mPixFmt);
            }
        }
        break;
    }
    case kParamIndexPixelFormat:
    {
        C2StreamPixelFormatInfo::input inFmt;
        if (param.index() == inFmt.index()) {
            mPixFmt = ((C2StreamPixelFormatInfo::input*)(&param))->value;
            QLOGI("Configuring input pixel format = %s", PixFormat::Str(mPixFmt));
            if (mGraphicPool) {
                mGraphicPool->setFormat(mPixFmt);
            }
        }
        break;
    }
    case kParamIndexVideoMockColorEnhancer:
    {
        C2VideoMockColorEnhance::output enh;
        if (param.index() == enh.index()) {
            mColorEnhance = ((C2VideoMockColorEnhance::output*)(&param))->value;
            QLOGI("Configuring color enhance = %u", mColorEnhance);
        }
        break;
    }
    case kParamIndexVideoMockFrameMultiplier:
    {
        C2VideoMockFrameMultiplier::output enh;
        if (param.index() == enh.index()) {
            mFRCMultiplier = ((C2VideoMockFrameMultiplier::output*)(&param))->value;
            QLOGI("Configuring FRC factor = %u", mFRCMultiplier);
        }
        break;
    }
    case kParamIndexVideoMockSizeMultiplier:
    {
        C2VideoMockSizeMultiplier::output enh;
        if (param.index() == enh.index()) {
            mUpscaleMultiplier = ((C2VideoMockSizeMultiplier::output*)(&param))->value;
            QLOGI("Configuring Resize factor = %u", mUpscaleMultiplier);
            if (mGraphicPool) {
                mGraphicPool->setResolution(mWidth * mUpscaleMultiplier,
                                            mHeight * mUpscaleMultiplier);
            }
        }
        break;
    }
    default:
        break;
    }

    return QC2_OK;
}

QC2Status QC2MockFilter::queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const {
	(void)params;
    return QC2_OMITTED;
}

QC2Status QC2MockFilter::query(C2Param *const param) {
    (void)param;
    return QC2_OK;
}

QC2Status QC2MockFilter::start() {
    if (mFailStart) {
        QLOGE("Return Start failure");
        return QC2_BAD_STATE;
    }
    QLOGV("starting...");

    // Get handles for different buffer pools
    if (mBufPoolProvider == nullptr) {
        QLOGE("No Provider to request buffer-pools !");
        return QC2_NO_INIT;
    }
    mGraphicPool = mBufPoolProvider->requestGraphicPool(C2BlockPool::BASIC_GRAPHIC);
    if (mGraphicPool == nullptr) {
        QLOGE("Failed to create Graphic buffer pool !");
        return QC2_ERROR;
    }

    mGraphicPool->setUsage(MemoryUsage::HW_CODEC_WRITE | MemoryUsage::CPU_WRITE);
    mGraphicPool->setResolution(mWidth * mUpscaleMultiplier, mHeight * mUpscaleMultiplier);
    mGraphicPool->setFormat(mPixFmt);
    QLOGI("Graphic pool initialized for %u x %u", mWidth , mHeight);

#if 0
    if (mFailStart) {
        QLOGI("Simulation: Forcing an error on start");
        std::shared_ptr<Event> e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_ERROR);
        mListener->postAsync(e);
        return QC2_ERROR;
    }
#endif

    mThread = std::make_unique<std::thread>(QC2MockFilter::threadWrapper, std::ref(*this));
    QLOGV("started...");

    return QC2_OK;
}

QC2Status QC2MockFilter::stop() {
    QLOGV("stopping..");
    {
        std::lock_guard<std::mutex> lock(mLock);
        mStopRequest.store(true);
        mInQ.interrupt();
        mOutQ.interrupt();
    }

    if (mStalled) {
        mStalledCondition.notify_one();
    }

    if (mThread && mThread->joinable()) {
        mThread->join();
    }

    std::lock_guard<std::mutex> lock(mLock);
    mThread = nullptr;
    QLOGV("stopped..");
    return QC2_OK;
}

QC2Status QC2MockFilter::queueInputs(
        const std::list<std::shared_ptr<InputBufferPack>>& inputs,
        ReportConfigFailure_cb configFailureReporter,
        uint32_t *numInputsQueued) {
    (void)configFailureReporter;

    if (numInputsQueued == nullptr) {
        QLOGE("queueInputs: Invalid arg");
        return QC2_ERROR;
    }
    *numInputsQueued = 0;
    for (auto& input : inputs) {
        if (!(input->mInput)) {
            QLOGE("queueInputs: Invalid input buffer");
            return QC2_ERROR;
        }
        if (mKind == KIND_FILTER && input->mInput->isGraphic()) {
            input->mInput->bufferInfo().put(kInputSequenceIndex, (uint64_t)mNumInputsQueued);
            mNumInputsQueued++;
            mInQ.enqueue(input);
        } else {
            QLOGE("queueInputs: incompatible input type!");
            return QC2_ERROR;
        }

        // TODO(PC) apply tunings/infos (assumed to be pre-sanitized by component/intf)
        //   return failure via configFailureReporter to simulate a failure to apply some tuning

        ++(*numInputsQueued);
    }

    // allocate and queue outputs
    return QC2_OK;
}

QC2Status QC2MockFilter::flush() {
    return QC2_OK;
}

QC2Status QC2MockFilter::drain() {
    QLOGD("drain()");
    std::shared_ptr<QC2Buffer> drainBuf;
    mGraphicPool->allocate(&drainBuf);
    if (!drainBuf) {
        QLOGD("drain: Failed to allocate drain buffer");
        return QC2_ERROR;
    }
    drainBuf->bufferInfo().put("drain", (uint32_t)1);
    auto drainInput = std::make_shared<InputBufferPack>(drainBuf);

    mInQ.enqueue(drainInput);
    return QC2_OK;
}

QC2Status QC2MockFilter::reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) {
    QC2Status ret = QC2_OK;
    // 1. apply configuration. This should set new resolution and also configure buffer-pool
    for (auto& param : targetConfig) {
        ret = configure(*param);
        if (ret != QC2_OK) {
            return ret;
        }
    }
    // 2. free previous generation outputs
    mOutQ.flush(nullptr);

    // 3. allocate and queue new-sized buffers
    ret = allocateAndQueueOutputs();

    // 4. signal processor to resume
    {
        std::unique_lock<std::mutex> lock(mLock);
        if (mStalled) {
            mStalledCondition.notify_one();
        }
    }
    return ret;
}

QC2Status QC2MockFilter::release() {
    return QC2_OK;
}

QC2Status QC2MockFilter::threadWrapper(QC2MockFilter& mock) {
    mock.ProcessorLoop();
    return QC2_OK;
}

void QC2MockFilter::ProcessorLoop() {
    QLOGV("ProcessorLoop: begin");
    while (!mStopRequest) {
        // inspect the first input
        std::shared_ptr<InputBufferPack> inPack = nullptr;
        if (mInQ.peek(&inPack) != QC2_OK || !inPack || !(inPack->mInput)) {
            usleep(10000);
            QLOGI("processorLoop...waiting for an input..");
            continue;
        }

        // store and apply tunings
        if (inPack->mTunings.size() != 0) {
            uint64_t inputIndex = inPack->mInput->inputIndex();
            uint64_t sequenceId;
            inPack->mInput->bufferInfo().get(kInputSequenceIndex, &sequenceId);
            mAllTunings[sequenceId] = inPack->mTunings;
            for (auto &param: inPack->mTunings) {
                uint32_t coreIndex = param->coreIndex().coreIndex();
                QLOGV("Got tuning %" PRIu32 " for input index %" PRIu64 " sequence id %" PRIu64,
                      coreIndex, inputIndex, sequenceId);
                if (coreIndex == C2VideoMockFrameMultiplier::CORE_INDEX) {
                    C2VideoMockFrameMultiplier::output frc;
                    if (frc.updateFrom(*param)) {
                        mFRCMultiplier = frc.value;
                        QLOGV("Update frc multiplier to %d", (int)frc.value);
                    } else {
                        QLOGE("Failed to get C2VideoMockFrameMultiplier");
                    }
                }
                // TODO(PC) apply other tunings/infos (assumed to be pre-sanitized by component/intf)
                // simulate failing tunings here (that will be equivalent to FW throwing an error)
            }
        }

        if (inPack->mInput->hasBufferInfo("drain")) {
            QLOGD("Draining %u outputs", mOutQ.size());
            dispatchOutput(true /*purge*/);

            std::shared_ptr<InputBufferPack> inPack = nullptr;
            mInQ.dequeue(&inPack);

            continue;
        }

        uint64_t bufFlags = inPack->mInput->flags();

        // dequeue output
        std::shared_ptr<QC2Buffer> outBuf = nullptr;

        if (bufFlags & BufFlag::EMPTY) {
            QLOGE("Empty input buffer");
            outBuf = inPack->mInput;
            mInQ.dequeue(&inPack);
            produceOutput(*inPack, outBuf);
            dispatchOutput(!!(bufFlags & BufFlag::EOS));
            continue;
        }

        bool inputPending = false;

        for (uint32_t repeat = 0; repeat < mFRCMultiplier;) {

            if (allocateAndQueueOutputs() != QC2_OK) {
                QLOGE("Failed to allocate buffer... retry");
                usleep(10000);
                continue;
            }

            QLOGI("processorLoop...waiting for an output..");
            if (mOutQ.dequeue(&outBuf) != QC2_OK || outBuf == nullptr) {
                QLOGE("Can't dequeue an output");
                continue;
            }

            // generate output
            if (produceOutput(*inPack, outBuf) != QC2_OK) {
                return;
            }

            // signal output(s) done
            dispatchOutput(!!(bufFlags & BufFlag::EOS));

            outBuf.reset();
            ++repeat;
        }

        // if mInputPending (i.e input generates multiple outputs), do not dequeue & dispatch input
        if (!inputPending) {
            dispatchInput();
        }

    }
    QLOGV("ProcessorLoop: end");
}

QC2Status QC2MockFilter::allocateAndQueueOutputs() {
    int minCount = static_cast<int>(mLatency) - static_cast<int>(mOutQ.size());
    for (int i = 0; i < minCount; ++i) {
        std::shared_ptr<QC2Buffer> buf;
        QC2Status ret = mGraphicPool->allocate(&buf);
        if (ret == QC2_OK && buf != nullptr && buf->isGraphic()) {
            QLOGV("Graphic output-buf allocated");
            mOutQ.enqueue(buf);
        } else {
            QLOGE("Graphic output-buf allocation failed!");
            return QC2_NO_MEMORY;
        }
    }
    return QC2_OK;
}

QC2Status QC2MockFilter::produceOutput(const InputBufferPack& inPack,
        std::shared_ptr<QC2Buffer> outBuf) {
    if (inPack.mInput == nullptr) {
        QLOGE("produceOutput: Invalid input !");
        return QC2_BAD_ARG;
    }
    if (!outBuf) {
        QLOGE("produceOutput: Invalid output !");
        return QC2_BAD_ARG;
    }

    QC2Buffer& inBuf = *(inPack.mInput);
    uint64_t flags = inBuf.flags() & BufFlag::EOS;
/*
    auto& outModel = mModel->mOutputs.front();
    flags |= (outModel.mInputPending) ? BufFlag::INPUT_PENDING : 0;*/

    outBuf->setInputIndex(inBuf.inputIndex());
    outBuf->setTimestamp(inBuf.timestamp());
    outBuf->setFlags(flags);
    outBuf->setOutputIndex(mOutputCounter++);
    QLOGI("generated output for input=%u", (uint32_t)inBuf.inputIndex());
    mPipeline.push_back(outBuf);
    return QC2_OK;
}

void QC2MockFilter::dispatchOutput(bool purge) {
    size_t sz = mPipeline.size();
    QLOGI("dispatchOutput : %d . sz=%zu", purge, sz);
    int numToGo = purge ? static_cast<int>(sz) : static_cast<int>(sz) - static_cast<int>(mLatency);
    QLOGI("dispatching %d outputs", numToGo);
    if (numToGo < 1) {
        return;
    }
    for (; numToGo > 0; --numToGo) {
        auto buf = mPipeline.front();
        mPipeline.pop_front();

        auto oDone = std::make_shared<OutputBuffersDoneInfo>();
        oDone->mOutputs.push_back(QC2Codec::OutputBufferPack(buf));

        auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_OUTPUT_BUFS_DONE);
        e->info().put(QC2Codec::NotifyKey::kBufsDone, oDone);

        QLOGE("Output done @ %u", (int)(buf->timestamp()));
        mListener->postAsync(e);
    }
}

void QC2MockFilter::dispatchInput(bool purge) {
    uint32_t n = mInQ.size();
    QLOGI("dispatching %u inputs", n);
    if (n < 1) {
        return;
    }
    n = purge ? n : 1;
    for (uint32_t i = 0; i < n; ++i) {
        std::shared_ptr<InputBufferPack> inPack = nullptr;
        mInQ.dequeue(&inPack);
        if (!inPack) {
            QLOGE("bug! : failed to dequeue input buffer");
            return;
        }

        auto iDone = std::make_shared<InputBuffersDoneInfo>();
        iDone->mInputFrameIds = {inPack->mInput->inputIndex()};

        auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_INPUT_BUFS_DONE);
        e->info().put(QC2Codec::NotifyKey::kBufsDone, iDone);

        QLOGI("Returning Input @ %u", (int)inPack->mInput->timestamp());
        mListener->postAsync(e);
    }
}

void QC2MockFilter::dispatchReconfig(uint32_t w, uint32_t h, uint32_t f __unused) {
    // ReconfigParamsInfo kReconfig CODEC_NOTIFY_RECONFIG
    auto sizeParam = std::make_unique<C2StreamPictureSizeInfo::output>();
    sizeParam->width = w;
    sizeParam->height = h;

    auto reconfInfo = std::make_shared<ReconfigParamsInfo>();
    reconfInfo->mTargetConfig.push_back(std::move(sizeParam));

    auto e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_RECONFIG);
    e->info().put(QC2Codec::NotifyKey::kReconfig, reconfInfo);

    QLOGI("Posting reconfig event");
    mListener->postAsync(e);
}

void QC2MockFilter::reportError(QC2Status errorCode, const std::string& errorMsg) {
    std::shared_ptr<Event> e = std::make_shared<Event>(QC2Codec::CODEC_NOTIFY_ERROR);
    e->info().put(QC2Codec::NotifyKey::kErrorCode, errorCode);
    e->info().put(QC2Codec::NotifyKey::kErrorMsg, errorMsg);
    mListener->postAsync(e);
}

}  // namespace qc2

