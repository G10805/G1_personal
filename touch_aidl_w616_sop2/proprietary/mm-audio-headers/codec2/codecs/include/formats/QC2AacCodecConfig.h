/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODECS_INCLUDE_FORMATS_QC2_AAC_CODEC_CONFIG_H_
#define CODEC2_CODECS_INCLUDE_FORMATS_QC2_AAC_CODEC_CONFIG_H_

#include <algorithm>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include <C2Config.h>
#include <QC2Constants.h>


namespace qc2audio {

enum AacFormatFlag: uint16_t {
    AACFormat_ADTS    = 0x0,
    AACFormat_LOAS    = 0x1,
    AACFormat_ADIF    = 0x2,
    AACFormat_RAW     = 0x3,
    AACFormat_LATM    = 0x4,
    AACFormat_DEFAULT = AACFormat_RAW,
};

enum AacEncodingMode : uint16_t {
    AACEncodingMode_AAC_LC     = 0x2,
    AACEncodingMode_DEFAULT    = AACEncodingMode_AAC_LC,
    AACEncodingMode_AAC_HE     = 0x5,
    AACEncodingMode_AAC_HE_PS  = 0x1d,
};

// C2Config::profile_t to AacEncodingMode
static const std::unordered_map<uint32_t, uint16_t> C2ProfileToAacEncodingMode = {
    {C2Config::PROFILE_AAC_LC, AACEncodingMode_AAC_LC},
    {C2Config::PROFILE_AAC_HE, AACEncodingMode_AAC_HE},
    {C2Config::PROFILE_AAC_HE_PS, AACEncodingMode_AAC_HE_PS},
};

// AacEncodingMode to C2Config::profile_t
static const std::unordered_map<uint16_t, uint32_t> AacEncodingModeToC2Profile = {
    {AACEncodingMode_AAC_LC, C2Config::PROFILE_AAC_LC},
    {AACEncodingMode_AAC_HE, C2Config::PROFILE_AAC_HE},
    {AACEncodingMode_AAC_HE_PS, C2Config::PROFILE_AAC_HE_PS},
};

static const std::vector<uint32_t> sSupportedHwAacEncoderProfiles = { C2Config::PROFILE_AAC_LC };

// C2Config::aac_packaging_t to AacFormatFlag
static const std::unordered_map<uint32_t, uint16_t> C2PackagingTypeToAacFormatFlag = {
    {C2Config::AAC_PACKAGING_RAW, AACFormat_RAW},
    {C2Config::AAC_PACKAGING_ADTS, AACFormat_ADTS},
};

// AacFormatFlag to C2Config::aac_packaging_t
static const std::unordered_map<uint16_t, uint32_t> AacFormatFlagToC2PackagingType = {
    {AACFormat_RAW, C2Config::AAC_PACKAGING_RAW},
    {AACFormat_ADTS, C2Config::AAC_PACKAGING_ADTS},
};

enum AacChannels : uint32_t {
    AAC_MONO = 1,
    AAC_STEREO = 2,
};

// RAW MP4FF header
static const constexpr uint32_t AUDAAC_MAX_MP4FF_HEADER_LENGTH = 2;
static const constexpr uint32_t AUDAAC_MP4FF_OBJ_TYPE  = 5;
static const constexpr uint32_t AUDAAC_MP4FF_FREQ_IDX = 4;
static const constexpr uint32_t AUDAAC_MP4FF_CH_CONFIG = 4;


static const std::unordered_map<uint32_t, uint32_t> sampleRateToSampleIdx = {
    {8000,  0x0b},
    {11025, 0x0a},
    {12000, 0x09},
    {16000, 0x08},
    {22050, 0x07},
    {24000, 0x06},
    {32000, 0x05},
    {44100, 0x04},
    {48000, 0x03},
    {64000, 0x02},
};

struct AacDefaults {
    static const uint32_t AacDefaultBitsPerSample = 16;
    static const uint32_t AacDefaultNumChannels = 2;
    static const uint32_t AacDefaultSampleRate = 48000;
    static const uint32_t AacDefaultBitRate = 96000;
    static const uint32_t AacDefaultProfile = C2Config::PROFILE_AAC_LC;
    static const uint32_t AacDefaultPackagingType = C2Config::AAC_PACKAGING_RAW;
};

enum AacParamIndex : uint32_t {
    AAC_MAX_INPUT_BUF_SIZE = 0,
    AAC_MAX_OUTPUT_BUF_SIZE,
    AAC_MIN_FRAME_SIZE,
    AAC_MAX_CHANNELS,
    AAC_SAMPLE_RATE_RANGE,
    AAC_BITS_PER_SAMPLE_SET
};

enum AacProfileIndex : uint32_t {
    AAC_DEFAULT = 0x0,
    AAC_LC = AAC_DEFAULT,
    AAC_HE = 0x1,
    AAC_HE_PS = 0x2,
};

enum AacBitRateRange : uint32_t {
    AAC_MIN_BITRATE = 0,
    AAC_MAX_BITRATE
};

enum AacBitRateLimits : uint32_t {
    AAC_MIN_BITRATE_LIMIT = 24000,
    AAC_MAX_BITRATE_LIMIT = 192000,
};

static const std::unordered_map<uint32_t, uint32_t> C2ProfileToAacProfileIndex = {
    {C2Config::PROFILE_AAC_LC, AAC_LC},
    {C2Config::PROFILE_AAC_HE, AAC_HE},
    {C2Config::PROFILE_AAC_HE_PS, AAC_HE_PS},
};

static const std::vector<uint32_t> AacEncSampleRateSet = {
    8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
};

// tuple params:  AAC_MAX_INPUT_BUF_SIZE, AAC_MAX_OUTPUT_BUF_SIZE, AAC_MIN_FRAME_SIZE,
//                AAC_MAX_CHANNELS, AAC_SAMPLE_RATE_RANGE, AAC_BITS_PER_SAMPLE_SET
static std::vector<std::tuple<uint32_t, uint32_t, uint32_t, uint8_t,
                std::pair<uint32_t, uint32_t>, std::vector<uint32_t>>> AacEncMinMaxList = {
    {8192, 2048, 1024, 2, {8000, 48000}, {16}},  // AAC-LC
    {8192, 2048, 2048, 2, {24000, 48000}, {16}},   // AAC-HE V1
    {8192, 2048, 2048, 2, {24000, 48000}, {16}},   // AAC-HE V2
};

// tuple params:  AAC_MAX_INPUT_BUF_SIZE, AAC_MAX_OUTPUT_BUF_SIZE, AAC_MIN_FRAME_SIZE,
//                AAC_MAX_CHANNELS, AAC_SAMPLE_RATE_RANGE, AAC_BITS_PER_SAMPLE_SET
static std::vector<std::tuple <uint32_t, uint32_t, uint32_t, uint8_t,
                std::pair<uint32_t, uint32_t>, std::vector<uint32_t>>> AacDecMinMaxList = {
    {6146, 65536, 1024, 6, {8000, 96000}, {16, 24}}  // for all aac profiles
   //  8000, 1536000}  // for all aac profiles
};


enum AacDecDefaults : uint32_t {
    kMinBitrate = 8000,
    kMaxBitrate = 153600
};

class QC2AacCodecConfig {

public:

