/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_SPEECH_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_SPEECH_CODEC_CONFIG_H_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>

#include <C2Config.h>
#include <QC2Constants.h>


namespace qc2audio {

enum SpeechCodecType : uint32_t {
    EVRC = 0,
    QCELP,
};

enum BufferSizeType : uint32_t {
    SPEECH_MAX_INPUT_BUF_SIZE = 0,
    TRANSCODE_BUF_SIZE,
    RESIDUAL_BUF_SIZE,
};

enum FrameLengthType : uint32_t {
    TRANSCODE_FRAME_LENGTH = 0,
    MAX_FRAME_LENGTH,
};

enum VocoderRates : uint32_t {
    VOCODER_ONE_EIGHTH_RATE = 0x1,
    VOCODER_ONE_FOURTH_RATE,
    VOCODER_HALF_RATE,
    VOCODER_FULL_RATE,
    VOCODER_INVALID_RATE,
};

struct SpeechDefaults {
    static const uint32_t SpeechDefaultSampleRate = 8000;
    static const uint32_t SpeechDefaultBitDepth = 16;
    static const uint32_t SpeechDefaultChannels = 1;
};

struct EvrcDefaults {
    static const uint32_t EvrcDefaultMinRate = VocoderRates::VOCODER_FULL_RATE;
    static const uint32_t EvrcDefaultMaxRate = VocoderRates::VOCODER_FULL_RATE;
    static const uint32_t EvrcDefaultRateModCmd = 0; // Rate control disabled
};

struct QcelpDefaults {
    static const uint32_t QcelpDefaultMinRate = VocoderRates::VOCODER_FULL_RATE;
    static const uint32_t QcelpDefaultMaxRate = VocoderRates::VOCODER_FULL_RATE;
    static const uint32_t QcelpDefaultReducedRateCmd = 0; //V13K_FS_ENCODE_MODE_MR1440
    static const uint32_t QcelpDefaultRateModCmd = 0; // Rate control disabled
};

static constexpr const int MAX_SPEECH_CHANNELS = 1;
static const std::pair<uint32_t, uint32_t> SpeechSampleRateRange = {8000, 8000};
static constexpr const uint32_t sSpeechOutBufSize = 9600;

static const std::vector<uint32_t> VocoderRateSet = {
    VOCODER_ONE_EIGHTH_RATE, VOCODER_ONE_FOURTH_RATE,
    VOCODER_HALF_RATE, VOCODER_FULL_RATE };

// tuple of BufferSizeType(s)
using triplet = std::tuple<uint32_t, uint32_t, uint32_t>;
const std::array<triplet, 2> sBufferSizes = {
          {{720, 720, 24},       // EVRC
          {1080, 1080, 35}}      // QCELP
};

// pair of FrameLengthType(s)
using couple = std::pair<uint32_t, uint32_t>;
const std::array<couple, 2> sFrameLengths = {
          {{24, 23},      // EVRC
           {36, 35}}      // QCELP
};

enum SpeechParamIndex : uint8_t {
    SPEECH_ENC_MAX_INPUT_BUF_SIZE = 0,
    SPEECH_ENC_MAX_OUTPUT_BUF_SIZE,
    SPEECH_ENC_MIN_FRAME_SIZE,
};

// tuple params:
// SPEECH_ENC_MAX_INPUT_BUF_SIZE, SPEECH_ENC_MAX_OUTPUT_BUF_SIZE, SPEECH_ENC_MIN_FRAME_SIZE
static const std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> SpeechEncoderMinMaxList = {
    { 320, 23, 160 },  // EVRC encoder
    { 320, 35, 160 },  // QCELP encoder
};

class QC2SpeechCodecConfig {

public:
    QC2SpeechCodecConfig() { mCommonConfig = std::make_shared<SpeechCommonConfig_t>(); }

    typedef struct {
        uint32_t nSampleRate;
        uint8_t nBitDepth;
        uint8_t nChannels;
    } SpeechCommonConfig_t;

    /* Individual setter functions for members */
    void setBitDepth(uint8_t bitDepth) {
        mCommonConfig->nBitDepth = bitDepth;
    }

    void setChannels(uint8_t channels) {
        mCommonConfig->nChannels = channels;
    }

    void setSampleRate(uint32_t sampleRate) {
        mCommonConfig->nSampleRate = sampleRate;
    }

    /* Individual getter functions for members */
    uint8_t getBitDepth() {
        return mCommonConfig->nBitDepth;
    }

    uint8_t getChannels() {
        return mCommonConfig->nChannels;
    }

    uint32_t getSampleRate() {
        return mCommonConfig->nSampleRate;
    }

    bool isSampleRateValid(uint32_t sampleRate) {
        return (sampleRate >= std::get<MIN_SAMPLE_RATE_IDX>(SpeechSampleRateRange) &&
                sampleRate <= std::get<MAX_SAMPLE_RATE_IDX>(SpeechSampleRateRange));
    }

    bool isChannelCountValid(uint8_t channels) {
        if (channels > 0 && channels <= MAX_SPEECH_CHANNELS) {
            return true;
        }
        return false;
    }

