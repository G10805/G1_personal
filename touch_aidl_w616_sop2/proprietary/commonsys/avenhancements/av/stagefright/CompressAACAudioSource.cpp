/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// #define LOG_NDEBUG 0
#define LOG_TAG "CompressAACAudioSource"

#include <tuple>
#include <functional>
#include <utility>

#include <inttypes.h>
#include <stdlib.h>

#include <binder/IPCThreadState.h>
#include <cutils/properties.h>
#include <media/AidlConversion.h>
#include <media/AudioRecord.h>
#include <media/openmax/OMX_Audio.h>
#include <media/stagefright/AudioSource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MetaDataUtils.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <stagefright/AVExtensions.h>
#include <utils/Log.h>

#include "CompressAACAudioSource.h"

#define MIN_ADTS_LENGTH_TO_PARSE 9
#define ADTS_HEADER_LENGTH 7

namespace std {
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const {
        return std::hash<T1>{}(pair.first) ^ std::hash<T2>{}(pair.second);
    }
};
}  // namespace std

namespace android {

using content::AttributionSourceState;

CompressAACAudioSource::CompressAACAudioSource(
    audio_format_t audioFormat, int32_t audioReqBitRate,
    const audio_attributes_t *attr,
    const AttributionSourceState &attributionSource, uint32_t sampleRate,
    uint32_t channelCount, uint32_t outSampleRate,
    audio_port_handle_t selectedDeviceId,
    audio_microphone_direction_t selectedMicDirection,
    float selectedMicFieldDimension)
    : AudioSource() {
    mAudioFormat = audioFormat;
    mAudioBitRateReq = audioReqBitRate;
    mCompressMetadata = nullptr;
    set(attr, attributionSource, sampleRate, channelCount,
        outSampleRate, selectedDeviceId, selectedMicDirection,
        selectedMicFieldDimension);
}

void CompressAACAudioSource::set(
    const audio_attributes_t *attr,
    const AttributionSourceState &attributionSource, uint32_t sampleRate,
    uint32_t channelCount, uint32_t outSampleRate,
    audio_port_handle_t selectedDeviceId,
    audio_microphone_direction_t selectedMicDirection,
    float selectedMicFieldDimension) {
    mStarted = false;
    mPaused = false;
    mSampleRate = sampleRate;
    mOutSampleRate = outSampleRate > 0 ? outSampleRate : sampleRate;
    mTrackMaxAmplitude = false;
    mStartTimeUs = 0;
    mStopSystemTimeUs = -1;
    mFirstSampleSystemTimeUs = -1LL;
    mLastFrameTimestampUs = 0;
    mMaxAmplitude = 0;
    mPrevSampleTimeUs = 0;
    mInitialReadTimeUs = 0;
    mPCMNumFramesReceived = 0;
    mPCMNumFramesSkipped = 0;
    mNumFramesLost = 0;
    mNumClientOwnedBuffers = 0;
    mNoMoreFramesToRead = false;
    CHECK(channelCount == 1 || channelCount == 2);
    CHECK(sampleRate > 0);

    mRecord = new AudioRecord(
        AUDIO_SOURCE_DEFAULT, sampleRate, mAudioFormat,
        audio_channel_in_mask_from_count(channelCount), attributionSource, 200,
        this, 100 /*notificationFrames*/, AUDIO_SESSION_ALLOCATE,
        AudioRecord::TRANSFER_CALLBACK, /* force to callback to reconstrcut the
                                           encoded frames*/
        AUDIO_INPUT_FLAG_NONE, attr, selectedDeviceId, selectedMicDirection,
        selectedMicFieldDimension);

    // Set caller name so it can be logged in destructor.
    // MediaMetricsConstants.h: AMEDIAMETRICS_PROP_CALLERNAME_VALUE_MEDIA
    mRecord->setCallerName("media");
    mInitCheck = mRecord->initCheck();
    if (mInitCheck != OK) {
        ALOGE("%s: failed to create AudioRecord", __func__);
        mRecord.clear();
        return;
    } else {
        ALOGV("%s: AudioRecord created successfully with session id: %d",
              __func__, mRecord->getSessionId());
    }

    if(mAudioFormat == AUDIO_FORMAT_AAC_LC) {
        mPCMSamplesPerFrame = kAacLcPCMSamplesPerFrame;
    } else {
        mPCMSamplesPerFrame = kHeAacPCMSamplesPerFrame;
    }

    // default bitRate
    mAudioBitRate = kSampleRateToDefaultBitRate.at(mRecord->getSampleRate());

    if (mAudioBitRateReq > 0) setDSPBitRate(mAudioBitRateReq);

    ALOGI(
        "%s:created compress audio source (%p) with %s format, sample rate: "
        "%d, channel count: %d",
        __func__, this, audio_format_to_string(mAudioFormat),
        mRecord->getSampleRate(), mRecord->channelCount());
}

status_t CompressAACAudioSource::setDSPBitRate(int32_t bitRate) {
    status_t err = OK;
    String8 result = kAudioParameterDSPAacBitRate;
    result.appendFormat("=%d;", bitRate);

    err = mRecord->setParameters(result);
    if (err != OK) {
        ALOGE("%s: setParameters failed for CompressAAC audio", __func__);
        return OK;
    } else {
        ALOGV("%s: setParameters success for CompressAAC audio; %s", __func__,
              result.string());
    }

    int32_t DSPBitRate;
    result = mRecord->getParameters(kAudioParameterDSPAacBitRate);
    AudioParameter audioParam = AudioParameter(result);
    if (audioParam.getInt(kAudioParameterDSPAacBitRate, DSPBitRate) ==
        NO_ERROR) {
        mAudioBitRate = DSPBitRate;
        ALOGI("%s: CompressAAC with DSP bitrate: %d", __func__, mAudioBitRate);
    } else {
        ALOGI("%s: CompressAAC with default bitrate: %d", __func__,
              mAudioBitRate);
    }
    return OK;
}

status_t CompressAACAudioSource::setCutOffFrequency(int32_t cutOffFrequency) {
    status_t err = OK;
    String8 result = kAudioParameterDSPAacGlobalCutoffFrequency;
    result.appendFormat("=%d;", cutOffFrequency);

    err = mRecord->setParameters(result);
    if (err != OK) {
        ALOGE("%s: setParameters failed for CompressAAC audio; %s", __func__,
              result.string());
        return FAILED_TRANSACTION;
    } else {
        ALOGV("%s: setParameters success for CompressAAC audio; %s", __func__,
              result.string());
    }
    return OK;
}

CompressAACAudioSource::~CompressAACAudioSource() {
    ALOGV("%s: destroying compress aac audio source (%p)", __func__, this);
    if (mStarted) {
        reset();
    }
    mRecord.clear();
    mRecord = 0;
    mByteStream.clear();
}


status_t CompressAACAudioSource::start(MetaData *params) {
    Mutex::Autolock autoLock(mLock);
    /**
     * source started and not paused
     * */
    if (mStarted && !mPaused) {
        ALOGW("%s: source already started but not paused",__func__);
        return OK;
    }

    /**
     * AudioRecord init check
     * */
    if (mInitCheck != OK) {
        ALOGE("%s: source not initialized yet",__func__);
        return NO_INIT;
    }

    /**
     * resume the source when
     * source is started but paused
     * */
    if (mStarted && mPaused) {
        mPaused = false;
        ALOGV("%s: resume the source", __func__);
        return OK;
    }
    mTrackMaxAmplitude = false;
    mMaxAmplitude = 0;
    mInitialReadTimeUs = 0;
    mStartTimeUs = 0;

    int64_t startTimeUs;
    if (params && params->findInt64(kKeyTime, &startTimeUs)) {
        mStartTimeUs = startTimeUs;
    }

    status_t err = mRecord->start();
    if (err == OK) {
        mStarted = true;
        ALOGV("%s: AudioRecord started with session id: %d", __func__,
              mRecord->getSessionId());
    } else {
        ALOGE("%s: AudioRecord start failed", __func__);
        mRecord.clear();
    }

    return err;
}

void CompressAACAudioSource::releaseQueuedFrames_l() {
    ALOGV("releaseQueuedFrames_l");
    clearBuffersReceived_l();
    clearEncodedBuffers_l();
}

void CompressAACAudioSource::clearBuffersReceived_l() {
    ALOGV("clearBuffersReceived_l");
    while (!mBuffersReceived.empty()) {
        auto it = mBuffersReceived.begin();
        (*it)->release();
        mBuffersReceived.erase(it);
    }
}

void CompressAACAudioSource::clearEncodedBuffers_l() {
    ALOGV("clearEncodedBuffers_l");
    while (!mEncodedBuffersFormed.empty()) {
        auto it = mEncodedBuffersFormed.begin();
        (*it)->release();
        mEncodedBuffersFormed.erase(it);
    }
}

void CompressAACAudioSource::waitOutstandingEncodingFrames_l() {
    ALOGV("waitOutstandingEncodingFrames_l: %" PRId64, mNumClientOwnedBuffers);
    while (mNumClientOwnedBuffers > 0) {
        mFrameEncodingCompletionCondition.wait(mLock);
    }
}

status_t CompressAACAudioSource::reset() {
    Mutex::Autolock autoLock(mLock);

    if (!mStarted) {
        return OK;
    }

    if (mInitCheck != OK) {
        return NO_INIT;
    }

    mStarted = false;
    mStopSystemTimeUs = -1;
    mRecord->stop();

    mNoMoreFramesToRead = true;
    mFrameAvailableCondition.signal();
    waitOutstandingEncodingFrames_l();
    releaseQueuedFrames_l();
    ALOGV("%s: AudioRecord stop called with session id: %d", __func__,
          mRecord->getSessionId());
    mStarted = false;
    return OK;
}

sp<MetaData> CompressAACAudioSource::getFormat() {
    uint16_t profile;

    Mutex::Autolock autoLock(mLock);
    if (mInitCheck != OK) {
        return 0;
    }

    if(mCompressMetadata){
        return mCompressMetadata;
    }
    mCompressMetadata = new MetaData;

    mCompressMetadata->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AAC);
    mCompressMetadata->setInt32(kKeySampleRate, mRecord->getSampleRate());
    mCompressMetadata->setInt32(kKeyChannelCount, mRecord->channelCount());
    mCompressMetadata->setInt32(kKeyMaxInputSize, kMaxBufferSize);
    mCompressMetadata->setInt32(kKeyPcmEncoding, kAudioEncodingPcm16bit);
    mCompressMetadata->setInt32(kKeyBitRate, mAudioBitRate);
    mCompressMetadata->setInt32(kKeyMaxBitRate, mAudioBitRate);

