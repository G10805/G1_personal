/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_COMPONENT_REGISTRY_H_
#define _QC2AUDIO_COMPONENT_REGISTRY_H_

#include <map>

#include "QC2Constants.h"
#include "QC2Codec.h"

namespace qc2audio {

struct QC2CodecEntry;
/**
 * Cached registry of codecs available on the platform
 *
 * This class unifies the entries exposed by different codec plugins (v4l2, vpp ..)
 */
struct QC2CodecRegistry {
    /**
     * @brief Registry entry for each supported codec populated by the codec plugin
     */
    struct Details {
        ComponentKind kind() const { return mKind; }
        std::string inputMime() const { return mInputMime; }
        std::string outputMime() const { return mOutputMime; }
        std::string nickName() const { return mNickName; }
        uint32_t variant() const { return mVariant; }
        uint32_t maxConcurrentInstances() const { return mMaxConcurrentInstances; }

        Details() = default;

     private:
        friend struct QC2CodecRegistry;
        explicit Details(const QC2CodecEntry&);

        ComponentKind mKind = KIND_OTHER;   /// type of codec (ENCODER, DECODER, FILTER)
        std::string mInputMime = "";        /// mime type of input port
        std::string mOutputMime = "";       /// mime type of output port
        std::string mNickName = "";         /// short-name (for logging)
        uint32_t mVariant = QC2CodecVariant::NONE;    /// variant features
        uint32_t mMaxConcurrentInstances
                    = QC2MaxCodecInstances::DEFAULT_MAX;    /// maximum concurrent instances
        // TODO(PC): expose these without bringing in Plugin as a dependency
        // QC2CodecEntry::PipelinedParamIndices mPipelinedParamIndices = {0u, 0u, 0u};
    };

    static QC2Status ListCodecs(const std::list<uint32_t> &filter, std::vector<std::string> *names);

    static bool HasCodec(const std::string& name);

    static QC2Status DescribeCodec(const std::string& name, Details *entry);

    static QC2Status GetCapsHelpersForCodec(const std::string& name,
                 std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>> *capsHelpers);

    // TODO(PC): move to private
    struct Impl;

    static QC2Status BuildCodecForInterface(const std::string& name,
                                            uint32_t sessionId,
                                            std::shared_ptr<C2ComponentInterface> intf,
                                            std::shared_ptr<EventHandler> listener,
                                            std::unique_ptr<QC2Codec> * const codec);

 protected:
    static QC2Status CreateCodec(
            const std::string& name,
            uint32_t sessionId,
            const std::string& upstreamMime,
            const std::string& downstreamMime,
            std::shared_ptr<EventHandler> listener,
            std::unique_ptr<QC2Codec> * const codec);

 private:
    QC2CodecRegistry() = default;
};

};   // namespace qc2audio

#endif  // _QC2AUDIO_COMPONENT_REGISTRY_H_

