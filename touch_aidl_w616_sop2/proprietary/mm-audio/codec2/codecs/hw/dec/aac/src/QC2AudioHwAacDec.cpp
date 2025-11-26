/*
 **************************************************************************************************
 * Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include <unordered_map>
#include <memory>
#include <string>
#include <list>
#include <algorithm>
#include <cutils/properties.h>

#include "QC2EventQueue.h"
#include "QC2AudioHwAacDec.h"
#undef LOG_TAG
#define LOG_TAG "QC2AudioHwAacDec"
#define QC2_TRACE_DOMAIN QC2_TRACE_DOMAIN_CODEC

#define RETURN_IF_ERROR(retVal, str) \
do {\
    if (retVal) { \
        QLOGE_INST(str);\
        return retVal;\
     } \
} while (0); \

namespace qc2audio {

///////////////////////////////////////////////////////////////////////////////////////////////
//                              QC2AudioHwAacDec                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

QC2AudioHwAacDec::QC2AudioHwAacDec(
        const std::string& codecName,
        uint32_t sessionId,
        const std::string& instanceName,
        const std::string& mime,
        uint32_t variant,
        std::shared_ptr<EventHandler> listener)
            : QC2AudioHwCodec(codecName, sessionId, instanceName, mime,
              MimeType::RAW, KIND_DECODER, variant, listener) {

        mDecoderConfig = std::make_shared<QC2AacDecConfig>();
        mOutBufSize = 0;
}

QC2Status QC2AudioHwAacDec::configure(const C2Param& param) {
    QC2Status retVal = QC2_OK;
    QLOGV("Configure %s, core-index=%x baseIndex=%x",
               DebugString::C2Param(param.index()).c_str(),
               param.coreIndex().coreIndex(), param.index());

    switch (param.coreIndex().typeIndex()) {
        case kParamIndexSampleRate: {
            C2StreamSampleRateInfo::output sampleRateInfo;
            if (param.index() == sampleRateInfo.index()) {
                auto sampleRate =
                    ((C2StreamSampleRateInfo::output*)&param)->value;
                if (!mDecoderConfig->isSampleRateValid(sampleRate)) {
                    QLOGE("Invalid sample rate %d", sampleRate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: sample rate %d", __func__, sampleRate);
                mDecoderConfig->setSampleRate(sampleRate);
            }
            break;
        }
        case kParamIndexChannelCount: {
            C2StreamChannelCountInfo::output channelCountInfo;
            if (param.index() == channelCountInfo.index()) {
                auto channels =
                    ((C2StreamChannelCountInfo::output*)&param)->value;
                if (!mDecoderConfig->isChannelCountValid(channels)) {
                    QLOGE("Invalid channel count %d", channels);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: channels %d", __func__, channels);
                mDecoderConfig->setChannelCount(channels);
            }
            break;
        }
        case  kParamIndexMaxChannelCount: {
            C2StreamMaxChannelCountInfo::input maxChannelCountInfo;
            if (param.index() == maxChannelCountInfo.index()) {
                auto channels =
                    ((C2StreamMaxChannelCountInfo::input*)&param)->value;
                if (!mDecoderConfig->isChannelCountValid(channels)) {
                    QLOGE("Invalid max channel count %d", channels);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: max output channel count %d", __func__, channels);
                mDecoderConfig->setMaxOutputChannelCount(channels);
            }
            break;
        }
        case kParamIndexPcmEncoding: {
            C2StreamPcmEncodingInfo::output pcmEncodingInfo;
            if (param.index() == pcmEncodingInfo.index()) {
                auto pcmEncoding =
                    ((C2StreamPcmEncodingInfo::output*)&param)->value;
                auto bps =
                    FormatMapper::mapPcmEncodingToBitsPerSample(pcmEncoding);
                if (!mDecoderConfig->isBitsPerSampleValid(bps)) {
                    QLOGE("Invalid bits per sample %d", bps);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bits per sample %d", __func__, bps);
                mDecoderConfig->setBitsPerSample(bps);
            }
            break;
        }
        case kParamIndexBitrate: {
            C2StreamBitrateInfo::input bitRateInfo;
            if (param.index() == bitRateInfo.index()) {
                auto bitrate = ((C2StreamBitrateInfo::input*)&param)->value;
                if (!mDecoderConfig->isBitRateValid(bitrate)) {
                    QLOGE("Invalid bit rate %d", bitrate);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: bit rate %d", __func__, bitrate);
                mDecoderConfig->setBitRate(bitrate);
            }
            break;
        }
        case kParamIndexAacPackaging: {
            C2StreamAacFormatInfo::input pkgTypeInfo;
            if (param.index() == pkgTypeInfo.index()) {
                auto pkgType = ((C2StreamAacFormatInfo::input*)&param)->value;
                QLOGE("%s: AAC packaging type %u", __func__, (unsigned int)pkgType);
                if (!mDecoderConfig->isPackagingTypeValid((uint32_t)pkgType)) {
                    QLOGE("Invalid AAC Packaging Type %u", (uint32_t)pkgType);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: AAC package type %u", __func__, (uint32_t)pkgType);
                mDecoderConfig->setAacPackagingType(pkgType);
            }
            break;
        }
        case kParamIndexProfileLevel: {
            C2StreamProfileLevelInfo::input profileInfo;
            if (param.index() == profileInfo.index()) {
                auto profile = ((C2StreamProfileLevelInfo::input*)&param)->profile;
                if (!mDecoderConfig->isProfileLevelValid(profile)) {
                    QLOGE("Invalid AAC Profile Info %d", profile);
                    retVal = QC2_BAD_VALUE;
                    break;
                }
                QLOGV("%s: AAC profile %u", __func__, (uint32_t)profile);
                mDecoderConfig->setProfileLevel(profile);
            }
            break;
        }
        default: {
            QC2AudioHwCodec::configure(param);
            break;
        }
    }
    //updateConfigParams();

    return retVal;
}

////////////////////////////// Helper Functions ///////////////////////////////

void QC2AudioHwAacDec::audaac_extract_bits(
                                uint8_t  *input,
                                uint8_t  num_bits_reqd,
                                uint32_t *out_buf
)
{

    if (input == nullptr || out_buf == nullptr){
        QLOGE("%s invalid parameters ",__func__);
        return;
    }

    QLOGI("%s number=%u, bits to read=%u",__func__, *input, num_bits_reqd);
    uint32_t output = 0;
    uint32_t value = 0;
    uint32_t byte_index;
    uint8_t   bit_index;
    uint8_t   bits_avail_in_byte;
    uint8_t   num_to_copy;
    uint8_t   mask;
    uint8_t   num_remaining = num_bits_reqd;

    while (num_remaining) {
        byte_index = m_aac_hdr_bit_index / 8;
        bit_index  = m_aac_hdr_bit_index % 8;

        bits_avail_in_byte = (uint8_t)(8 - bit_index);
        num_to_copy = (uint8_t)MIN(bits_avail_in_byte, num_remaining);

        mask = (uint8_t)(~(0xff << bits_avail_in_byte));

        value = input[byte_index] & mask;
        value = value >> (bits_avail_in_byte - num_to_copy);

        m_aac_hdr_bit_index += num_to_copy;
        num_remaining = (uint8_t)(num_remaining - num_to_copy);

        output = (output << num_to_copy) | value;
    }
    *out_buf=output;

}

void QC2AudioHwAacDec::audaac_extract_adif_header(
                                uint8_t *data,
                                std::shared_ptr<QC2AacDecConfig>& aac_config)
{
    if (data == nullptr || aac_config == nullptr) {
        QLOGE("%s invalid parameters",__func__);
        return;
    }

    uint32_t  buf8 = 0;
    uint32_t buf32 = 0;
    uint32_t  num_pfe = 0, num_fce = 0, num_sce = 0,
         num_bce = 0, num_lfe = 0, num_ade = 0, num_vce = 0;
    uint32_t  pfe_index = 0, variable_bit_rate = 0, freq_index = 0;
    uint8_t  i = 0;
    uint32_t sample_rate = 0;
    uint32_t temp = 0;
    uint32_t isCpe = 0,totalChannels = 0;
    QLOGI("%s",__func__);

    /* We already parsed the 'ADIF' keyword. Skip it. */
    m_aac_hdr_bit_index = 32;

    audaac_extract_bits(data, AAC_COPYRIGHT_PRESENT_SIZE, &buf8);

    if (buf8) {
        /* Copyright present; Just discard it for now */
        m_aac_hdr_bit_index += 72;
    }

    audaac_extract_bits(data, AAC_ORIGINAL_COPY_SIZE,
    &temp);

    audaac_extract_bits(data, AAC_HOME_SIZE, &temp);

    audaac_extract_bits(data, AAC_BITSTREAM_TYPE_SIZE,
    &temp);
    variable_bit_rate = (uint8_t)temp;

    audaac_extract_bits(data, AAC_BITRATE_SIZE, &temp);

    audaac_extract_bits(data, AAC_NUM_PFE_SIZE, &num_pfe);

    for (pfe_index=0; pfe_index<num_pfe+1; pfe_index++) {
        if (!variable_bit_rate) {
            /* Discard */
            audaac_extract_bits(data, AAC_BUFFER_FULLNESS_SIZE, &buf32);
        }

        /* Extract Program Config Element */

        /* Discard element instance tag */
        audaac_extract_bits(data, AAC_ELEMENT_INSTANCE_TAG_SIZE, &buf8);

        audaac_extract_bits(data, AAC_PROFILE_SIZE,
        &buf8);
        aac_config->setAudioObjectType((uint32_t)buf8);

        buf8 = 0;
        audaac_extract_bits(data, AAC_SAMPLING_FREQ_INDEX_SIZE, &buf8);
        freq_index = (uint8_t)buf8;
        if (aac_config->get_sr_from_freq_index(buf8, sample_rate)) {
            aac_config->setSampleRate(sample_rate);
        }

        audaac_extract_bits(data, AAC_NUM_FRONT_CHANNEL_ELEMENTS_SIZE, &num_fce);

        audaac_extract_bits(data, AAC_NUM_SIDE_CHANNEL_ELEMENTS_SIZE, &num_sce);

        audaac_extract_bits(data, AAC_NUM_BACK_CHANNEL_ELEMENTS_SIZE, &num_bce);

        audaac_extract_bits(data, AAC_NUM_LFE_CHANNEL_ELEMENTS_SIZE, &num_lfe);

        audaac_extract_bits(data, AAC_NUM_ASSOC_DATA_ELEMENTS_SIZE, &num_ade);

        audaac_extract_bits(data, AAC_NUM_VALID_CC_ELEMENTS_SIZE, &num_vce);

        audaac_extract_bits(data, AAC_MONO_MIXDOWN_PRESENT_SIZE, &buf8);
        if (buf8) {
            audaac_extract_bits(data, AAC_MONO_MIXDOWN_ELEMENT_SIZE, &buf8);
        }

        audaac_extract_bits(data, AAC_STEREO_MIXDOWN_PRESENT_SIZE, &buf8);
        if (buf8) {
            audaac_extract_bits(data, AAC_STEREO_MIXDOWN_ELEMENT_SIZE, &buf8);
        }

        audaac_extract_bits(data, AAC_MATRIX_MIXDOWN_PRESENT_SIZE, &buf8);
        if (buf8) {
            audaac_extract_bits(data, AAC_MATRIX_MIXDOWN_SIZE, &buf8);
        }

        for (i=0; i<num_fce; i++) {
            audaac_extract_bits(data, 1, &isCpe);
            totalChannels += (isCpe ? 2 : 1);
            audaac_extract_bits(data, (AAC_FCE_SIZE - 1), &buf8);
        }

        for (i=0; i<num_sce; i++) {
            audaac_extract_bits(data, 1, &isCpe);
            totalChannels += (isCpe ? 2 : 1);
            audaac_extract_bits(data, (AAC_SCE_SIZE -1), &buf8);
        }

        for (i=0; i<num_bce; i++) {
            audaac_extract_bits(data, 1, &isCpe);
            totalChannels += (isCpe ? 2 : 1);
            audaac_extract_bits(data, (AAC_BCE_SIZE - 1), &buf8);
        }

        for (i=0; i<num_lfe; i++) {
            audaac_extract_bits(data, 1, &isCpe);
            totalChannels += (isCpe ? 2 : 1);
            audaac_extract_bits(data, (AAC_LFE_SIZE - 1), &buf8);
        }

        for (i=0; i<num_ade; i++) {
            audaac_extract_bits(data, 1, &isCpe);
            totalChannels += (isCpe ? 2 : 1);
            audaac_extract_bits(data, (AAC_ADE_SIZE - 1), &buf8);
        }

        for (i=0; i<num_vce; i++) {
            audaac_extract_bits(data, 1, &isCpe);
            totalChannels += (isCpe ? 2 : 1);
            audaac_extract_bits(data, (AAC_VCE_SIZE - 1), &buf8);
        }

        // byte_alignment()
        buf8 = (uint8_t)(m_aac_hdr_bit_index % 8);
        if (buf8) {
            m_aac_hdr_bit_index = (uint32_t)(m_aac_hdr_bit_index + 8 - buf8);
        }

        // get comment_field_bytes
        buf8 = data[m_aac_hdr_bit_index/8];

        m_aac_hdr_bit_index += AAC_COMMENT_FIELD_BYTES_SIZE;

        // Skip the comment
        m_aac_hdr_bit_index =
        (uint32_t)(m_aac_hdr_bit_index + buf8 * AAC_COMMENT_FIELD_DATA_SIZE);
    }
    //set channels
    aac_config->setChannelCount(totalChannels);
    // byte_alignment()
    buf8 = (uint8_t)(m_aac_hdr_bit_index % 8);
    if (buf8) {
        m_aac_hdr_bit_index = (uint32_t)(m_aac_hdr_bit_index + 8 - buf8);
    }
}