    if(!makeCodecSpecificData()){
        ALOGE("%s: csd creation failed", __func__);
        mCompressMetadata.clear();
        return nullptr;
    }

    ALOGD("%s: metadata success", __func__);
    return mCompressMetadata;
}

bool CompressAACAudioSource::makeCodecSpecificData() {
    std::vector<uint8_t> csd0;
    int csd0size = 0;
    const uint8_t AAC_LC_AOT = 2;
    const uint8_t AAC_HE_AOT = 5;
    const uint8_t AAC_HE_PS_AOT = 29;
    const auto sampleRate = mRecord->getSampleRate();
    const auto channelCount = mRecord->channelCount();

    if (mAudioFormat == AUDIO_FORMAT_AAC_ADTS_LC ||
        mAudioFormat == AUDIO_FORMAT_AAC_LC) {
        csd0size = 2;
        const uint8_t ooooo = 0x1F & AAC_LC_AOT;
        const uint8_t ffff = 0x0F & sampleRateToSampleIdx.at(sampleRate);
        const uint8_t cccc = 0x0F & channelCount;

        // csd0 is of oooo offf fccc c000
        csd0 = {static_cast<uint8_t>((ooooo << 3) | (ffff >> 1)),
                static_cast<uint8_t>((ffff << 7) | (cccc << 3))};

        ALOGV("%s: csd0: {%u,%u}", __func__, csd0[0], csd0[1]);

        mCompressMetadata->setInt32(kKeyAACAOT, AAC_LC_AOT);
    } else if (mAudioFormat == AUDIO_FORMAT_AAC_ADTS_HE_V1) {
        csd0size = 4;

        const std::unordered_map<std::pair<uint32_t, uint32_t>,
                                 std::vector<uint8_t>, std::pair_hash>
            kSampleRateChannelCountToCSD0_1_2 = {
                {{24000, 1}, {44, 139, 8, 0}},  {{24000, 2}, {44, 147, 8, 0}},
                {{32000, 1}, {44, 10, 136, 0}}, {{32000, 2}, {44, 18, 136, 0}},
                {{44100, 1}, {43, 138, 8, 0}},  {{44100, 2}, {43, 146, 8, 0}},
                {{48000, 1}, {43, 9, 136, 0}},  {{48000, 2}, {43, 17, 136, 0}},
            };

        csd0 = kSampleRateChannelCountToCSD0_1_2.at({sampleRate, channelCount});
        if (csd0.size() != csd0size) {
            ALOGE(
                "%s: no codec specific data found for %s format with sample "
                "rate %d and channel count %d",
                __func__, audio_format_to_string(mAudioFormat), sampleRate,
                channelCount);
            return false;
        }

        ALOGV("%s: csd0: {%u,%u,%u,%u}", __func__, csd0[0], csd0[1], csd0[2],
              csd0[3]);

        mCompressMetadata->setInt32(kKeyAACAOT, AAC_HE_AOT);

    } else if (mAudioFormat == AUDIO_FORMAT_AAC_ADTS_HE_V2) {
        csd0size = 4;

        const std::unordered_map<uint32_t, std::vector<uint8_t>>
            kSampleRateToCSD0_1_2 = {
                {24000, {236, 139, 8, 0}},
                {32000, {236, 10, 136, 0}},
                {44100, {235, 138, 8, 0}},
                {48000, {235, 9, 136, 0}},
            };

        csd0 = kSampleRateToCSD0_1_2.at(sampleRate);
        if (csd0.size() != csd0size) {
            ALOGE(
                "%s: no codec specific data found for %s format with sample "
                "rate %d and channel count %d",
                __func__, audio_format_to_string(mAudioFormat), sampleRate,
                channelCount);
            return false;
        }

        ALOGV("%s: csd0: {%u,%u,%u,%u}", __func__, csd0[0], csd0[1], csd0[2],
              csd0[3]);

        mCompressMetadata->setInt32(kKeyAACAOT, AAC_HE_PS_AOT);
    }

    std::vector<char> esds(csd0size + 31);
    reassembleESDS(csd0, esds.data());
    mCompressMetadata->setData(kKeyESDS, kTypeESDS, esds.data(), esds.size());
    return true;
}

