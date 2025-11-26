/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_ALAC_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_ALAC_CODEC_CONFIG_H_

#include <algorithm>
#include <cstdint>
#include <memory>

#include <C2Config.h>
#include <QC2Constants.h>


namespace qc2audio {

enum AlacParamIndex : uint8_t {
    ALAC_MAX_INPUT_BUF_SIZE = 0,
    ALAC_MAX_OUTPUT_BUF_SIZE,
    ALAC_MIN_BIT_RATE,
    ALAC_MAX_BIT_RATE,
    ALAC_MAX_CHANNELS,
    ALAC_SAMPLE_RATE_RANGE,
    ALAC_BITS_PER_SAMPLE_SET
};

// tuple params:
// ALAC_MAX_INPUT_BUF_SIZE, ALAC_MAX_OUTPUT_BUF_SIZE, ALAC_MIN_BIT_RATE, ALAC_MAX_BIT_RATE,
// ALAC_MAX_CHANNELS, ALAC_SAMPLE_RATE_RANGE, ALAC_BITS_PER_SAMPLE_SET
static const std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                    std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> AlacDecMinMaxList = {
    131072, 131072, 2048, 49152000, 8,
    {8000, 192000},
    {16, 24, 32}
};

static const uint32_t sAlacMaxMultiChSampleRate = 48000;

struct AlacDefaults {
    static const uint32_t ALACCompatibleVersion = 0;
    static const uint32_t ALACDefaultFrameSize = 4096;
    static const uint32_t ALACDefaultSampleRate = 44100;
    static const uint32_t ALACDefaultBitDepth = 16;
    static const uint32_t ALACDefaultChannels = 2;
    static const uint32_t ALACDefaultAvgBitRate = 534580;
    static const uint32_t ALACDefaultMaxFrameBytes = 9530;
    static const uint32_t ALACDefaultPbTuningParam = 40;
    static const uint32_t ALACDefaultMbTuningParam = 10;
    static const uint32_t ALACDefaultKbTuningParam = 14;
    static const uint32_t ALACDefaultMaxRun = 255;
    static const uint32_t ALACDefaultChannelLayoutTag = 0;
};

enum AlacKeys : uint32_t {
    kKeyIndexAlacFrameLength = 0,
    kKeyIndexAlacCompatibleVersion = 4,
    kKeyIndexAlacBitDepth = 5,
    kKeyIndexAlacPb = 6,
    kKeyIndexAlacMb = 7,
    kKeyIndexAlacKb = 8,
    kKeyIndexAlacNumChannels = 9,
    kKeyIndexAlacMaxRun = 10,
    kKeyIndexAlacMaxFrameBytes = 12,
    kKeyIndexAlacAvgBitRate = 16,
    kKeyIndexAlacSamplingRate = 20,
    kKeyIndexAlacChannelLayout = 24,
};

enum AlacChannelLayoutKeys : uint32_t {
    kKeyIndexAlacChannelLayoutInfoSize = 0,
    kKeyIndexAlacChannelLayoutInfoID = 4,
    kKeyIndexAlacVersionFlags = 8,
    kKeyIndexAlacChannelLayoutTag = 12,
    kKeyIndexAlacReserved1 = 16,
    kKeyIndexAlacReserved2 = 20,
};

class QC2AlacCodecConfig {

public:

    QC2AlacCodecConfig(uint32_t maxInputBufSize, uint32_t maxOutputBufSize,
                      uint32_t minBitRate, uint32_t maxBitRate,
                      uint16_t maxChannels, std::pair<uint32_t, uint32_t> SampleRateRange,
                      std::vector<uint32_t> BitsPerSampleRange) {
        mMaxInputBufSize = maxInputBufSize;
        mMaxOutputBufSize = maxOutputBufSize;
        mMinBitRate = minBitRate;
        mMaxBitRate = maxBitRate;
        mMaxChannels = maxChannels;
        mSampleRateRange = std::move(SampleRateRange);
        mBitsPerSampleSet = std::move(BitsPerSampleRange);
    }

    uint32_t getMaxInputBufSize() { return mMaxInputBufSize; }

    uint32_t getMaxOutputBufSize() { return mMaxOutputBufSize; }

