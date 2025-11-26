/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_WMA_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_WMA_CODEC_CONFIG_H_

#include <algorithm>
#include <cstdint>
#include <memory>

#include <C2Config.h>
#include <QC2Constants.h>


namespace qc2audio {

enum WmaVersion : uint32_t {
    WMA_STD      = 0,
    WMA_PRO      = 1,
    WMA_DEFAULT  = WMA_STD,
    WMA_LOSSLESS = 2,
};

enum WmaFormatTag : uint32_t {
    WMA_V9_STD = 353,
    WMA_V9_PRO = 354,
    WMA_V9_LOSSLESS = 355,
    WMA_V10_PRO = 358,
    WMA_V10_LOSSLESS = 359,
};

enum WmaParamIndex : uint8_t {
    WMA_MAX_INPUT_BUF_SIZE = 0,
    WMA_MAX_OUTPUT_BUF_SIZE,
    WMA_MIN_BIT_RATE,
    WMA_MAX_BIT_RATE,
    WMA_MAX_CHANNELS,
    WMA_SAMPLE_RATE_RANGE,
    WMA_BITS_PER_SAMPLE_SET
};

static const std::unordered_set<uint32_t> WmaVersionSet = { WMA_STD, WMA_PRO, WMA_LOSSLESS};
static const uint32_t WMAPRO_MAX_DURATION_PER_PACKET = 2;

// tuple params:
// WMA_MAX_INPUT_BUF_SIZE, WMA_MAX_OUTPUT_BUF_SIZE, WMA_MIN_BIT_RATE, WMA_MAX_BIT_RATE,
// WMA_MAX_CHANNELS, WMA_SAMPLE_RATE_RANGE, WMA_BITS_PER_SAMPLE_SET
static std::vector<std::tuple <uint32_t, uint32_t, uint32_t, uint32_t, uint16_t,
                      std::pair<uint32_t, uint32_t>, std::vector<uint32_t>>> WmaMinMaxList = {
    {8917, 262144, 128, 256008, 2, {8000, 48000}, {16}},          // WMA_STD
    {8917, 4608000, 8000, 1536000, 8, {8000, 96000}, {16, 24}},   // WMA_PRO
    {8917, 4608000, 8000, 1152000, 8, {8000, 96000}, {16, 24}},   // WMA_LOSSLESS
};

enum WmaChannelMask : uint32_t {
    CHMASK_MONO = 0x4,
    CHMASK_STEREO = 0x3,
    CHMASK_2POINT1 = 0x7,
    CHMASK_QUAD = 0x33,
    CHMASK_PENTA = 0x37,
    CHMASK_5POINT1 = 0x3F,
    CHMASK_6POINT1 = 0x13F,
    CHMASK_7POINT1 = 0x63F,
};

enum WmaStdDefaults : uint32_t {
    kWmaStdDefaultSampleRate = 44100,
    kWmaStdDefaultChannelCount = 2,
    kWmaStdDefaultBitsPerSample = 16,
    kWmaStdDefaultAvgBytesPerSecond = 8003,
    kWmaStdDefaultFmtTag = WMA_V9_STD,
    kWmaStdDefaultBlkAlign = 2973,
    kWmaStdDefaultChannelMask = CHMASK_STEREO,
    kWmaStdDefaultEncOptions = 15,
    kWmaStdDefaultReserved = 0,
};

enum WmaProDefaults : uint32_t {
    kWmaProDefaultSampleRate = 44100,
    kWmaProDefaultChannelCount = 2,
    kWmaProDefaultBitsPerSample = 16,
    kWmaProDefaultAvgBytesPerSecond = 8003,
    kWmaProDefaultFmtTag = WMA_V9_PRO,
    kWmaLosslessDefaultFmtTag = WMA_V9_LOSSLESS,
    kWmaProDefaultBlkAlign = 2973,
    kWmaProDefaultChannelMask = CHMASK_STEREO,
    kWmaProDefaultEncOptions = 224,
    kWmaProDefaultAdvancedEncOption = 16579,
    kWmaProDefaultAdvancedEncOptions2 = 0,
};

class QC2WmaCodecConfig {

public:
    typedef struct {
        uint16_t          fmtTag;  // WMA format tag.
        uint16_t          numChannels; // Number of channels in the stream
        uint32_t          sampleRate;  // Number of samples per second in Hertz
        uint32_t          avgBytesPerSec;  // Bitrate expressed as the average bytes per second
        uint16_t          blkAlign;  // Block align. All WMA files with a maximum packet size of 13376 are supported.
        uint16_t          bitsPerSample;  // Number of bits per sample in the output
        uint32_t          channelMask;  // Channel mask.
        uint16_t          encOptions;  // Options used during encoding.
    } WmaCommonConfig;

