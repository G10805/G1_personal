/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
 */
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_DSD_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_DSD_CODEC_CONFIG_H_

#include <C2Config.h>
#include <QC2Constants.h>

#include <algorithm>
#include <cstdint>
#include <memory>

namespace qc2audio {

enum DsdParamIndex : uint8_t {
    DSD_MAX_INPUT_BUF_SIZE = 0,
    DSD_MAX_OUTPUT_BUF_SIZE,
    DSD_MAX_OUTPUT_CHANNELS,
    DSD_OUTPUT_SAMPLE_RATE_RANGE,
    DSD_OUTPUT_BITS_PER_SAMPLE_SET,
    DSD_NUM_OUT_SAMPLES_PER_CHANNEL_SET,
    DSD_INPUT_SAMPLE_RATE_RANGE
};

// output config
enum DsdSupportedBitsPerSample : uint32_t {
    DSD_PCM_BIT_DEPTH_16 = 16,
    DSD_PCM_BIT_DEPTH_24 = 24,
    DSD_PCM_BIT_DEPTH_32 = 32
};

// input config to decoder
enum DsdSupportedNumOutSamplesPerChannel : uint32_t {
    DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L1 = 882,
    DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L2 = 1764,
    DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L3 = 3528,
    DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L4 = 7056
};

// input config to decoder
enum DsdSupportedGain : uint32_t {
    DSD_GAIN_L1 = 86393,  // 2.4dB
    DSD_GAIN_L2 = 91512   // 2.9dB
};

// input config to decoder
struct DsdInputSampleRate {
    static const uint32_t SR_DSD64 = 2822400;
    static const uint32_t SR_DSD128 = 5644800;
    static const uint32_t SR_DSD256 = 11289600;
    static const uint32_t SR_DSD512 = 22579200;
};

// output config from decoder
struct DsdOutputSampleRate {
    static const uint32_t SR_DSD176_4K = 176400; //176.4k
    static const uint32_t SR_DSD352_8K = 352800; //352.8k
};

// input config to decoder
enum DsdToOutQFormat : uint8_t {
    PCM_QFMT_Q27 = 0,  // Q4.27: CAN express greater than 0dB
    PCM_QFMT_Q23,      // Q23 packed: CANNOT express greater than 0dB
    PCM_QFMT_Q31,      // Q31: CANNOT express greater than 0dB
    PCM_QFMT_NOT_SUPPORTED
};

static const std::tuple<uint32_t, uint32_t, uint32_t,
                        std::pair<uint32_t, uint32_t>, std::vector<uint32_t>,
                        std::vector<uint32_t>, std::pair<uint32_t, uint32_t>>
    DsdDecMinMaxList = {
        // DSD MAX input size
        65536 * 4,  // 384KB
        // DSD MAX output size
        4096 * 48 * 4,  // 768KB
        // DSD_MAX_OUTPUT_CHANNELS (1 to 6)
        6,
        // DSD_OUTPUT_SAMPLE_RATE_RANGE
        {DsdOutputSampleRate::SR_DSD176_4K, DsdOutputSampleRate::SR_DSD352_8K},
        // DSD_OUTPUT_BITS_PER_SAMPLE_SET
        {DSD_PCM_BIT_DEPTH_16, DSD_PCM_BIT_DEPTH_24, DSD_PCM_BIT_DEPTH_32},
        // DSD_NUM_OUT_SAMPLES_PER_CHANNEL_SET
        {DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L1, DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L2,
         DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L3, DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L4},
        // DSD_INPUT_SAMPLE_RATE_RANGE
        {DsdInputSampleRate::SR_DSD64, DsdInputSampleRate::SR_DSD512}};

struct DsdDefaults {
    static const uint32_t DSDDefaultOutputSampleRate =
        DsdOutputSampleRate::SR_DSD176_4K;  // 176.4k
    static const uint32_t DSDDefaultInputSampleRate =
        DsdInputSampleRate::SR_DSD64;
    static const uint32_t DSDDefaultNumChannels = 2;
    static const uint32_t DSDDefaultBitsPerSample = DSD_PCM_BIT_DEPTH_24;
    static const uint32_t DSDDefaultNumOutSamplesPerChannel =
        DSD_NUM_OUT_SAMPLES_PER_CHANNEL_L3;
    static const uint32_t DSDDefaultGain = DSD_GAIN_L1;
    static const uint32_t DSDDefaultQFormat = PCM_QFMT_Q23;
};

class QC2DsdCodecConfig {
   public:
    QC2DsdCodecConfig(uint32_t maxInputBufSize, uint32_t maxOutputBufSize,
                      uint16_t maxChannels,
                      std::pair<uint32_t, uint32_t> OutputSampleRateRange,
                      std::vector<uint32_t> BitsPerSampleRange,
                      std::vector<uint32_t> numOutSamplesPerChannelSet,
                      std::pair<uint32_t, uint32_t> InputSampleRateRange) {
        mMaxInputBufSize = maxInputBufSize;
        mMaxOutputBufSize = maxOutputBufSize;
        mMaxChannels = maxChannels;
        mInputSampleRateRange = std::move(InputSampleRateRange);
        mOutputBitsPerSampleSet = std::move(BitsPerSampleRange);
        mNumOutSamplesPerChannelSet = std::move(numOutSamplesPerChannelSet);
        mOutputSampleRateRange = std::move(OutputSampleRateRange);
    }

