/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_APE_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_APE_CODEC_CONFIG_H_

#include <algorithm>
#include <cstdint>
#include <memory>

#include <C2Config.h>
#include <QC2Constants.h>


namespace qc2audio {

enum ApeParamIndex : uint8_t {
    APE_MAX_INPUT_BUF_SIZE = 0,
    APE_MAX_OUTPUT_BUF_SIZE,
    APE_MAX_CHANNELS,
    APE_SAMPLE_RATE_RANGE,
    APE_BITS_PER_SAMPLE_SET
};

static const std::tuple<uint32_t, uint32_t, uint32_t,
                    std::pair<uint32_t, uint32_t>, std::vector<uint32_t>> ApeDecMinMaxList = {
    // APE input from parses has 72k blocks per frame
    1024 * 2 * 4 * 72,
    // APE CSIM processes 72k input in 2k blocks
    // Each 2k block input generates output of size:
    // frame length (2048) * channels * bytes per sample
    2048 * 2 * 4 * 36,
    // APE_MAX_CHANNELS
    2,
    // APE_SAMPLE_RATE_RANGE
    {8000, 192000},
    // APE_BITS_PER_SAMPLE_SET
    {16, 24}
};

struct ApeDefaults {
    static const uint32_t APECompatibleVersion = 0;
    static const uint32_t APEDefaultCompressionLevel = 2000;
    static const uint32_t APEDefaultFormatFlags = 0;
    static const uint32_t APEDefaultBlocksPerFrame = 0;
    static const uint32_t APEDefaultFinalFrameBlocks = 0;
    static const uint32_t APEDefaultTotalFrames = 0;
    static const uint32_t APEDefaultBitsPerSample = 16;
    static const uint32_t APEDefaultNumChannels = 2;
    static const uint32_t APEDefaultSampleRate = 48000;
    static const uint32_t APEDefaultSeekTablePresent = 1;
};

enum ApeKeys : uint32_t {
    kKeyIndexApeCompatibleVersion = 0,
    kKeyIndexApeCompressionLevel = 2,
    kKeyIndexApeFormatFlags = 4,
    kKeyIndexApeBlocksPerFrame = 8,
    kKeyIndexApeFinalFrameBlocks = 12,
    kKeyIndexApeTotalFrames = 16,
    kKeyIndexApeBitsPerSample = 20,
    kKeyIndexApeNumChannels = 22,
    kKeyIndexApeSampleRate = 24,
    kKeyIndexApeSeekTablePresent = 28,
    kKeyIndexApeMax = kKeyIndexApeSeekTablePresent,
};

enum ApeCompressionLevel : uint32_t {
    APE_COMPRESSION_LEVEL_FAST = 1000,
    APE_COMPRESSION_LEVEL_NORMAL = 2000,
    APE_COMPRESSION_LEVEL_HIGH = 3000,
    APE_COMPRESSION_LEVEL_EXTRA_HIGH = 4000,
    APE_COMPRESSION_LEVEL_INSANE = 5000,
};

static const uint32_t sApeExtraHighModeInputSizeMultiplier = 4;
static const uint32_t sApeExtraHighModeOutputSizeMultiplier = 8;
static const uint32_t sApeInsaneModeInputSizeMultiplier = 16;
static const uint32_t sApeInsaneModeOutputSizeMultiplier = 16;
static const uint32_t sApeDefaultOutputSizeMultiplier = 4;

class QC2ApeCodecConfig {

public:

    QC2ApeCodecConfig(uint32_t maxInputBufSize, uint32_t maxOutputBufSize,
                      uint16_t maxChannels, std::pair<uint32_t, uint32_t> SampleRateRange,
                      std::vector<uint32_t> BitsPerSampleRange) {
        mMaxInputBufSize = maxInputBufSize;
        mMaxOutputBufSize = maxOutputBufSize;
        mMaxChannels = maxChannels;
        mSampleRateRange = std::move(SampleRateRange);
        mBitsPerSampleSet = std::move(BitsPerSampleRange);
    }

    uint32_t getMaxInputBufSize() { return mMaxInputBufSize; }

    uint32_t getMaxOutputBufSize() { return mMaxOutputBufSize; }

    uint16_t getMaxChannels() { return mMaxChannels; }

    std::vector<uint32_t> getSupportedBitsPerSampleSet() { return mBitsPerSampleSet; }

    std::pair<uint32_t, uint32_t> getSupportedSampleRateSet() { return mSampleRateRange; }

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

protected:
    void setMaxInputBufferSize(uint32_t maxInBufSize) {
        mMaxInputBufSize = maxInBufSize;
    }

    void setMaxOutputBufferSize(uint32_t maxOutBufSize) {
        mMaxOutputBufSize = maxOutBufSize;
    }

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
    uint16_t  mMaxChannels;
    std::pair<uint32_t, uint32_t> mSampleRateRange;
    std::vector<uint32_t> mBitsPerSampleSet;
};

class QC2ApeDecConfig : public QC2ApeCodecConfig {

public:

    QC2ApeDecConfig()
        : QC2ApeCodecConfig(std::get<APE_MAX_INPUT_BUF_SIZE>(ApeDecMinMaxList),
                            std::get<APE_MAX_OUTPUT_BUF_SIZE>(ApeDecMinMaxList),
                            std::get<APE_MAX_CHANNELS>(ApeDecMinMaxList),
                            std::get<APE_SAMPLE_RATE_RANGE>(ApeDecMinMaxList),
                            std::get<APE_BITS_PER_SAMPLE_SET>(ApeDecMinMaxList)) {
        mDecConfig = std::make_shared<ApeDecConfig>();
    }