    uint16_t getMaxChannels() { return mMaxChannels; }

    uint32_t getMinBitRate() { return mMinBitRate; }

    uint32_t getMaxBitRate() { return mMaxBitRate; }

    std::vector<uint32_t> getSupportedBitsPerSampleSet() { return mBitsPerSampleSet; }

    std::pair<uint32_t, uint32_t> getSupportedSampleRateRange() { return mSampleRateRange; }

    bool isSampleRateValid(uint32_t sampleRate) {
        return (sampleRate >= std::get<MIN_SAMPLE_RATE_IDX>(mSampleRateRange) &&
                sampleRate <= std::get<MAX_SAMPLE_RATE_IDX>(mSampleRateRange));
    }

    bool isChannelCountValid(uint8_t channelCount) {
        return (channelCount > 0 && channelCount <= mMaxChannels);
    }

    bool isBitsPerSampleValid(uint32_t bps) {
        return (std::find(std::begin(mBitsPerSampleSet),
            std::end(mBitsPerSampleSet), bps) != std::end(mBitsPerSampleSet));
    }

    bool isBitRateValid(uint32_t bitRate) {
        return (getMinBitRate() <= bitRate  && bitRate <= getMaxBitRate());
    }

protected:
    void setMaxInputBufferSize(uint32_t maxInBufSize) {
        mMaxInputBufSize = maxInBufSize;
    }

    void setMaxOutputBufferSize(uint32_t maxOutBufSize) {
        mMaxOutputBufSize = maxOutBufSize;
    }

    void setMinBitRate(uint32_t minBitRate) { mMinBitRate = minBitRate; }

    void setMaxBitRate(uint32_t maxBitRate) { mMaxBitRate = maxBitRate; }

    void setMaxChannels(uint16_t maxChannels) {
        mMaxChannels = maxChannels;
    }

    void setSampleRateRange(std::pair<uint32_t, uint32_t> sampleRateRange) {
        mSampleRateRange = sampleRateRange;
    }

    void setBitsPerSampleSet(std::vector<uint32_t> bitsPerSampleSet) {
        mBitsPerSampleSet = bitsPerSampleSet;
    }

private:
    uint32_t mMaxInputBufSize;
    uint32_t mMaxOutputBufSize;
    uint32_t mMinBitRate;
    uint32_t mMaxBitRate;
    uint16_t  mMaxChannels;
    std::pair<uint32_t, uint32_t> mSampleRateRange;
    std::vector<uint32_t> mBitsPerSampleSet;
};

class QC2AlacDecConfig : public QC2AlacCodecConfig {

public:

    QC2AlacDecConfig()
        : QC2AlacCodecConfig(std::get<ALAC_MAX_INPUT_BUF_SIZE>(AlacDecMinMaxList),
                             std::get<ALAC_MAX_OUTPUT_BUF_SIZE>(AlacDecMinMaxList),
                             std::get<ALAC_MIN_BIT_RATE>(AlacDecMinMaxList),
                             std::get<ALAC_MAX_BIT_RATE>(AlacDecMinMaxList),
                             std::get<ALAC_MAX_CHANNELS>(AlacDecMinMaxList),
                             std::get<ALAC_SAMPLE_RATE_RANGE>(AlacDecMinMaxList),
                             std::get<ALAC_BITS_PER_SAMPLE_SET>(AlacDecMinMaxList)) {
        mDecConfig = std::make_shared<AlacDecConfig>();
        mChannelLayoutInfo = std::make_shared<AlacChannelLayoutInfo>();
    }

    // ---------------- ALAC Specific Info (24 bytes) (mandatory) ---------------------------
    // (AlacDecConfig) Decoder Info

    typedef struct {
        /* Frames per packet when no explicit frames per packet
           setting is present in the packet header */
        uint32_t nFrameLength;
        /* Indicates the compatible version */
        uint8_t nCompatibleVersion;
        /* Bit depth of the source PCM data */
        uint8_t nBitDepth;
        /* Tuning Parameter; currently not used */
        uint8_t nPb;
        /* Tuning Parameter; currently not used */
        uint8_t nMb;
        /* Tuning Parameter; currently not used */
        uint8_t nKb;
        /* Number of channels for multichannel decoding */
        uint8_t nChannels;
        /* Currently not used */
        uint16_t nMaxRun;
        /* Max size of an Apple Lossless packet within the encoded stream */
        uint32_t nMaxFrameBytes;
        /* Average bit rate in bits per second of the Apple Lossless stream */
        uint32_t nAvgBitRate;
        /* Number of samples per second in Hertz */
        uint32_t nSampleRate;
        /* Indicates whether channel layout information is present in the bitstream */
        uint32_t nChannelLayoutTag;
    } AlacDecConfig;

