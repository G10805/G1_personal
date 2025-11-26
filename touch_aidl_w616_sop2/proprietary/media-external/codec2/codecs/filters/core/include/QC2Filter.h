/*
 *******************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#ifndef _QC2_FILTER_H_
#define _QC2_FILTER_H_

#include <unordered_map>
#include <utility>
#include <algorithm>
#include <memory>
#include <string>
#include <list>
#include <map>

#include "QC2.h"
#include "QC2Codec.h"
#include "QC2FilterCommon.h"


namespace qc2 {
//------------------------------------------------------------------------------
// QC2Filter: Provides a standard implmentation for a bulk of QC2Codec APIs
//------------------------------------------------------------------------------
class QC2Filter : public QC2Codec {

protected:
    struct FilterPortProp {
        uint32_t width;
        uint32_t height;
        uint32_t stride;
        uint32_t scanLines;
        uint32_t format;
    };

    enum FilterPort {
        FILTER_PORT_INPUT,
        FILTER_PORT_OUTPUT,
    };

    enum FilterMode {
        FILTER_MODE_NORMAL,     // Filter operates on input & output buffer
        FILTER_MODE_INPLACE,    // Filter operates only on input buffers
        FILTER_MODE_BYPASS      // Filter operates in bypass mode
    };

    struct FilterBuffer {
        std::shared_ptr<QC2Buffer> buffer;
        std::shared_ptr<QC2Buffer> extradata;

        uint64_t cookieInToOut; //Holds QC2Buffer->Index. Must be copied to all output buffers
        uint64_t cookie;        //Cookie associated with this buffer

        uint64_t timestamp;     //Buffer timestamp
        uint32_t filledLen;     //Filled Length

        bool isCodec_Config;    //Indicates codec_config flag
        bool isValidOutput;     //Use to distinguish bypass vs actual op buffers

        bool isEOS;             //Indicates EOS buffer
        bool isEmpty;           //Indicates filledLen 0 buffer
        bool isPendingOutput;   //Indicates additional outputs would be generated for input
    };

public:
    virtual ~QC2Filter();

protected:
    QC2Filter(const std::string& name,
              const uint32_t sessionId,
              const std::string& instName,
              const std::string& inputMime,
              const std::string& outputMime,
              ComponentKind kind,
              uint32_t variant,
              std::shared_ptr<EventHandler> listener);
public:
//------------------------------------------------------------------------------
// QC2Codec APIs
//------------------------------------------------------------------------------
    QC2Status init(std::unique_ptr<BufferPoolProvider> bufPoolProvider) final override;

    QC2Status configure(const C2Param& param) final override;

    QC2Status queryRequiredInfos(std::list<std::unique_ptr<C2Param>> *params ) const override;

    QC2Status query(C2Param *const param) final override;

    QC2Status start() final override;

    QC2Status stop() final override;

    QC2Status queueInputs(
        const std::list<std::shared_ptr<InputBufferPack>>& inputs,
        ReportConfigFailure_cb configFailureReporter,
        uint32_t *numInputsQueued) final override;

    QC2Status flush() final override;

    QC2Status drain() final override;

    QC2Status reconfigure(std::list<std::unique_ptr<C2Param>> targetConfig) final override;

    QC2Status release() final override;

    QC2Status getCapsHelpers(std::unordered_map<uint32_t,
                             std::shared_ptr<QC2ParamCapsHelper>>* helpers) const final override;

protected:
//------------------------------------------------------------------------------
// Functions to notify events from underlying custom filter
//------------------------------------------------------------------------------
    QC2Status onFilterHandleInputDone(const FilterBuffer& buf);

    QC2Status onFilterHandleOutputDone(const FilterBuffer& buf);

    QC2Status onFilterInPlaceOutputDone(const FilterBuffer& buf);

    QC2Status onFilterHandleReconfigure(
        std::list<std::unique_ptr<C2Param>> targetConfig);

    QC2Status onFilterHandleError(QC2Status errorCode,
                                    const std::string& errorMsg);

protected:
//------------------------------------------------------------------------------
// Helper methods for underlying custom filter
//------------------------------------------------------------------------------
    virtual QC2Status allocateAndRequestOutputBuffer(const FilterPortProp& props,
                                                     FilterBuffer& opBuffer);

    void logBuffer(const std::shared_ptr<QC2Buffer>& buf, const char* prefix);

    int extractFD(const std::shared_ptr<QC2Buffer>& qc2Buf);

    QC2Status updateFilterOperatingMode(FilterMode mode);

    QC2Status addOutputConfigUpdates(std::unique_ptr<C2Param>& param);

private:
//------------------------------------------------------------------------------
// Mandatory methods for underlying custom filter
//------------------------------------------------------------------------------
    virtual QC2Status onInit() = 0;

    virtual QC2Status onStart() = 0;

    virtual QC2Status onStop() = 0;

    virtual QC2Status onFlush(FilterPort port) = 0;

    virtual QC2Status onRelease()  = 0;

    virtual QC2Status onProcessInput(FilterBuffer& buffer) = 0;

    virtual QC2Status onConfigure(const C2Param& param) = 0;

    virtual QC2Status onQuery(C2Param *const param) = 0;

    virtual QC2Status onSessionReconfigure(FilterPortProp& ipPort, FilterPortProp& opPort) = 0;

    virtual QC2Status onGetCapsHelpers(
                    std::unordered_map<uint32_t,
                    std::shared_ptr<QC2ParamCapsHelper>>* helpers) const = 0;

    virtual QC2Status getPortProperties(FilterPort port, FilterPortProp& props) = 0;

    virtual QC2Status setPortProperties(FilterPort port, const FilterPortProp& props) = 0;

    virtual QC2Status setFilterOperatingMode(FilterMode& mode) = 0;

protected:
    //For unit test module
    uint32_t getNumInputsQueued() {
        return mNumInputsQueued;
    }

protected:
    std::string mInstName;
    FilterMode mFilterMode;

private:
//------------------------------------------------------------------------------
// QC2Filter member variables
//------------------------------------------------------------------------------
    DECLARE_NON_COPYASSIGNABLE(QC2Filter);

    std::unique_ptr<BufferPoolProvider> m_bufPoolProvider;

    enum State {
        STATE_FILTER_CREATED,
        STATE_FILTER_INITIALIZED,
        STATE_FILTER_STARTED,
        STATE_FILTER_STOPPED,
        STATE_FILTER_CLOSED,
    };
    State mState;

    enum MsgId {
        MSG_ID_FILTER_START,
        MSG_ID_FILTER_STOP,
        MSG_ID_FILTER_QUEUE_INPUT,
        MSG_ID_FILTER_FLUSH,
        MSG_ID_FILTER_RECONFIGURE,
        MSG_ID_FILTER_DRAIN,
    };

    //Handlers called in EventQueue's context
    QC2Status handleInit();
    QC2Status handleStart();
    QC2Status handleStop();
    QC2Status handleQueueInputs(std::list<std::shared_ptr<InputBufferPack>> *inputs);
    QC2Status handleFlush();
    QC2Status handleReconfigure(std::list<std::unique_ptr<C2Param>> targetConfig);
    QC2Status handleDrain();
    QC2Status handleRelease();

    // Handler to handle messages posted to the EventQueue
    class Handler : public EventHandler {
     public:
        explicit Handler(QC2Filter& _parent) : mCodec(_parent) {}
        virtual ~Handler() = default;
     protected:
        QC2Filter& mCodec;
        QC2Status onEvent(std::shared_ptr<Event> event) override {
            return mCodec.handleMessage(event);
        }
    };
    std::mutex mHandlerLock;
    std::shared_ptr<Handler> mHandler = nullptr;
    QC2Status handleMessage(std::shared_ptr<Event> event);

    //Event Queue
    std::shared_ptr<EventQueue> mEventQueue = nullptr;
    QC2Status startEventQueue();
    QC2Status stopEventQueue();

    //key info in message posted
    struct MsgInfoKey {
        static constexpr const char * kInputs = "inputs";
        static constexpr const char * kReconfig = "reconfig";
    };

    //General Mutex
    std::mutex mLock;

    //Buffer Pools
    std::shared_ptr<QC2GraphicBufferPool> mGraphicPool = nullptr;
    std::unique_ptr<QC2LinearBufferPool> mLinearPool = nullptr;

    //Buffer Lists
    std::shared_ptr<QC2FilterBufferList> mIpBufList;
    std::shared_ptr<QC2FilterBufferList> mOpBufList;

    std::shared_ptr<QC2FilterBufferList> mIpMetadataBufList;
    std::shared_ptr<QC2FilterBufferList> mOpMetadataBufList;

    std::list<std::shared_ptr<InputBufferPack>> mPendingInputs;

    //Buffer flow handling
    QC2Status prepAndQInputBuffer(std::shared_ptr<QC2Buffer> qc2Buf,
                                  std::shared_ptr<QC2Buffer> extraDataBuf);

    QC2Status returnPendingInputs();
    void dispatchInputBuffer(std::shared_ptr<QC2Buffer> inBuf);

    void dispatchOutputBuffer(std::shared_ptr<QC2Buffer> outBuf);
    QC2Status validateOutputBuffer(const std::shared_ptr<QC2Buffer>& output);
    QC2Status dispatchBypassedBuffer(const FilterBuffer& buf);

    //counters
    size_t mNumInputsQueued = 0;
    size_t mNumOutputsQueued = 0;

    uint32_t mFilterOutputCounter = 0;
    bool mbUpdateDelays;

    // Other structs
    struct InputPackMsg : Bundle::Storable {
        std::list<std::shared_ptr<InputBufferPack>> inputs;
    };
    std::unordered_map<uint64_t, std::vector<std::shared_ptr<C2Info>>> mInputExtraData;
    std::unordered_map<uint32_t, std::shared_ptr<QC2Buffer>> mOutputExtraData;

    //Updated infos in output pack
    std::list<std::unique_ptr<C2Param>> mUpdatedInfos;

    //Synchronize access to graphic-pool
    std::shared_ptr<QC2GraphicBufferPool> getGraphicBufferPool() {
        std::lock_guard<std::mutex> lock(mLock);
        return mGraphicPool;
    }
    std::shared_ptr<QC2GraphicBufferPool> updateGraphicBufferPool(
            std::unique_ptr<QC2GraphicBufferPool>&& newPool) {
        std::lock_guard<std::mutex> lock(mLock);
        mGraphicPool = std::move(newPool);
        return mGraphicPool;
    }

    //state variables
    volatile bool mFlushing = false;
    volatile bool mStopping = false;

    //Helper APIs
    QC2Status getTargetReconfigInfo(std::shared_ptr<QC2Buffer> inpBuf,
                                   std::shared_ptr<QC2Codec::ReconfigParamsInfo> reconfigInfo);

    QC2Status checkBufferParams(std::shared_ptr<QC2Buffer> buf, FilterPort port);

    QC2Status configureParam(C2StreamPictureSizeInfo::output* outSize);

    QC2Status configureParam(C2StreamPictureSizeInfo::input* inpSize);

    QC2Status configureParam(C2StreamPixelFormatInfo::output* outFmt);

    QC2Status configureParam(C2StreamPixelFormatInfo::input* inpFmt);

    QC2Status configureParam(C2PortBlockPoolsTuning::output* outBlockPools);

    QC2Status configureParam(C2SecureModeTuning *secureMode);

};
};  // namespace qc2
#endif  // _QC2_FILTER_H_