void CompressAACAudioSource::reassembleESDS(const std::vector<uint8_t> &csd0,
                                            char *esds) {
    int csd0size = csd0.size();
    esds[0] = 3;  // kTag_ESDescriptor;
    int esdescriptorsize = 26 + csd0size;
    CHECK(esdescriptorsize < 268435456);  // 7 bits per byte, so max is 2^28-1
    esds[1] = 0x80 | (esdescriptorsize >> 21);
    esds[2] = 0x80 | ((esdescriptorsize >> 14) & 0x7f);
    esds[3] = 0x80 | ((esdescriptorsize >> 7) & 0x7f);
    esds[4] = (esdescriptorsize & 0x7f);
    esds[5] = esds[6] = 0;  // es id
    esds[7] = 0;            // flags
    esds[8] = 4;            // kTag_DecoderConfigDescriptor
    int configdescriptorsize = 18 + csd0size;
    esds[9] = 0x80 | (configdescriptorsize >> 21);
    esds[10] = 0x80 | ((configdescriptorsize >> 14) & 0x7f);
    esds[11] = 0x80 | ((configdescriptorsize >> 7) & 0x7f);
    esds[12] = (configdescriptorsize & 0x7f);
    esds[13] = 0x40;  // objectTypeIndication
    // bytes 14-25 are examples from a real file. they are unused/overwritten by
    // muxers.
    esds[14] = 0x15;  // streamType(5), upStream(0),
    esds[15] = 0x00;  // 15-17: bufferSizeDB (6KB)
    esds[16] = 0x18;
    esds[17] = 0x00;
    esds[18] = 0x00;  // 18-21: maxBitrate (64kbps)
    esds[19] = 0x00;
    esds[20] = 0xfa;
    esds[21] = 0x00;
    esds[22] = 0x00;  // 22-25: avgBitrate (64kbps)
    esds[23] = 0x00;
    esds[24] = 0xfa;
    esds[25] = 0x00;
    esds[26] = 5;  // kTag_DecoderSpecificInfo;
    esds[27] = 0x80 | (csd0size >> 21);
    esds[28] = 0x80 | ((csd0size >> 14) & 0x7f);
    esds[29] = 0x80 | ((csd0size >> 7) & 0x7f);
    esds[30] = (csd0size & 0x7f);
    memcpy((void *)&esds[31], csd0.data(), csd0size);
    // data following this is ignored, so don't bother appending it
}