    typedef struct {
        uint32_t sampleRate;
        uint16_t channelCount;
        uint32_t bitsPerSample;
        uint32_t streamFormat;
    } AacStreamConfig;

    QC2AacCodecConfig(uint32_t maxInputBufSize, uint32_t maxOutputBufSize,
                      uint32_t minFrameSize, uint16_t maxChannels,
                      std::pair<uint32_t, uint32_t> SampleRateRange,
                      std::vector<uint32_t> BitsPerSampleRange) {
        mMaxInputBufSize = maxInputBufSize;
        mMaxOutputBufSize = maxOutputBufSize;
        mMinFrameSize = minFrameSize;
        mMaxChannels = maxChannels;
        mSampleRateRange = std::move(SampleRateRange);
        mBitsPerSampleSet = std::move(BitsPerSampleRange);
        mStreamConfig = std::make_shared<AacStreamConfig>();
    }

    ~QC2AacCodecConfig() = default;

    void setChannelCount(uint16_t numChannels) {
        mStreamConfig->channelCount = numChannels;
    }

    void setBitsPerSample(uint32_t bps) {
        mStreamConfig->bitsPerSample = bps;
    }

    void setSampleRate(uint32_t sampleRate) {
        mStreamConfig->sampleRate = sampleRate;
    }

    uint16_t getChannelCount() {
        return mStreamConfig->channelCount;
    }

    uint16_t getBitsPerSample() {
        return mStreamConfig->bitsPerSample;
    }

