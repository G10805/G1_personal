/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_AMR_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_AMR_CODEC_CONFIG_H_

#include <algorithm>
#include <cstdint>
#include <memory>

#include <C2Config.h>
#include <QC2Constants.h>


namespace qc2audio {

enum AmrVersion: uint8_t {
    AMRNB     = 0,
    AMRWB     = 1,
    AMRWBPlus = 2
};

enum AmrParamIndex: uint8_t {
    AMR_MAX_DEC_INPUT_BUF_SIZE  = 0,
    AMR_MAX_DEC_OUTPUT_BUF_SIZE,
    AMR_MAX_ENC_INPUT_BUF_SIZE,
    AMR_MAX_ENC_OUTPUT_BUF_SIZE,
    AMR_MIN_FRAME_SIZE,
    AMR_MIN_BIT_RATE,
    AMR_MAX_BIT_RATE,
    AMR_MAX_CHANNELS,
    AMR_SAMPLE_RATE_RANGE,
    AMR_BITS_PER_SAMPLE_SET
};

// tuple params:  max_dec_ip_buffer_size, max_dec_op_buffer_size, max_enc_ip_buffer_size,
//                max_enc_op_buffer_size, min_frame_size, min_bitrate, max_bitrate,
//                channels, sampling_rate, bits_per_sample
static std::vector<std::tuple <uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
        uint32_t, uint8_t, std::pair<uint32_t, uint32_t>, std::vector<uint32_t>>> AmrMinMaxList =
                    {
                        {36, 320, 320, 36, 160, 4750, 12200, 1, {8000, 8000}, {16}},       //AMRNB
                        {64, 640, 640, 64, 320, 6600, 23200, 1, {16000, 16000}, {16}},      //AMRWB
                        {2048, 30720, 0, 0, 320, 5200, 48000, 2, {8000, 48000}, {16}}       //AMRWB+
                    };

struct AmrDefaults {
    static const uint32_t AmrNbDefaultBitsPerSample = 16;
    static const uint32_t AmrNbDefaultNumChannels = 1;
    static const uint32_t AmrNbDefaultSampleRate = 8000;
    static const uint32_t AmrNbDefaultBitRate = 4750;
    static const uint32_t AmrWbDefaultBitsPerSample = 16;
    static const uint32_t AmrWbDefaultNumChannels = 1;
    static const uint32_t AmrWbDefaultSampleRate = 16000;
    static const uint32_t AmrWbDefaultBitRate = 6600;
    static const uint32_t AmrWbPlusDefaultBitsPerSample = 16;
    static const uint32_t AmrWbPlusDefaultNumChannels = 2;
    static const uint32_t AmrWbPlusDefaultSampleRate = 48000;
    static const uint32_t AmrWbPlusDefaultBitRate = 5200;
};

class QC2AmrCodecConfig {
    uint32_t max_dec_input_buf_size;
    uint32_t max_dec_output_buf_size;
    uint32_t max_enc_input_buf_size;
    uint32_t max_enc_output_buf_size;
    uint32_t min_bit_rate;
    uint32_t max_bit_rate;
    uint32_t min_frame_size;
    uint8_t  max_channels;
    std::pair<uint32_t, uint32_t> sample_rate_range;
    std::vector<uint32_t> bits_per_sample_set;

protected:
    typedef struct {
        uint8_t channel_count;
        uint8_t bits_per_sample;
        uint32_t sample_rate;
        uint32_t bit_rate;
    }codec_config;
    std::shared_ptr<codec_config> mAmrCodecConfig;

public:

    QC2AmrCodecConfig(uint32_t maxDecInputBufSize, uint32_t maxDecOutputBufSize,
                        uint32_t maxEncInputBufSize, uint32_t maxEncOutputBufSize,
                        uint32_t minFrameSize, uint32_t minBitRate,
                        uint32_t maxBitRate, uint8_t maxChannels,
                        std::pair<uint32_t, uint32_t> SampleRateRange,
                        std::vector<uint32_t> BitsPerSampleRange) {
        max_dec_input_buf_size = maxDecInputBufSize;
        max_dec_output_buf_size = maxDecOutputBufSize;
        max_enc_input_buf_size = maxEncInputBufSize;
        max_enc_output_buf_size = maxEncOutputBufSize;
        min_bit_rate = minBitRate;
        max_bit_rate = maxBitRate;
        min_frame_size = minFrameSize;
        max_channels = maxChannels;
        sample_rate_range = std::move(SampleRateRange);
        bits_per_sample_set = std::move(BitsPerSampleRange);
        
        mAmrCodecConfig = std::make_shared<codec_config>();
        mAmrCodecConfig->sample_rate = std::get<MIN_SAMPLE_RATE_IDX>(sample_rate_range);
        mAmrCodecConfig->channel_count = max_channels;
        mAmrCodecConfig->bits_per_sample = bits_per_sample_set[0];
    }