status_t CompressAACAudioSource::read(MediaBufferBase **out,
                                      const ReadOptions * /* options */) {
    Mutex::Autolock autoLock(mLock);
    *out = NULL;

    if (mInitCheck != OK) {
        ALOGE("%s: AudioRecord init check failed", __func__);
        return NO_INIT;
    }
    if (!mStarted) {
        return OK;
    }

    /**
     * source started but either no buffer available
     * or source paused
     * */
    while (mStarted && (mBuffersReceived.empty() || mPaused)) {
        ALOGV_IF(mPaused, "%s: source paused, wait until resume", __func__);
        ALOGV_IF(mBuffersReceived.empty(), "%s: wait until compress data",
                 __func__);
        mFrameAvailableCondition.wait(mLock);
        if (mNoMoreFramesToRead) {
            return OK;
        }
    }

    MediaBuffer *buffer = *mBuffersReceived.begin();
    mBuffersReceived.erase(mBuffersReceived.begin());
    ++mNumClientOwnedBuffers;
    buffer->setObserver(this);
    buffer->add_ref();

    // Mute/suppress the recording sound
    int64_t timeUs;
    CHECK(buffer->meta_data().findInt64(kKeyTime, &timeUs));
    int64_t elapsedTimeUs = timeUs - mStartTimeUs;
    if (elapsedTimeUs < kAutoRampStartUs) {
        ALOGW("%s: elapsed time is less than  StartTime", __func__);
        memset((uint8_t *)buffer->data(), 0, buffer->range_length());
    } else if (elapsedTimeUs < kAutoRampStartUs + kAutoRampDurationUs) {
        ALOGW("%s: aac data is not adjusted for rampvolume", __func__);
    }

    *out = buffer;
    return OK;
}

