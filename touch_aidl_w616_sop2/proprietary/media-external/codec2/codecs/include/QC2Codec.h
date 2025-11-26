/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_QC2CODEC_H_
#define CODEC2_CODECS_INCLUDE_QC2CODEC_H_

#include <string>
#include <list>
#include <memory>
#include "QC2.h"
#include "QC2Constants.h"
#include "QC2EventQueue.h"
#include "QC2Buffer.h"
#include "QC2MetadataTranslator.h"
#include "QC2CodecCapsHelper.h"
#include <C2Work.h>

namespace qc2 {

/// @addtogroup codec_impl Codec Abstraction Layer
/// @{

/**
 * @brief Interface that abstracts a Codec's implmentation
 *
 * Codec models a generic data-processor and is used by Component to handle codec-specific tasks\n
 * Codec is backed by different flavors of implementation : _v4l2, dsp, vpp_ ..\n
 * Each implementation-type may support different kinds (encode/decode/filter) and coding-types\n
 *   eg: v4l2-based-avc-encoder / dsp-based-mpeg4-decoder.\n
 * Codec does not have the notion of Component's state. It simply responds to the api invoked
 * by the parent Component.\n\n
 * Codec notifies the Component of any async events via a Listener:\n
 * -# CODEC_NOTIFY_RECONFIG - to notify that either input or output port static-settings changed\n
 *        Priority = NORMAL |  info -> kReconfig : ReconfigParamsInfo
 * -# CODEC_NOTIFY_CONFIG_UPDATE - to notify framework that port static-settings changed\n
 *        Priority = NORMAL |  info -> kConfUpdate : ConfigUpdateParamsInfo
 * -# CODEC_NOTIFY_OUTPUT_BUFS_DONE - to notify that Output(s)/Output-meta(s) is(are) available\n
 *        Priority = NORMAL |  info -> kBufsDone : OutputBuffersDoneInfo
 * -# CODEC_NOTIFY_INPUT_BUFS_DONE - to notify that input(s) is(are) consumed/flushed\n
 *        Priority = NORMAL |  info -> kBufsDone : InputBuffersDoneInfo
 * -# CODEC_NOTIFY_ERROR - to signal any irrecoverable error\n
 *        Priority = HIGH   |  info -> kErrorCode : {uint32_t (QC2Status)} \n
 *                                     kErrorMsg  : {std::string} (optional) \n
 */
class QC2Codec {
 public:
    /// Keys for infos added in notification events
    struct NotifyKey {
        static constexpr const char * kBufsDone = "buffers";  ///< key for Out/InputBuffersDoneInfo
        static constexpr const char * kBufsDrop = "bufdrop";  ///< key for InputBuffersDroppedInfo
        static constexpr const char * kReconfig = "reconfig";   ///< key for ReconfigParamsInfo
        static constexpr const char * kConfUpdate = "confupdate";   ///< key for ConfigUpdateParamsInfo
        static constexpr const char * kErrorCode = "error-code";    ///< key for error-code
        static constexpr const char * kErrorMsg = "error-msg";      ///< key for error-message
    };

    /// Event-ID for responses posted by Codec to the listener
    enum NotifyEventId : uint32_t {
        CODEC_NOTIFY_OUTPUT_BUFS_DONE = 0x100,  ///< notify produced/flushed output/metadata\n
                                                /// kBufsDone : {OutputBuffersDoneInfo}
        CODEC_NOTIFY_INPUT_BUFS_DONE = 0x101,   ///< notify consumed/flushed input\n
                                                /// kBufsDone : {InputBuffersDoneInfo}
        CODEC_NOTIFY_INPUT_BUFS_DROPPED = 0x102,  ///< notify dropped input\n
                                                /// kBufsDrop : {InputBuffersDroppedInfo}
        CODEC_NOTIFY_RECONFIG = 0x103,          ///< notify output format change\n
                                                /// kReconfig :  ReconfigParamsInfo
        CODEC_NOTIFY_CONFIG_UPDATE = 0x104,     ///< notify output format change\n
                                                /// kReconfig :  ReconfigParamsInfo
        CODEC_NOTIFY_ERROR = 0x105,             ///< notify error\n
                                                /// kErrorCode : {int32_t} (QC2Status) \n
                                                /// kErrorMsg  : {string} (optional msg)
    };

