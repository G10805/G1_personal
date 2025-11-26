/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_AUDIO_HW_H_
#define _QC2AUDIO_AUDIO_HW_H_

#include <fcntl.h>
#include <sys/eventfd.h>
#include <stack>
#include <array>
#include <poll.h>
#include <numeric>
#include <condition_variable>
#include <chrono>
#include <string>
#include <unordered_map>
#include <memory>
#include <utility>
#include <list>
#include "QC2.h"
#include "QC2Config.h"
#include "QC2Buffer.h"
#include "QC2Codec.h"
#include "QC2Thread.h"
#include "QC2EventQueue.h"
#include "QC2AudioHwUtils.h"

namespace qc2audio {
class BufferList;

class QC2AudioHw {
 public:
    class Notifier;
    virtual QC2Status init(Notifier* notifier) = 0;
    //virtual QC2Status registerCallbacks() = 0;
    virtual QC2Status open_session(QC2AudioCommonCodecConfig& inputMediaFmt,
                                QC2AudioCommonCodecConfig& outputMediaFmt,
                                uint64_t** hw_session_handle) = 0;
    virtual QC2Status start_session(uint64_t* hw_session_handle) = 0;
    virtual QC2Status stop_session(uint64_t* hw_session_handle) = 0;
    virtual QC2Status write_data(uint64_t* hw_session_handle, std::shared_ptr<QC2AudioHwBuffer> inputBuffer) = 0;
    virtual QC2Status queue_read_buffer(uint64_t* hw_session_handle, std::shared_ptr<QC2AudioHwBuffer>) = 0;
    virtual QC2Status flush(uint64_t* hw_session_handle) = 0;
    virtual QC2Status drain(uint64_t* hw_session_handle) = 0;
    virtual QC2Status suspend_session(uint64_t* hw_session_handle) = 0;
    virtual QC2Status resume_session(uint64_t* hw_session_handle) = 0;
    virtual QC2Status set_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) = 0;
    virtual QC2Status get_codec_config(uint64_t* hw_session_handle,
                               CodecType codec,
                               std::shared_ptr<QC2AudioCodecConfig>& codecConfig,
                               uint32_t param_index) = 0;
    virtual QC2Status close_session(uint64_t* hw_session_handle) = 0;
    virtual QC2Status deinit() = 0;

    class Notifier {
     public:
        virtual ~Notifier() = default;
        virtual void onInputsDone(std::shared_ptr<QC2AudioHwBuffer> inBuf) = 0;
        virtual void onOutputsDone(std::shared_ptr<QC2AudioHwBuffer> outBuf) = 0;
        virtual void onOutputReferenceReleased(uint64_t outputIndex) = 0;
        virtual void onReconfig(std::list<std::unique_ptr<C2Param>> targetConfig) = 0;
        virtual void onError(QC2Status errorCode, const std::string& errorMsg) = 0;
    };

    QC2AudioHw() {};
    virtual ~QC2AudioHw() = default;

 private:
    const std::string mInstName;
};

/**
 * @brief Factory to construct concrete audio hw  object
 *
 * Constructs concrete audio hw object based on current chipset
 */
class QC2AudioHwFactory {
private:
        enum AudioHwType: uint32_t {
        TYPE_ELITE = 1,           // Elite-based DSP
        TYPE_GECKO = 2,           // Gecko-based DSP
    };

 public:
    /**
     * @brief constructs a concrete audio hw object based on current chipset
     *
     * @param[out] codec    handle to the audio hardware object if created
     *                      successfully
     * @return QC2_OK Audio HW was instantiated successfully
     * @return QC2_ERROR something else went wrong
     */
    static QC2Status CreateHwInstance(
            std::unique_ptr<QC2AudioHw>& hw);
};

class BufferList {
 public:
    BufferList(const std::string& name, int maxBufs);
    QC2Status store(const std::shared_ptr<QC2Buffer>& buf, int& index);
    QC2Status pop(int index, std::shared_ptr<QC2Buffer>& buf);
    size_t listDepth();
    QC2Status waitUntilEmpty(uint32_t timeOutMs);
 private:
    std::string mName;
    std::mutex mLock;   //  mutex for the buffer lists
    std::condition_variable mEmptyCondition;
    std::list<std::pair<int, std::shared_ptr<QC2Buffer>>> mUseBufferList;
    std::list<int> mFreeBufferList;
};

};  // namespace qc2audio


#endif