status_t CompressAACAudioSource::pause() {
    Mutex::Autolock autoLock(mLock);
    mPaused = true;
    ALOGV("%s: paused", __func__);
    return OK;
}

status_t CompressAACAudioSource::setStopTimeUs(int64_t stopTimeUs) {
    Mutex::Autolock autoLock(mLock);
    ALOGV("Set stoptime: %lld us", (long long)stopTimeUs);

    if (stopTimeUs < -1) {
        ALOGE("Invalid stop time %lld us", (long long)stopTimeUs);
        return BAD_VALUE;
    } else if (stopTimeUs == -1) {
        ALOGI("reset stopTime to be -1");
    }

    mStopSystemTimeUs = stopTimeUs;
    return OK;
}

void CompressAACAudioSource::signalBufferReturned(MediaBufferBase *buffer) {
    ALOGV("signalBufferReturned: %p", buffer->data());
    Mutex::Autolock autoLock(mLock);
    --mNumClientOwnedBuffers;
    buffer->setObserver(0);
    buffer->release();
    mFrameEncodingCompletionCondition.signal();
    return;
}

bool CompressAACAudioSource::isNextFramePossible(
    const std::vector<uint8_t> &byteStream, uint16_t &nextFrameSize) {
    if (byteStream.size() <= MIN_ADTS_LENGTH_TO_PARSE) return false;
    if (kSyncWord ==
        ((((uint16_t)byteStream[0]) << 4) | (byteStream[1] >> 4))) {
        // framestart
        // framesize is between  31 to 43 (both bits included) from
        // framestart
        nextFrameSize =
            ((((uint16_t)(byteStream[3] & 0x03)) << 11) |
             (((uint16_t)byteStream[4]) << 3) | (byteStream[5] >> 5));
    } else {
        ALOGE("%s: No syncword detected, clearing the compress bit stream",
              __func__);
        mByteStream.clear();
        return false;
    }
    if (byteStream.size() >= nextFrameSize) return true;
    return false;
}