    /**
     * @brief Response packet containing output buffer and updates to configuration
     *
     * Output buffer should contain following in BufferInfo (unless specified optional)
     *  -# "timestamp"            : {uint64_t} : milliseconds
     *  -# "output-frame-index"   : {uint64_t} : output counter
     *  -# "input-frame-index"    : {uint64_t} : index of input that produced this output
     *  -# "flags"                : {uint64_t} : @see BufFlag
     *  -# "decode-timestamp"     : {uint64_t} : (optional) dts for encoder
     *  -# "custom-ordinal"       : {uint64_t} : (optional) if in decode-order\n
     *
     * @note Associated metadata (C2Infos) shall be added to the buffer itself. The Codec must
     * propagate all metadata from input to the corresponding output(s).
     *
     */
    struct OutputBufferPack {
        std::shared_ptr<QC2Buffer> mBuf;      ///< output buffer
        std::vector<std::unique_ptr<C2Param>> mConfigUpdates;    ///< updated configuration

        explicit OutputBufferPack(std::shared_ptr<QC2Buffer> buf)
            : mBuf(buf) {
        }
    };

    /**
     * @brief Payload for output-buf/output-metadata-buf done event (CODEC_NOTIFY_OUTPUT_BUFS_DONE)
     *
     * Added with key = NotifyEventInfoKey::kBufsDone \n
     * @note For Dynamic buffer mode:
     *  -# Codec must keep a shared_ptr<QC2Buffer> in case the buffer is still being referred to
     *  -# Codec must clear the shared_ptr<QC2Buffer> when decoder releases the reference
     */
    struct OutputBuffersDoneInfo : Bundle::Storable {
        std::list<OutputBufferPack> mOutputs;     ///< list of outputs
    };

    /**
     * @brief Payload for input done event (CODEC_NOTIFY_INPUT_BUFS_DONE)
     *
     * Added with key = NotifyEventInfoKey::kBufsDone \n
     */
    struct InputBuffersDoneInfo : Bundle::Storable {
        std::list<uint64_t> mInputFrameIds;  ///< frame-indices of consumed/flushed inputs
    };

    /**
     * @brief Payload for input dropped event (CODEC_NOTIFY_INPUT_BUFS_DROPPED)
     * List of inputs (IDs) that are unprocessed and do not result in an output are returned via
     * this event.
     *
     * Added with key = NotifyEventInfoKey::kBufsDrop \n
     */
    struct InputBuffersDroppedInfo : Bundle::Storable {
        std::list<uint64_t> mDroppedInputFrameIds;  ///< frame-indices of dropped inputs
    };

    /**
     * @brief Payload for signaling listener that codec needs reconfig (CODEC_NOTIFY_RECONFIG)
     *
     * Added with key = NotifyEventInfoKey::kReconfig \n
     * When codec detects that a static parameter has changed and that it needs a reconfiguration
     * before proceeding, it notifies the handler and provides the list of params(settings) that
     * need to be re-applied.\n
     * The implementer of listener (component) is expected to invoke reconfigure(...)
     */
    struct ReconfigParamsInfo : Bundle::Storable {
        std::list<std::unique_ptr<C2Param>> mTargetConfig;   ///< params to reconfigure codec
    };

    /**
     * @brief Payload for signaling listener that codec wants to update configs (CODEC_NOTIFY_CONFIG_UPDATE)
     *
     * Added with key = NotifyEventInfoKey::kConfUpdate \n
     * When codec detects that a static parameter has changed and it wants to update to framework
     * before proceeding, it notifies the handler and provides the list of params(settings) that
     * need to be re-applied.\n
     * The implementer of listener (component) is expected to issue dummyWorkDone to apply configs(...)
     */
    struct ConfigUpdateParamsInfo : Bundle::Storable {
        std::vector<std::unique_ptr<C2Param>> mCodecConfig;   ///< params to update configs
    };

    /**
     * @brief Input packet containing input buffer and per-frame configs/metadata
     *
     * Why not use C2Work here ?
     *  -# The tunings in C2Work are not sanitized. May contain unspported/conflicting params.
     *  -# The tunings in Input packet are sanitized by the interface and updated with best-effort
     *     values. Therefore, the codec can treat all configs as strict\n
     * This packet will be queued to the codec via queueInputs(..) method
     *
     * Associated metadata (C2Infos) shall be added to the buffer itself. The Codec must propagate
     * all metadata from input to the corresponding output(s).
     */
    struct InputBufferPack {
        std::shared_ptr<QC2Buffer> mInput;      ///< input buffer
        std::list<std::shared_ptr<C2Param>> mTunings;    ///< pre-sanitized frame-precise configs