    uint32_t getSampleRate() {
        return mStreamConfig->sampleRate;
    }

    uint32_t getMaxInputBufSize() { return mMaxInputBufSize; }

    uint32_t getMaxOutputBufSize() { return mMaxOutputBufSize; }

    uint32_t getMinFrameSize() { return mMinFrameSize; }

    uint16_t getMaxChannels() { return mMaxChannels; }

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

    std::shared_ptr<AacStreamConfig> getAacStreamConfig() {
        return mStreamConfig;
    }

protected:
    void setMaxInputBufferSize(uint32_t maxInBufSize) {
        mMaxInputBufSize = maxInBufSize;
    }

    void setMaxOutputBufferSize(uint32_t maxOutBufSize) {
        mMaxOutputBufSize = maxOutBufSize;
    }

    void setMinFrameSize(uint32_t minFrameSize) {
        mMinFrameSize = minFrameSize;
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
    uint32_t mMinFrameSize;
    uint16_t  mMaxChannels;
    std::pair<uint32_t, uint32_t> mSampleRateRange;
    std::vector<uint32_t> mBitsPerSampleSet;
    std::shared_ptr<AacStreamConfig> mStreamConfig;
};

class QC2AacEncConfig : public QC2AacCodecConfig {

public:

    typedef struct {
        uint16_t encMode;
        uint16_t formatFlag;
    } AacEncOutputConfig;

    typedef struct {
        uint32_t bitRate;
        AacEncOutputConfig encOpConfig;
    } AacEncConfig;

    // Constructor params:
    // AAC_MAX_INPUT_BUF_SIZE, AAC_MAX_OUTPUT_BUF_SIZE, AAC_MIN_FRAME_SIZE,
    // AAC_MAX_CHANNELS, AAC_SAMPLE_RATE_RANGE, AAC_BITS_PER_SAMPLE_SET
    QC2AacEncConfig(uint32_t profile)
            : QC2AacCodecConfig(std::get<AAC_MAX_INPUT_BUF_SIZE>(AacEncMinMaxList[profile]),
                                std::get<AAC_MAX_OUTPUT_BUF_SIZE>(AacEncMinMaxList[profile]),
                                std::get<AAC_MIN_FRAME_SIZE>(AacEncMinMaxList[profile]),
                                std::get<AAC_MAX_CHANNELS>(AacEncMinMaxList[profile]),
                                std::get<AAC_SAMPLE_RATE_RANGE>(AacEncMinMaxList[profile]),
                                std::get<AAC_BITS_PER_SAMPLE_SET>(AacEncMinMaxList[profile])) {
        mEncConfig = std::make_shared<AacEncConfig>();
        mProfile = profile;
        mMinBitRate = 0;
        mMaxBitRate = 0;
    }
    ~QC2AacEncConfig() = default;

    bool isBitRateValid(uint32_t bitRate) {
        return (getMinBitRate() <= bitRate  && bitRate <= getMaxBitRate());
    }

    bool isProfileValid(uint32_t profile) {
        return (C2ProfileToAacEncodingMode.find(profile) != C2ProfileToAacEncodingMode.end());
    }

    bool isStreamFormatValid(uint32_t streamFormat) {
        return (C2PackagingTypeToAacFormatFlag.find(streamFormat) != C2PackagingTypeToAacFormatFlag.end());
    }

    void setEncodingMode(uint32_t profile) {
        uint16_t encMode = C2ProfileToAacEncodingMode.at(profile);
        mEncConfig->encOpConfig.encMode = encMode;

        mProfile = C2ProfileToAacProfileIndex.at(profile);
        setMaxInputBufferSize(std::get<AAC_MAX_INPUT_BUF_SIZE>(AacEncMinMaxList[mProfile]));
        setMaxOutputBufferSize(std::get<AAC_MAX_OUTPUT_BUF_SIZE>(AacEncMinMaxList[mProfile]));
        setMinFrameSize(std::get<AAC_MIN_FRAME_SIZE>(AacEncMinMaxList[mProfile]));
        setMaxChannels(std::get<AAC_MAX_CHANNELS>(AacEncMinMaxList[mProfile]));
        setSampleRateRange(std::get<AAC_SAMPLE_RATE_RANGE>(AacEncMinMaxList[mProfile]));
        setBitsPerSampleSet(std::get<AAC_BITS_PER_SAMPLE_SET>(AacEncMinMaxList[mProfile]));
    }