int QC2AudioHwAacDec::audaac_extract_loas_header(
                                uint8_t *data, uint32_t len)
{

    if (data == nullptr) {
        return QC2_BAD_VALUE;
    }

    QLOGI("%s",__func__);
    uint32_t      aac_frame_length = 0;
    uint32_t      value    = 0;
    uint32_t       obj_type = 0;
    uint32_t       ext_flag = 0;

    uint32_t       num_fc   = 0;
    uint32_t       num_sc   = 0;
    uint32_t       num_bc   = 0;
    uint32_t       num_lfe  = 0;
    uint32_t       num_ade  = 0;
    uint32_t       num_vce  = 0;
    uint32_t       sample_rate = 0;
    uint32_t       freq_index = 0;
    uint32_t       channel_config = 0;

    uint8_t      num_bits_to_skip = 0;
    QLOGI("%s: LOAS HEADER -->len=%u\n",__func__, (unsigned int)len);
    /* Length is in the bits 11 to 23 from left in the bitstream */
    m_aac_hdr_bit_index = 11;
    if (((m_aac_hdr_bit_index+42)) > len)
    return -1;

    audaac_extract_bits(data, 13, &aac_frame_length);

    /* useSameStreamMux */
    audaac_extract_bits(data, 1, &value);

    if (value)
    {
        return -1;
    }
    /* Audio mux version */
    audaac_extract_bits(data, 1, &value);
    if (value)
    {
        /* Unsupported format */
        return -1;
    }
    /* allStreamsSameTimeFraming */
    audaac_extract_bits(data, 1, &value);
    if (!value)
    {
        /* Unsupported format */
        return -1;
    }
    /* numSubFrames */
    audaac_extract_bits(data, 6, &value);
    if (value)
    {
        /* Unsupported format */
        return -1;
    }
    /* numProgram */
    audaac_extract_bits(data, 4, &value);
    if (value)
    {
        /* Unsupported format */
        return -1;
    }
    /* numLayer */
    audaac_extract_bits(data, 3, &value);
    if (value)
    {
        /* Unsupported format */
        return -1;
    }

    /* Audio specific config */
    /* audioObjectType */
    audaac_extract_bits(data, 5, &value);
    if (!LOAS_IS_AUD_OBJ_SUPPORTED(value))
    {
        /* Unsupported format */
        return -1;
    }
    obj_type = value;

    /* SFI */
    audaac_extract_bits(data, 4, &value);
    if (!LOAS_IS_SFI_SUPPORTED(value))
    {
        /* Unsupported format */
        return -1;
    }
    freq_index = value;
    if (mDecoderConfig->get_sr_from_freq_index(freq_index, sample_rate) == QC2_ERROR)
        return -1;

    /* Channel config */
    audaac_extract_bits(data, 4, &value);
    if (!LOAS_IS_CHANNEL_CONFIG_SUPPORTED(value, obj_type))
    {
        /* Unsupported format */
        return -1;
    }
    channel_config = value;

    if (obj_type == 5)
    {
        /* Extension SFI */
        if ((m_aac_hdr_bit_index+9) > len)
        return -1;
        audaac_extract_bits(data, 4, &value);
        if (!LOAS_IS_EXT_SFI_SUPPORTED(value))
        {
            /* Unsupported format */
            return -1;
        }
        /* Audio object_type */
        audaac_extract_bits(data, 5, &value);
        if (!LOAS_IS_AUD_OBJ_SUPPORTED(value))
        {
            /* Unsupported format */
            return -1;
        }
        obj_type = (uint8_t)value;
    }

    if (LOAS_GA_SPECIFIC_CONFIG(obj_type))
    {
        if ((m_aac_hdr_bit_index+3) > len)
        return -1;
        /* framelengthFlag */
        audaac_extract_bits(data, 1, &value);
        if (value)
        {
            /* Unsupported format */
            return -1;
        }

        /* dependsOnCoreCoder */
        audaac_extract_bits(data, 1, &value);
        if (value)
        {
            /* Unsupported format */
            return -1;
        }

        /* extensionFlag */
        audaac_extract_bits(data, 1, &ext_flag);
        if (!LOAS_IS_EXT_FLG_SUPPORTED(ext_flag,obj_type) )
        {
            /* Unsupported format */
            return -1;
        }
        if (!channel_config)
        {
            /* Skip 10 bits */
            if ((m_aac_hdr_bit_index+45) > len)
            return -1;
            audaac_extract_bits(data, 10, &value);

            audaac_extract_bits(data, 4, &num_fc);
            audaac_extract_bits(data, 4, &num_sc);
            audaac_extract_bits(data, 4, &num_bc);
            audaac_extract_bits(data, 2, &num_lfe);
            audaac_extract_bits(data, 3, &num_ade);
            audaac_extract_bits(data, 4, &num_vce);

            /* mono_mixdown_present */
            audaac_extract_bits(data, 1, &value);
            if (value) {
                /* mono_mixdown_element_number */
                audaac_extract_bits(data, 4, &value);
            }

            /* stereo_mixdown_present */
            audaac_extract_bits(data, 1, &value);
            if (value) {
                /* stereo_mixdown_element_number */
                audaac_extract_bits(data, 4, &value);
            }

            /* matrix_mixdown_idx_present */
            audaac_extract_bits(data, 1, &value);
            if (value) {
                /* matrix_mixdown_idx and presudo_surround_enable */
                audaac_extract_bits(data, 3, &value);
            }
            num_bits_to_skip = (uint8_t)((num_fc * 5) + (num_sc * 5) + (num_bc * 5) +
            (num_lfe * 4) + (num_ade * 4) + (num_vce * 5));

            if ((m_aac_hdr_bit_index+num_bits_to_skip) > len)
            return -1;

            while (num_bits_to_skip != 0){
                if (num_bits_to_skip > 32) {
                    audaac_extract_bits(data, 32, &value);
                    num_bits_to_skip = (uint8_t)(num_bits_to_skip - 32);
                } else {
                    audaac_extract_bits(data, num_bits_to_skip, &value);
                    num_bits_to_skip = 0;
                }
            }

            if (m_aac_hdr_bit_index & 0x07)
            {
                m_aac_hdr_bit_index +=  (8 - (m_aac_hdr_bit_index & 0x07));
            }
            if ((m_aac_hdr_bit_index + 8) > len)
            return -1;
            /* comment_field_bytes */
            audaac_extract_bits(data, 8, &value);

            num_bits_to_skip = (uint8_t)(value * 8);

            if ((m_aac_hdr_bit_index + num_bits_to_skip) > len)
            return -1;

            while (num_bits_to_skip != 0){
                if (num_bits_to_skip > 32) {
                    audaac_extract_bits(data, 32, &value);
                    num_bits_to_skip = (uint8_t)(num_bits_to_skip - 32);
                } else {
                    audaac_extract_bits(data, num_bits_to_skip, &value);
                    num_bits_to_skip = 0;
                }
            }
        }

        if (ext_flag)
        {
            if (((obj_type == 17) || (obj_type == 19) ||
                        (obj_type == 20) || (obj_type == 23)))
            {
                if ((m_aac_hdr_bit_index + 3) > len)
                return -1;
                audaac_extract_bits(data, 1, &value);
                audaac_extract_bits(data, 1, &value);
                audaac_extract_bits(data, 1, &value);
            }
            /* extensionFlag3 */
            if ((m_aac_hdr_bit_index + 1) > len)
            return -1;
            audaac_extract_bits(data, 1, &value);
        }
    }
    if ((obj_type != 18) && (obj_type >= 17) && (obj_type <= 27))
    {
        /* epConfig */
        if ((m_aac_hdr_bit_index + 2) > len)
        return -1;
        audaac_extract_bits(data, 2, &value);
        if (value)
        {
            /* Unsupported format */
            return -1;
        }
    }
    /* Back in StreamMuxConfig */
    /* framelengthType */
    if ((m_aac_hdr_bit_index + 3) > len)
    return -1;
    audaac_extract_bits(data, 3, &value);
    if (value)
    {
        /* Unsupported format */
        return -1;
    }

    if ((freq_index > 12) ||
            (channel_config > 2))
    {
        return -1;
    }
    mDecoderConfig->setChannelCount((uint16_t)channel_config);
    mDecoderConfig->setSampleRate(sample_rate);
    mDecoderConfig->setAudioObjectType(obj_type);
    return 0;
}

