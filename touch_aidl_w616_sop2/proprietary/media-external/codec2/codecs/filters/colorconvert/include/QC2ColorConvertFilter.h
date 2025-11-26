/*
 *******************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#ifndef _QC2_COLORCONVERT_FILTER_H_
#define _QC2_COLORCONVERT_FILTER_H_

#include "QC2Filter.h"
#include "QC2ImageConverter.h"
#include "QC2ColorConvertFilterConfig.h"

namespace qc2 {

class QC2ColorConverter;

class QC2ColorConvertFilter : public QC2Filter {

public:
    QC2ColorConvertFilter(const std::string& name,
                          const uint32_t sessionId,
                          const std::string& instName,
                          const std::string& upStreamCodec,
                          const std::string& downStreamCodec,
                          uint32_t variant,
                          std::shared_ptr<EventHandler> listener);
    ~QC2ColorConvertFilter();

private:
    //Component workflow API
    QC2Status onInit() override;

    QC2Status onStart() override;

    QC2Status onStop() override;

    QC2Status onFlush(FilterPort port) override;

    QC2Status onRelease() override;

    QC2Status onProcessInput(FilterBuffer& buffer) override;

    QC2Status onConfigure(const C2Param& param) override;

    QC2Status onQuery(C2Param *const param) override;

    QC2Status onGetCapsHelpers(
        std::unordered_map<uint32_t,
        std::shared_ptr<QC2ParamCapsHelper>>* helpers) const override;

    QC2Status onSessionReconfigure(FilterPortProp& ipPort, FilterPortProp& opPort) override;

    //Query Port Properties
    QC2Status getPortProperties(FilterPort port, FilterPortProp& props) override;

    QC2Status setPortProperties(FilterPort port, const FilterPortProp& props) override;

    QC2Status setFilterOperatingMode(FilterMode& mode) override;

private:
    //Helper for in place processing of input buffer
    QC2Status processInPlace(FilterBuffer& buffer);
    QC2Status initConverter();
    uint32_t overrideOutputColorFormat();
    bool forceBypassMode();


private:
    FilterPortProp mInputProps;
    FilterPortProp mOutputProps;
    std::unique_ptr<QC2ImageConverter> mConverter;
    QC2ImageConverter::Capabilities::RotationAngle mRotateAngle;
    QC2ImageConverter::Capabilities::FlipDirection mFlipDirection;
    bool mEnableFilter;

};
};
#endif