    // ---------------- Channel Layout Info (24 bytes) (optional) ----------------------------
    // (ALAC Channel Layout Info) Channel Layout Info

    // If the channel layout is absent from the cookie, then the following assumptions are made:
    // 1 channel - mono
    // 2 channels - stereo in left, right order
    // > 2 channels - no specific channel designation or role.
    typedef struct {
        uint32_t channelLayoutInfoSize;
        uint32_t channelLayoutInfoID;
        uint32_t versionFlags;
        uint32_t channelLayoutTag;
        uint32_t reserved1;
        uint32_t reserved2;
    } AlacChannelLayoutInfo;

    /* Individual setter functions for members */
    void setFrameLength(uint32_t frameLength) {
        mDecConfig->nFrameLength = frameLength;
    }

    void setCompatibleVersion(uint8_t version) {
        mDecConfig->nCompatibleVersion = version;
    }

    void setBitDepth(uint8_t bitDepth) {
        mDecConfig->nBitDepth = bitDepth;
    }

    void setPbTuningParam(uint8_t pBVal) {
        mDecConfig->nPb = pBVal;
    }

    void setMbTuningParam(uint8_t mBVal) {
        mDecConfig->nMb = mBVal;
    }

    void setKbTuningParam(uint8_t kBVal) {
        mDecConfig->nKb = kBVal;
    }

    void setChannels(uint8_t channels) {
        mDecConfig->nChannels = channels;
    }

    void setMaxRun(uint16_t maxRun) {
        mDecConfig->nMaxRun = maxRun;
    }

    void setMaxFrameBytes(uint32_t maxFrameBytes) {
        mDecConfig->nMaxFrameBytes = maxFrameBytes;
    }

    void setAvgBitRate(uint32_t avgBitRate) {
        mDecConfig->nAvgBitRate = avgBitRate;
    }

    void setSampleRate(uint32_t sampleRate) {
        mDecConfig->nSampleRate = sampleRate;
    }

    void setChannelLayoutTag(uint32_t channelLayoutTag) {
        mDecConfig->nChannelLayoutTag = channelLayoutTag;
    }

    /* Individual getter functions for members */
    uint32_t getFrameLength() {
        return mDecConfig->nFrameLength;
    }

    uint8_t getCompatibleVersion() {
        return mDecConfig->nCompatibleVersion;
    }

    uint8_t getBitDepth() {
        return mDecConfig->nBitDepth;
    }

    uint8_t getPbTuningParam() {
        return mDecConfig->nPb;
    }

    uint8_t getMbTuningParam() {
        return mDecConfig->nMb;
    }

    uint8_t getKbTuningParam() {
        return mDecConfig->nKb;
    }

    uint8_t getChannels() {
        return mDecConfig->nChannels;
    }

    uint16_t getMaxRun() {
        return mDecConfig->nMaxRun;
    }

    uint32_t getMaxFrameBytes() {
        return mDecConfig->nMaxFrameBytes;
    }

    uint32_t getAvgBitRate() {
        return mDecConfig->nAvgBitRate;
    }

    uint32_t getSampleRate() {
        return mDecConfig->nSampleRate;
    }

    uint32_t getChannelLayoutTag() {
        return mDecConfig->nChannelLayoutTag;
    }

    bool isMaxFrameBytesValid(uint32_t maxFrameBytes) {
        return (maxFrameBytes > 0 && maxFrameBytes <= UINT32_MAX);
    }

    bool isMaxRunValid(uint16_t maxRun) {
        return (maxRun >= 0 && maxRun <= UINT16_MAX);
    }

    bool isFrameLengthValid(uint32_t frameLength) {
        return (frameLength > 0 && frameLength <= UINT32_MAX);
    }