QC2Status QC2AudioHwAacDec::parseAndExtractAacInfoFromHeader(uint8_t* buf, size_t buf_length) {

    uint32_t ext_aud_obj_type = 0;
    uint32_t sample_rate = 0;

    if (buf == nullptr) {
        QLOGE("%s invalid parameter ",__func__);
        return QC2_BAD_VALUE;
    }

    if ((buf_length >= 4) && (buf[0] == 0x41) &&
        (buf[1] == 0x44) && (buf[2] == 0x49) &&  (buf[3] == 0x46) )
    /*check "ADIF" */
    {
        // format is ADIF
        QLOGI("%s: ADIF format info detected in frame header", __func__);
        mDecoderConfig->setAacStreamFormat(AACFormat_ADIF);
        //Get the sample rate, channels and audio object type(profile) from header
        audaac_extract_adif_header(buf, mDecoderConfig);
    } else {
        QLOGI("%s: Parser-->RAW or Partial frame to be found", __func__);
        uint8_t *data;
        uint32_t temp, ext_flag, status = 0, i = 0;
        uint8_t sync_word_found = 0;
        while (i < (buf_length))
        {
            if ((buf[i] == 0xFF) && ((buf[i+1] & 0xF6) == 0xF0) )
            {
                mDecoderConfig->setAacStreamFormat(AACFormat_ADTS);
                QLOGI("%s: ADTS format info detected in frame header", __func__);
                uint32_t sf = 0, index = 0;
                if ((i+3) >= buf_length)
                    break;

                sf = (buf[i+2] & 0x3C) >> 2;
                // Get the frequency index from bits 19,20,21,22
                //Get the channel configuration bits 24,25,26
                index = (buf[i+2] & 0x01) << 0x02 ;
                index |= (buf[i+3] & 0xC0) >> 0x06;
                QLOGI("%s: ADTS-->freq_index=%u ch-cfg=%u\n", __func__, sf, index);

                //if not ok continue, false sync words found
                if ((sf > 12) || (index > 6)) {
                    QLOGI("%s: Parser-->fake sync words sf=%u index=%u\n", __func__, sf, index);
                    i++;
                    // invalid SF/ch-cfg, maybe fake sync word found
                    continue;
                }
                sync_word_found = 1;
                m_bytes_to_skip = i;
                QLOGI("%s: ADTS-->bytes to skip : %d\n",__func__, m_bytes_to_skip);
                if (index == 0)
                    //TODO: Need to figure out how to extract channel count from AOT specific index
                    QLOGW("%s: ADTS-->channels(index =0 case): %d\n",  __func__, index);
                mDecoderConfig->setChannelCount(index);
                status = mDecoderConfig->get_sr_from_freq_index((uint8_t)sf, sample_rate);
                if (status)
                    mDecoderConfig->setSampleRate((uint16_t)sample_rate);
                else
                    return QC2_ERROR;
                break;
            }
            else if ((buf[i] == 0x56) && ((buf[i+1] & 0xE0) == 0xE0) )
            {
                mDecoderConfig->setAacStreamFormat(AACFormat_LOAS);
                QLOGI("%s: LOAS format info detected in frame header", __func__);
                uint32_t ret = 0;
                //DEBUG_PRINT("i=%u buf[i]=%u buf[i+1] %u\n",i,buf[i], buf[i+1]);

                ret = audaac_extract_loas_header(
                buf,
                ((buf_length-i)*8));
                if ((int32_t)ret == -1 || (int32_t)ret == QC2_BAD_VALUE )
                {
                    QLOGI("%s: Parser--> fake sync words SF=%u cf=%u\n",\
                    __func__, \
                    mDecoderConfig->getSampleRate(), \
                    (uint32_t)mDecoderConfig->getChannelCount());
                    i++;
                    // invalid SF/ch-cfg, maybe fake sync word found
                    continue;
                }
                sync_word_found = 1;
                m_bytes_to_skip = i;
                QLOGI("%s: LOAS-->bytes to skip : %d\n",__func__, m_bytes_to_skip);
                break;
            } else
                break;
            i++;
        }
        if (!sync_word_found)
        {
            // RAW AAC-ADTS PARSER
            uint32_t aud_obj_type = 0;
            uint32_t freq_index = 0, ext_freq_index = 0;
            uint32_t channel_config = 0;
            bool sbr_present_flag = 0, sbr_ps_present_flag = 0;
            ext_aud_obj_type = 0;

            // In-case configure has set the packing type to ADTS, don't
            // update
            if (mDecoderConfig->getPackagingType() != C2Config::AAC_PACKAGING_ADTS)
                mDecoderConfig->setAacStreamFormat(AACFormat_RAW);
            m_aac_hdr_bit_index = 0;
            data = (uint8_t*)buf;

            audaac_extract_bits(data, 5, &temp);
            aud_obj_type = temp;
            ext_aud_obj_type = temp;
            QLOGI("%s: RAW-->aud_obj_type=0x%x\n",\
                                                 __func__, aud_obj_type);

            audaac_extract_bits(data, AAC_SAMPLING_FREQ_INDEX_SIZE, &temp);
            freq_index = temp;
            ext_freq_index = temp;
            QLOGI("%s: RAW-->freq_index=0x%x\n", __func__, freq_index);
            if (freq_index == 0xf)
            {
                /* Current release does not support sampling freqiencies
                 other than frequencies listed in aac_frequency_index[] */
                /* Extract sampling frequency value */
                audaac_extract_bits(data, 12, &temp);
                audaac_extract_bits(data, 12, &temp);
            }
            /* Extract channel config value */
            audaac_extract_bits(data, 4, &temp);
            channel_config = temp;
            QLOGI("%s: RAW-->ch cfg=0x%x\n",__func__, channel_config);

            /* If the audioOjectType is SBR or PS */
            if (aud_obj_type == 5 || aud_obj_type == 29)
            {
                ext_aud_obj_type = 5;
                sbr_present_flag = 1;
                if (aud_obj_type == 29)
                {
                    ext_aud_obj_type = 29;
                    sbr_ps_present_flag = 1;
                }
                /* extensionSamplingFrequencyIndex */
                audaac_extract_bits(data, 4, &temp);
                ext_freq_index = temp;

                if (ext_freq_index == 0x0f)
                {
                    /* Extract sampling frequency value */
                    audaac_extract_bits(data, 12, &temp);
                    audaac_extract_bits(data, 12, &temp);
                }
                audaac_extract_bits(data, 5, &temp);
                aud_obj_type = temp;
            }
            /* If audioObjectType is AAC_LC or AAC_LTP */
            if (aud_obj_type == 2 || aud_obj_type == 4)
            {
                /* frame_length_flag : 0 - 1024; 1 - 960
        Current release supports AAC frame length of 1024 only */
                audaac_extract_bits(data, 1, &temp);
                if (temp) status = 1;
                /* dependsOnCoreCoder == 1, core coder has different sampling rate
        in a scalable bitstream. This is mainly set fro AAC_SCALABLE Object only.
        Currrent release does not support AAC_SCALABLE Object */
                audaac_extract_bits(data, 1, &temp);
                if (temp) status = 1;

                /* extensionFlag */
                audaac_extract_bits(data, 1, &ext_flag);

                if (channel_config == 0 && status == 0)
                {
                    uint32_t num_fce, num_bce, num_sce, num_lfe, num_ade, num_vce;
                    uint32_t i, profile, samp_freq_idx, num_ch = 0;

                    /* Parse Program configuration element information */
                    audaac_extract_bits(data, AAC_ELEMENT_INSTANCE_TAG_SIZE, &temp);
                    audaac_extract_bits(data, AAC_PROFILE_SIZE, &profile);
                    QLOGI("%s: RAW-->PCE -- profile=%u\n", __func__, profile);
                    audaac_extract_bits(data,
                    AAC_SAMPLING_FREQ_INDEX_SIZE,
                    &samp_freq_idx);
                    QLOGI("%s: RAW-->PCE -- samp_freq_idx=%u\n",__func__, samp_freq_idx);
                    audaac_extract_bits(data, AAC_NUM_FRONT_CHANNEL_ELEMENTS_SIZE,
                    &num_fce);
                    QLOGI("%s: RAW-->PCE -- num_fce=%u\n", __func__, num_fce);

                    audaac_extract_bits(data, AAC_NUM_SIDE_CHANNEL_ELEMENTS_SIZE,
                    &num_sce);
                    QLOGI("%s: RAW-->PCE -- num_sce=%u\n", __func__, num_sce);
                    audaac_extract_bits(data, AAC_NUM_BACK_CHANNEL_ELEMENTS_SIZE,
                    &num_bce);
                    QLOGI("%s: RAW-->PCE -- num_bce=%u\n", __func__, num_bce);

                    audaac_extract_bits(data, AAC_NUM_LFE_CHANNEL_ELEMENTS_SIZE,
                    &num_lfe);
                    QLOGI("%s: RAW-->PCE -- num_lfe=%u\n", __func__, num_lfe);

                    audaac_extract_bits(data, AAC_NUM_ASSOC_DATA_ELEMENTS_SIZE,
                    &num_ade);
                    QLOGI("%s: RAW-->PCE -- num_ade=%u\n", __func__, num_ade);

                    audaac_extract_bits(data, AAC_NUM_VALID_CC_ELEMENTS_SIZE,
                    &num_vce);
                    QLOGI("%s: RAW-->PCE -- num_vce=%u\n", __func__, num_vce);

                    audaac_extract_bits(data, AAC_MONO_MIXDOWN_PRESENT_SIZE,
                    &temp);
                    if (temp) {
                        audaac_extract_bits(data, AAC_MONO_MIXDOWN_ELEMENT_SIZE,
                        &temp);
                    }

                    audaac_extract_bits(data, AAC_STEREO_MIXDOWN_PRESENT_SIZE,
                    &temp);
                    if (temp) {
                        audaac_extract_bits(data, AAC_STEREO_MIXDOWN_ELEMENT_SIZE,
                        &temp);
                    }

                    audaac_extract_bits(data, AAC_MATRIX_MIXDOWN_PRESENT_SIZE,
                    &temp);
                    if (temp) {
                        audaac_extract_bits(data, AAC_MATRIX_MIXDOWN_SIZE, &temp);
                    }

                    for (i=0; i<(num_fce+num_sce+num_bce); i++) {
                        /* Is Element CPE ?*/
                        audaac_extract_bits(data, 1, &temp);
                        /* If the element is CPE, increment num channels by 2 */
                        if (temp) num_ch += 2;
                        else     num_ch += 1;
                        QLOGI("%s: RAW-->PCE -- num_ch=%u\n", __func__, num_ch);
                        audaac_extract_bits(data, 4, &temp);
                    }
                    for (i=0; i<num_lfe; i++) {
                        /* LFE element can not be CPE */
                        num_ch += 1;
                        QLOGI("%s: RAW-->PCE -- num_ch=%u\n", __func__, num_ch);
                        audaac_extract_bits(data, AAC_LFE_SIZE, &temp);
                    }
                    for (i=0; i<num_ade; i++) {
                        audaac_extract_bits(data, AAC_ADE_SIZE, &temp);
                    }
                    for (i=0; i<num_vce; i++) {
                        /* Coupling channels can not be counted as output
                         channels as they will be
                         coupled with main audio channels */
                        audaac_extract_bits(data, AAC_VCE_SIZE, &temp);
                    }
                    channel_config = (uint8_t)num_ch;
                    /* byte_alignment() */
                    temp = (uint8_t)(m_aac_hdr_bit_index % 8);
                    if (temp) {
                        m_aac_hdr_bit_index = (unsigned int)
                              (m_aac_hdr_bit_index + 8 - temp);
                    }
                    /* get comment_field_bytes */
                    temp = data[m_aac_hdr_bit_index/8];
                    m_aac_hdr_bit_index += AAC_COMMENT_FIELD_BYTES_SIZE;
                    /* Skip the comment */
                    m_aac_hdr_bit_index = (unsigned int)
                    (m_aac_hdr_bit_index + temp * AAC_COMMENT_FIELD_DATA_SIZE);
                    /* byte_alignment() */
                    temp = (uint8_t)(m_aac_hdr_bit_index % 8);
                    if (temp) {
                        m_aac_hdr_bit_index =
                           (unsigned int)(m_aac_hdr_bit_index + 8 - temp);
                    }
                    QLOGI("%s: RAW-->PCE -- m_aac_hdr_bit_index=0x%x\n",\
                    __func__, m_aac_hdr_bit_index);
                }
                if (ext_flag)
                {
                    /* 17: ER_AAC_LC 19:ER_AAC_LTP
                    20:ER_AAC_SCALABLE 23:ER_AAC_LD */
                    if (aud_obj_type == 17 ||
                            aud_obj_type == 19 ||
                            aud_obj_type == 20 ||
                            aud_obj_type == 23)
                    {
                        audaac_extract_bits(data, 1, &temp);
                        audaac_extract_bits(data, 1, &temp);
                        audaac_extract_bits(data, 1, &temp);
                    }
                    /* extensionFlag3 */
                    audaac_extract_bits(data, 1, &temp);
                    if (temp) status = 1;
                }
            }

            /* SBR tool explicit signaling ( backward compatible ) */
            if (ext_aud_obj_type != 5)
            {
                /* syncExtensionType */
                audaac_extract_bits(data, 11, &temp);
                if (temp == 0x2b7)
                {
                    /*extensionAudioObjectType*/
                    audaac_extract_bits(data, 5, &ext_aud_obj_type);
                    if (ext_aud_obj_type == 5)
                    {
                        /*sbrPresentFlag*/
                        audaac_extract_bits(data, 1, &temp);
                        sbr_present_flag = (uint8_t)temp;
                        if (temp)
                        {
                            /*extensionSamplingFrequencyIndex*/
                            audaac_extract_bits(data,
                            AAC_SAMPLING_FREQ_INDEX_SIZE,
                            &temp);
                            ext_freq_index = (uint8_t)temp;
                            if (temp == 0xf)
                            {
                                /*Extract 24 bits for sampling frequency value */
                                audaac_extract_bits(data, 12, &temp);
                                audaac_extract_bits(data, 12, &temp);
                            }

                            /* syncExtensionType */
                            audaac_extract_bits(data, 11, &temp);

                            if (temp == 0x548)
                            {
                                /*psPresentFlag*/
                                audaac_extract_bits(data, 1, &temp);
                                sbr_ps_present_flag = (uint8_t)temp;
                                if (temp)
                                {
                                    /* extAudioObject is PS */
                                    ext_aud_obj_type = 29;
                                }
                            }
                        }
                    }
                }
            }
            QLOGI("%s: RAW-->ch cfg=0x%x", __func__, channel_config);
            mDecoderConfig->setChannelCount(channel_config);
            QLOGI("%s: RAW-->freq_index=0x%x", __func__, freq_index);
            QLOGI("%s: RAW-->ext_freq_index=0x%x", __func__, ext_freq_index);
            mDecoderConfig->get_sr_from_freq_index(freq_index, sample_rate);
            mDecoderConfig->setSampleRate(sample_rate);
            QLOGI("%s: RAW-->aud_obj_type=0x%x\n", __func__, aud_obj_type);
            QLOGI("%s: RAW-->ext_aud_obj_type=0x%x\n", __func__, ext_aud_obj_type);
            if (mDecoderConfig->isAudioObjectTypeValid(aud_obj_type)) {
                mDecoderConfig->setAudioObjectType(aud_obj_type);
            } else
                return QC2_BAD_VALUE;
        }
    }
    return QC2_OK;
}