        explicit InputBufferPack(std::shared_ptr<QC2Buffer> buf)
            : mInput(buf) {
        }

     private:
        DECLARE_NON_COPYASSIGNABLE(InputBufferPack);
    };

    /**
     * @brief Base constructor
     *
     * Constructs the base codec
     * @param[in] name          name of the codec/component (listed in the registry)
     * @param[in] sessionId     unique ID of the session (for informative/logging purposes)
     * @param[in] inputMime     mime-type of the input stream(s)
     * @param[in] outputMime    mime-type of the output stream(s)
     * @param[in] kind          instance kind; encoder/decoder/filter
     * @param[in] canBeSecure   true if the codec can be configured in secure mode
     * @param[in] listener      reference to a handler to post replies to
     */
    QC2Codec(const std::string& name,
            uint32_t sessionId,
            const std::string& inputMime,
            const std::string& outputMime,
            ComponentKind kind,
            uint32_t variant,
            std::shared_ptr<EventHandler> listener)
        : mCodecName(name),
          mSessionId(sessionId),
          mInputMime(inputMime),
          mOutputMime(outputMime),
          mKind(kind),
          mVariant(variant),
          mListener(listener),
          mBufPoolProvider(nullptr),
          mGraphicPoolId(C2BlockPool::BASIC_GRAPHIC) {
    }

    virtual ~QC2Codec() = default;

    /**
     *@brief Interface to get Codec name
     */
    const std::string& getCodecName() const {
        return mCodecName;
    }

    ComponentKind getCodecKind() const {
        return mKind;
    }

    /**
     * @brief Get codec-specific metadata translator to read/write metadata.
     *
     * Component relies on the metadata reader/writer to parse/package the codec2 infos/tunings
     * to codec-specific-flattened metadata format.
     * @return default QC2MetadataTranslator or specialized translator that suits the Codec
     */
    virtual std::shared_ptr<QC2MetadataTranslator> getMetadataTranslator() {
        return std::make_shared<QC2MetadataTranslator>(mKind);
    }

    /**
     * @brief Interface to request block-pools
     */
    struct BufferPoolProvider {
        virtual ~BufferPoolProvider() = default;

        /// request linear buffer-pool (for bitstream/metadata)
        virtual std::unique_ptr<QC2LinearBufferPool> requestLinearPool(int64_t poolId) = 0;

        /// request graphic buffer-pool
        virtual std::unique_ptr<QC2GraphicBufferPool> requestGraphicPool(int64_t poolId) = 0;
    };

    /**
     * @brief Open and intialize driver/algorithm.
     *
     * This is a blocking call.
     * Codec possibly retrieves the capabilities for the corresponding components here
     *
     * Codec must request the appropriate buffer-pools. i.e based on
     * the codec's kind, linear (encoder) or graphic (decoder/filter) could be requested.
     * @note Codec may request multiple buffer-pools of same type Eg:
     *  -# 2 graphic pools in case of split-mode for decoder, one each for DPB and OPB.
     *  -# 3 linear pools [1] bitstream(output) [2] input-meta [3] output-meta.
     *
     * @param[in] bufPoolProvider to request buffer-pool(s)
     *
     * @return QC2_OK if the codec was successfully initialized
     */
    virtual QC2Status init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) = 0;

    /**
     * @brief Set a config/parameter
     *
     * Set the supplied config/parameter.
     * @note This method is used to set configs out-of-band. The frame-accurate ones will be queued
     * to the codec along with the input
     * @note The parameter value provided is not interpreted as flexible. Codec shall apply the
     * actual value as-is or may reject if the value is not supported.
     *
     * @param[in]       param       parameter to be applied
     * @param[in,out]   metadata    metadata that corresponds to the input for which is this config
     *                              is bound to be applied
     * @return QC2_OK param was set successfully
     * @return QC2_BAD_INDEX param is not supported
     * @return QC2_BAD_VALUE param value is not supported
     */
    virtual QC2Status configure(const C2Param& param) = 0;