    bool isSampleRateValid(uint32_t sampleRate) {
        return (sampleRate >= std::get<MIN_SAMPLE_RATE_IDX>(sample_rate_range) &&
                sampleRate <= std::get<MAX_SAMPLE_RATE_IDX>(sample_rate_range));
    }

    bool isChannelConfigValid(uint8_t channelCount) {
        return (channelCount > 0 && channelCount <= max_channels);
    }

    bool isBitsPerSampleValid(uint32_t bps) {
        return (std::find(std::begin(bits_per_sample_set),
            std::end(bits_per_sample_set), bps) != std::end(bits_per_sample_set));
    }

    bool isBitRateValid(uint32_t bitRate) {
        return (min_bit_rate <= bitRate && bitRate <= max_bit_rate);
    }

    void setChannelCount(uint8_t numChannels) {
        mAmrCodecConfig->channel_count = numChannels;
    }
    void setBitsPerSample(uint8_t bps) {
        mAmrCodecConfig->bits_per_sample = bps;
    }
    void setBitRate(uint32_t bitRate) {
        mAmrCodecConfig->bit_rate = bitRate;
    }
    void setSampleRate(uint32_t sampleRate) {
        mAmrCodecConfig->sample_rate = sampleRate;
    }
    uint8_t getChannelCount() {
        return mAmrCodecConfig->channel_count;
    }
    uint8_t getBitsPerSample() {
        return mAmrCodecConfig->bits_per_sample;
    }
    uint32_t getBitRate() {
        return mAmrCodecConfig->bit_rate;
    }
    uint32_t getSampleRate() {
        return mAmrCodecConfig->sample_rate;
    }
};



class QC2AmrNbCodecConfig: public QC2AmrCodecConfig {

public:
    // Constructor params:  max_dec_ip_buffer_size, max_dec_op_buffer_size, max_enc_ip_buffer_size,
    //                      max_enc_op_buffer_size, min_frame_size, min_bitrate, max_bitrate, channels,
    //                      sampling_rate, bits_per_sample
    QC2AmrNbCodecConfig()
            : QC2AmrCodecConfig(std::get<AMR_MAX_DEC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_MAX_DEC_OUTPUT_BUF_SIZE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_MAX_ENC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_MAX_ENC_OUTPUT_BUF_SIZE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_MIN_FRAME_SIZE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_MIN_BIT_RATE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_MAX_BIT_RATE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_MAX_CHANNELS>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_SAMPLE_RATE_RANGE>(AmrMinMaxList[AMRNB]),
                                std::get<AMR_BITS_PER_SAMPLE_SET>(AmrMinMaxList[AMRNB]))
                                {}
    ~QC2AmrNbCodecConfig() = default;
};

class QC2AmrWbCodecConfig: public QC2AmrCodecConfig {
public:
    // Constructor params:  max_dec_ip_buffer_size, max_dec_op_buffer_size, max_enc_ip_buffer_size,
    //                      max_enc_op_buffer_size, min_frame_size, min_bitrate, max_bitrate, channels,
    //                      sampling_rate, bits_per_sample
    QC2AmrWbCodecConfig()
            : QC2AmrCodecConfig(std::get<AMR_MAX_DEC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_MAX_DEC_OUTPUT_BUF_SIZE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_MAX_ENC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_MAX_ENC_OUTPUT_BUF_SIZE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_MIN_FRAME_SIZE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_MIN_BIT_RATE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_MAX_BIT_RATE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_MAX_CHANNELS>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_SAMPLE_RATE_RANGE>(AmrMinMaxList[AMRWB]),
                                std::get<AMR_BITS_PER_SAMPLE_SET>(AmrMinMaxList[AMRWB]))
                                {}
    ~QC2AmrWbCodecConfig() = default;
};

class QC2AmrWbPlusCodecConfig: public QC2AmrCodecConfig {
public:
    // Constructor params:  max_dec_ip_buffer_size, max_dec_op_buffer_size, max_enc_ip_buffer_size,
    //                      max_enc_op_buffer_size, min_frame_size, min_bitrate, max_bitrate, channels,
    //                      sampling_rate, bits_per_sample
    QC2AmrWbPlusCodecConfig()
            : QC2AmrCodecConfig(std::get<AMR_MAX_DEC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_MAX_DEC_OUTPUT_BUF_SIZE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_MAX_ENC_INPUT_BUF_SIZE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_MAX_ENC_OUTPUT_BUF_SIZE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_MIN_FRAME_SIZE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_MIN_BIT_RATE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_MAX_BIT_RATE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_MAX_CHANNELS>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_SAMPLE_RATE_RANGE>(AmrMinMaxList[AMRWBPlus]),
                                std::get<AMR_BITS_PER_SAMPLE_SET>(AmrMinMaxList[AMRWBPlus]))
                                {}
    ~QC2AmrWbPlusCodecConfig() = default;
};

};  // namespace qc2audio

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_AMR_CODEC_CONFIG_H_