    typedef struct {
        /* Indicates the compatible version */
        uint16_t nCompatibleVersion;
        /* The compression level present in the encoded packet */
        uint16_t nCompressionLevel;
        /* Reserved parameter for future use */
        uint32_t nFormatFlags;
        /* Indicates the number of audio blocks in one frame
           present in the encoded packet header */
        uint32_t nBlocksPerFrame;
        /* Indicates the number of audio blocks in the final
           frame present in the encoded packet header */
        uint32_t nFinalFrameBlocks;
        /* Indicates the total number of frames */
        uint32_t nTotalFrames;
        /* Bit depth of the source PCM data */
        uint16_t nBitsPerSample;
        /* Number of channels for decoding */
        uint16_t nChannels;
        /* Samples per second in Hertz */
        uint32_t nSampleRate;
        /* Flag to indicate if seek table is present or not */
        uint32_t nSeekTablePresent;
    } ApeDecConfig;

    /* Individual setter functions for members */
    void setCompatibleVersion(uint16_t version) {
        mDecConfig->nCompatibleVersion = version;
    }

    void setCompressionLevel(uint16_t compressionLevel) {
        mDecConfig->nCompressionLevel = compressionLevel;
    }

    void setFormatFlags(uint32_t formatFlags) {
        mDecConfig->nFormatFlags = formatFlags;
    }

    void setBlocksPerFrame(uint32_t blocksPerFrame) {
        mDecConfig->nBlocksPerFrame = blocksPerFrame;
    }

    void setFinalFrameBlocks(uint32_t finalFrameBlocks) {
        mDecConfig->nFinalFrameBlocks = finalFrameBlocks;
    }

    void setTotalFrames(uint32_t totalFrames) {
        mDecConfig->nTotalFrames = totalFrames;
    }

    void setBitsPerSample(uint16_t bitsPerSample) {
        mDecConfig->nBitsPerSample = bitsPerSample;
    }

    void setChannels(uint16_t channels) {
        mDecConfig->nChannels = channels;
    }

    void setSampleRate(uint32_t sampleRate) {
        mDecConfig->nSampleRate = sampleRate;
    }

    void setSeekTablePresent(uint32_t seekTablePresent) {
        mDecConfig->nSeekTablePresent = seekTablePresent;
    }

    /* Individual getter functions for members */
    uint16_t getCompatibleVersion() {
        return mDecConfig->nCompatibleVersion;
    }

    uint16_t getCompressionLevel() {
        return mDecConfig->nCompressionLevel;
    }

    uint32_t getFormatFlags() {
        return mDecConfig->nFormatFlags;
    }

    uint32_t getBlocksPerFrame() {
        return mDecConfig->nBlocksPerFrame;
    }

    uint32_t getFinalFrameBlocks() {
        return mDecConfig->nFinalFrameBlocks;
    }

    uint32_t getTotalFrames() {
        return mDecConfig->nTotalFrames;
    }

    uint16_t getBitsPerSample() {
        return mDecConfig->nBitsPerSample;
    }

    uint16_t getChannels() {
        return mDecConfig->nChannels;
    }

    uint32_t getSampleRate() {
        return mDecConfig->nSampleRate;
    }

    uint32_t isSeekTablePresent() {
        return mDecConfig->nSeekTablePresent;
    }

    /* Set APE decoder configuration for all members */
    void setApeDecConfig(uint8_t* config) {
        std::shared_ptr<ApeDecConfig> param = mDecConfig;
        memcpy(&param->nCompatibleVersion, config + kKeyIndexApeCompatibleVersion,
                sizeof(param->nCompatibleVersion));
        memcpy(&param->nCompressionLevel, config + kKeyIndexApeCompressionLevel,
               sizeof(param->nCompressionLevel));
        memcpy(&param->nFormatFlags, config + kKeyIndexApeFormatFlags,
               sizeof(param->nFormatFlags));
        memcpy(&param->nBlocksPerFrame, config + kKeyIndexApeBlocksPerFrame,
               sizeof(param->nBlocksPerFrame));
        memcpy(&param->nFinalFrameBlocks, config + kKeyIndexApeFinalFrameBlocks,
               sizeof(param->nFinalFrameBlocks));
        memcpy(&param->nTotalFrames, config + kKeyIndexApeTotalFrames,
               sizeof(param->nTotalFrames));
        memcpy(&param->nBitsPerSample, config + kKeyIndexApeBitsPerSample,
               sizeof(param->nBitsPerSample));
        memcpy(&param->nChannels, config + kKeyIndexApeNumChannels,
               sizeof(param->nChannels));
        memcpy(&param->nSampleRate, config + kKeyIndexApeSampleRate,
               sizeof(param->nSampleRate));
        memcpy(&param->nSeekTablePresent, config + kKeyIndexApeSeekTablePresent,
               sizeof(param->nSeekTablePresent));
    }

    /* Get APE decoder configuration */
    std::shared_ptr<ApeDecConfig> getApeDecConfig() {
        return mDecConfig;
    }

private:
    std::shared_ptr<ApeDecConfig> mDecConfig;
};

};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_APE_CODEC_CONFIG_H_