    void setFormatFlag(uint32_t streamFormat) {
        uint16_t formatFlag = C2PackagingTypeToAacFormatFlag.at(streamFormat);
        mEncConfig->encOpConfig.formatFlag = formatFlag;
    }

    // Bitrate is dependent on sample rate and channels
    QC2Status setBitRate(uint32_t bitRate) {
        if (!getAacStreamConfig() || !getAacStreamConfig()->sampleRate ||
                !getAacStreamConfig()->channelCount) {
            QLOGE("%s: stream config not populated, setting default bitrate", __func__);
            mEncConfig->bitRate = AacDefaults::AacDefaultBitRate;
            return QC2_CANNOT_DO;
        }
        std::tuple<uint32_t, uint32_t> aacBitRateRange;
        uint32_t profile = AacEncodingModeToC2Profile.at(getEncodingMode());
        if (getAacBitRateRange(profile, getSampleRate(),
                        getChannelCount(), aacBitRateRange) != QC2_OK) {
            QLOGE("%s: failed to fetch bitrate range, setting default bitrate", __func__);
            mEncConfig->bitRate = AacDefaults::AacDefaultBitRate;
            return QC2_CANNOT_DO;
        }
        setMinBitRate(std::get<AAC_MIN_BITRATE>(aacBitRateRange));
        setMaxBitRate(std::get<AAC_MAX_BITRATE>(aacBitRateRange));
        QLOGV("%s: min aac bitrate %d, max aac bitrate %d", __func__,
                getMinBitRate(), getMaxBitRate());
        if (!isBitRateValid(bitRate)) {
            uint32_t origBitRate = bitRate;
            if (bitRate > getMaxBitRate()) {
                bitRate = getMaxBitRate();
            } else {
                bitRate = getMinBitRate();
            }
            QLOGW("%s: Bitrate %d not supported for this profile. Correcting it to %d",
                   __func__, origBitRate, bitRate);
        }
        mEncConfig->bitRate = bitRate;
        return QC2_OK;
    }

    static QC2Status getAacBitRateRange(
                uint32_t profile, uint32_t sampleRate, uint32_t channels,
                std::tuple<uint32_t, uint32_t>& minMaxBitRates) {
        uint32_t profileIndex = C2ProfileToAacProfileIndex.at(profile);
        auto isSampleRateValid = [&](auto profileIndex, auto sampleRate) {
            auto sampleRateRange =
                std::get<AAC_SAMPLE_RATE_RANGE>(AacEncMinMaxList[profileIndex]);
            return (sampleRate >= std::get<MIN_SAMPLE_RATE_IDX>(sampleRateRange) &&
                    sampleRate <= std::get<MAX_SAMPLE_RATE_IDX>(sampleRateRange));
        };
        auto isChannelCountValid = [&](auto profileIndex, auto channels) {
            auto maxChannels = std::get<AAC_MAX_CHANNELS>(AacEncMinMaxList[profileIndex]);
                return (channels > 0 && channels <= maxChannels);
        };
        if (!isSampleRateValid(profileIndex, sampleRate) ||
                !isChannelCountValid(profileIndex, channels)) {
            QLOGE("%s: Invalid input parameters", __func__);
            return QC2_BAD_VALUE;
        }
        switch (profile) {
            case C2Config::PROFILE_AAC_LC: {
                if (channels == AAC_MONO) {
                    minMaxBitRates = std::make_tuple(
                                         4000, std::min((uint32_t)192000, 6 * sampleRate));
                } else if (channels == AAC_STEREO) {
                    minMaxBitRates = std::make_tuple(
                                         8000, std::min((uint32_t)384000, 12 * sampleRate));
                }
                break;
            }
            case C2Config::PROFILE_AAC_HE: {
                if (channels == AAC_MONO) {
                    if (sampleRate == 24000 || sampleRate == 32000) {
                        minMaxBitRates = std::make_tuple(
                                             10000, std::min((uint32_t)192000, 6 * sampleRate));
                    } else if (sampleRate == 44100 || sampleRate == 48000) {
                        minMaxBitRates = std::make_tuple(
                                             12000, std::min((uint32_t)192000, 6 * sampleRate));
                    }
                } else if (channels == AAC_STEREO) {
                    if (sampleRate == 24000 || sampleRate == 32000) {
                        minMaxBitRates = std::make_tuple(
                                             18000, std::min((uint32_t)192000, 12 * sampleRate));
                    } else if (sampleRate == 44100 || sampleRate == 48000) {
                        minMaxBitRates = std::make_tuple(
                                             24000, std::min((uint32_t)192000, 12 * sampleRate));
                    }
                }
                break;
            }
            case C2Config::PROFILE_AAC_HE_PS: {
                if (sampleRate == 24000 || sampleRate == 32000) {
                    minMaxBitRates = std::make_tuple(
                                        10000, std::min((uint32_t)192000, 6 * sampleRate));
                } else if (sampleRate == 44100 || sampleRate == 48000) {
                    minMaxBitRates = std::make_tuple(
                                        12000, std::min((uint32_t)192000, 6 * sampleRate));
                }
                break;
            }
            default: {
                QLOGE("%s: Unrecognized AAC profile %d", __func__, profile);
                return QC2_ERROR;
            }
        }
        auto minBitRate = std::get<AAC_MIN_BITRATE>(minMaxBitRates);
        auto maxBitRate = std::get<AAC_MAX_BITRATE>(minMaxBitRates);
        if (minBitRate > AAC_MIN_BITRATE_LIMIT) {
            std::get<AAC_MIN_BITRATE>(minMaxBitRates) = AAC_MIN_BITRATE_LIMIT;
        } else if (maxBitRate > AAC_MAX_BITRATE_LIMIT) {
            std::get<AAC_MAX_BITRATE>(minMaxBitRates) = AAC_MAX_BITRATE_LIMIT;
        }
        return QC2_OK;
    }