    bool isVocoderRateValid(uint32_t vocoderRate) {
        if (std::find(VocoderRateSet.begin(), VocoderRateSet.end(),
                vocoderRate) != VocoderRateSet.end()) {
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

    static uint32_t getMaxOutputBufferSize() {
        return sSpeechOutBufSize;
    }

    static triplet getBufferSizeTriplet(SpeechCodecType type) {
        return sBufferSizes[type];
    }

    static couple getFrameLengthCouple(SpeechCodecType type) {
        return sFrameLengths[type];
    }

    // Get buffer sizes
    static uint32_t getMaxInputBufferSize(SpeechCodecType type) {
        return std::get<SPEECH_MAX_INPUT_BUF_SIZE>(getBufferSizeTriplet(type));
    }

    static uint32_t getTranscodeBufSize(SpeechCodecType type) {
        return std::get<TRANSCODE_BUF_SIZE>(getBufferSizeTriplet(type));
    }

    static uint32_t getResidualBufSize(SpeechCodecType type) {
        return std::get<RESIDUAL_BUF_SIZE>(getBufferSizeTriplet(type));
    }

    // Get frame lengths
    static uint32_t getTranscodeFrameLength(SpeechCodecType type) {
        return std::get<TRANSCODE_FRAME_LENGTH>(getFrameLengthCouple(type));
    }

    static uint32_t getMaxFrameLength(SpeechCodecType type) {
        return std::get<MAX_FRAME_LENGTH>(getFrameLengthCouple(type));
    }

    // Get transcoded number of frames
    static uint32_t getTranscodedFrames(SpeechCodecType type) {
        return getTranscodeBufSize(type) / getTranscodeFrameLength(type);
    }

private:
    std::shared_ptr<SpeechCommonConfig_t> mCommonConfig;
};

class QC2EvrcEncConfig : public QC2SpeechCodecConfig {

public:
    QC2EvrcEncConfig()
        : QC2SpeechCodecConfig() {
        mEncConfig = std::make_shared<EvrcEncConfig>();
    }

    typedef struct  {
        uint16_t minRate;
        uint16_t maxRate;
        uint16_t rateModCmd;
        uint16_t reserved;
    } EvrcEncConfig;

    void setMinRate(uint16_t minRate) {
        mEncConfig->minRate = minRate;
    }

    void setMaxRate(uint16_t maxRate) {
        mEncConfig->maxRate = maxRate;
    }

    void setRateModulation(uint16_t rateMod) {
        mEncConfig->rateModCmd = rateMod;
    }

    void setReserved(uint16_t reserved) {
        mEncConfig->reserved = reserved;
    }

    /* Individual getter functions for members */
    uint16_t getMinRate() {
        return mEncConfig->minRate;
    }

    uint16_t getMaxRate() {
        return mEncConfig->maxRate;
    }

    uint16_t getRateModulation() {
        return mEncConfig->rateModCmd;
    }

    uint16_t getReserved() {
        return mEncConfig->reserved;
    }

    static uint32_t getMinFrameSize() {
        return std::get<SPEECH_ENC_MIN_FRAME_SIZE>(SpeechEncoderMinMaxList[EVRC]);
    }

    static uint32_t getMaxInputBufferSize() {
        return std::get<SPEECH_ENC_MAX_INPUT_BUF_SIZE>(SpeechEncoderMinMaxList[EVRC]);
    }

    static uint32_t getMaxOutputBufferSize() {
        return std::get<SPEECH_ENC_MAX_OUTPUT_BUF_SIZE>(SpeechEncoderMinMaxList[EVRC]);
    }

private:
    std::shared_ptr<EvrcEncConfig> mEncConfig;
};


class QC2QcelpEncConfig : public QC2SpeechCodecConfig {

public:
    QC2QcelpEncConfig()
        : QC2SpeechCodecConfig() {
        mEncConfig = std::make_shared<QcelpEncConfig>();
    }

    typedef struct  {
        uint16_t maxRate;
        uint16_t minRate;
        uint16_t reducedRateCmd;
        uint16_t rateModCmd;
    } QcelpEncConfig;

    void setMinRate(uint16_t minRate) {
        mEncConfig->minRate = minRate;
    }

    void setMaxRate(uint16_t maxRate) {
        mEncConfig->maxRate = maxRate;
    }

    void setRateModulation(uint16_t rateMod) {
        mEncConfig->rateModCmd = rateMod;
    }

    void setReducedRateCmd(uint16_t cmd) {
        mEncConfig->reducedRateCmd = cmd;
    }

    /* Individual getter functions for members */
    uint16_t getMinRate() {
        return mEncConfig->minRate;
    }

    uint16_t getMaxRate() {
        return mEncConfig->maxRate;
    }

    uint16_t getRateModulation() {
        return mEncConfig->rateModCmd;
    }

    uint16_t getReducedRateCmd() {
        return mEncConfig->reducedRateCmd;
    }

    static uint32_t getMinFrameSize() {
        return std::get<SPEECH_ENC_MIN_FRAME_SIZE>(SpeechEncoderMinMaxList[QCELP]);
    }

    static uint32_t getMaxInputBufferSize() {
        return std::get<SPEECH_ENC_MAX_INPUT_BUF_SIZE>(SpeechEncoderMinMaxList[QCELP]);
    }

    static uint32_t getMaxOutputBufferSize() {
        return std::get<SPEECH_ENC_MAX_OUTPUT_BUF_SIZE>(SpeechEncoderMinMaxList[QCELP]);
    }

private:
    std::shared_ptr<QcelpEncConfig> mEncConfig;
};
};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_SPEECH_CODEC_CONFIG_H_