    /**
     * @brief query Infos required by a codec
     *
	 * A codec based on the configuration may need certain Infos
	 * from an upstream codec when pipelined.
	 * @note This method is used to query C2 Infos that the codec is
	 *  	 interested in for its operation.
     *
     * @param[out]      params      list of infos that will be used
     *                              to configure upstream codecs to
     *                              subscribe to those infos.
     *                              C2Params are allocated by the
     *                              codec.
     * @return QC2_OK param was set successfully
	 * @return QC2_BAD_VALUE param value is not supported
	 * @return QC2_OMITTED API not supported.
     */
    virtual QC2Status queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const = 0;

    /**
     * @brief Query a config/parameter
     *
     * Query the requested parameter
     * @return QC2_OK param was queried successfully
     * @return C2_BAD_INDEX param is not supported
     *
     */
    virtual QC2Status query(C2Param *const param) = 0;

    /**
     * @brief Starts the codec and provides buffer-pools for output and metadata buffers
     *
     * This method starts the codec synchronously. After having started successfully, codec must
     * start accepting inputs.
     *
     * @return QC2_OK codec started successfully
     * @return QC2_NO_MEMORY codec failed to allocate internal memory
     * @return QC2_NO_INIT codec was not initialized/configured to be able to start
     * @return QC2_ERROR encountered a fatal error
     */
    virtual QC2Status start() = 0;

    /**
     * @brief Stops the codec
     *
     * This method stops the codec synchronously. Codec must perform following
     * -# driver/algorithm must be flushed internally and all the buffers returned/released
     * -# signal the driver/algorithm to stop\n
     * @return QC2_OK codec was stopped successfully
     * @return QC2_ERROR encountered a fatal error
     */
    virtual QC2Status stop() = 0;

    /// callback for reporting config failures
    using ReportConfigFailure_cb =
            std::function<void(uint64_t /*index*/, const C2Param& /*param*/, QC2Status /*error*/)>;
    /**
     * @brief Queue input-buffer(s) with corresponding config(s) and info(s).
     *
     * For each input, codec must
     *  -# associate infos/tunings and queue them along with the input to be processed together
     *  -# allocate and queue (as many) output buffer(s) required to process the input \n
     * Codec must queue the input buffers to HW/implementation in the same order as provided\n.
     * Infos/infoBuffers provided with the frame (possibly flatten in metabuf) must be associated
     * with the corresponding frame.\n
     * Codec must apply the frame-accurate tunings before processing the input\n
     * Although tunings are pre-sanitized by interface and are not expected to fail, they
     * may still fail (eg: client tried to change a static setting that is not expected to change
     * at run-time). In such cases, Codec must report the failure immediately via
     * configFailureReporter and indicate the {input, parameter, failure} and stop queueing further
     * inputs to the underlying driver and return immediately (Component shall trip here).
     * @note When the buffers are returned by the underlying implementation, codec must return the
     * buffers via CODEC_NOTIFY_INPUT_BUFS_DONE and release refs of the buffers (if any held)
     * @note If list of inputs provided is empty, codec must still re-evaluate if it requires
     *  outputs for queued inputs and try to allocate and queue output buffers.
     *
     * @param[in] inputs    list of inputs to be queued
     * @param[in] configFailureReporter callback to report failure to set a config for an input\n
     *              index : list-index of the buffer for which the config failed to apply\n
     *              param : parameter that failed\n
     *              error : error-code for failure
     * @param[out] numInputsQueued number of inputs that were queued successfully. This must always
     *      match the list-size unless there was an error.
     *
     * @return QC2_OK all inputs (along with infos/configs) and outputs were queued successfully
     * @return QC2_CANNOT_DO if any of the tunings turned out to be unsupported. NOTE: in this case,
     *      exact tuning that failed must be notified via ReportConfigFailure_cb. Component will
     *      move to TRIPPED state and indicate the failed config to the client. Codec must set
     *      numInputsQueued to the number of frame that it was able to queue successfully.
     * @return QC2_NO_MEMORY if allocating an output buffer failed. In this case, component will
     *      move to TRIPPED state and indicate resource error to client.
     * @return QC2_ERROR if any of the args are invalid. Eg: numInputsQueued is null, input-buffer
     *      type does not match the kind (Eg: decoder saw a graphic-buffer as input).. In this case,
     *      component will move to ERROR state
     */
    virtual QC2Status queueInputs(
            const std::list<std::shared_ptr<InputBufferPack>>& inputs,
            ReportConfigFailure_cb configFailureReporter,
            uint32_t *numInputsQueued) = 0;