    uint32_t getMaxInputBufSize() { return mMaxInputBufSize; }

    uint32_t getMaxOutputBufSize() { return mMaxOutputBufSize; }

    uint16_t getMaxChannels() { return mMaxChannels; }

    std::vector<uint32_t> getSupportedBitsPerSampleSet() {
        return mOutputBitsPerSampleSet;
    }

    std::pair<uint32_t, uint32_t> getSupportedInputSampleRateSet() {
        return mInputSampleRateRange;
    }

    std::pair<uint32_t, uint32_t> getSupportedOutputSampleRateSet() {
        return mOutputSampleRateRange;
    }

    std::vector<uint32_t> getSupportedNumOutSamplesPerChannelSet() {
        return mNumOutSamplesPerChannelSet;
    }

    bool isInputSampleRateValid(uint32_t sampleRate) {
        return (sampleRate >= std::get<MIN_SAMPLE_RATE_IDX>(mInputSampleRateRange) &&
                sampleRate <= std::get<MAX_SAMPLE_RATE_IDX>(mInputSampleRateRange));
    }

    bool isChannelCountValid(uint8_t channelCount) {
        return (channelCount > 0 && channelCount <= mMaxChannels);
    }

    bool isOutputBitsPerSampleValid(uint32_t bps) {
        return (std::find(std::begin(mOutputBitsPerSampleSet),
                          std::end(mOutputBitsPerSampleSet),
                          bps) != std::end(mOutputBitsPerSampleSet));
    }

    bool isNumOutSamplesPerChannelValid(uint32_t numOutSamplesPerChannel) {
        return (std::find(std::begin(mNumOutSamplesPerChannelSet),
                          std::end(mNumOutSamplesPerChannelSet),
                          numOutSamplesPerChannel) !=
                std::end(mNumOutSamplesPerChannelSet));
    }

   protected:
    void setMaxInputBufferSize(uint32_t maxInBufSize) {
        mMaxInputBufSize = maxInBufSize;
    }

    void setMaxOutputBufferSize(uint32_t maxOutBufSize) {
        mMaxOutputBufSize = maxOutBufSize;
    }

   private:
    uint32_t mMaxInputBufSize;
    uint32_t mMaxOutputBufSize;
    uint16_t mMaxChannels;
    std::pair<uint32_t, uint32_t> mInputSampleRateRange;
    std::pair<uint32_t, uint32_t> mOutputSampleRateRange;
    std::vector<uint32_t> mOutputBitsPerSampleSet;
    std::vector<uint32_t> mNumOutSamplesPerChannelSet;
};

class QC2DsdDecConfig : public QC2DsdCodecConfig {
   public:
    QC2DsdDecConfig()
        : QC2DsdCodecConfig(
              std::get<DSD_MAX_INPUT_BUF_SIZE>(DsdDecMinMaxList),
              std::get<DSD_MAX_OUTPUT_BUF_SIZE>(DsdDecMinMaxList),
              std::get<DSD_MAX_OUTPUT_CHANNELS>(DsdDecMinMaxList),
              std::get<DSD_OUTPUT_SAMPLE_RATE_RANGE>(DsdDecMinMaxList),
              std::get<DSD_OUTPUT_BITS_PER_SAMPLE_SET>(DsdDecMinMaxList),
              std::get<DSD_NUM_OUT_SAMPLES_PER_CHANNEL_SET>(DsdDecMinMaxList),
              std::get<DSD_INPUT_SAMPLE_RATE_RANGE>(DsdDecMinMaxList)) {
        mDecInputConfig = std::make_shared<DsdDecInputConfig>();
        mDecOutputFmt = std::make_shared<DsdDecOutputFmt>();
    }