void QC2AudioHwAacDec::updateConfigParams(bool *hasCSDChanged) {

    // Update sample rate on interface and output buffer
    auto sampleRate = mDecoderConfig->getSampleRate();
    if (sampleRate != mIntfParamValuesMap[kParamIndexSampleRate]) {
        auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sampleRate);
        mUpdatedInfos.push_back(sampleRateParam);
        mIntfParamValuesMap[kParamIndexSampleRate] = sampleRate;
        *hasCSDChanged = true;
    }

    // Update bits per sample on interface and output buffer
    C2Config::pcm_encoding_t pcmEncoding =
        FormatMapper::mapBitsPerSampletoPcmEncoding(mDecoderConfig->getBitsPerSample());
    if (pcmEncoding != mIntfParamValuesMap[kParamIndexPcmEncoding]) {
        auto pcmEncodingParam = std::make_shared<C2StreamPcmEncodingInfo::output>(0, pcmEncoding);
        mUpdatedInfos.push_back(pcmEncodingParam);
        mIntfParamValuesMap[kParamIndexPcmEncoding] = pcmEncoding;
        *hasCSDChanged = true;
    }

    // Update channels
    auto channels = mDecoderConfig->getChannelCount();
    if (channels != mIntfParamValuesMap[kParamIndexChannelCount]) {
        auto channelsParam = std::make_shared<C2StreamChannelCountInfo::output>(0, channels);
        mUpdatedInfos.push_back(channelsParam);
        mIntfParamValuesMap[kParamIndexChannelCount] = channels;
        *hasCSDChanged = true;
    }

    // Update Max output channels
    auto maxChannels = mDecoderConfig->getMaxOutputChannelCount();
    if (maxChannels != mIntfParamValuesMap[kParamIndexMaxChannelCount]) {
        auto maxChannelsParam = std::make_shared<C2StreamMaxChannelCountInfo::input>(0, maxChannels);
        mUpdatedInfos.push_back(maxChannelsParam);
        mIntfParamValuesMap[kParamIndexMaxChannelCount] = maxChannels;
        *hasCSDChanged = true;
    }

    // Update bit rate
    auto bitRate = mDecoderConfig->getBitRate();
    if (bitRate != mIntfParamValuesMap[kParamIndexBitrate]) {
        auto bitRateParam = std::make_shared<C2StreamBitrateInfo::input>(0, bitRate);
        mUpdatedInfos.push_back(bitRateParam);
        mIntfParamValuesMap[kParamIndexBitrate] = bitRate;
        *hasCSDChanged = true;
    }

    // Update AAC Profile level
    auto aacProfileInfo = mDecoderConfig->getProfileLevel();
    if (aacProfileInfo != mIntfParamValuesMap[kParamIndexProfileLevel]) {
        auto profileInfoParam = std::make_shared<C2StreamProfileLevelInfo::input>(0u ,
                                                            static_cast<C2Config::profile_t>(aacProfileInfo),
                                                            static_cast<C2Config::level_t>(C2Config::LEVEL_UNUSED));
        mUpdatedInfos.push_back(profileInfoParam);
        mIntfParamValuesMap[kParamIndexProfileLevel] = aacProfileInfo;
        *hasCSDChanged = true;
    }

    // Update AAC stream packaging type
    auto aacPackagingType = mDecoderConfig->getPackagingType();
    if (aacPackagingType != mIntfParamValuesMap[kParamIndexAacPackaging]) {
        auto packagingTypeParam = std::make_shared<C2StreamAacFormatInfo::input>(0u, aacPackagingType);
        mUpdatedInfos.push_back(packagingTypeParam);
        mIntfParamValuesMap[kParamIndexAacPackaging] = aacPackagingType;
        *hasCSDChanged = true;
    }

}