    uint32_t getBitRate() { return mEncConfig->bitRate; }

    uint16_t getEncodingMode() { return mEncConfig->encOpConfig.encMode; }

    uint16_t getFormatFlag() { return mEncConfig->encOpConfig.formatFlag; }

    uint32_t getMinBitRate() { return mMinBitRate; }

    uint32_t getMaxBitRate() { return mMaxBitRate; }

    std::shared_ptr<AacEncConfig> getAacEncConfig() { return mEncConfig; }

protected:
    void setMinBitRate(uint32_t minBitRate) { mMinBitRate = minBitRate; }

    void setMaxBitRate(uint32_t maxBitRate) { mMaxBitRate = maxBitRate; }

private:
    uint32_t mProfile;
    uint32_t mMinBitRate;
    uint32_t mMaxBitRate;
    std::shared_ptr<AacEncConfig> mEncConfig;
};


/* The following defines are used to extract all data from the AAC header.
** Each is divided into
**  the byte offset into the header
**  the byte mask for the bits
**  the number of bits to right-shift to extract a 0-based value
*/
#define AAC_SAMPLING_FREQ_INDEX_SIZE          4
#define AAC_ORIGINAL_COPY_SIZE                1
#define AAC_HOME_SIZE                         1
#define AAC_COPYRIGHT_PRESENT_SIZE            1
#define AAC_PROFILE_SIZE                      2
#define AAC_BITSTREAM_TYPE_SIZE               1
#define AAC_BITRATE_SIZE                      23
#define AAC_NUM_PFE_SIZE                      4
#define AAC_BUFFER_FULLNESS_SIZE              20
#define AAC_ELEMENT_INSTANCE_TAG_SIZE         4
#define AAC_NUM_FRONT_CHANNEL_ELEMENTS_SIZE   4
#define AAC_NUM_SIDE_CHANNEL_ELEMENTS_SIZE    4
#define AAC_NUM_BACK_CHANNEL_ELEMENTS_SIZE    4
#define AAC_NUM_LFE_CHANNEL_ELEMENTS_SIZE     2
#define AAC_NUM_ASSOC_DATA_ELEMENTS_SIZE      3
#define AAC_NUM_VALID_CC_ELEMENTS_SIZE        4
#define AAC_MONO_MIXDOWN_PRESENT_SIZE         1
#define AAC_MONO_MIXDOWN_ELEMENT_SIZE         4
#define AAC_STEREO_MIXDOWN_PRESENT_SIZE       1
#define AAC_STEREO_MIXDOWN_ELEMENT_SIZE       4
#define AAC_MATRIX_MIXDOWN_PRESENT_SIZE       1
#define AAC_MATRIX_MIXDOWN_SIZE               3
#define AAC_FCE_SIZE                          5
#define AAC_SCE_SIZE                          5
#define AAC_BCE_SIZE                          5
#define AAC_LFE_SIZE                          4
#define AAC_ADE_SIZE                          4
#define AAC_VCE_SIZE                          5
#define AAC_COMMENT_FIELD_BYTES_SIZE          8
#define AAC_COMMENT_FIELD_DATA_SIZE           8


#define LOAS_GA_SPECIFIC_CONFIG(o) \
    (((o != 5) && (o >=1 ) && (o <= 7)) || \
    ((o != 18) && (o >= 17) && (o <= 23)))

#define LOAS_IS_AUD_OBJ_SUPPORTED(x) \
    ((x == 2) || (x == 4) || (x == 5) || (x == 17))

#define LOAS_IS_SFI_SUPPORTED(x) ((x >= 3) && (x <= 0x0B))

/* c is channel config and o is Audio object type */
#define LOAS_IS_CHANNEL_CONFIG_SUPPORTED(c, o) \
    (((c <= 2) && ((o == 2) || (o == 4) || (o == 5))) || \
    (((c == 1) || (c == 2)) && (o == 17)))

#define LOAS_IS_EXT_SFI_SUPPORTED(x)  ((x >= 0x03) && (x <= 0x08))

/* Extension Flag is e and Audio object type is o */
#define LOAS_IS_EXT_FLG_SUPPORTED(e,o) \
    ((((o == 2) || (o == 4) || (o == 5)) && (e == 0)) || \
    ((o == 17) && (e == 1)))

static std::vector<uint32_t> aac_frequency_index{ 96000, 88200, 64000, 48000,
    44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350,
    0x000,//invalid index
    0x000,//invalid index
    0x001 // no index, value provided is actual frequency
};


class QC2AacDecConfig : public QC2AacCodecConfig {


// QC2AacEncConfig(uint16_t profile)
//             : QC2AacCodecConfig(std::get<AAC_MAX_INPUT_BUF_SIZE>(AacEncMinMaxList[profile]),
//                                 std::get<AAC_MAX_OUTPUT_BUF_SIZE>(AacEncMinMaxList[profile]),
//                                 std::get<AAC_MIN_FRAME_SIZE>(AacEncMinMaxList[profile]),
//                                 std::get<AAC_MAX_CHANNELS>(AacEncMinMaxList[profile]),
//                                 std::get<AAC_SAMPLE_RATE_RANGE>(AacEncMinMaxList[profile]),
//                                 std::get<AAC_BITS_PER_SAMPLE_SET>(AacEncMinMaxList[profile])) {
public:
     // Constructor params:
    // AAC_MAX_INPUT_BUF_SIZE, AAC_MAX_OUTPUT_BUF_SIZE,
    // AAC_MAX_CHANNELS, AAC_SAMPLE_RATE_RANGE, AAC_BITS_PER_SAMPLE_SET
    typedef struct {
        uint32_t bitRate;
        AacEncodingMode aud_obj_type;
        AacFormatFlag streamFormat;
    }AacDecConfig;