    /**
     * @brief Flush the codec and wait till flush is done
     *
     * Signal flush to the codec and wait till flush is completed.
     * Codec must do following:
     * -# Completed output must be returned to the listener along with the meta-buffer (if any).
     * -# References of flushed input/output buffers must be reset
     * -# References of flushed metadata buffers must be reset
     *
     * @return QC2_OK flush was successful
     * @return QC2_ERROR an unknown error occurred while trying to flush
     */
    virtual QC2Status flush() = 0;

    /**
     * @brief signal drain to the codec
     *
     * Signal codec to complete processing all input buffers in the pipeline without expecting more
     * inputs. This call does not block
     *
     * @return QC2_OK is signaling drain was sucessful
     * @return QC2_ERROR encountered an unknown error
     */
    virtual QC2Status drain() = 0;

    /**
     * @brief update the input/output port format
     *
     * This is called in response to CODEC_PORT_FORMAT_CHANGED event previously signaled by Codec.
     * Based on the capabilities and situation, the codec may choose to:
     * -# Accept the new format\n
     *     Then codec must update the format, reconfigure itself and return QC2_OK;\n
     *     In this case, component updates it's (interface's) configuration to the new config.
     * -# Does not accept the new format\n
     *     Then codec returns appropriate error (strange, since the codec asked for it!)
     *
     * @param[in] targetConfig updated set of configuration
     * @return QC2_OK if format is accepted
     * @return QC2_BAD_VALUE if format is not accepted
     * @todo(PC) : does the information about what exactly is not supported help ? in case
     *      Component decides to trip ?
     */
    virtual QC2Status reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) = 0;

    /**
     * @brief Release all resources and close the driver
     *
     * This api will be called after stop() has been called. Codec must terminate the session and
     * release all the resource held.
     *
     * @return QC2_OK is release was successful
     * @return QC2_ERROR encountered an error
     */
    virtual QC2Status release() = 0;

    /**
     * @brief Return caps helpers
     *
     * This method will return a map of param index and corresponding caps helpers  for
     * all supported parameteres by this codec
     *
     * @param[out] helpers returns map of index and corresponding CapsHelpers for supported params
     *
     * @return QC2_OK if helpers are returned correctly
     * @return QC2_ERROR if helpers are not returned correctly
     */
    virtual QC2Status getCapsHelpers(
        std::unordered_map<uint32_t, std::shared_ptr<QC2ParamCapsHelper>>* helpers) const = 0;

    /**
     *@brief Interface to get the session id
     */
    virtual uint32_t getSessionId() const {
        return mSessionId;
    }

    /**
     *@brief Interface to get the graphic block pool id
     */
    virtual C2BlockPool::local_id_t getGraphicPoolId() const {
        return mGraphicPoolId;
    }

    /**
     * @brief Sets Listener
     *
     * This method will set a listener
     *
     * @param[in] listener object from client
     *
     * @return QC2_OK if no listeners are set
     * @return QC2_CANNOT_DO if a listener is already set
     */
    QC2Status setListener(std::shared_ptr<EventHandler> listener) {
        if (!mListener) {
            mListener = listener;
            return  QC2_OK;
        }
        return QC2_CANNOT_DO;
    }

 protected:
    const std::string mCodecName;   ///< Name of the codec eg: "c2.qti.avc.decoder"
    const uint32_t mSessionId;      ///< Unique ID of the session
    const std::string mInputMime;   ///< input mime-type of instance eg: "video/avc" / "video/raw"
    const std::string mOutputMime;  ///< mime-type of instance eg: "video/avc" / "video/raw"
    const ComponentKind mKind;      ///< instance kind (encoder/decoder/filter)
    const uint32_t mVariant;  ///< features supported by variant codecs

    std::shared_ptr<EventHandler> mListener;        ///< handler listening to this codec
    std::unique_ptr<BufferPoolProvider> mBufPoolProvider;   ///< buffer-pool provider
    C2BlockPool::local_id_t mGraphicPoolId;  ///< graphic block pool id

 private:
    DECLARE_NON_COPYASSIGNABLE(QC2Codec);
};

/// @}

};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_QC2CODEC_H_