    /*
    dsd_dec_format_t contains output information according
    to the input from configurations.
    */
    typedef struct {
        uint32_t nOutSampleRate;
        /**< Output sampling rate. */

        uint32_t nDownRatio;
        /**< The ratio between input and output sampling rate. */

        uint32_t nInputBytesPerCh;
        /**< Number of input bytes per channel. */

        uint32_t nInputBytesPerFrame;
        /**< Number of input bytes per frame. */

        uint32_t nOutSampleFrame;
        /**< Number of output samples per frame. */

        uint32_t nOutBytesSample;
        /**< Number of bytes per output sample. */

        uint32_t nOutChannels;
        /**< Number of channels for output. */
    } DsdDecOutputFmt;

    /*
    dsd_dec_config_t contains configurations for initializing
    decoder library memory.
    */
    typedef struct {
        uint32_t nSampleRateInput;
        /**< Input sampling rate.
             Supported rates are 64*44.1e3, 128*44.1e3, 256*44.1e3, and
           512*44.1e3 [Hz]. */

        int32_t nOutSamplePerCh;
        /**< Number of output PCM samples per channel at every frame
           (5msec/10msec). (882/1764), (882/1764), (1764/3528), (1764/3528), at
           sampling rates 64*44.1e3, 128*44.1e3, 256*44.1e3, and 512*44.1e3
           [Hz], respectively. */

        uint32_t nchannels;
        /**< Number of channels for input == number of channels for output,
             Supported number of channels are 1 to 6. */

        uint32_t nGain;
        /**< Setting gain is only allowed during initialization.
             Decoder must be reset to set a new gain.
            If gain is set to zero, default gain will be applied. */

        uint32_t qFormat;
        /**< output Q format.
             Supported formats are Q27, Q23, Q31*/
    } DsdDecInputConfig;

    /* Individual input setters and getters */
    void setInputSampleRate(uint32_t sampleRate) {
        mDecInputConfig->nSampleRateInput = sampleRate;
    }

    void setInputNumOutSamplesPerChannel(uint32_t numOutSamplesPerChannel) {
        mDecInputConfig->nOutSamplePerCh = (int32_t)numOutSamplesPerChannel;
    }

    void setInputChannelCount(uint32_t channelCount) {
        mDecInputConfig->nchannels = channelCount;
    }

    void setInputGain(uint32_t gain) { mDecInputConfig->nGain = gain; }

    void setInputQFormat(uint32_t qFormat) { mDecInputConfig->qFormat = qFormat; }

    uint32_t getInputSampleRate() { return mDecInputConfig->nSampleRateInput; }

    uint32_t getInputNumOutSamplesPerChannel() {
        return (int32_t)mDecInputConfig->nOutSamplePerCh;
    }

    uint32_t getInputChannelCount() { return mDecInputConfig->nchannels; }

    uint32_t getInputGain() { return mDecInputConfig->nGain; }

    uint32_t getInputQFormat() { return mDecInputConfig->qFormat; }

    /* Get DSD decoder input configuration */
    std::shared_ptr<DsdDecInputConfig> getDsdDecInputConfig() {
        return mDecInputConfig;
    }

    // TODO may be write Individual output setters and getters
    void setOutputBitsPerSample(uint32_t bitsPerSample) {
        mOutputBitsPerSample = bitsPerSample;
        if (mOutputBitsPerSample == DSD_PCM_BIT_DEPTH_32) {
            setInputQFormat(PCM_QFMT_Q31);
        }
    }

    uint32_t getOutputBitsPerSample() { return mOutputBitsPerSample; }

    /* Get DSD decoder output configuration */
    std::shared_ptr<DsdDecOutputFmt> getDsdDecOutputFmt() { return mDecOutputFmt; }

   private:
    std::shared_ptr<DsdDecInputConfig> mDecInputConfig;
    uint32_t mOutputBitsPerSample;
    std::shared_ptr<DsdDecOutputFmt> mDecOutputFmt;
};

};  // namespace qc2audio

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_DSD_CODEC_CONFIG_H_