    QC2WmaCodecConfig(uint32_t maxInputBufSize, uint32_t maxOutputBufSize,
                      uint32_t minBitRate, uint32_t maxBitRate,
                      uint16_t maxChannels, std::pair<uint32_t, uint32_t> SampleRateRange,
                      std::vector<uint32_t> BitsPerSampleRange, uint32_t version) {
        mMaxInputBufSize = maxInputBufSize;
        mMaxOutputBufSize = maxOutputBufSize;
        mMinBitRate = minBitRate;
        mMaxBitRate = maxBitRate;
        mMaxChannels = maxChannels;
        mSampleRateRange = std::move(SampleRateRange);
        mBitsPerSampleSet = std::move(BitsPerSampleRange);
        mWmaVersion = version;
    }

    uint32_t getMaxInputBufSize() { return mMaxInputBufSize; }

    uint32_t getMaxOutputBufSize() { return mMaxOutputBufSize; }

    uint16_t getMaxChannels() { return mMaxChannels; }

    uint32_t getWmaVersion() { return mWmaVersion; }

    void setWmaVersion(uint32_t version) {
        mWmaVersion = version;
        // Set params based on version
        mMaxInputBufSize = std::get<WMA_MAX_INPUT_BUF_SIZE>(WmaMinMaxList[version]);
        mMaxOutputBufSize = std::get<WMA_MAX_OUTPUT_BUF_SIZE>(WmaMinMaxList[version]);
        mMinBitRate = std::get<WMA_MIN_BIT_RATE>(WmaMinMaxList[version]);
        mMaxBitRate = std::get<WMA_MAX_BIT_RATE>(WmaMinMaxList[version]);
        mMaxChannels = std::get<WMA_MAX_CHANNELS>(WmaMinMaxList[version]);
        mSampleRateRange = std::get<WMA_SAMPLE_RATE_RANGE>(WmaMinMaxList[version]);
        mBitsPerSampleSet = std::get<WMA_BITS_PER_SAMPLE_SET>(WmaMinMaxList[version]);
    }

    bool isSampleRateValid(uint32_t sampleRate) {
        return (sampleRate >= std::get<MIN_SAMPLE_RATE_IDX>(mSampleRateRange) &&
                sampleRate <= std::get<MAX_SAMPLE_RATE_IDX>(mSampleRateRange));
    }

    bool isChannelCountValid(uint8_t channelCount) {
        return (channelCount && channelCount <= mMaxChannels);
    }

    bool isBitsPerSampleValid(uint32_t bps) {
        return (std::find(std::begin(mBitsPerSampleSet),
            std::end(mBitsPerSampleSet), bps) != std::end(mBitsPerSampleSet));
    }

    bool isBitRateValid(uint32_t bitRate) {
        return (mMinBitRate <= bitRate  && bitRate <= mMaxBitRate);
    }

    bool isWmaVersionValid(uint32_t version) {
        auto ver =  WmaVersionSet.find(version);
        if (ver == WmaVersionSet.end()) {
            return false;
        }
        return true;
    }

private:
    uint32_t mMaxInputBufSize;
    uint32_t mMaxOutputBufSize;
    uint32_t mMinBitRate;
    uint32_t mMaxBitRate;
    uint16_t  mMaxChannels;
    std::pair<uint32_t, uint32_t> mSampleRateRange;
    std::vector<uint32_t> mBitsPerSampleSet;
    uint32_t mWmaVersion;
};

class QC2WmaDecConfig : public QC2WmaCodecConfig {

public:
    typedef struct {
        uint16_t reserved;
    } WmaStdExtConfig;

    typedef struct {
        uint16_t advancedEncOption;
        uint32_t advancedEncOptions2;
    } WmaProExtConfig;

    typedef struct WmaDecConfig {
        WmaCommonConfig commonConfig;
        WmaStdExtConfig extStdConfig;
        WmaProExtConfig extProConfig;
    } WmaDecConfig;

    // WMA PRO and LOSSLESS DSP structures
    typedef struct {
        uint16_t fmtTag;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t avgBytesPerSec;
        uint16_t blkAlign;
        uint16_t bitsPerSample;
        uint32_t channelMask;
        uint16_t encOptions;
        uint16_t reserved;
    } WmaStdDecConfig;

    typedef struct {
        uint16_t fmtTag;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t avgBytesPerSec;
        uint16_t blkAlign;
        uint16_t bitsPerSample;
        uint32_t channelMask;
        uint16_t encOptions;
        uint16_t advancedEncOption;
        uint32_t advancedEncOptions2;
    } WmaProDecConfig;

    // Constructor params:
    // WMA_MAX_INPUT_BUF_SIZE, WMA_MAX_OUTPUT_BUF_SIZE, WMA_MIN_BIT_RATE, WMA_MAX_BIT_RATE,
    // WMA_MAX_CHANNELS, WMA_SAMPLE_RATE_RANGE, WMA_BITS_PER_SAMPLE_SET, WmaVersion
    QC2WmaDecConfig(uint32_t version)
            : QC2WmaCodecConfig(std::get<WMA_MAX_INPUT_BUF_SIZE>(WmaMinMaxList[version]),
                                std::get<WMA_MAX_OUTPUT_BUF_SIZE>(WmaMinMaxList[version]),
                                std::get<WMA_MIN_BIT_RATE>(WmaMinMaxList[version]),
                                std::get<WMA_MAX_BIT_RATE>(WmaMinMaxList[version]),
                                std::get<WMA_MAX_CHANNELS>(WmaMinMaxList[version]),
                                std::get<WMA_SAMPLE_RATE_RANGE>(WmaMinMaxList[version]),
                                std::get<WMA_BITS_PER_SAMPLE_SET>(WmaMinMaxList[version]),
                                version) {
        mWmaDecConfig = std::make_shared<WmaDecConfig>();
    }
    ~QC2WmaDecConfig() = default;