QC2Status QC2AudioHwAacDec::onInit() {
    QLOGD_INST("%s", __func__);
    mOutBufSize = mDecoderConfig->getMaxOutputBufSize();
    mDecoderConfig->setMaxOutputChannelCount(0);
    mDecoderConfig->setAacStreamFormat(AACFormat_RAW);
    return QC2_OK;
}

QC2Status QC2AudioHwAacDec::onStart() {
    QLOGD_INST("%s", __func__);
    //turn mAudioHw->start_session(mHwSessionId.get());
    return QC2_OK;
}

QC2Status QC2AudioHwAacDec::onCodecConfig(std::shared_ptr<QC2Buffer> input) {
    QC2Status retVal = QC2_OK;
    uint32_t availLength = input->linear().size();
    uint64_t flags = input->flags();
    bool hasCSDChanged = false;
    //std::shared_ptr<QC2AudioHwCommonCodecConfig> = std::make_shared<QC2AudioHwCommonCodecConfig>();

    QLOGV_INST("%s", __func__);

    // Get input buffer's data offset
    std::unique_ptr<QC2Buffer::Mapping> bufMappingInfo = input->linear().map();
    if (bufMappingInfo == nullptr) {
        return QC2_NO_MEMORY;
    }
    auto basePtr = bufMappingInfo->baseRW();
    if (basePtr == nullptr) {
        QLOGE("%s: Base ptr is null for input buffer", __func__);
        return QC2_NO_MEMORY;
    }

    QLOGV("%s: size %d offset %d flags %" PRIu64 "\n",
          __func__, availLength, input->linear().offset(), flags);

    QLOGD("%s: AAC Config Param values: samplingRate %d bitDepth %d numChannels %d"
                        " BitRate %d Audio Object Type %d AAC packaging Type %d\n",
            __func__, mDecoderConfig->getSampleRate(), mDecoderConfig->getBitsPerSample(),
            mDecoderConfig->getChannelCount(), mDecoderConfig->getBitRate(),
            mDecoderConfig->getAudioObjectType(), mDecoderConfig->getPackagingType());


    // Set AAC decoder config from input CSD
    uint8_t *inputOffset = basePtr + input->linear().offset();
    retVal = parseAndExtractAacInfoFromHeader(inputOffset, availLength);
    if (retVal != QC2_OK)
        return retVal;

    //update Interface and config params
    updateConfigParams(&hasCSDChanged);

    if (getHwSessionId()) {
        if (hasCSDChanged) {
            // onCodecConfig in a running session can come with seek operation
            // Flush is assumed to be handled by client as a part of seek
            QLOGW("%s: CSD changed in a running session", __func__);
        } else {
            QLOGW("%s: CSD unchanged, no action needed", __func__);
        }
        return QC2_OK;
    }

    QLOGD("%s: AAC CSD values: samplingRate %d bitDepth %d numChannels %d"
                        " BitRate %d Audio Object Type %d AAC packaging Type %d\n",
            __func__, mDecoderConfig->getSampleRate(), mDecoderConfig->getBitsPerSample(),
            mDecoderConfig->getChannelCount(), mDecoderConfig->getBitRate(),
            mDecoderConfig->getAudioObjectType(), mDecoderConfig->getPackagingType());

    mAacDecoderExtnConfig = { .aac_format = (AACPackaging)mDecoderConfig->getAacStreamFormat(),
                                .audio_object_type = (AACProfile)mDecoderConfig->getAudioObjectType(),
                                .num_channels = mDecoderConfig->getChannelCount(),
                                .total_size_of_pce_bits = 0,
                                .sample_rate = mDecoderConfig->getSampleRate()};

    mInputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                        .channel_count = mDecoderConfig->getChannelCount(),
                        .codec_type = Type_AAC,
                        .codec_info_size = sizeof(mDecoderConfig),
                        .codec_info = (void *)(&mAacDecoderExtnConfig)};
    mOutputMediaFmt = {.sample_rate = mDecoderConfig->getSampleRate(),
                        .bits_per_sample = mDecoderConfig->getBitsPerSample(),
                        .channel_count = mDecoderConfig->getMaxOutputChannelCount()?
                                                mDecoderConfig->getMaxOutputChannelCount():
                                                mDecoderConfig->getChannelCount(),
                        .codec_type = Type_PCM};
    QLOGV("%s: Input channels =%u, Output channels =%u", __func__, mInputMediaFmt.channel_count, mOutputMediaFmt.channel_count);
    retVal = mAudioHw->open_session(mInputMediaFmt, mOutputMediaFmt, &mHwSessionId);
    if (retVal != QC2_OK) {
        QLOGE("Open session failed for codec type %d  with error code %d", mInputMediaFmt.codec_type, retVal);
        return retVal;
    }

    return mAudioHw->start_session(getHwSessionId());
}

