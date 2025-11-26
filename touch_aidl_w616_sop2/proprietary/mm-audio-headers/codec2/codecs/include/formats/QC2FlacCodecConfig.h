/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_FLAC_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_FLAC_CODEC_CONFIG_H_

#include <algorithm>
#include <cstdint>
#include <memory>

#include <C2Config.h>
#include <QC2Constants.h>


namespace qc2audio {

static constexpr const int MAX_FLAC_CHANNELS = 8;
static constexpr const uint32_t MAX_FLAC_SAMPLE_RATE = 655350;
static constexpr const uint32_t MAX_INP_BUFFER = 8192*8*2*4;

struct FlacDefaults {
    static const uint32_t FLACDefaultSampleRate = 48000;
    static const uint32_t FLACDefaultBitDepth = 16;
    static const uint32_t FLACDefaultChannels = 2;
    static const uint32_t FLACDefaultMinBlkSize = 16;
    static const uint32_t FLACDefaultMaxBlkSize = 16;
    static const uint32_t FLACDefaultMinFrameSize = 0;
    static const uint32_t FLACDefaultMaxFrameSize = 0;
};

static constexpr const uint32_t sFlacMaxOutBufSize = (8192*8)*4*8;
static constexpr const uint32_t sFlacMaxInBufSize = MAX_INP_BUFFER;
static constexpr const uint32_t sFlacInstSize = 2048 + MAX_INP_BUFFER + 65536*8*4;
static constexpr const uint32_t sFlacMaxDecInstance = 10;
static constexpr const uint32_t sFlacSupersetThreshold = 14000;
static constexpr const uint32_t sFlacThreshold = 8192*2;
static constexpr const uint32_t sFlacParserBufSize = 8192*8;
static constexpr const uint32_t sFlacBufferingSize = (MAX_INP_BUFFER) > (sFlacParserBufSize*2) ?
                                                     (MAX_INP_BUFFER) : (sFlacParserBufSize*2);
static constexpr const uint32_t sFlacOutBufThresholdMsec = 20; // In milliseconds
static const char * sFlacSignBytes = "fLaC";
static constexpr const uint32_t sFlacSignSize = 4;

static constexpr const uint8_t sFlacMDBlkStreamInfo = 0x00;
static constexpr const uint8_t sFlacMDBlkPadding = 0x01;
static constexpr const uint8_t sFlacMDBlkApplication = 0x02;
static constexpr const uint8_t sFlacMDBlkSeekTable = 0x03;
static constexpr const uint8_t sFlacMDBlkVorbisContent = 0x04;
static constexpr const uint8_t sFlacMDBlkCueSheet = 0x05;
static constexpr const uint8_t sFlacMDBlkPicture = 0x06;
static constexpr const uint8_t sFlacMDBlkUnknownBeg = 0x07;
static constexpr const uint8_t sFlacMDBlkUnknownEnd = 0x7F;
static constexpr const uint8_t sFlacMD5SigSize = 16;

enum FlacParamIndex : uint8_t {
    FLAC_MAX_INPUT_BUF_SIZE = 0,
    FLAC_MAX_OUTPUT_BUF_SIZE,
    FLAC_MAX_CHANNELS,
    FLAC_SAMPLE_RATE_RANGE,
    FLAC_BITS_PER_SAMPLE_SET
};

// tuple params:
// FLAC_MAX_INPUT_BUF_SIZE, FLAC_MAX_OUTPUT_BUF_SIZE, FLAC_MAX_CHANNELS,
// FLAC_SAMPLE_RATE_RANGE, FLAC_BITS_PER_SAMPLE_SET
static const std::tuple<uint32_t, uint32_t, uint32_t,
                    std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> FlacDecMinMaxList = {
    MAX_INP_BUFFER, sFlacMaxOutBufSize, MAX_FLAC_CHANNELS,
    {8000, 384000},
    {16, 24}
};

class QC2FlacCodecConfig {

public:

    QC2FlacCodecConfig() { mConfig = std::make_shared<FlacDecConfig_t>(); }

    typedef struct {
        uint8_t nChannels;
        uint32_t nSampleRate;
        uint8_t nBitDepth;
        uint32_t nMinBlkSize;
        uint32_t nMaxBlkSize;
        uint32_t nMinFrameSize;
        uint32_t nMaxFrameSize;
    } FlacDecConfig_t;

    /* Individual setter functions for members */

    void setChannels(uint8_t channels) {
        mConfig->nChannels = channels;
    }

    void setSampleRate(uint32_t sampleRate) {
        mConfig->nSampleRate = sampleRate;
    }

    void setBitDepth(uint8_t bitDepth) {
        mConfig->nBitDepth = bitDepth;
    }

    void setMinBlkSize(uint32_t minBlkSize) {
        mConfig->nMinBlkSize = minBlkSize;
    }

    void setMaxBlkSize(uint32_t maxBlkSize) {
        mConfig->nMaxBlkSize = maxBlkSize;
    }

    void setMinFrameSize(uint32_t minFrameSize) {
        mConfig->nMinFrameSize = minFrameSize;
    }

    void setMaxFrameSize(uint32_t maxFrameSize) {
        mConfig->nMaxFrameSize = maxFrameSize;
    }

    /* Individual getter functions for members */
    uint8_t getChannels() {
        return mConfig->nChannels;
    }

    uint32_t getSampleRate() {
        return mConfig->nSampleRate;
    }

    uint8_t getBitDepth() {
        return mConfig->nBitDepth;
    }

    uint32_t getMinBlkSize() {
        return mConfig->nMinBlkSize;
    }

    uint32_t getMaxBlkSize() {
        return mConfig->nMaxBlkSize;
    }

    uint32_t getMinFrameSize() {
        return mConfig->nMinFrameSize;
    }

    uint32_t getMaxFrameSize() {
        return mConfig->nMaxFrameSize;
    }

    bool isSampleRateValid(uint32_t sampleRate) {
        auto flacSampleRateRange = std::get<FLAC_SAMPLE_RATE_RANGE>(FlacDecMinMaxList);
        return (sampleRate >= std::get<MIN_SAMPLE_RATE_IDX>(flacSampleRateRange) &&
                sampleRate <= std::get<MAX_SAMPLE_RATE_IDX>(flacSampleRateRange));
    }

    bool isChannelCountValid(uint8_t channels) {
        if (channels >= 1 && channels <= MAX_FLAC_CHANNELS) {
            return true;
        }
        return false;
    }

    uint8_t mapPcmEncodingToBitsPerSample(C2Config::pcm_encoding_t pcmEncoding) {
        auto it = sPcmEncodingtoBitsPerSample.find(pcmEncoding);
        if (it != sPcmEncodingtoBitsPerSample.end()) {
            return it->second;
        }
        return 0;
    }

private:
    std::shared_ptr<FlacDecConfig_t> mConfig;
};

};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_FLAC_CODEC_CONFIG_H_