    void setChannelCount(uint16_t numChannels) {
        mWmaDecConfig->commonConfig.numChannels = numChannels;
    }

    void setBitsPerSample(uint16_t bps) {
        mWmaDecConfig->commonConfig.bitsPerSample = bps;
    }

    void setBitRate(uint32_t bitRate) {
        if (isBitRateValid(bitRate)) {
            mWmaDecConfig->commonConfig.avgBytesPerSec = bitRate / 8;
        }
    }

    void setSampleRate(uint32_t sampleRate) {
        mWmaDecConfig->commonConfig.sampleRate = sampleRate;
    }

    void setFmtTag(uint16_t fmtTag) {
        mWmaDecConfig->commonConfig.fmtTag = fmtTag;
    }

    void setBlkAlign(uint16_t blkAlign) {
        mWmaDecConfig->commonConfig.blkAlign = blkAlign;
    }

    void setChannelMask(uint32_t channelMask) {
        mWmaDecConfig->commonConfig.channelMask = channelMask;
    }

    void setEncOptions(uint16_t encOptions) {
        mWmaDecConfig->commonConfig.encOptions = encOptions;
    }

    void setReserved(uint16_t reserved) {
        mWmaDecConfig->extStdConfig.reserved = reserved;
    }

    void setAdvancedEncOption(uint16_t advancedEncOption) {
        mWmaDecConfig->extProConfig.advancedEncOption = advancedEncOption;
    }

    void setAdvancedEncOptions2(uint32_t advancedEncOptions2) {
        mWmaDecConfig->extProConfig.advancedEncOptions2 = advancedEncOptions2;
    }


    uint16_t getChannelCount() {
        return mWmaDecConfig->commonConfig.numChannels;
    }

    uint16_t getBitsPerSample() {
        return mWmaDecConfig->commonConfig.bitsPerSample;
    }

    uint32_t getBitRate() {
        return mWmaDecConfig->commonConfig.avgBytesPerSec * 8;
    }

    uint32_t getSampleRate() {
        return mWmaDecConfig->commonConfig.sampleRate;
    }

    uint16_t getFmtTag() {
        return mWmaDecConfig->commonConfig.fmtTag;
    }

    uint16_t getBlkAlign() {
        return mWmaDecConfig->commonConfig.blkAlign;
    }

    uint32_t getChannelMask() {
        return mWmaDecConfig->commonConfig.channelMask;
    }

    uint16_t getEncOptions() {
        return mWmaDecConfig->commonConfig.encOptions;
    }

    uint16_t getReserved() {
        return mWmaDecConfig->extStdConfig.reserved;
    }

    uint16_t getAdvancedEncOption() {
        return mWmaDecConfig->extProConfig.advancedEncOption;
    }

    uint32_t getAdvancedEncOptions2() {
        return mWmaDecConfig->extProConfig.advancedEncOptions2;
    }

    std::shared_ptr<WmaDecConfig> getWmaDecConfig() {
        return mWmaDecConfig;
    }

    void getWmaStdDecConfig(WmaStdDecConfig *wmaStdConfig) {
        wmaStdConfig->fmtTag = getFmtTag();
        wmaStdConfig->numChannels = getChannelCount();
        wmaStdConfig->sampleRate = getSampleRate();
        wmaStdConfig->avgBytesPerSec = (getBitRate() / 8);
        wmaStdConfig->blkAlign = getBlkAlign();
        wmaStdConfig->bitsPerSample = getBitsPerSample();
        wmaStdConfig->channelMask = getChannelMask();
        wmaStdConfig->encOptions = getEncOptions();
        wmaStdConfig->reserved = getReserved();
    }

    void getWmaProDecConfig(WmaProDecConfig *wmaProConfig) {
        wmaProConfig->fmtTag = getFmtTag();
        wmaProConfig->numChannels = getChannelCount();
        wmaProConfig->sampleRate = getSampleRate();
        wmaProConfig->avgBytesPerSec = (getBitRate() / 8);
        wmaProConfig->blkAlign = getBlkAlign();
        wmaProConfig->bitsPerSample = getBitsPerSample();
        wmaProConfig->channelMask = getChannelMask();
        wmaProConfig->encOptions = getEncOptions();
        wmaProConfig->advancedEncOption = getAdvancedEncOption();
        wmaProConfig->advancedEncOptions2 = getAdvancedEncOptions2();
    }

private:
    std::shared_ptr<WmaDecConfig> mWmaDecConfig;
};

};  // namespace qc2audio

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_WMA_CODEC_CONFIG_H_