int32_t CompressAACAudioSource::doAdtsFrameReConstruction(const uint8_t *buf,
                                                          size_t bufSize) {
    uint16_t nextFrameSize = 0;
    int32_t numEncodedFramesGenerated = 0;
    ALOGV("%s: audio buffer recevied, size:%zu called with source object %x",
          __func__, bufSize, this);
    if (bufSize <= 0) {
        return 0;
    }
    std::copy(buf, buf + bufSize, std::back_inserter(mByteStream));

    while (isNextFramePossible(mByteStream, nextFrameSize) && nextFrameSize > 0) {
        // construct frame with nextFrameSize
        ALOGV("%s: ADTS frame size:%d", __func__, nextFrameSize);
        // generate new buffer
        MediaBuffer *buffer =
            new MediaBuffer(nextFrameSize - ADTS_HEADER_LENGTH);
        memcpy((uint8_t *)buffer->data(),
               mByteStream.data() + ADTS_HEADER_LENGTH,
               nextFrameSize - ADTS_HEADER_LENGTH);
        buffer->set_range(0, nextFrameSize - ADTS_HEADER_LENGTH);
        mEncodedBuffersFormed.push_back(buffer);
        mByteStream.erase(mByteStream.begin(),
                          mByteStream.begin() + nextFrameSize);
        nextFrameSize = 0;
        numEncodedFramesGenerated++;
    }

    return numEncodedFramesGenerated;
}

void CompressAACAudioSource::queueInputBuffer_l(MediaBuffer *buffer,
                                                int64_t timeUs) {
    const size_t bufferSize = buffer->range_length();
    const size_t frameSize = getAACFramePCMSize();
    if (mPCMNumFramesReceived == 0) {
        buffer->meta_data().setInt64(kKeyAnchorTime, mStartTimeUs);
    }
    /* since one encoded buffer is worth of 1024 PCM sample */
    mPCMNumFramesReceived += mPCMSamplesPerFrame;
    const int64_t timestampUs =
        mStartTimeUs +
        ((1000000LL * mPCMNumFramesReceived) + (mSampleRate >> 1)) /
            mSampleRate;
    if (mFirstSampleSystemTimeUs < 0LL) {
        mFirstSampleSystemTimeUs = timeUs;
    }
    buffer->meta_data().setInt64(kKeyTime, mPrevSampleTimeUs);
    buffer->meta_data().setInt64(kKeyDriftTime, timeUs - mInitialReadTimeUs);
    mPrevSampleTimeUs = timestampUs;
    mBuffersReceived.push_back(buffer);
    mFrameAvailableCondition.signal();
}