    bool isTuningParamValid(uint8_t tuningParam) {
        return (tuningParam >= 0 && tuningParam <= UINT8_MAX);
    }

    /* Set ALAC decoder configuration for all members */
    void setAlacDecConfig(uint8_t* config) {
        std::shared_ptr<AlacDecConfig> param = mDecConfig;
        memcpy(&param->nFrameLength, config + kKeyIndexAlacFrameLength,
               sizeof(param->nFrameLength));
        memcpy(&param->nCompatibleVersion, config + kKeyIndexAlacCompatibleVersion,
               sizeof(param->nCompatibleVersion));
        memcpy(&param->nBitDepth, config + kKeyIndexAlacBitDepth,
               sizeof(param->nBitDepth));
        memcpy(&param->nPb, config + kKeyIndexAlacPb, sizeof(param->nPb));
        memcpy(&param->nMb, config + kKeyIndexAlacMb, sizeof(param->nMb));
        memcpy(&param->nKb, config + kKeyIndexAlacKb, sizeof(param->nKb));
        memcpy(&param->nChannels, config + kKeyIndexAlacNumChannels,
               sizeof(param->nChannels));
        memcpy(&param->nMaxRun, config + kKeyIndexAlacMaxRun,
               sizeof(param->nMaxRun));
        memcpy(&param->nMaxFrameBytes, config + kKeyIndexAlacMaxFrameBytes,
               sizeof(param->nMaxFrameBytes));
        memcpy(&param->nAvgBitRate, config + kKeyIndexAlacAvgBitRate,
               sizeof(param->nAvgBitRate));
        memcpy(&param->nSampleRate, config + kKeyIndexAlacSamplingRate,
               sizeof(param->nSampleRate));
    }

    /* Get ALAC decoder configuration */
    std::shared_ptr<AlacDecConfig> getAlacDecConfig() {
        return mDecConfig;
    }

    QC2Status checkAlacCSDParameters() {
        if (!isSampleRateValid(getSampleRate()) ||
                !isChannelCountValid(getChannels()) ||
                !isBitsPerSampleValid(getBitDepth()) ||
                !isBitRateValid(getAvgBitRate()) ||
                !isFrameLengthValid(getFrameLength()) ||
                !isMaxFrameBytesValid(getMaxFrameBytes()) ||
                !isMaxRunValid(getMaxRun()) ||
                !isTuningParamValid(getPbTuningParam()) ||
                !isTuningParamValid(getMbTuningParam()) ||
                !isTuningParamValid(getKbTuningParam())) {
            QLOGE("%s: Found an invalid ALAC CSD param", __func__);
            return QC2_ERROR;
        }
        return QC2_OK;
    }

    /* Set ALAC channel layout info */
    void setAlacChannelLayoutInfo(uint8_t* channelLayout) {
        std::shared_ptr<AlacChannelLayoutInfo> param = mChannelLayoutInfo;
        memcpy(&param->channelLayoutInfoSize, channelLayout + kKeyIndexAlacChannelLayoutInfoSize,
               sizeof(param->channelLayoutInfoSize));
        memcpy(&param->channelLayoutInfoID, channelLayout + kKeyIndexAlacChannelLayoutInfoID,
               sizeof(param->channelLayoutInfoID));
        memcpy(&param->versionFlags, channelLayout + kKeyIndexAlacVersionFlags,
               sizeof(param->versionFlags));
        memcpy(&param->channelLayoutTag, channelLayout + kKeyIndexAlacChannelLayoutTag,
               sizeof(param->channelLayoutTag));
        memcpy(&param->reserved1, channelLayout + kKeyIndexAlacReserved1, sizeof(param->reserved1));
        memcpy(&param->reserved2, channelLayout + kKeyIndexAlacReserved2, sizeof(param->reserved2));
    }

    /* Get ALAC channel layout */
    std::shared_ptr<AlacChannelLayoutInfo> getAlacChannelLayoutInfo() {
        return mChannelLayoutInfo;
    }

private:
    std::shared_ptr<AlacDecConfig> mDecConfig;
    std::shared_ptr<AlacChannelLayoutInfo> mChannelLayoutInfo;
};

};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_ALAC_CODEC_CONFIG_H_