void QC2AudioHwAacDec::onOutputsDone(
        std::shared_ptr<QC2AudioHwBuffer> outBuf) {

    std::lock_guard<std::mutex> outputLock(mPendingOutputsMutex);
    auto outputPair = mPendingOutputs.find(outBuf->input_frame_id);
    if (outputPair == mPendingOutputs.end()) {
        QLOGE("%s: Invalid output returned from HW, output frame id = %llu. Dropping output",
                                     __func__, (unsigned long long)outBuf->output_frame_id);
        return;
    }

    auto output = outputPair->second;
    output->setTimestamp(outBuf->ts);
    output->linear().setRange(outBuf->offset, outBuf->filled_length);
    QLOGV("%s: Output flags = 0x%x", __func__, (unsigned int)output->flags());
    if (output->flags() & BufFlag::EOS)
        QLOGD("%s: EOS ACK Buffer detected in read buffer %llu", __func__,
                                                    (unsigned long long)outBuf->output_frame_id);
    if (outBuf->payload != nullptr) {
        QC2AudioCommonCodecConfig* updated_aac_config =
                                    reinterpret_cast<QC2AudioCommonCodecConfig*>(outBuf->payload);
        auto sr = updated_aac_config->sample_rate;
        auto ch = updated_aac_config->channel_count;
        if (sr && sr != mDecoderConfig->getSampleRate()) {
            QLOGV("%s: New Sample Rate = %u", __func__, (unsigned int)sr);
            auto sampleRateParam = std::make_shared<C2StreamSampleRateInfo::output>(0, sr);
            mUpdatedInfos.push_back(sampleRateParam);
            mDecoderConfig->setSampleRate(sr);
        }
        if (ch && ch != mDecoderConfig->getChannelCount()) {
            QLOGV("%s: New Channel Count = %u", __func__, (unsigned int)ch);
            auto channelCountParam = std::make_shared<C2StreamChannelCountInfo::output>(0, ch);
            mUpdatedInfos.push_back(channelCountParam);
            mDecoderConfig->setChannelCount(ch);
        }
    }
    if (mUpdatedInfos.size()) {
        QLOGD_INST("ConfigUpdate: sending %zu updates", mUpdatedInfos.size());
        //Ensure Component Interface is also updated with latest param values
        onConfigUpdate(mUpdatedInfos);
    }
    //Signal to component that output is ready
    signalOutputsDone(output);
    mUpdatedInfos.clear();
    mPendingOutputs.erase(outputPair);
}

}   // namespace qc2audio
