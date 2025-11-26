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
#include "QC2ColorConvertFilterCapsHelper.h"
#include "QC2ColorConvertFilterConfig.h"
#include "QC2FilterConstants.h"

#undef LOG_TAG
#define LOG_TAG "QC2ColorConvertFilterCapsHelper"

namespace qc2 {

#define filterMime "MimeType::RAW"
using C2R = C2SettingResultsBuilder;
class QC2ColorConvertFilter;

class QC2ColorConvertFilterEnableHelper : public QC2ParamCapsHelper {
public:
    QC2ColorConvertFilterEnableHelper(const std::string& codecName, const std::string& inputMime,
                                      const std::string& outputMime, uint32_t variant, ComponentKind kind,
                                      const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)device;
        (void)codecName;
        (void)inputMime;
        (void)outputMime;

        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>(C2_FALSE);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).inRange(C2_FALSE, C2_TRUE)};

        mName = QC2_PARAMKEY_COLORCONV_FILTER_ENABLED;

        mDependencies = {};
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

        // Enable Filter based on property setting
        if (OS::SystemProperty::GetUInt32("vendor.qc2.filter.colorconvert.enable", 0u) > 0) {
            QLOGD("Enable Filter as property is set");
            mParam->value = C2_TRUE;
        } else {
            QLOGE("Disable Filter as property is not set");
            mParam->value = C2_FALSE;
        }

        auto paramT = reinterpret_cast<std::shared_ptr<T>*>(param);
        auto pb = C2InterfaceHelper::DefineParam(*paramT, mName);
        pb.withDefault(getTempCopy(mParam))
          .withFields({mRange.begin(), mRange.end()})
          .withSetter(FilterEnableSetter);
        applyAttributes(&pb, mAttributes);
        paramHelpers->push_back(pb.build());
    }

private:
    using T = QC2ColorConvertEnableInfo;
    std::shared_ptr<T> mParam;

    static C2R FilterEnableSetter(bool mayBlock, C2InterfaceHelper::C2P<T>& me) {
        (void) mayBlock;

        // Enable Filter based on property setting
        if (OS::SystemProperty::GetUInt32("vendor.qc2.filter.colorconvert.enable", 0u) > 0) {
            QLOGD("Enable Filter based on property from setter()");
            me.set().value = C2_TRUE;
        } else {
            QLOGD("Filter disabled based on property from setter()");
            me.set().value = C2_FALSE;
        }
        return C2R::Ok();
    }
};

//--- C2VideoForceNonUBWCFormatParam::output --------------------------------------------------
class QC2ColorConvertForceNonUBWCFormatHelper : public QC2ParamCapsHelper {
 public:
    QC2ColorConvertForceNonUBWCFormatHelper(const std::string& codecName,
            const std::string& inputMime, const std::string& outputMime,
            uint32_t variant,
            ComponentKind kind, const std::shared_ptr<IDeviceQuery> device)
        : QC2ParamCapsHelper(filterMime, variant, kind) {
        (void)inputMime;
        (void)codecName;
        (void)outputMime;
        (void)device;
        mStructDesc.emplace(T::CORE_INDEX, C2StructDescriptor(static_cast<T*>(nullptr)));
        mParam = std::make_shared<T>();
        RETURN_IF_NULL(mParam);
        mDefault = C2Param::Copy(*mParam);
        mRange = {C2F(mParam, value).oneOf({C2_TRUE, C2_FALSE})};
        mName = QC2_PARAMKEY_VIDEO_FORCENONUBWCFORMAT;
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

        mParam->value = C2_FALSE;
        // Enable Filter based on property setting
        if (OS::SystemProperty::GetUInt32("vendor.qc2.filter.colorconvert.forceNonUBWC", 0u) > 0) {
            QLOGD("Force non UBWC format");
            mParam->value = C2_TRUE;
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
    using T = C2VideoForceNonUBWCFormatParam::output;
    std::shared_ptr<T> mParam;
};


ParamCapsHelperFactories QC2ColorConvertFilterCapsHelper::mSFCapsFactories = {
    CapsHelperFactory<QC2ColorConvertFilterEnableHelper>,
    CapsHelperFactory<QC2ColorConvertForceNonUBWCFormatHelper>
};

};//namespace qc2
