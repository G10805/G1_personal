/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_QC2CODEC_PLUGIN_H_
#define CODEC2_CODECS_INCLUDE_QC2CODEC_PLUGIN_H_

#include <string>
#include <list>
#include <memory>
#include <vector>
#include <tuple>

#include "QC2.h"
#include "QC2Codec.h"

namespace qc2audio {

/// @addtogroup codec_impl Codec Abstraction Layer
/// @{

/**
 * @brief Abstract factory to construct concrete codec implementation objects
 */
class QC2CodecFactory {
 public:
    virtual ~QC2CodecFactory() = default;

    /**
     * @brief constructs a concrete codec object that matches the request
     *
     * @param[in] sessionId      unique ID of the session (for informative/logging purposes)
     * @param[in] upstreamMime   optional mime of codec pipelined *before* this codec
     * @param[in] downstreamMime optional mime of codec pipelined *before* this codec
     * @param[in] listener       handler to listen to codec's responses
     * @param[out] codec         handle to the codec if created successfully
     * @return QC2_OK Codec was instantiated successfully
     * @return QC2_NOT_FOUND matching codec was not found
     * @return QC2_ERROR something else went wrong
     */
    virtual QC2Status createCodec(
            uint32_t sessionId,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec) = 0;

    virtual std::string name() {
        return "dummy";
    }
};

/**
 * @brief Registry entry for each supported codec populated by the codec plugin
 */
struct QC2CodecEntry {
    std::string mName;
    ComponentKind mKind;        /// type of codec (ENCODER, DECODER, FILTER)
    std::string mInputMime;     /// mime type of input port
    std::string mOutputMime;    /// mime type of output port
    std::string mNickName;      /// short-name (for logging)
    CodecImplType mVariant;   /// mask indicating the flavor of codec

    /**
     * @brief If a codec participates in pipeline, it must expose following mandatory params.
     * -# 1. typedef C2StreamParam<C2Info, bool, kMyPortIsPipelinedIndex> MyPortIsPipelinedInfo;
     * -# 2. typedef C2GlobalParam<C2Info, bool, kMyEnabledIndex> MyEnabledParam;
     * Following struct fields must be populated with the appropriate indices   \n
     * i.e inputPortPipelinedIndex = MyPictureSizeInfo::input::PARAM_TYPE       \n
     *     outputPortPipelinedIndex = MyPictureSizeInfo::output::PARAM_TYPE     \n
     *     enabledIndex = MyEnabledParam::PARAM_TYPE                            \n
     */
    struct PipelinedParamIndices {
        uint32_t inputPortPipelinedIndex = 0u;   /// param indicating input port is pipelined
        uint32_t outputPortPipelinedIndex = 0u;  /// param indicating output port is pipelined
        uint32_t enabledIndex = 0u;     /// index of enabled param
    } mPipelinedParamIndices;

    uint32_t mMaxConcurrentInstances;   /// max supported concurrent instances of codec
};

/**
 * @brief QC2Codec Plugin interface
 *
 * Implementations of QC2Codec must implement this interface and expose GetPlugin API via
 * symbol-name "QC2Codec_GetPlugin"
 * This interface allows enumerating supported codecs and their basic details
 * The plugin shall supply a factory object to instantiate each supported codec
 */
struct QC2CodecPlugin {
    virtual ~QC2CodecPlugin() = default;

    virtual std::string name() = 0;

    virtual uint32_t version() = 0;

    typedef std::tuple<std::string, QC2CodecEntry, std::unique_ptr<QC2CodecFactory>> Entry;
    virtual std::vector<QC2CodecPlugin::Entry> getSupportedCodecs() = 0;

    /**
     * Codec plugins must expose following symbol to get the plugin
     */
    static constexpr const char * kGetPluginFuncName = "QC2Codec_GetPlugin";
    typedef QC2CodecPlugin* (*GetPluginFunc)();
};

/// @}

};  // namespace qc2audio

#endif  // CODEC2_CODECS_INCLUDE_QC2CODEC_PLUGIN_H_