    QC2AacDecConfig()
                : QC2AacCodecConfig(std::get<AAC_MAX_INPUT_BUF_SIZE>(AacDecMinMaxList[AAC_DEFAULT]),
                                std::get<AAC_MAX_OUTPUT_BUF_SIZE>(AacDecMinMaxList[AAC_DEFAULT]),
                                std::get<AAC_MIN_FRAME_SIZE>(AacDecMinMaxList[AAC_DEFAULT]),
                                std::get<AAC_MAX_CHANNELS>(AacDecMinMaxList[AAC_DEFAULT]),
                                std::get<AAC_SAMPLE_RATE_RANGE>(AacDecMinMaxList[AAC_DEFAULT]),
                                std::get<AAC_BITS_PER_SAMPLE_SET>(AacDecMinMaxList[AAC_DEFAULT])) {

        mMaxOutputChannelCount = std::get<AAC_MAX_CHANNELS>(AacDecMinMaxList[AAC_DEFAULT]);
        mBitRateRange = {.min = kMinBitrate,
                        .max = kMaxBitrate};
        std::vector<uint32_t> supportedProfiles;
        std::vector<uint32_t> supportedPkgTypes;
        auto addSupportedProfiles = [&](const std::pair<uint32_t, uint16_t>& it)
                                      { supportedProfiles.push_back(it.first);};
        std::for_each(C2ProfileToAacEncodingMode.begin(), C2ProfileToAacEncodingMode.end(),
                      addSupportedProfiles);

        auto addSupportedPkgTypes = [&](const std::pair<uint32_t, uint16_t>& it)
                                      { supportedPkgTypes.push_back(it.first); };
        std::for_each(C2PackagingTypeToAacFormatFlag.begin(), C2PackagingTypeToAacFormatFlag.end(),
                      addSupportedPkgTypes);

        mSupportedProfiles = std::move(supportedProfiles);
        mSupportedPkgTypes = std::move(supportedPkgTypes);
        mAacDecStreamConfig = {
                .bitRate = kMinBitrate,
                .aud_obj_type = AACEncodingMode_DEFAULT,
                .streamFormat = AACFormat_DEFAULT};
    }

