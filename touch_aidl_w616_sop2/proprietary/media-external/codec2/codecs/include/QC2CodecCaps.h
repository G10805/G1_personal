/*
 **************************************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_QC2CODECCAPABILITIES_H_
#define CODEC2_CODECS_INCLUDE_QC2CODECCAPABILITIES_H_

#include <C2Param.h>
#include <util/C2InterfaceHelper.h>
#include "QC2.h"
#include "QC2CodecCapsHelper.h"
#include <memory>
#include <vector>


namespace qc2 {

/// @addtogroup codec_impl Codec Abstraction Layer
/// @{

/**
 * @brief QTI Parameter capabilities for a codec
 *
 * This class acts as a wrapper for Codec capabilities.
 * This class exposes methods for interface to to populate
 * ParamHelpers for supported parameters.
 */
class QC2CodecCaps {
 public:
    virtual ~QC2CodecCaps() = default;

    explicit QC2CodecCaps(
            const std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>& helpers);

    /// returns pointers to instances of C2Param from QC2HwCaps
    void getSupportedParams(std::vector<std::shared_ptr<C2Param>>* params) const;

    /// returns the paramHelpers
    void getParamHelpers(
            const std::vector<std::shared_ptr<C2Param>*>&    params,
            std::vector<std::shared_ptr<ParamHelper>>*       paramHelpers) const;

    /// returns a vector of C2Param indices which have default values different than firmware
    void getDefaultOverrides(std::vector<C2Param::Index>* params) const;

    /// returns a vector of indices for params that match the flag
    void getParamsOfKind(
            std::vector<C2Param::Index>* params, QC2ParamCapsHelper::platform_flags_t kind) const;

    void getStructDescriptors(std::unordered_map<uint32_t, C2StructDescriptor>* descr) const;

    // debug
    void print() const;

 private:
    QC2Status getConflictFreeOrder(std::vector<uint32_t>* outputList) const;

    std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>> mParamCapsHelpers;

    DECLARE_NON_COPYASSIGNABLE(QC2CodecCaps);
};

/// @ }
};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_QC2CODECCAPABILITIES_H_