status_t CompressAACAudioSource::dataCallback(
    const AudioRecord::Buffer *audioBuf) {
    int64_t timeUs, position, timeNs;
    ExtendedTimestamp ts;
    ExtendedTimestamp::Location location;
    const int32_t usPerSec = 1000000;
    int32_t numEncodedFramesGenerated = 0;

    if (audioBuf->size() <= 0) {
        return NOT_ENOUGH_DATA;
    }

    numEncodedFramesGenerated =
        doAdtsFrameReConstruction((uint8_t *)audioBuf->raw, audioBuf->size());

    while (numEncodedFramesGenerated--) {
        if (mRecord->getTimestamp(&ts) == OK &&
            ts.getBestTimestamp(&position, &timeNs,
                                ExtendedTimestamp::TIMEBASE_MONOTONIC,
                                &location) == OK) {
            // Intention is to always use LOCATION_KERNEL
            if (location == ExtendedTimestamp::LOCATION_KERNEL) {
                ALOGV(
                    "%s: using timestamp of LOCATION_KERNEL: %d, postion: "
                    "%d, timeNs: %d",
                    __func__, location, position, timeNs);
            } else {
                ALOGE(
                    "%s: using timestamp of unintended location: %d, postion: "
                    "%d, timeNs: %d",
                    __func__, location, position, timeNs);
            }
            // Use audio timestamp.
            timeUs = timeNs / 1000 - (position - mPCMNumFramesSkipped -
                                      mPCMNumFramesReceived + mNumFramesLost) *
                                         usPerSec / mSampleRate;
        } else {
            // This should not happen in normal case.
            ALOGW("Failed to get audio timestamp, fallback to use systemclock");
            timeUs = systemTime() / 1000LL;
            // Estimate the real sampling time of the 1st sample in this buffer
            // from AudioRecord's latency. (Apply this adjustment first so that
            // the start time logic is not affected.)
            timeUs -= mRecord->latency() * 1000LL;
        }

        ALOGV("dataCallbackTimestamp: %" PRId64 " us", timeUs);

        Mutex::Autolock autoLock(mLock);
        if (mPaused) {
            clearEncodedBuffers_l();
            ALOGV("%s: need not queue the buffer when paused", __func__);
            return OK;
        }
        if (!mStarted) {
            clearEncodedBuffers_l();
            ALOGW("%s: Spurious callback from AudioRecord. Drop the audio data",
                  __func__);
            return OK;
        }

        // Drop retrieved and previously lost audio data.
        if (mPCMNumFramesReceived == 0 && timeUs < mStartTimeUs) {
            (void)mRecord->getInputFramesLost();
            int64_t receievedFrames = 1;
            ALOGD("Drop audio data(%" PRId64 " frames) at %" PRId64 "/%" PRId64
                  " us",
                  receievedFrames, timeUs, mStartTimeUs);
            auto it = mEncodedBuffersFormed.begin();
            (*it)->release();
            mEncodedBuffersFormed.erase(it);
            mPCMNumFramesSkipped += mPCMSamplesPerFrame;
            continue;
        }

        if (mStopSystemTimeUs != -1 && timeUs >= mStopSystemTimeUs) {
            ALOGD("Drop Audio frame at %lld  stop time: %lld us",
                  (long long)timeUs, (long long)mStopSystemTimeUs);
            mNoMoreFramesToRead = true;
            mFrameAvailableCondition.signal();
            auto it = mEncodedBuffersFormed.begin();
            (*it)->release();
            mEncodedBuffersFormed.erase(it);
            continue;
        }

        if (mPCMNumFramesReceived == 0 && mPrevSampleTimeUs == 0) {
            mInitialReadTimeUs = timeUs;
            // Initial delay
            if (mStartTimeUs > 0) {
                mStartTimeUs = timeUs - mStartTimeUs;
            }
            mPrevSampleTimeUs = mStartTimeUs;
        }
        mLastFrameTimestampUs = timeUs;

        MediaBuffer *buffer = *mEncodedBuffersFormed.begin();
        mEncodedBuffersFormed.erase(mEncodedBuffersFormed.begin());
        queueInputBuffer_l(buffer, timeUs);
    }
    return OK;
}

int64_t CompressAACAudioSource::getFirstSampleSystemTimeUs() {
    Mutex::Autolock autoLock(mLock);
    return mFirstSampleSystemTimeUs;
}

size_t CompressAACAudioSource::onMoreData(const AudioRecord::Buffer &buffer) {
    dataCallback(&buffer);
    return buffer.size();
}

void CompressAACAudioSource::onOverrun() {
    ALOGW("%s:AudioRecord reported overrun!", __func__);
    return;
}

}  // namespace android