    ~QC2AacDecConfig() = default;

    void setMaxOutputChannelCount(uint16_t channels) {
        mMaxOutputChannelCount = channels;
    }

    uint16_t getMaxOutputChannelCount() {
        return mMaxOutputChannelCount;
    }

    bool isBitRateValid(uint32_t bitRate) {
        return (mBitRateRange.min <= bitRate  && bitRate <= mBitRateRange.max);
    }

    void setBitRate(uint32_t bit_rate) {
        mAacDecStreamConfig.bitRate = bit_rate;        
    }

    inline uint32_t getBitRate() { return mAacDecStreamConfig.bitRate;}

    bool isProfileLevelValid(uint32_t profile){
        return (std::find(std::begin(mSupportedProfiles),
            std::end(mSupportedProfiles), profile) != std::end(mSupportedProfiles));
    }

    bool isAudioObjectTypeValid(uint32_t object_type) {
        return (AacEncodingModeToC2Profile.find(object_type)
                                            != AacEncodingModeToC2Profile.end());
    }

    void setProfileLevel(uint32_t profile) {
        mAacDecStreamConfig.aud_obj_type =
                    (AacEncodingMode)C2ProfileToAacEncodingMode.at((C2Config::profile_t)profile);
    }

    C2Config::profile_t getProfileLevel() {
        return (C2Config::profile_t)
            AacEncodingModeToC2Profile.at((AacEncodingMode)mAacDecStreamConfig.aud_obj_type);
    }

    void setAudioObjectType(uint32_t object_type) {
        mAacDecStreamConfig.aud_obj_type = (AacEncodingMode)object_type;
    }

    uint32_t getAudioObjectType() {
        return (uint32_t)mAacDecStreamConfig.aud_obj_type;
    }

    bool isPackagingTypeValid(uint32_t pkgType){
        return (std::find(std::begin(mSupportedPkgTypes),
            std::end(mSupportedPkgTypes), pkgType) != std::end(mSupportedPkgTypes));
    }

    void setAacPackagingType(C2Config::aac_packaging_t pkg_type) {
        QLOGE("%s: setAacPackagingType called with value %u", __func__, (unsigned int)pkg_type);
        mAacDecStreamConfig.streamFormat = (AacFormatFlag)C2PackagingTypeToAacFormatFlag.at(pkg_type);
    }

    void setAacStreamFormat(uint32_t aacFormat) {
        QLOGE("%s: setAacStreamFormat called with value %u", __func__, (unsigned int)aacFormat);
        mAacDecStreamConfig.streamFormat = (AacFormatFlag)aacFormat;
    }

    uint32_t getPackagingType() {
        return (uint32_t)AacFormatFlagToC2PackagingType.at((uint16_t)mAacDecStreamConfig.streamFormat);
    }

    uint32_t getAacStreamFormat() {
        return (uint32_t)mAacDecStreamConfig.streamFormat;
    }

    QC2Status get_sr_from_freq_index(uint8_t index, uint32_t &sr) {
        if(index > (aac_frequency_index.size() -1) || aac_frequency_index[index] == 0)
            return QC2_ERROR;
        else if(aac_frequency_index[index] == 1)
            sr = index;
        else
            sr = aac_frequency_index[index];
        return QC2_OK;
    }

    private:

    typedef struct {
        uint32_t min;
        uint32_t max;
    }bitRateRange;

    uint16_t mMaxOutputChannelCount;
    bitRateRange mBitRateRange;
    std::vector<uint32_t> mSupportedProfiles;
    std::vector<uint32_t> mSupportedPkgTypes;
    AacDecConfig mAacDecStreamConfig;
};

};  // namespace qc2

#endif  // CODEC2_CODECS_INCLUDE_FORMATS_QC2_AAC_CODEC_CONFIG_H_
